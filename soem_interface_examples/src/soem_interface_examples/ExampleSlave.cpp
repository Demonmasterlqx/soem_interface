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
        can_drivers_.resize(CAN_CHANNEL_NUM, nullptr);
        for (int i = 0; i < CAN_CHANNEL_NUM; ++i)
        {
            can_drivers_[i] = std::make_shared<CanDriver>(CAN_INTERFACE_PREFIX + std::to_string(i));
        }

        // Do nothing else
        return true;
    }

    void ExampleSlave::updateRead()
    {
        // auto total_start = std::chrono::high_resolution_clock::now();
        bus_->readTxPdo(address_, reading_);

        // auto read_start = std::chrono::high_resolution_clock::now();

        // MELO_DEBUG_STREAM("Received frame count: " << static_cast<int>(reading_.frame_count));
        for (int i = 0; i < reading_.frame_count; ++i)
        {
            const can_frame_struct &frame = reading_.can_frames[i];
            // MELO_INFO_STREAM("CAN Channel: " << static_cast<int>(frame.can_channel)
            //                                  << ", Std ID: " << frame.std_id
            //                                  << ", DLC: " << static_cast<int>(frame.dlc)
            //                                  << ", Data: ");
            // for (int j = 0; j < frame.dlc; ++j)
            // {
            //     MELO_INFO_STREAM(std::hex << static_cast<int>(frame.data[j]) << " ");
            // }
            // MELO_INFO_STREAM(std::dec);

            // 发送到对应的can口

            // auto begin = std::chrono::high_resolution_clock::now();

            int channel = frame.can_channel;
            if (channel >= 0 && channel < CAN_CHANNEL_NUM)
            {
                can_frame canFrame;
                canFrame.can_id = frame.std_id;
                canFrame.can_dlc = frame.dlc;
                std::copy(std::begin(frame.data), std::begin(frame.data) + frame.dlc, std::begin(canFrame.data));
                if(!can_drivers_[channel]->sendMessage(canFrame)){
                    MELO_ERROR_STREAM("Failed to send CAN message on channel " << channel);
                    can_drivers_[channel]->reopenCanSocket();
                }
            }
            else
            {
                MELO_ERROR_STREAM("Invalid CAN channel: " << channel);
            }

            // auto end = std::chrono::high_resolution_clock::now();
            // auto diff = end - begin;
            // MELO_INFO_STREAM("Processing time for CAN frame: " << (diff).count()/1000.0 << " us");
        }
        // auto total_end = std::chrono::high_resolution_clock::now();
        // auto total_diff = total_end - total_start;
        // auto read_diff = read_start - total_start;
        // MELO_INFO_STREAM("Time taken to read PDO: " << (read_diff).count()/1000.0 << " us");
        // MELO_INFO_STREAM("Total processing time for all CAN frames: " << (total_diff).count()/1000.0 << " us");
    }

    void ExampleSlave::updateWrite()
    {
        // auto start_time = std::chrono::high_resolution_clock::now();
        bus_->writeRxPdo(address_, command_);
        command_.command = static_cast<uint8_t>(Ecat2Can_Command::WORK_MODE);
        // 接受各个can口的消息
        command_.frame_count = 0;
        for (int channel = 0; channel < CAN_CHANNEL_NUM; ++channel)
        {
            std::vector<can_frame> frames;
            if(!can_drivers_[channel]->receiveAllUniqueMessages(frames)){
                // 处理错误
                if(can_drivers_[channel]->isCanOk() == false){
                    MELO_ERROR_STREAM("CAN bus error on channel " << channel);
                    can_drivers_[channel]->reopenCanSocket();
                }
                continue;
            }

            for(const auto& frame : frames){
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
        // auto end_time = std::chrono::high_resolution_clock::now();
        // auto diff = end_time - start_time;
        // MELO_INFO_STREAM("Time taken to write PDO: " << (diff).count()/1000.0 << " us");
        // MELO_INFO_STREAM("Sending frame count: " << static_cast<int>(command_.frame_count));
    }

    void ExampleSlave::shutdown()
    {
        // Do nothing
    }

} // namespace soem_interface_examples
