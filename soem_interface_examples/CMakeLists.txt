# Copyright (2019-2020) Robotics Systems Lab - ETH Zurich:
# Markus Staeuble, Jonas Junger, Johannes Pankert, Philipp Leemann,
# Tom Lankhorst, Samuel Bachmann, Gabriel Hottiger, Lennert Nachtigall,
# Mario Mauerer, Remo Diethelm
#
# This file is part of the soem_interface_rsl.
#
# The soem_interface_rsl is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The seom_interface is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with the soem_interface_rsl.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.8)
project(soem_interface_examples)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Werror)
add_definitions(-DMELO_USE_COUT)

# Add RPATH settings
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

find_package(ament_cmake REQUIRED)
find_package(message_logger REQUIRED)
find_package(soem_rsl REQUIRED)
find_package(soem_interface_rsl REQUIRED)
find_package(candriver REQUIRED)
find_package(realtime_tools REQUIRED)
find_package(rclcpp REQUIRED)

include_directories(
  include
)

add_library(
  ${PROJECT_NAME}
  src/${PROJECT_NAME}/ExampleSlave.cpp
)
target_link_libraries(${PROJECT_NAME}
  message_logger::message_logger
  soem_rsl::soem_rsl
  soem_interface_rsl::soem_interface_rsl
  candriver::candriver)

add_executable(
  ${PROJECT_NAME}_1
  src/example_slave.cpp
)
target_link_libraries(${PROJECT_NAME}_1
  ${PROJECT_NAME}
  message_logger::message_logger
  soem_rsl::soem_rsl
  soem_interface_rsl::soem_interface_rsl
  candriver::candriver)

ament_target_dependencies(${PROJECT_NAME}
  realtime_tools
  rclcpp)

install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_1
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
)

ament_export_targets(${PROJECT_NAME}Targets HAS_LIBRARY_TARGET)
ament_export_dependencies(message_logger soem_rsl soem_interface_rsl candriver)
ament_package()
