#include "RubyHook.hpp"
#include <ruby.h>

#define ScriptName                 "RubyHook"
#define ScriptPathParam            "script_path"
#define SlaveRunTaskLabelDecorator "slaveRunTaskLabelDecorator"
#define SlaveRemoveExecutorHook    "slaveRemoveExecutorHook"

extern "C" {
VALUE slaveRunTaskLabelDecorator_wrapper(VALUE obj)
{
  VALUE result = rb_funcall(rb_mKernel, rb_intern(SlaveRunTaskLabelDecorator), 1, obj);
  return result;
}

VALUE slaveRemoveExecutorHook_wrapper(VALUE obj)
{
  VALUE result = rb_funcall(rb_mKernel, rb_intern(SlaveRemoveExecutorHook), 1, obj);
  return result;
}
}

RubyHook::RubyHook(const mesos::Parameters& parameters)
  : ruby_engine(ScriptName)
{
  foreach (const mesos::Parameter& parameter, parameters.parameter()) {
    if (parameter.has_key() && parameter.key() == ScriptPathParam) {
      if (parameter.has_value()) ruby_engine.load_script(parameter.value());
    }
  }
}

Result<mesos::Labels> RubyHook::slaveRunTaskLabelDecorator(
  const mesos::TaskInfo& taskInfo,
  const mesos::ExecutorInfo& executorInfo,
  const mesos::FrameworkInfo& frameworkInfo,
  const mesos::SlaveInfo& slaveInfo)
{
  synchronized (mutex) {
    int state = 0;
    VALUE taskname = rb_str_new_cstr(taskInfo.name().c_str());
    VALUE result = rb_protect(::slaveRunTaskLabelDecorator_wrapper, taskname, &state);
    if (state != 0) handle_ruby_exception();
  }

  mesos::Labels labels;
  //XXX:TODO: fill labels with result
  // auto l = labels.add_labels();
  // l->set_key("foo");
  // l->set_value("bar");
  return labels;
}

Try<Nothing> RubyHook::slaveRemoveExecutorHook(
  const mesos::FrameworkInfo& frameworkInfo,
  const mesos::ExecutorInfo& executorInfo)
{
  synchronized (mutex) {
    int state = 0;
    VALUE execname = rb_str_new_cstr(executorInfo.name().c_str());
    VALUE result = rb_protect(::slaveRemoveExecutorHook_wrapper, execname, &state);
    if (state != 0) handle_ruby_exception();
  }

  return Nothing();
}

void RubyHook::handle_ruby_exception()
{
  const VALUE lasterr = rb_errinfo();
  LOG(ERROR) << "caught interpreter error: " << RSTRING_PTR(rb_obj_as_string(lasterr));
  rb_set_errinfo(Qnil);
}

mesos::modules::Module<mesos::Hook> com_criteo_mesos_rubyhook(
    MESOS_MODULE_API_VERSION,
    MESOS_VERSION,
    "Criteo Mesos",
    "mesos@criteo.com",
    "Ruby hook module",
    nullptr,
    createHook);
