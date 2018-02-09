
#undef NORETURN
#undef UNREACHABLE
#include <ruby.h>

#include "RubyIsolator.hpp"
#include "RubyEngine.hpp"

#include <mesos/mesos.hpp>
#include <mesos/module/isolator.hpp>
#include <mesos/slave/isolator.hpp>

#include <process/future.hpp>
#include <process/owned.hpp>
#include <process/process.hpp>

#include <stout/try.hpp>
#include <stout/option.hpp>

using criteo::mesos::RubyIsolator;
using mesos::ContainerID;
using mesos::ExecutorInfo;
using mesos::Parameters;
using mesos::Parameter;
using mesos::slave::Isolator;
using mesos::slave::ContainerConfig;

const std::string RUBY_SCRIPT_NAME = "RubyIsolator";
const std::string PARAM_SCRIPT_PATH = "script_path";


extern "C" {

VALUE ruby_isolator_cleanup(VALUE obj)
{
  return rb_funcall(rb_cObject, rb_intern(CleanupCallbackName), 1, obj);
}

VALUE ruby_isolator_prepare(VALUE obj)
{
  return rb_funcall(rb_cObject, rb_intern(PrepareCallbackName), 1, obj);
}

} // /extern "C"

RubyIsolator::RubyIsolator(const Parameters& parameters)
  :ruby(RUBY_SCRIPT_NAME)
{
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
  if (!ruby.load_script(scriptpath)) {
    throw std::runtime_error(ruby.handle_exception());
  }
}

RubyIsolator::~RubyIsolator(){

}

process::Future<Option<::mesos::slave::ContainerLaunchInfo>> RubyIsolator::prepare(
    const ContainerID& containerId,
    const ContainerConfig& containerConfig)
{
  LOG(INFO) << "RubyIsolator::prepare( " << stringify(containerId) << ", ..)";

  // Now call Ruby
  synchronized(mutex){ if (ruby.is_callback_defined(PrepareCallbackName)) {
    
    // Gather params for Ruby
    VALUE h_params = rb_hash_new();
    rb_hash_aset(h_params, rb_str_new_cstr("container_id"), rb_str_new_cstr(containerId.value().c_str()));
    rb_hash_aset(h_params, rb_str_new_cstr("user"), 
      rb_str_new_cstr((containerConfig.has_user() ? containerConfig.user().c_str(): "")));
    
    VALUE h_env = rb_hash_new();
    if( containerConfig.command_info().has_environment() ){
      auto env = containerConfig.command_info().environment();
      foreach( auto v, env.variables() ){
        auto value = v.has_value()? v.value().c_str() : "";
        auto name = v.name().c_str();
        rb_hash_aset(h_env, rb_str_new_cstr(name), rb_str_new_cstr(value));
      }
    }
    rb_hash_aset(h_params, rb_str_new_cstr("environment"), h_env); 
      
    // call Ruby in a protect env to avoid exception leakage
    int state = 0;
    VALUE result = rb_protect(::ruby_isolator_prepare, h_params, &state);
    //if (state != 0) {
    //  return Error(ruby.handle_exception());
    //}
  }}

  return None();
}

process::Future<Nothing> RubyIsolator::cleanup(
    const ContainerID& containerId)
{
  LOG(INFO) << "RubyIsolator::cleanup()";

  synchronized(mutex){
    if (ruby.is_callback_defined(CleanupCallbackName)) {
      int state = 0;
      VALUE result = rb_protect(::ruby_isolator_cleanup, Qnil, &state);
      //if (state != 0) {
      //  return Error(ruby.handle_exception());
      //
      //
      //
      //}
    }
  }
  return Nothing();
}    

// Callback used by registration below...
static Isolator* createRubyIsolator(const ::mesos::Parameters& parameters)
{
  try {
    return new RubyIsolator(parameters);
  } catch (const std::exception& e) {
    LOG(ERROR) << "RubyHook error: " << e.what();
    return nullptr; // already wrapped in a Try<> at calling site
  }
}

// Declaration of the isolator
mesos::modules::Module<Isolator> com_criteo_mesos_RubyIsolator(
    MESOS_MODULE_API_VERSION,
    MESOS_VERSION,
    "Criteo Mesos",
    "mesos@criteo.com",
    "Ruby isolator module",
    nullptr,
    createRubyIsolator);
