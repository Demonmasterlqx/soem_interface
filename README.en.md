# soem_interface — EtherCAT + CAN bridge interface (ROS 2 ament)

Overview

This repository contains a C++ library and examples for creating and managing multiple EtherCAT slave devices on a Linux host. It also provides utilities for bridging EtherCAT <-> CAN traffic.

The project builds on top of `soem_rsl` (a wrapper around the SOEM EtherCAT library) and several ROS 2 packages using `ament_cmake`.

Packages in this repository

- `soem_interface_rsl`: Core library with abstract base classes such as `EthercatSlaveBase` and `EthercatBusBase`.
- `soem_rsl`: SOEM wrapper used for low-level EtherCAT communication and tests.
- `candriver`: CAN driver implementation (socketcan wrapper) and test executables.
- `soem_interface_examples`: Example code showing how to create slaves, start the bus and perform PDO read/write in a loop.
- `message_logger`: Logging utility (external dependency, BSD license).

License: GPLv3 (see the repository `LICENSE` file).

Note: The original project was developed for ROS 1 / catkin (earlier README refers to ROS Melodic). The current repository uses ROS 2-style package metadata (package.xml format 3) and `ament_cmake`. Use the appropriate ROS 2 distribution when building.

Quick start — build & run

Prerequisites: install ROS 2 (use your distro, e.g. `jazzy`, `humble`, etc.), and developer tools (cmake, colcon).

1) Install ROS 2 environment

```bash
source /opt/ros/<your-distro>/setup.bash
```

Install `colcon` if needed.

2) Clone the repository

```bash
cd ~/dev
git clone https://github.com/Demonmasterlqx/soem_interface.git
cd soem_interface
```

3) Build (example using colcon)

```bash
# from repository root
colcon build --packages-select soem_interface_examples candriver soem_interface_rsl soem_rsl message_logger
# or build all packages
colcon build
```

After build completes, source the install space:

```bash
source install/setup.bash
```

4) Run example executable

The example binary is installed at `install/soem_interface_examples/bin/soem_interface_examples_1` (CMake produces `soem_interface_examples_1`).

Note on running with sudo:

Running the binary under `sudo` may drop environment variables (for example `LD_LIBRARY_PATH`) and cause runtime library loading errors such as "error while loading shared libraries: lib...so: cannot open shared object file".

Recommendations:

```bash
# recommended (no sudo)
./install/soem_interface_examples/bin/soem_interface_examples_1

# if sudo is required, preserve environment and set LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/ros/<your-distro>/lib
sudo -E env LD_LIBRARY_PATH=$LD_LIBRARY_PATH ./install/soem_interface_examples/bin/soem_interface_examples_1
```

High-level project layout

Top-level ROS 2 packages (built with `ament_cmake`):

- `soem_interface_rsl/`: Core library, EtherCAT device abstractions and bus manager.
- `soem_rsl/`: SOEM wrapper and tests.
- `candriver/`: CAN driver implementation and test/example executables.
- `soem_interface_examples/`: Example library and example executable showing EtherCAT bus startup, slave management and PDO read/write.
- `message_logger/`: Logging utility (external dependency).

There are also helper scripts like `launch.sh` and a `jenkins-pipeline` directory for CI.

Key concepts & interfaces

- EthercatSlaveBase: Base class for an EtherCAT slave. Derived classes should implement `updateRead()` / `updateWrite()` to stage data for PDO operations.
- EthercatBusBase: Represents an EtherCAT bus and manages multiple `EthercatSlaveBase` instances. Provides `startup()`, `setState()`, `updateRead()`, and `updateWrite()`.
- CanDriver / CanManager: SocketCAN wrapper with thread-safe buffers (using `realtime_tools::RealtimeBuffer`). The example uses timestamps to deduplicate messages to avoid repeated sends.

Troubleshooting & common issues

- "error while loading shared libraries: lib...so: cannot open shared object file"
  - Cause: runtime environment lacks the proper `LD_LIBRARY_PATH` or ROS environment is not sourced.
  - Fix: `source install/setup.bash` before running, or add library paths to `LD_LIBRARY_PATH`. When using `sudo`, preserve environment with `sudo -E` and pass `LD_LIBRARY_PATH`.

- Segfaults or unexpected EtherCAT "working counter" values
  - Check physical bus connections, network interface (example uses `enp2s0`), slave power status and SOEM version compatibility.
  - Use `bus->waitForState(EC_STATE_OPERATIONAL, addr)` to ensure slaves reach OP state.

- CAN issues
  - Ensure the socketcan interface is created and configured with the correct bitrate (repository tests use `canbusload`/`candump`).
  - The example `CanManager` performs timestamp-based deduplication to prevent re-sending identical frames.

Development & contribution suggestions

- Develop locally as a non-root user and use `colcon build` + `source install/setup.bash` to avoid environment issues.
- Consider adding:
  - Unit tests for critical logic (e.g. CanManager deduplication and buffer read/write)
  - CI pipeline (colcon test + linters) integrated into `jenkins-pipeline`
  - More extensive documentation (per-package usage and API examples)

License & contact

This project is licensed under GPLv3 (see `LICENSE`).

Maintainers and contributors are listed in the `maintainer` fields in each package's `package.xml`.
