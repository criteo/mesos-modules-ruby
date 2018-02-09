
#include "RubyEngine.hpp"

#include <ruby.h>
#include <mesos/mesos.hpp>
#include <stout/foreach.hpp>

using criteo::mesos::RubyEngine;
using criteo::mesos::UniqueRubyEngine;
using mesos::Parameters;
using mesos::Parameter;

const std::string RUBY_SCRIPT_NAME = "mesos_ruby_modules";

// --------------------------------------------------------
// RubyEngine
// --------------------------------------------------------

RubyEngine::RubyEngine(const std::string& name)
{
  ruby_init();
  ruby_script(name.c_str());
  ruby_init_loadpath();
  ruby_options(0, NULL);
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

// --------------------------------------------------------
// UniqueRubyEngine
// --------------------------------------------------------

UniqueRubyEngine& UniqueRubyEngine::getInstance(){
  static UniqueRubyEngine instance;
  return instance;
}

void UniqueRubyEngine::start(const Parameters& parameters){

  if(started){
    return;
  }

  std::string scriptpath;
  foreach (const Parameter& parameter, parameters.parameter()) {
    if (parameter.has_key() && parameter.key() == PARAM_SCRIPT_PATH && parameter.has_value()) {
       scriptpath = parameter.value();
       break;
    }
  }

  if (scriptpath.empty()) {
    throw std::invalid_argument("missing parameter " + PARAM_SCRIPT_PATH);
  }
  if (!load_script(scriptpath)) {
    throw std::runtime_error(handle_exception());
  }

  started = true;
}

UniqueRubyEngine::UniqueRubyEngine()
  :RubyEngine(RUBY_SCRIPT_NAME)
{
}

std::mutex * UniqueRubyEngine::mutex(){
  return &m_mutex;
}
