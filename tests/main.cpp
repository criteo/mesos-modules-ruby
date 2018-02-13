
#include "hook_tests.hpp"
#include "isolator_tests.hpp"

#include <gtest/gtest.h>

int main(int argc, char* argv[])
{
  google::InitGoogleLogging(argv[0]);

  ::testing::InitGoogleTest(&argc, argv);

  RubyHookTest::script_path_str = (argc > 1 ? argv[1] : "./hook.rb");
  RubyIsolatorTest::script_path_str = (argc > 1 ? argv[1] : "./hook.rb");

  auto res = RUN_ALL_TESTS();

  google::ShutdownGoogleLogging();

  return res;
}
