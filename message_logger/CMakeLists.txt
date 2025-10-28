cmake_minimum_required(VERSION 3.8)
project(message_logger)

# Find required packages
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

# Configuration options
include(CMakeDependentOption)
cmake_dependent_option(MELO_USE_COUT "Use std::cout" OFF
                      "DEFINED ENV{ROS_DISTRO}" ON)

# Configure header file
if(MELO_USE_COUT)
  add_definitions(-DMELO_USE_COUT)
endif()

configure_file(
  include/message_logger/log/log_messages_backend_config.hpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/message_logger/log/log_messages_backend_config.hpp
  @ONLY
)

if(DEFINED MELO_FUNCTION_PRINTS)
  add_definitions(-DMELO_FUNCTION_PRINTS)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall)

# Create library
add_library(${PROJECT_NAME} SHARED
  src/time/Time.cpp
  src/time/TimeStd.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if(NOT MELO_USE_COUT)
  target_link_libraries(${PROJECT_NAME} PUBLIC
    rclcpp::rclcpp
  )
endif()

# Install headers
install(
  DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
  PATTERN "*.in" EXCLUDE
)

# Install generated headers
install(
  DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
)

# Install library
install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

# Export targets
ament_export_targets(${PROJECT_NAME}Targets HAS_LIBRARY_TARGET)
if(NOT MELO_USE_COUT)
  ament_export_dependencies(rclcpp)
endif()

# Tests
if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_${PROJECT_NAME}
    test/EmptyTests.cpp
  )
  if(TARGET test_${PROJECT_NAME})
    target_link_libraries(test_${PROJECT_NAME}
      ${PROJECT_NAME}
    )
  endif()
endif()

ament_package()
