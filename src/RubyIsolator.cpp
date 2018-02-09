
#include "RubyIsolator.hpp"
#include <ruby.h>

#include <mesos/mesos.hpp>
#include <mesos/module/isolator.hpp>
#include <mesos/slave/isolator.hpp>

using criteo::mesos::UniqueRubyEngine;
using criteo::mesos::RubyIsolator;
using mesos::ContainerID;
using mesos::ExecutorInfo;
using mesos::Parameters;
using mesos::Parameter;
using mesos::slave::Isolator;
using mesos::slave::ContainerConfig;
using process::Future;
using process::Failure;

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
{
  UniqueRubyEngine::getInstance().start(parameters);
}

Future<Option<::mesos::slave::ContainerLaunchInfo>> RubyIsolator::prepare(
    const ContainerID& containerId,
    const ContainerConfig& containerConfig)
{
  LOG(INFO) << "RubyIsolator::prepare( " << stringify(containerId) << ", ..)";

  UniqueRubyEngine& ruby = UniqueRubyEngine::getInstance();

  // Now call Ruby
  std::mutex * mutex = ruby.mutex();
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
    if (state != 0) {
      return Failure(ruby.handle_exception());
    }
  }}

  return None();
}

Future<Nothing> RubyIsolator::cleanup(
    const ContainerID& containerId)
{
  LOG(INFO) << "RubyIsolator::cleanup()";

  UniqueRubyEngine& ruby = UniqueRubyEngine::getInstance();

  std::mutex * mutex = ruby.mutex();
  synchronized(mutex){
    if (ruby.is_callback_defined(CleanupCallbackName)) {
      int state = 0;
      VALUE result = rb_protect(::ruby_isolator_cleanup, Qnil, &state);
      if (state != 0) {
        return Failure(ruby.handle_exception());
      }
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
