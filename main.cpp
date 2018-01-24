#include "RubyHook.hpp"
#include <glog/logging.h>
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
  p->set_value(argc > 1 ? argv[1] : "../hook.rb");

  taskInfo.set_name("test_task");
  executorInfo.set_name("test_exec");

  std::unique_ptr<mesos::Hook> hook(createHook(parameters));

  hook->slaveRunTaskLabelDecorator(taskInfo,
                                   executorInfo,
                                   frameworkInfo,
                                   slaveInfo);

  hook->slaveRemoveExecutorHook(frameworkInfo, executorInfo);

  google::ShutdownGoogleLogging();
  return 0;
}
