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

#pragma once

#include <soem_interface_rsl/EthercatBusBase.hpp>
#include <soem_interface_rsl/EthercatSlaveBase.hpp>

#include "candriver/candriver.hpp"

#define RX_PDO_ID 0x6000
#define TX_PDO_ID 0x7000

namespace soem_interface_examples {

struct can_frame_struct
{
    uint8_t can_channel;  // CAN通道号
    uint32_t std_id : 11; // 标准帧ID
    uint16_t dlc : 4;     // 数据长度
    uint8_t data[8];      // 数据
}__attribute__((packed));

struct ecat2can_tx_rx_message{
  uint8_t command;     // 命令字
  uint8_t frame_count; // 有效数据帧数
  uint8_t reserved[3];
  can_frame_struct can_frames[17];
}  __attribute__((packed));

enum class Ecat2Can_Command : uint8_t {
    LOOP_TEST = 0x00, // 回环模式，即接收到的消息原封不动发回去
    WORK_MODE = 0x01, // 工作模式，即正常转运can消息
    HW_RESET = 0x02,  // 模块硬件重启复位
    ONLY_FDB = 0x03,  // 只发送从站收到的can反馈，不发送主站消息
};

struct RxPdo {
  float command1 = 0.0;
  float command2 = 0.0;
} __attribute__((packed));

using CanDriver=RM_communication::CanDriver;

class ExampleSlave : public soem_interface_rsl::EthercatSlaveBase {
 public:
  ExampleSlave(const std::string& name, soem_interface_rsl::EthercatBusBase* bus, const uint32_t address);
  ~ExampleSlave() override = default;

  std::string getName() const override { return name_; }

  bool startup() override;
  void updateRead() override;
  void updateWrite() override;
  void shutdown() override;

  PdoInfo getCurrentPdoInfo() const override { return pdoInfo_; }

 private:
  const std::string name_;
  PdoInfo pdoInfo_;
  ecat2can_tx_rx_message reading_;
  ecat2can_tx_rx_message command_;

  std::vector<std::shared_ptr<CanDriver>> can_drivers_;
  const std::string CAN_INTERFACE_PREFIX = "can";
};

}  // namespace soem_interface_examples
