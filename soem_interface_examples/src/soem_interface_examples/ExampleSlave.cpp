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

namespace soem_interface_examples {

ExampleSlave::ExampleSlave(const std::string& name, soem_interface_rsl::EthercatBusBase* bus, const uint32_t address)
    : soem_interface_rsl::EthercatSlaveBase(bus, address), name_(name) {
  pdoInfo_.rxPdoId_ = RX_PDO_ID;
  pdoInfo_.txPdoId_ = TX_PDO_ID;
  pdoInfo_.rxPdoSize_ = sizeof(command_);
  pdoInfo_.txPdoSize_ = sizeof(reading_);
  pdoInfo_.moduleId_ = 0x00123456;
}

bool ExampleSlave::startup() {

  // 初始化4个can口
  can_drivers_.resize(4,nullptr);
  for (int i = 0; i < 4; ++i) {
      can_drivers_[i] = std::make_shared<CanDriver>(CAN_INTERFACE_PREFIX + std::to_string(i));
      if (!can_drivers_[i]->isCanOk()) {
          MELO_ERROR_STREAM("Failed to open CAN channel " << i);
      }
  }

  // Do nothing else
  return true;
}

void ExampleSlave::updateRead() {
  bus_->readTxPdo(address_, reading_);
  std::cout<<"Received frame count: "<<static_cast<int>(reading_.frame_count)<<std::endl;
  for (int i = 0; i < reading_.frame_count; ++i) {
      const can_frame_struct& frame = reading_.can_frames[i];
      std::cout << "CAN Channel: " << static_cast<int>(frame.can_channel)
                << ", Std ID: " << frame.std_id
                << ", DLC: " << static_cast<int>(frame.dlc)
                << ", Data: ";
      for (int j = 0; j < frame.dlc; ++j) {
          std::cout << std::hex << static_cast<int>(frame.data[j]) << " ";
      }
      std::cout << std::dec << std::endl; 

      // 发送到对应的can口
      int channel = frame.can_channel;
      if (channel >= 0 && channel < static_cast<int>(can_drivers_.size())) {
          can_frame canFrame;
          canFrame.can_id = frame.std_id;
          canFrame.can_dlc = frame.dlc;
          std::copy(std::begin(frame.data), std::begin(frame.data) + frame.dlc, std::begin(canFrame.data));
          if (!can_drivers_[channel]->sendMessage(canFrame)) {
              MELO_ERROR_STREAM("Failed to send CAN message on channel " << channel);
              can_drivers_[channel]->reopenCanSocket();
          }
      } else {
          MELO_ERROR_STREAM("Invalid CAN channel: " << channel);
      }
      std::this_thread::sleep_for(std::chrono::nanoseconds(50)); // 避免发送过快
    }
}

void ExampleSlave::updateWrite() {
  bus_->writeRxPdo(address_, command_);
  command_.command = static_cast<uint8_t>(Ecat2Can_Command::WORK_MODE);
  // 接受各个can口的消息
  command_.frame_count = 0;
  for (size_t channel = 0; channel < can_drivers_.size(); ++channel) {
      can_frame canFrame;
      while (can_drivers_[channel]->receiveMessage(canFrame)) {
          if (command_.frame_count < 17) {
              can_frame_struct& frame = command_.can_frames[command_.frame_count];
              frame.can_channel = static_cast<uint8_t>(channel);
              frame.std_id = canFrame.can_id;
              frame.dlc = canFrame.can_dlc;
              std::copy(std::begin(canFrame.data), std::begin(canFrame.data) + canFrame.can_dlc, std::begin(frame.data));
              command_.frame_count++;
          } else {
              MELO_WARN_STREAM("Maximum CAN frame count reached, dropping additional frames.");
              break;
          }
      }
  }
}

void ExampleSlave::shutdown() {
  // Do nothing
}

}  // namespace soem_interface_examples
