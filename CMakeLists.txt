cmake_minimum_required(VERSION 2.8)
project(ruby_hook)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fpermissive -std=c++11")

find_package(PkgConfig REQUIRED)

pkg_check_modules(RUBY REQUIRED ruby-2.4)

if(DEFINED ENV{MESOS_BUILD_DIR})
  set(MESOS_BUILD_DIR "$ENV{MESOS_BUILD_DIR}")
else()
  set(MESOS_BUILD_DIR "${PWD}")
endif()

include_directories(
  "${MESOS_BUILD_DIR}/3rdparty/boost-1.53.0"
  "${MESOS_BUILD_DIR}/3rdparty/glog-0.3.3/src"
  "${MESOS_BUILD_DIR}/3rdparty/protobuf-3.3.0/src"
)

pkg_check_modules(MESOS QUIET mesos)

if (NOT MESOS_FOUND) # Assume build tree has not been installed yet
  set(MESOS_CFLAGS -DPICOJSON_USE_INT64 -D__STDC_FORMAT_MACROS)
  set(MESOS_INCLUDE_DIRS ${MESOS_BUILD_DIR}/include)
  set(MESOS_LIBRARY_DIRS ${MESOS_BUILD_DIR}/src/.libs)
  set(MESOS_LIBRARIES mesos)
  include_directories(
    "${MESOS_BUILD_DIR}/3rdparty/libprocess/include"
    "${MESOS_BUILD_DIR}/3rdparty/picojson-1.3.0"
    "${MESOS_BUILD_DIR}/3rdparty/stout/include"
  )
endif()

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

add_executable(test_hook main.cpp)
target_include_directories(test_hook PUBLIC ${MESOS_BUILD_DIR}/3rdparty/googletest-release-1.8.0/googletest/include)
target_link_libraries(test_hook rubyhook ${MESOS_BUILD_DIR}/3rdparty/.libs/libgmock.a)
add_custom_target(check COMMAND test_hook ${CMAKE_SOURCE_DIR}/hook.rb)
