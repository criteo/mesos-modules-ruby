
#include "RubyEngine.hpp"
#include "RubyIsolator.hpp"

#include <mesos/mesos.hpp>
#include <mesos/module/isolator.hpp>
#include <mesos/slave/isolator.hpp>

using namespace criteo::mesos::helpers;

using criteo::mesos::RubyEngineSingleton;
using criteo::mesos::RubyIsolator;
using criteo::mesos::PrepareCallbackName;
using criteo::mesos::CleanupCallbackName;

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

VALUE unwrap_rb_pre_exec_commands(VALUE rb_hash_command_info, ContainerLaunchInfo& cli)
{
  // Currently only _shell_ commands (w/o environments neither URIs are supported
  // so, we just extract it...
  VALUE ci_value = rb_hash_lookup(rb_hash_command_info, rb_str_new_cstr("value"));
  if( RTEST(ci_value) ){
    cli.add_pre_exec_commands()->set_value( StringValueCStr(ci_value) );
  }
  return Qnil;
}

} // /extern "C"

RubyIsolator::RubyIsolator(const Parameters& parameters)
{
  RubyEngineSingleton::getInstance().start(parameters);
}

Future<Option<::mesos::slave::ContainerLaunchInfo>> RubyIsolator::prepare(
    const ContainerID& containerId,
    const ContainerConfig& containerConfig)
{
  LOG(INFO) << "RubyIsolator::prepare( " << stringify(containerId) << ", ..)";

  RubyEngineSingleton& ruby = RubyEngineSingleton::getInstance();
  std::mutex * mutex = ruby.mutex();
  synchronized(mutex){
    if (ruby.is_callback_defined(PrepareCallbackName)) {

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

      // Unwrap result, if possible ;-)
      ContainerLaunchInfo cli;
      if( rb2ContainerLaunchInfo(cli, result) ){
        return cli;
      }
    }
  }

  return None();
}

bool RubyIsolator::rb2ContainerLaunchInfo(
  ContainerLaunchInfo& cli,
  VALUE ruby_value) const
{

  // fill in the Env from Ruby from return value; expect same structure as input
  if (!is_non_empty_hash(ruby_value)) {
    return false;
  }

  VALUE pre_exec_commands = rb_hash_lookup(ruby_value, rb_str_new_cstr("pre_exec_commands"));
  if(!is_non_empty_array(pre_exec_commands)){
    return false;
  }

  for( int i = 0; i < RARRAY_LEN(pre_exec_commands); ++i ){
    VALUE command = rb_ary_entry(pre_exec_commands, i);
    if( RTEST(command) ){
      unwrap_rb_pre_exec_commands(command, cli);
    }
  }

  return true;
}

Future<Nothing> RubyIsolator::cleanup(
    const ContainerID& containerId)
{
  LOG(INFO) << "RubyIsolator::cleanup()";

  RubyEngineSingleton& ruby = RubyEngineSingleton::getInstance();

  std::mutex * mutex = ruby.mutex();
  synchronized(mutex){
    if (ruby.is_callback_defined(CleanupCallbackName)) {

      // Gather params for Ruby
      VALUE h_params = rb_hash_new();
      rb_hash_aset(h_params, rb_str_new_cstr("container_id"), rb_str_new_cstr(containerId.value().c_str()));

      int state = 0;
      VALUE result = rb_protect(::ruby_isolator_cleanup, h_params, &state);
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
