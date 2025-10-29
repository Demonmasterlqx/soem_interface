## soem_interface — EtherCAT + CAN bridge interface (ROS 2 ament)

简体中文 README（已根据仓库源码重写）

本仓库提供一套 C++ 库与示例，用于在 Linux 主机上通过 EtherCAT 总线创建并管理多个从站设备，同时支持将 EtherCAT <-> CAN 的数据桥接。
代码基于 soem_rsl（SOEM 封装）与若干 ROS 2 包（使用 ament_cmake 构建）。仓库里的包包括：

- `soem_interface_rsl`：核心库，定义 EthercatSlaveBase / EthercatBusBase 等抽象类。
- `soem_rsl`：SOEM 封装（第三方/子模块，供底层 EtherCAT 通信使用）。
- `candriver`：CAN 驱动封装（对 socketcan 的封装与测试程序）。
- `soem_interface_examples`：示例程序，展示如何创建从站、启动总线并在循环中读写 PDO。
- `message_logger`：日志工具（已作为依赖，BSD 许可）。

许可：GPLv3（见仓库顶层 `LICENSE`）。

历史说明：原项目最初在 ROS 1 / catkin 环境中开发（早期 README 提及 ROS Melodic），仓库现为 ROS 2 风格（`package.xml` 格式 3、`ament_cmake`、CMakeLists 中使用 `rclcpp`），请按下面说明使用合适的 ROS 2 版本进行构建。

## 快速开始（构建与运行）

前提：已安装 ROS 2（下文示例以 `jazzy` 或您机器上的发行版为例），并安装基本开发工具（cmake、colcon/ament）。

1. 安装系统依赖

   - 安装 ROS 2 并 source 对应的环境：

     ```bash
     source /opt/ros/<your-distro>/setup.bash
     ```

   - 推荐安装 `colcon`（或使用 ament/colcon 的工作区工具）

2. 获取源码（若尚未）

   ```bash
   cd ~/dev
   git clone https://github.com/Demonmasterlqx/soem_interface.git
   cd soem_interface
   ```

3. 构建（示例使用 colcon）

   ```bash
   # 在仓库根目录（包含 package.xml 的包目录）执行
   colcon build --packages-select soem_interface_examples candriver soem_interface_rsl soem_rsl message_logger
   # 或构建全部包
   colcon build
   ```

   构建完成后，source 安装空间：

   ```bash
   source install/setup.bash
   ```

4. 运行示例二进制

   示例可执行文件位于 `install/soem_interface_examples/bin/soem_interface_examples_1`（由 CMakeLists 生成 `soem_interface_examples_1`）。

   注意：如果以 `sudo` 运行二进制，环境变量（例如 LD_LIBRARY_PATH）可能不会传递，导致找不到共享库（如 `liblibstatistics_collector.so`）。推荐使用非 sudo 用户，或使用 `sudo -E` 并确保正确的 `LD_LIBRARY_PATH` 与 ROS 环境已导出：

   ```bash
   # 推荐（不使用 sudo）
   ./install/soem_interface_examples/bin/soem_interface_examples_1

   # 如果非 root 权限不足，使用 sudo 时保留环境
   export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/ros/<your-distro>/lib
   sudo -E env LD_LIBRARY_PATH=$LD_LIBRARY_PATH ./install/soem_interface_examples/bin/soem_interface_examples_1
   ```

## 项目结构（高层）

顶层目录包含若干 ROS2 包（以 `ament_cmake` 为构建系统）：

- `soem_interface_rsl/`：核心库，提供 EtherCAT 设备抽象和总线管理。
- `soem_rsl/`：SOEM 的封装与一些测试工具（包含 SOEM 相关的 CMake targets）。
- `candriver/`：CAN 驱动实现以及若干测试/示例可执行文件。
- `soem_interface_examples/`：示例库与一个示例可执行，展示 EtherCAT 总线的启动、从站管理和 PDO 读取/写入流程。
- `message_logger/`：日志库（外部依赖，BSD）。

此外仓库包含 `launch.sh`、`jenkins-pipeline` 等辅助脚本与 CI 管线脚本。

## 主要概念与接口说明

- EthercatSlaveBase：从站基类，派生类应实现 `updateRead()` / `updateWrite()` 来把数据放到 PDO 中或从 PDO 中读取。
- EthercatBusBase：代表一个 EtherCAT 总线，管理多个 `EthercatSlaveBase`。提供 `startup()`、`setState()`、`updateRead()`、`updateWrite()` 等方法。
- CanDriver / CanManager：封装 socketcan 操作，支持线程安全的缓冲（使用 `realtime_tools::RealtimeBuffer`）。示例中使用时间戳检测消息重复以避免重复发送。

## 常见问题与调试提示

- 错误："error while loading shared libraries: lib...so: cannot open shared object file"
  - 原因：运行环境没有正确的 LD_LIBRARY_PATH 或者没有 source ROS 环境。
  - 解决：在运行前执行 `source install/setup.bash`，或将库路径加入 `LD_LIBRARY_PATH`，并在 sudo 时使用 `sudo -E` 保留环境。

- 运行时段错误（segfault）或 EtherCAT 工作计数（working counter）异常
  - 检查物理总线连接、网口配置（例如 `enp2s0` 在示例中）、从站供电和 SOEM 版本匹配。
  - 使用 `bus->waitForState(EC_STATE_OPERATIONAL, addr)` 来确认从站进入 OP 状态。

- CAN 相关问题
  - 确保 `socketcan` 接口已创建并且波特率正确（仓库里的测试脚本使用 `canbusload`/`candump` 等）。
  - 示例里的 CanManager 使用时间戳去重，保证不会重复发送同一帧。

## 开发与贡献建议

- 建议在本地使用非 root 用户开发并使用 `colcon build` + `source install/setup.bash` 流程来避免环境问题。
- 可以考虑添加：
  - 单元测试覆盖关键逻辑（例如 CanManager 的去重、缓冲读写）
  - CI（colcon test + lint）并在 `jenkins-pipeline` 中整合
  - 更完整的文档（每个包的用法与 API 示例）

## 联系与许可证

本项目基于 GPLv3（详见 `LICENSE`）。

维护者与贡献者信息请参照 `package.xml` 中的 `maintainer` 字段。

---

如果你希望我把这个 README 翻译为英文版本、把某个包的用法写成独立的文档，或把运行示例的步骤补充成逐步脚本（含调试命令），告诉我你优先要哪一项，我会继续完善。
