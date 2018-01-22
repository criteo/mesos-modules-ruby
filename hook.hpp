#include <mesos/hook.hpp>
#include <mesos/mesos.hpp>
#include <mesos/module.hpp>
#include <mesos/module/hook.hpp>

#include "wrapper.hpp"

#include <string>

class RubyHook : public mesos::Hook
{
public:
  RubyHook() : state(0) {}

  bool success() { return state == 0; }

  VALUE load_script(const std::string& scriptname);

  VALUE slaveRunTaskLabelDecorator(VALUE obj);

  VALUE slaveRemoveExecutorHook(VALUE obj);

  void handle_ruby_exception(); //TODO: handle SIGABRT too

private:
  int state;
};

static mesos::Hook* createHook(const mesos::Parameters& parameters);

extern mesos::modules::Module<mesos::Hook> com_criteo_mesos_rubyhook;
