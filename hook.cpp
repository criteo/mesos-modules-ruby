#include "hook.hpp"
#include "wrapper.hpp"

VALUE RubyHook::load_script(const std::string& scriptname)
{
  rb_load_protect(rb_str_new_cstr(scriptname.c_str()), 0, &state);
}

VALUE RubyHook::slaveRunTaskLabelDecorator(VALUE obj)
{
  return rb_protect(::slaveRunTaskLabelDecorator_wrapper, obj, &state);
}

VALUE RubyHook::slaveRemoveExecutorHook(VALUE obj)
{
  return rb_protect(::slaveRemoveExecutorHook_wrapper, obj, &state);
}

void RubyHook::handle_ruby_exception()
{
  ::handle_ruby_exception(&state);
}

static mesos::Hook* createHook(const mesos::Parameters& parameters)
{
  return new RubyHook();
}

mesos::modules::Module<mesos::Hook> com_criteo_mesos_rubyhook(
    MESOS_MODULE_API_VERSION,
    MESOS_VERSION,
    "Criteo Mesos",
    "mesos@criteo.com",
    "Ruby hook module",
    nullptr,
    createHook);
