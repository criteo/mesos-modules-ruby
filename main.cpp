#include "hook.hpp"
#include <memory>

struct RubyEngine
{
  RubyEngine(const std::string& name) {
    ruby_init();
    ruby_script(name.c_str());
    ruby_init_loadpath();
  }

  ~RubyEngine() {
    ruby_cleanup(0);
  }
};

int main(int argc, char* argv[])
{
  RubyEngine engine("hook");

  RubyHook hook;
  hook.load_script(argc > 1 ? argv[1] : "./hook.rb");
  if (!hook.success()) {
    hook.handle_ruby_exception();
    return -1;
  }

  VALUE taskname = rb_str_new_cstr("test");

  hook.slaveRunTaskLabelDecorator(taskname);
  if (!hook.success()) {
    hook.handle_ruby_exception();
    return -1;
  }

  hook.slaveRemoveExecutorHook(taskname);
  if (!hook.success()) {
    hook.handle_ruby_exception();
    return -1;
  }

  return 0;
}
