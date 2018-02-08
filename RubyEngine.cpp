#include "RubyEngine.hpp"
#include <ruby.h>

RubyEngine::RubyEngine(const std::string& name)
{
  ruby_init();
  ruby_script(name.c_str());
  ruby_init_loadpath();
  ruby_options(0, 0);
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

bool RubyEngine::is_callback_defined(const std::string& symbol)
{
  return (rb_funcall(rb_cObject, rb_intern("private_method_defined?"), 1, rb_str_new_cstr(symbol.c_str())) == Qtrue);
}

std::string RubyEngine::handle_exception()
{
  if (state != 0) {
    state = 0;
    VALUE lasterr = rb_errinfo();
    rb_set_errinfo(Qnil);
    return RSTRING_PTR(rb_obj_as_string(lasterr));
  }
  return std::string();
}
