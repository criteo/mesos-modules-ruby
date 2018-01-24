cmake_minimum_required(VERSION 2.8)
project(ruby_hook)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wfatal-errors -std=c++11")

find_package(PkgConfig REQUIRED)
pkg_check_modules(MESOS REQUIRED mesos)
pkg_check_modules(RUBY REQUIRED ruby)

if(DEFINED ENV{MESOS_BUILD_DIR})
  set(MESOS_BUILD_DIR "$ENV{MESOS_BUILD_DIR}")
else()
  set(MESOS_BUILD_DIR "$ENV{HOME}/repos/mesos/build")
endif()

include_directories(
  "${MESOS_BUILD_DIR}/3rdparty/boost-1.53.0"
  "${MESOS_BUILD_DIR}/3rdparty/glog-0.3.3/src"
  "${MESOS_BUILD_DIR}/3rdparty/protobuf-3.3.0/src"
)

set(SOURCES
  RubyEngine.cpp
  RubyEngine.hpp
  RubyHook.cpp
  RubyHook.hpp
)

add_compile_options(${MESOS_CFLAGS} ${RUBY_CFLAGS})
include_directories(${MESOS_INCLUDE_DIRS} ${RUBY_INCLUDE_DIRS})
link_directories(${MESOS_LIBRARY_DIRS} ${RUBY_LIBRARY_DIRS})
link_libraries(${MESOS_LIBRARIES} ${RUBY_LIBRARIES})

add_library(rubyhook SHARED ${SOURCES})

add_executable(test main.cpp)
target_link_libraries(test rubyhook)
