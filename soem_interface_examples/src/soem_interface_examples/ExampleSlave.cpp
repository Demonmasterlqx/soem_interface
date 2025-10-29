/*
** Copyright (2019-2020) Robotics Systems Lab - ETH Zurich:
** Markus Staeuble, Jonas Junger, Johannes Pankert, Philipp Leemann,
** Tom Lankhorst, Samuel Bachmann, Gabriel Hottiger, Lennert Nachtigall,
** Mario Mauerer, Remo Diethelm
**
** This file is part of the soem_interface_rsl.
**
** The soem_interface_rsl is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** The seom_interface is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with the soem_interface_rsl.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <soem_interface_examples/ExampleSlave.hpp>
#include <thread>

namespace soem_interface_examples
{

    ExampleSlave::ExampleSlave(const std::string &name, soem_interface_rsl::EthercatBusBase *bus, const uint32_t address)
        : soem_interface_rsl::EthercatSlaveBase(bus, address), name_(name)
    {
        pdoInfo_.rxPdoId_ = RX_PDO_ID;
        pdoInfo_.txPdoId_ = TX_PDO_ID;
        pdoInfo_.rxPdoSize_ = sizeof(command_);
        pdoInfo_.txPdoSize_ = sizeof(reading_);
        pdoInfo_.moduleId_ = 0x00123456;
    }

    bool ExampleSlave::startup()
    {

        // 初始化4个can口
        can_drivers_.resize(4, nullptr);
        for (int i = 0; i < 4; ++i)
        {
            can_drivers_[i] = std::make_shared<CanManager>(CAN_INTERFACE_PREFIX + std::to_string(i));
        }

        // Do nothing else
        return true;
    }

    void ExampleSlave::updateRead()
    {
        bus_->readTxPdo(address_, reading_);
        MELO_INFO_STREAM("Received frame count: " << static_cast<int>(reading_.frame_count));
        for (int i = 0; i < reading_.frame_count; ++i)
        {
            const can_frame_struct &frame = reading_.can_frames[i];
            MELO_INFO_STREAM("CAN Channel: " << static_cast<int>(frame.can_channel)
                                             << ", Std ID: " << frame.std_id
                                             << ", DLC: " << static_cast<int>(frame.dlc)
                                             << ", Data: ");
            for (int j = 0; j < frame.dlc; ++j)
            {
                MELO_INFO_STREAM(std::hex << static_cast<int>(frame.data[j]) << " ");
            }
            MELO_INFO_STREAM(std::dec);

            // 发送到对应的can口
            int channel = frame.can_channel;
            if (channel >= 0 && channel < static_cast<int>(can_drivers_.size()))
            {
                can_frame canFrame;
                canFrame.can_id = frame.std_id;
                canFrame.can_dlc = frame.dlc;
                std::copy(std::begin(frame.data), std::begin(frame.data) + frame.dlc, std::begin(canFrame.data));
                can_drivers_[channel]->writeCanFrame(canFrame);
            }
            else
            {
                MELO_ERROR_STREAM("Invalid CAN channel: " << channel);
            }
            std::this_thread::sleep_for(std::chrono::nanoseconds(50)); // 避免发送过快
        }
    }

    void ExampleSlave::updateWrite()
    {
        bus_->writeRxPdo(address_, command_);
        command_.command = static_cast<uint8_t>(Ecat2Can_Command::WORK_MODE);
        // 接受各个can口的消息
        command_.frame_count = 0;
        for (size_t channel = 0; channel < can_drivers_.size(); ++channel)
        {
            std::vector<can_frame> frames;
            can_drivers_[channel]->getCanFrame(frames);
            for (const auto &frame : frames)
            {
                if (command_.frame_count < MAXCANRXTXMSGSIZE)
                {
                    can_frame_struct &pdo_frame = command_.can_frames[command_.frame_count];
                    pdo_frame.can_channel = static_cast<uint8_t>(channel);
                    pdo_frame.std_id = frame.can_id;
                    pdo_frame.dlc = frame.can_dlc;
                    std::copy(std::begin(frame.data), std::begin(frame.data) + frame.can_dlc, std::begin(pdo_frame.data));
                    command_.frame_count++;
                }
                else
                {
                    MELO_WARN_STREAM("Maximum number of CAN frames reached in PDO, some frames may be dropped.");
                    break;
                }
            }
        }
        MELO_INFO_STREAM("Sending frame count: " << static_cast<int>(command_.frame_count));
    }

    void ExampleSlave::shutdown()
    {
        // Do nothing
    }

    CanManager::CanManager(const std::string &interface_name)
    {
        can_driver_ = std::make_shared<CanDriver>(interface_name);
        read_can_thread_ = std::make_shared<std::thread>(&CanManager::readCanLoop, this);
    }

    void CanManager::writeCanFrame(const can_frame &frame)
    {
        std::shared_ptr<realtime_tools::RealtimeBuffer<CanFrameWrapper>> buffer = getInputCanFrameBuffer(frame.can_id);
        CanFrameWrapper wrapper;
        wrapper.frame = frame;
        wrapper.is_new = true;
        std::cout << 2 << std::endl;
        buffer->writeFromNonRT(wrapper);
    }

    void CanManager::getCanFrame(std::vector<can_frame> &frames)
    {
        frames.clear();
        std::vector<std::shared_ptr<realtime_tools::RealtimeBuffer<CanFrameWrapper>>> buffers = getAllOutputCanFrameBuffers();
        for (auto &buffer : buffers)
        {
            CanFrameWrapper wrapper = *buffer->readFromRT();
            if (wrapper.is_new)
            {
                frames.push_back(wrapper.frame);
                wrapper.is_new = false;
                buffer->writeFromNonRT(wrapper);
            }
        }
    }

    std::shared_ptr<realtime_tools::RealtimeBuffer<CanFrameWrapper>> CanManager::getInputCanFrameBuffer(int id)
    {
        std::lock_guard<std::mutex> lock(input_can_frame_buffers_mutex_);
        if (input_can_frame_buffers_.find(id) == input_can_frame_buffers_.end())
        {
            input_can_frame_buffers_[id] = std::make_shared<realtime_tools::RealtimeBuffer<CanFrameWrapper>>();
            input_can_frame_buffers_[id]->initRT(
                CanFrameWrapper{0});
        }
        return input_can_frame_buffers_[id];
    }

    std::shared_ptr<realtime_tools::RealtimeBuffer<CanFrameWrapper>> CanManager::getOutputCanFrameBuffer(int id)
    {
        std::lock_guard<std::mutex> lock(output_can_frame_buffers_mutex_);
        if (output_can_frame_buffers_.find(id) == output_can_frame_buffers_.end())
        {
            output_can_frame_buffers_[id] = std::make_shared<realtime_tools::RealtimeBuffer<CanFrameWrapper>>();
            output_can_frame_buffers_[id]->initRT(
                CanFrameWrapper{0});
        }
        return output_can_frame_buffers_[id];
    }

    std::vector<std::shared_ptr<realtime_tools::RealtimeBuffer<CanFrameWrapper>>> CanManager::getAllOutputCanFrameBuffers()
    {
        std::lock_guard<std::mutex> lock(output_can_frame_buffers_mutex_);
        std::vector<std::shared_ptr<realtime_tools::RealtimeBuffer<CanFrameWrapper>>> buffers;
        for (auto &pair : output_can_frame_buffers_)
        {
            buffers.push_back(pair.second);
        }
        return buffers;
    }

    std::vector<std::shared_ptr<realtime_tools::RealtimeBuffer<CanFrameWrapper>>> CanManager::getAllInputCanFrameBuffers()
    {
        std::lock_guard<std::mutex> lock(input_can_frame_buffers_mutex_);
        std::vector<std::shared_ptr<realtime_tools::RealtimeBuffer<CanFrameWrapper>>> buffers;
        for (auto &pair : input_can_frame_buffers_)
        {
            buffers.push_back(pair.second);
        }
        return buffers;
    }

    int CanManager::getInputCanFrameBufferSize()
    {
        std::lock_guard<std::mutex> lock(input_can_frame_buffers_mutex_);
        return static_cast<int>(input_can_frame_buffers_.size());
    }

    int CanManager::getOutputCanFrameBufferSize()
    {
        std::lock_guard<std::mutex> lock(output_can_frame_buffers_mutex_);
        return static_cast<int>(output_can_frame_buffers_.size());
    }

    CanManager::~CanManager()
    {
        stop_read_thread_.store(true);
        if (read_can_thread_ && read_can_thread_->joinable())
        {
            read_can_thread_->join();
        }
    }

    void CanManager::readCanLoop()
    {
        rclcpp::Rate rate(1000);
        std::vector<std::shared_ptr<realtime_tools::RealtimeBuffer<CanFrameWrapper>>> input_buffers;
        while (stop_read_thread_.load() == false)
        {
            can_frame frame;

            if (can_driver_->isCanOk() == false)
            {
                MELO_ERROR_STREAM("CAN interface error, trying to reopen...");
                can_driver_->reopenCanSocket();
                continue;
            }

            if (can_driver_->receiveMessage(frame))
            {
                std::shared_ptr<realtime_tools::RealtimeBuffer<CanFrameWrapper>> buffer = getOutputCanFrameBuffer(frame.can_id);
                CanFrameWrapper wrapper;
                wrapper.frame = frame;
                wrapper.is_new = true;
                buffer->writeFromNonRT(wrapper);
            }

            if (static_cast<int>(input_buffers.size()) != getInputCanFrameBufferSize())
            {
                input_buffers = getAllInputCanFrameBuffers();
            }

            for (auto &buffer : input_buffers)
            {
                CanFrameWrapper wrapper = *buffer->readFromRT();
                if (wrapper.is_new)
                {
                    std::cout << 1 << std::endl;
                    if (!can_driver_->sendMessage(wrapper.frame))
                    {
                        MELO_ERROR_STREAM("Failed to send CAN message on channel " << wrapper.frame.can_id);
                        can_driver_->reopenCanSocket();
                    }
                    wrapper.is_new = false;
                    buffer->writeFromNonRT(wrapper);
                }
                std::this_thread::sleep_for(std::chrono::nanoseconds(50)); // 避免发送过快
            }

            rate.sleep();
        }
    }

} // namespace soem_interface_examples
