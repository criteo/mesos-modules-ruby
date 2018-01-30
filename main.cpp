#include "RubyHook.hpp"
#include <iostream>
#include <memory>

int main(int argc, char* argv[])
{
  google::InitGoogleLogging(argv[0]);

  mesos::Parameters parameters;
  mesos::TaskInfo taskInfo;
  mesos::ExecutorInfo executorInfo;
  mesos::FrameworkInfo frameworkInfo;
  mesos::SlaveInfo slaveInfo;

  auto p = parameters.add_parameter();
  p->set_key("script_path");
  p->set_value(argc > 1 ? argv[1] : "./hook.rb");

  taskInfo.set_name("test_task");
  taskInfo.mutable_task_id()->set_value("1");
  taskInfo.mutable_slave_id()->set_value("2");
  auto l = taskInfo.mutable_labels()->add_labels();
  l->set_key("foo");
  l->set_value("bar");
  std::cout << "[C++] set: (key: \"foo\" => val: \"bar\")" << std::endl;

  executorInfo.set_name("test_exec");

  std::unique_ptr<mesos::Hook> hook(createHook(parameters));
  if (hook) {
    auto result = hook->slaveRunTaskLabelDecorator(taskInfo,
                                                   executorInfo,
                                                   frameworkInfo,
                                                   slaveInfo);

    if (result.isSome()) {
      for (const auto& l : result.get().labels()) {
        std::cout << "[C++] found: (key: \"" << l.key() << "\" => val: \"" << l.value() << "\")" << std::endl;
      }
    } else {
      std::cout << "[C++] no label returned" << std::endl;
    }

    hook->slaveRemoveExecutorHook(frameworkInfo, executorInfo);
  }

  google::ShutdownGoogleLogging();
  return 0;
}
