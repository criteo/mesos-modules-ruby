cmake_minimum_required(VERSION 2.8)
project(mesos_ruby_modules)

set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wno-macro-redefined -std=c++11")

find_package(PkgConfig REQUIRED)

pkg_check_modules(RUBY REQUIRED ruby-2.4)

if(DEFINED ENV{MESOS_BUILD_DIR})
  set(MESOS_BUILD_DIR $ENV{MESOS_BUILD_DIR})
else()
  set(MESOS_BUILD_DIR "${PWD}")
endif()

# Required include path lookup in Build dir
file(GLOB BOOST_SRCH_PATH "${MESOS_BUILD_DIR}/3rdparty/boost-*")
find_path(BOOST_INCLUDE_DIR NAMES "boost/version.hpp" PATHS ${BOOST_SRCH_PATH} NO_DEFAULT_PATH)

file(GLOB PROTO_SRCH_PATH "${MESOS_BUILD_DIR}/3rdparty/protobuf-*/src")
find_path(PROTOBUF_INCLUDE_DIR NAMES "google/protobuf/stubs/common.h" PATHS ${PROTO_SRCH_PATH} NO_DEFAULT_PATH)

file(GLOB GLOG_SRCH_PATH "${MESOS_BUILD_DIR}/3rdparty/glog-*/src")
find_path(GLOG_INCLUDE_DIR NAMES "glog/logging.h" PATHS ${GLOG_SRCH_PATH} NO_DEFAULT_PATH)

file(GLOB PICOJSON_SRCH_PATH "${MESOS_BUILD_DIR}/3rdparty/picojson-*")
find_path(PICOJSON_INCLUDE_DIR NAMES "picojson.h" PATHS ${PICOJSON_SRCH_PATH} NO_DEFAULT_PATH)

include_directories(
  ./include
  ${BOOST_INCLUDE_DIR}
  ${GLOG_INCLUDE_DIR}
  ${PROTOBUF_INCLUDE_DIR}
  ${PICOJSON_INCLUDE_DIR}
  ${MESOS_BUILD_DIR}/3rdparty/stout/include
  ${MESOS_BUILD_DIR}/3rdparty/libprocess/include
  )

pkg_check_modules(MESOS QUIET mesos)

if (NOT MESOS_FOUND) # Assume build tree has not been installed yet
  set(MESOS_CFLAGS -DPICOJSON_USE_INT64 -D__STDC_FORMAT_MACROS)
  set(MESOS_INCLUDE_DIRS ${MESOS_BUILD_DIR}/include)
  set(MESOS_LIBRARY_DIRS ${MESOS_BUILD_DIR}/src/.libs)
  set(MESOS_LIBRARIES mesos)
endif()

set(MODULES_SOURCES
  src/RubyEngine.cpp
  src/RubyHook.cpp
  src/RubyIsolator.cpp
)

add_compile_options(${MESOS_CFLAGS} ${RUBY_CFLAGS})
include_directories(${MESOS_INCLUDE_DIRS} ${RUBY_INCLUDE_DIRS})
link_directories(${MESOS_LIBRARY_DIRS} ${RUBY_LIBRARY_DIRS})
link_libraries(${MESOS_LIBRARIES} ${RUBY_LIBRARIES})

add_library(ruby_modules SHARED ${MODULES_SOURCES})

# Unit Tests building & execution

add_executable(test_rmodules tests/main.cpp)

file(GLOB GTEST_SRCH_PATH "${MESOS_BUILD_DIR}/3rdparty/googletest-*/googletest/include")
find_path(GTEST_INCLUDE_DIR NAMES "gtest/gtest.h" PATHS ${GTEST_SRCH_PATH} NO_DEFAULT_PATH)

target_include_directories(test_rmodules PUBLIC ${GTEST_INCLUDE_DIR})
target_link_libraries(test_rmodules ruby_modules ${MESOS_BUILD_DIR}/3rdparty/.libs/libgmock.a)
add_custom_target(check COMMAND test_rmodules ${CMAKE_SOURCE_DIR}/tests/mesos_modules.rb)

