cmake_minimum_required(VERSION 3.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")
project(soem_rsl C)

find_package(ament_cmake REQUIRED)

if(WIN32)
  set(OS "win32")
  include_directories(oshw/win32/wpcap/Include)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    link_directories(${CMAKE_CURRENT_LIST_DIR}/oshw/win32/wpcap/Lib/x64)
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    link_directories(${CMAKE_CURRENT_LIST_DIR}/oshw/win32/wpcap/Lib)
  endif()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D _CRT_SECURE_NO_WARNINGS")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  /WX")
  set(OS_LIBS wpcap.lib Packet.lib Ws2_32.lib Winmm.lib)
elseif(UNIX AND NOT APPLE)
  set(OS "linux")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
  set(OS_LIBS pthread rt)
elseif(APPLE)
  # This must come *before* linux or MacOSX will identify as Unix.
  set(OS "macosx")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
  set(OS_LIBS pthread pcap)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "rt-kernel")
  set(OS "rtk")
  message("ARCH is ${ARCH}")
  message("BSP is ${BSP}")
  include_directories(oshw/${OS}/${ARCH})
  file(GLOB OSHW_EXTRA_SOURCES oshw/${OS}/${ARCH}/*.c)
  set(OSHW_SOURCES "${OS_HW_SOURCES} ${OSHW_ARCHSOURCES}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-but-set-variable")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format")
  set(OS_LIBS "-Wl,--start-group -l${BSP} -l${ARCH} -lkern -ldev -lsio -lblock -lfs -lusb -llwip -leth -li2c -lrtc -lcan -lnand -lspi -lnor -lpwm -ladc -ltrace -lc -lm -Wl,--end-group")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "rtems")
  message("Building for RTEMS")
  set(OS "rtems")
  set(soem_rsl_LIB_INSTALL_DIR ${LIB_DIR})
  set(BUILD_TESTS FALSE)
endif()

message("OS is ${OS}")


set(soem_rsl_INCLUDE_INSTALL_DIR include/soem_rsl)
set(soem_rsl_LIB_INSTALL_DIR lib)
set(BUILD_TESTS TRUE)

# Configure ament package settings
ament_export_targets(soem_rslConfig HAS_LIBRARY_TARGET)



file(GLOB soem_rsl_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/soem_rsl/soem_rsl/*.c)
file(GLOB OSAL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/soem_rsl/osal/${OS}/*.c)
file(GLOB OSHW_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/soem_rsl/oshw/${OS}/*.c)

file(GLOB soem_rsl_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/soem_rsl/soem_rsl/*.h)
file(GLOB OSAL_HEADERS 
     ${CMAKE_CURRENT_SOURCE_DIR}/soem_rsl/osal/osal.h 
     ${CMAKE_CURRENT_SOURCE_DIR}/soem_rsl/osal/${OS}/*.h)
file(GLOB OSHW_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/soem_rsl/oshw/${OS}/*.h)

message("OSAL headers: ${OSAL_HEADERS}")

add_library(soem_rsl STATIC
  ${soem_rsl_SOURCES}
  ${OSAL_SOURCES}
  ${OSHW_SOURCES}
  ${OSHW_EXTRA_SOURCES})
target_link_libraries(soem_rsl ${OS_LIBS})
set_target_properties (soem_rsl PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(soem_rsl PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/soem_rsl>
  $<INSTALL_INTERFACE:include>)

message("LIB_DIR: ${soem_rsl_LIB_INSTALL_DIR}")

install(TARGETS soem_rsl
  EXPORT soem_rslConfig
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include/soem_rsl
)

# Install headers maintaining directory structure
# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/soem_rsl/soem_rsl/
  DESTINATION include/soem_rsl/soem_rsl
  FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/soem_rsl/osal/
  DESTINATION include/soem_rsl/osal
  FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/soem_rsl/osal/${OS}/
  DESTINATION include/soem_rsl/osal/${OS}
  FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/soem_rsl/oshw/${OS}/
  DESTINATION include/soem_rsl/oshw/${OS}
  FILES_MATCHING PATTERN "*.h"
)

install(EXPORT soem_rslConfig
  NAMESPACE soem_rsl::
  DESTINATION share/soem_rsl/cmake
)

#message(${CMAKE_INSTALL_PREFIX})
#add_custom_target(link_target ALL
 #                 COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/include/soem_rsl/ethercattype.h ${CMAKE_INSTALL_PREFIX}/include/ethercattype.h
 #                 VERBATIM)

if(BUILD_TESTS)
  if(BUILD_TESTING)
    find_package(ament_cmake_gtest REQUIRED)
    if(UNIX AND NOT APPLE)
      add_subdirectory(soem_rsl/test/linux/slaveinfo)
      add_subdirectory(soem_rsl/test/linux/eepromtool)
      add_subdirectory(soem_rsl/test/linux/simple_test)
    endif()
  endif()
endif()

ament_export_include_directories(include)
ament_package()