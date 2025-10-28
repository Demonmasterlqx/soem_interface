cmake_minimum_required(VERSION 3.8)
project(candriver)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE -Ofast)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(message_logger REQUIRED)
find_package(soem_rsl REQUIRED)
find_package(soem_interface_rsl REQUIRED)

# Add RPATH settings
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include_directories(include)

# Library
add_library(candriver SHARED
  src/candriver.cpp)

target_include_directories(candriver PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

# test

add_executable(test_candriver_listen test/test_candriver_listen.cpp)
target_link_libraries(test_candriver_listen PRIVATE candriver)


add_executable(test_candriver_send test/test_candriver_send.cpp)
target_link_libraries(test_candriver_send PRIVATE candriver)

add_executable(test_candriver_dmJ4310 test/test_candriver_dm-J4310.cpp)
target_link_libraries(test_candriver_dmJ4310 PRIVATE candriver)

add_executable(test_candriver_dmJ8090 test/test_candriver_dm-J8090.cpp)
target_link_libraries(test_candriver_dmJ8090 PRIVATE candriver)

add_executable(test_candriver_sendM3508 test/test_candriver_sendM3508.cpp)
target_link_libraries(test_candriver_sendM3508 PRIVATE candriver)

add_executable(test_candriver_sendM2006 test/test_candriver_sendM2006.cpp)
target_link_libraries(test_candriver_sendM2006 PRIVATE candriver)


# Install library
install(TARGETS candriver
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

# Install executables
install(TARGETS
  test_candriver_listen
  test_candriver_send
  test_candriver_dmJ4310
  test_candriver_dmJ8090
  test_candriver_sendM3508
  test_candriver_sendM2006
  DESTINATION lib/${PROJECT_NAME}
)

# Install headers
install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
)

# Export dependencies
ament_export_targets(${PROJECT_NAME}Targets HAS_LIBRARY_TARGET)
ament_export_dependencies(message_logger soem_rsl soem_interface_rsl)

ament_package()