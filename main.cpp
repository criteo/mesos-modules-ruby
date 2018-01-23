#include "RubyEngine.hpp"
#include "RubyHook.hpp"
#include <memory>

int main(int argc, char* argv[])
{
  RubyEngine engine("hook");
  if (!engine.load_script(argc > 1 ? argv[1] : "./hook.rb")) return -1;

  mesos::Parameters parameters;
  mesos::TaskInfo taskInfo;
  mesos::ExecutorInfo executorInfo;
  mesos::FrameworkInfo frameworkInfo;
  mesos::SlaveInfo slaveInfo;

  std::unique_ptr<mesos::Hook> hook(createHook(parameters));

  hook->slaveRunTaskLabelDecorator(taskInfo,
                                   executorInfo,
                                   frameworkInfo,
                                   slaveInfo);

  hook->slaveRemoveExecutorHook(frameworkInfo, executorInfo);

  return 0;
}
