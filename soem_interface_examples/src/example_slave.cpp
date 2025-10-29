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

#include <sched.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cerrno>

// This shows a minimal example on how to use the soem_interface_rsl library.
// Keep in mind that this is non-working example code, with only minimal error handling

std::atomic<bool> run_loop{true};

void signalHandler(int signum)
{
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    run_loop.store(false);
}

int main(int argc, char **argv)
{
  // 设置线程优先级
  pid_t pid = getpid(); // 获取当前进程 PID，也可以指定其他 PID
  struct sched_param param;
  param.sched_priority = 90; // 设置实时优先级为 50

  if (sched_setscheduler(pid, SCHED_FIFO, &param) == -1) {
    MELO_ERROR_STREAM("Failed to set real-time scheduler: " << strerror(errno));
    MELO_ERROR_STREAM("Need root privileges for real-time scheduling.");
  }

  signal(SIGINT, signalHandler);

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

  rclcpp::Rate rate(1000.0);

  while (run_loop.load())
  {


    if (!bus->busIsAvailable() || !bus->busIsOk())
    {
      MELO_ERROR_STREAM("Bus error detected, stopping main loop.");
      break;
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    bus->updateRead();
    auto update_time = std::chrono::high_resolution_clock::now();
    bus->updateWrite();
    auto end_time = std::chrono::high_resolution_clock::now();


    // Compute and print durations
    auto read_duration = std::chrono::duration_cast<std::chrono::microseconds>(update_time - start_time).count();
    auto write_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - update_time).count();
    // MELO_INFO_STREAM("Read duration: " << read_duration << " us, Write duration: " << write_duration << " us");
    if (!rate.sleep()) {
      rate.reset();
      MELO_WARN_STREAM("Loop overrun!");
      MELO_INFO_STREAM("Read duration: " << read_duration << " us, Write duration: " << write_duration << " us");
    }
    // std::this_thread::sleep_for(std::chrono::microseconds(250));
  }

  bus->shutdown();
  return 0;
}
