#ifndef __HOOK_HPP__
#define __HOOK_HPP__

#include "RubyEngine.hpp"

#include <glog/logging.h>

#include <mesos/hook.hpp>
#include <mesos/module/hook.hpp>

class RubyHook : public mesos::Hook
{
  std::mutex mutex;
  RubyEngine ruby;

public:
  RubyHook(const mesos::Parameters& parameters);

  virtual Result<mesos::Labels> slaveRunTaskLabelDecorator(
    const mesos::TaskInfo& taskInfo,
    const mesos::ExecutorInfo& executorInfo,
    const mesos::FrameworkInfo& frameworkInfo,
    const mesos::SlaveInfo& slaveInfo) override;

  virtual Result<mesos::Environment> slaveExecutorEnvironmentDecorator(
    const mesos::ExecutorInfo& executorInfo) override;

  virtual Try<Nothing> slaveRemoveExecutorHook(
    const mesos::FrameworkInfo& frameworkInfo,
    const mesos::ExecutorInfo& executorInfo) override;
};

static mesos::Hook* createHook(const mesos::Parameters& parameters)
{
  try {
    return new RubyHook(parameters);
  } catch (const std::exception& e) {
    LOG(ERROR) << "RubyHook error: " << e.what();
    return nullptr; // already wrapped in a Try<> at calling site
  }
}

mesos::modules::Module<mesos::Hook> com_criteo_mesos_RubyHook(
    MESOS_MODULE_API_VERSION,
    MESOS_VERSION,
    "Criteo Mesos",
    "mesos@criteo.com",
    "Ruby hook module",
    nullptr,
    createHook);

#endif // __HOOK_HPP__
