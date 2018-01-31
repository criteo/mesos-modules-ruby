#include "RubyHook.hpp"
#include <memory>
#include <gtest/gtest.h>


class RubyHookTest : public ::testing::Test
{
protected:
  static std::unique_ptr<mesos::Hook> hook;
  mesos::TaskInfo taskInfo;
  mesos::ExecutorInfo executorInfo;
  mesos::FrameworkInfo frameworkInfo;
  mesos::SlaveInfo slaveInfo;

public:
  static std::string script_path_str;

  static void SetUpTestCase()
  {
    mesos::Parameters parameters;
    auto p = parameters.add_parameter();
    p->set_key("script_path");
    p->set_value(script_path_str);
    hook.reset(createHook(parameters));
  }

  void SetUp()
  {
    taskInfo.set_name("test_task");
    taskInfo.mutable_task_id()->set_value("1");
    taskInfo.mutable_slave_id()->set_value("2");
    {
      auto l = taskInfo.mutable_labels()->add_labels();
      l->set_key("foo");
      l->set_value("bar");
    }

    executorInfo.set_name("test_exec");
  }
};

std::unique_ptr<mesos::Hook> RubyHookTest::hook;
std::string RubyHookTest::script_path_str;

TEST_F(RubyHookTest, SlaveRunTaskLabelDecoratorTest)
{
  ASSERT_TRUE(hook);

  auto result = hook->slaveRunTaskLabelDecorator(taskInfo,
                                                 executorInfo,
                                                 frameworkInfo,
                                                 slaveInfo);
  ASSERT_TRUE(result.isSome());

  for (const auto& l : result.get().labels()) {
    if (l.key() == "foo") { // modified by Ruby
      ASSERT_STREQ("barbaz", l.value().c_str());
    }
    if (l.key() == "toto") { // added by Ruby
      ASSERT_STREQ("titi", l.value().c_str());
    }
  }
}

TEST_F(RubyHookTest, SlaveRemoveExecutorHookTest)
{
  ASSERT_TRUE(hook);

  auto result = hook->slaveRemoveExecutorHook(frameworkInfo, executorInfo);
  ASSERT_FALSE(result.isError());
}

int main(int argc, char* argv[])
{
  google::InitGoogleLogging(argv[0]);

  ::testing::InitGoogleTest(&argc, argv);

  RubyHookTest::script_path_str = (argc > 1 ? argv[1] : "./hook.rb");

  auto res = RUN_ALL_TESTS();

  google::ShutdownGoogleLogging();

  return res;
}
