
#include "RubyEngine.hpp"

#include <mesos/mesos.hpp>
#include <stout/foreach.hpp>

using criteo::mesos::RubyEngine;
using criteo::mesos::RubyEngineSingleton;
using mesos::Parameters;
using mesos::Parameter;

const std::string RUBY_SCRIPT_NAME = "mesos_ruby_modules";

// --------------------------------------------------------
// Ruby Helpers
// --------------------------------------------------------

namespace criteo {
  namespace mesos {
    namespace helpers {

      void hash_set(VALUE hash, const std::string& key, const std::string& value)
      {
        rb_hash_aset(hash, rb_str_new_cstr(key.c_str()), rb_str_new_cstr(value.c_str()));
      }

      void hash_set(VALUE hash, const std::string& key, VALUE value)
      {
        rb_hash_aset(hash, rb_str_new_cstr(key.c_str()), value);
      }

      bool is_non_empty_hash(VALUE hash)
      {
        return (!NIL_P(hash) && RB_TYPE_P(hash, T_HASH) && RHASH_SIZE(hash) > 0);
      }

      bool is_non_empty_array(VALUE array)
      {
        return (!NIL_P(array) && RB_TYPE_P(array, T_ARRAY) && RARRAY_LEN(array) > 0);
      }

    }
  }
}

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

bool RubyEngine::is_callback_defined(const std::string& symbol) const
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

RubyEngineSingleton& RubyEngineSingleton::getInstance()
{
  static RubyEngineSingleton instance;
  return instance;
}

void RubyEngineSingleton::start(const Parameters& parameters)
{
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

RubyEngineSingleton::RubyEngineSingleton()
  :RubyEngine(RUBY_SCRIPT_NAME)
{
}

std::mutex* RubyEngineSingleton::mutex()
{
  return &m_mutex;
}
