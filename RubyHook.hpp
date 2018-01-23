#ifndef __HOOK_HPP__
#define __HOOK_HPP__

#include <mesos/hook.hpp>
#include <mesos/module/hook.hpp>

class RubyHook : public mesos::Hook
{
public:
  virtual Result<mesos::Labels> slaveRunTaskLabelDecorator(
    const mesos::TaskInfo& taskInfo,
    const mesos::ExecutorInfo& executorInfo,
    const mesos::FrameworkInfo& frameworkInfo,
    const mesos::SlaveInfo& slaveInfo) override;

  virtual Try<Nothing> slaveRemoveExecutorHook(
    const mesos::FrameworkInfo& frameworkInfo,
    const mesos::ExecutorInfo& executorInfo) override;

private:
  void handle_ruby_exception(); //TODO: handle SIGABRT too
};

static mesos::Hook* createHook(const mesos::Parameters& parameters)
{
  return new RubyHook();
}

extern mesos::modules::Module<mesos::Hook> com_criteo_mesos_rubyhook;

#endif // __HOOK_HPP__
