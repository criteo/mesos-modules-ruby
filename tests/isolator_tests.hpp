#ifndef ___ISOLATOR_TESTS_HPP__
#define ___ISOLATOR_TESTS_HPP__

#include "RubyIsolator.hpp"
#include <memory>
#include <gtest/gtest.h>


class RubyIsolatorTest : public ::testing::Test
{
protected:
  static std::unique_ptr<mesos::slave::Isolator> isolator;
  mesos::ContainerID containerId;
  mesos::slave::ContainerConfig containerConfig;

public:
  static std::string script_path_str;

  static void SetUpTestCase()
  {
    mesos::Parameters parameters;
    auto p = parameters.add_parameter();
    p->set_key("script_path");
    p->set_value(script_path_str);
    isolator.reset(createRubyIsolator(parameters));
  }

  void SetUp()
  {
    containerId.set_value( "1-2-3-4-5-6-7-8" );
    containerConfig.set_user( "good-user" );
    auto cmd_info = containerConfig.mutable_command_info();
    cmd_info->set_value( "echo \"Hello World\"" );
    {
      auto env = cmd_info->mutable_environment();
      auto var = env->add_variables();
      var->set_name("foo");
      var->set_value("bar");
      var = env->add_variables();
      var->set_name("HELLO");
      var->set_value("world");
    }
  }
};

std::unique_ptr<mesos::slave::Isolator> RubyIsolatorTest::isolator;
std::string RubyIsolatorTest::script_path_str;

TEST_F(RubyIsolatorTest, PrepareTest)
{
  ASSERT_TRUE(isolator);

  auto fut = isolator->prepare(containerId, containerConfig);
  ASSERT_TRUE(fut.isReady());
  ASSERT_TRUE(fut.get().isSome());

  auto lauchInfo = fut.get().get();

  ASSERT_EQ(1, lauchInfo.pre_exec_commands().size());
  ASSERT_STREQ("touch /tmp/rb_isolator", lauchInfo.pre_exec_commands(0).value().c_str());
}

#endif //___ISOLATOR_TESTS_HPP__