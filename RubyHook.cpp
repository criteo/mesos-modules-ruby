#include "RubyHook.hpp"
#include <ruby.h>
#include <stdexcept>

#define ScriptName                 "RubyHook"
#define ScriptPathParam            "script_path"
#define SlaveRunTaskLabelDecorator "slaveRunTaskLabelDecorator"
#define SlaveRemoveExecutorHook    "slaveRemoveExecutorHook"

extern "C" {
VALUE slaveRunTaskLabelDecorator_wrapper(VALUE obj)
{
  VALUE result = rb_funcall(rb_cObject, rb_intern(SlaveRunTaskLabelDecorator), 1, obj);
  return result;
}

VALUE slaveRemoveExecutorHook_wrapper(VALUE obj)
{
  VALUE result = rb_funcall(rb_cObject, rb_intern(SlaveRemoveExecutorHook), 1, obj);
  return result;
}
}

RubyHook::RubyHook(const mesos::Parameters& parameters)
  : ruby(ScriptName)
{
  std::string scriptpath;
  foreach (const mesos::Parameter& parameter, parameters.parameter()) {
    if (parameter.has_key() && parameter.key() == ScriptPathParam && parameter.has_value()) {
      scriptpath = parameter.value();
    }
  }

  if (scriptpath.empty()) {
    throw std::invalid_argument("missing parameter " ScriptPathParam);
  }
  if (!ruby.load_script(scriptpath)) {
    throw std::runtime_error(ruby.handle_exception());
  }
}

Result<mesos::Labels> RubyHook::slaveRunTaskLabelDecorator(
  const mesos::TaskInfo& taskInfo,
  const mesos::ExecutorInfo& executorInfo,
  const mesos::FrameworkInfo& frameworkInfo,
  const mesos::SlaveInfo& slaveInfo)
{
  synchronized (mutex) {
    if (ruby.is_callback_defined(SlaveRunTaskLabelDecorator)) {
      int state = 0;
      VALUE taskname = rb_str_new_cstr(taskInfo.name().c_str());
      VALUE result = rb_protect(::slaveRunTaskLabelDecorator_wrapper, taskname, &state);
      if (state != 0) {
        return Error(ruby.handle_exception());
      } else {
        mesos::Labels labels;
        //TODO: fill labels with result
        // auto l = labels.add_labels();
        // l->set_key("foo");
        // l->set_value("bar");
        return labels;
      }
    }
  }

  return None();
}

Try<Nothing> RubyHook::slaveRemoveExecutorHook(
  const mesos::FrameworkInfo& frameworkInfo,
  const mesos::ExecutorInfo& executorInfo)
{
  synchronized (mutex) {
    if (ruby.is_callback_defined(SlaveRemoveExecutorHook)) {
      int state = 0;
      VALUE execname = rb_str_new_cstr(executorInfo.name().c_str());
      VALUE result = rb_protect(::slaveRemoveExecutorHook_wrapper, execname, &state);
      if (state != 0) {
        return Error(ruby.handle_exception());
      }
    }
  }

  return Nothing();
}
