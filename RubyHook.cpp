#include "RubyHook.hpp"
#include <ruby.h>

extern "C" {
VALUE slaveRunTaskLabelDecorator_wrapper(VALUE obj)
{
  VALUE result = rb_funcall(rb_mKernel, rb_intern("slaveRunTaskLabelDecorator"), 1, obj);
  return result;
}

VALUE slaveRemoveExecutorHook_wrapper(VALUE obj)
{
  VALUE result = rb_funcall(rb_mKernel, rb_intern("slaveRemoveExecutorHook"), 1, obj);
  return result;
}
}

Result<mesos::Labels> RubyHook::slaveRunTaskLabelDecorator(
  const mesos::TaskInfo& taskInfo,
  const mesos::ExecutorInfo& executorInfo,
  const mesos::FrameworkInfo& frameworkInfo,
  const mesos::SlaveInfo& slaveInfo)
{
  int state = 0;
  VALUE taskname = rb_str_new_cstr(taskInfo.name().c_str());
  VALUE result = rb_protect(::slaveRunTaskLabelDecorator_wrapper, taskname, &state);
  if (state != 0) handle_ruby_exception();

  mesos::Labels labels;
  //TODO: fill labels with result
  // auto l = labels.add_labels();
  // l->set_key("foo");
  // l->set_value("bar");
  return labels;
}

Try<Nothing> RubyHook::slaveRemoveExecutorHook(
  const mesos::FrameworkInfo& frameworkInfo,
  const mesos::ExecutorInfo& executorInfo)
{
  int state = 0;
  VALUE execname = rb_str_new_cstr(executorInfo.name().c_str());
  VALUE result = rb_protect(::slaveRemoveExecutorHook_wrapper, execname, &state);
  if (state != 0) handle_ruby_exception();

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
