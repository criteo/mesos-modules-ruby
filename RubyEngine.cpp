#include "RubyEngine.hpp"
#include <ruby.h>

RubyEngine::RubyEngine(const std::string& name)
{
  ruby_init();
  ruby_script(name.c_str());
  ruby_init_loadpath();
}

RubyEngine::~RubyEngine()
{
  ruby_cleanup(state);
}

bool RubyEngine::load_script(const std::string& scriptname)
{
  rb_load_protect(rb_str_new_cstr(scriptname.c_str()), 0, &state);
  return (state == 0);
}
