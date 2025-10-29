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
#include <soem_interface_rsl/EthercatBusBase.hpp>
#include <soem_rsl/soem_rsl/ethercattype.h>

#include <thread>

// This shows a minimal example on how to use the soem_interface_rsl library.
// Keep in mind that this is non-working example code, with only minimal error handling

int main(int argc, char **argv)
{

  rclcpp::init(argc, argv);

  const std::string busName = "enp2s0";
  const std::string slaveName = "ExampleSlave";
  const uint32_t slaveAddress = 1; // First slave has address 1

  std::unique_ptr<soem_interface_rsl::EthercatBusBase> bus = std::make_unique<soem_interface_rsl::EthercatBusBase>(busName);

  std::shared_ptr<soem_interface_examples::ExampleSlave> slave =
      std::make_shared<soem_interface_examples::ExampleSlave>(slaveName, bus.get(), slaveAddress);

  bus->addSlave(slave);
  bus->startup(true); // Enable size check
  bus->setState(EC_STATE_OPERATIONAL);

  if (!bus->waitForState(EC_STATE_OPERATIONAL, slaveAddress))
  {
    // Something is wrong
    return 1;
  }

  while (true)
  {
    if (!bus->busIsAvailable() || !bus->busIsOk())
    {
      bus->shutdown();
      if (!bus->startup(true))
      {
        MELO_ERROR_STREAM("Failed to restart EtherCAT bus. Retrying in 1 second...");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        continue;
      }
      bus->setState(EC_STATE_OPERATIONAL);
    }
    bus->updateRead();
    bus->updateWrite();
  }

  bus->shutdown();
  return 0;
}
