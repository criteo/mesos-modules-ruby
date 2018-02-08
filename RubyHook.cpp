#include "RubyHook.hpp"
#include <stdexcept>

// undef already defined macros to reduce warnings
#undef NORETURN
#undef UNREACHABLE
#include <ruby.h>

#define ScriptName                 "RubyHook"
#define ScriptPathParam            "script_path"
#define SlaveRunTaskLabelDecorator "slaveRunTaskLabelDecorator"
#define SlaveExecutorEnvironmentDecorator "slaveExecutorEnvironmentDecorator"
#define SlaveRemoveExecutorHook    "slaveRemoveExecutorHook"

namespace {
  void hash_set(VALUE hash, const std::string& key, const std::string& value) {
    rb_hash_aset(hash, rb_str_new_cstr(key.c_str()), rb_str_new_cstr(value.c_str()));
  }

  void hash_set(VALUE hash, const std::string& key, VALUE value) {
    rb_hash_aset(hash, rb_str_new_cstr(key.c_str()), value);
  }

  bool isNonEmptyHash(VALUE hash) {
      return (!NIL_P(hash) && RB_TYPE_P(hash, T_HASH) && RHASH_SIZE(hash) > 0);
  }
}


extern "C" {
// unprotected call in mesos::Hook::slaveRunTaskLabelDecorator() callback
VALUE slaveRunTaskLabelDecorator_wrapper(VALUE obj)
{
  // 1 means we pass 1 argumenent which is "obj"
  VALUE result = rb_funcall(rb_cObject, rb_intern(SlaveRunTaskLabelDecorator), 1, obj);
  return result;
}

// unprotected call in mesos::Hook::slaveExecutorEnvironmentDecorator() callback
VALUE slaveExecutorEnvironmentDecorator_wrapper(VALUE obj)
{
  VALUE result = rb_funcall(rb_cObject, rb_intern(SlaveExecutorEnvironmentDecorator), 1, obj);
  return result;
}

// unprotected call in mesos::Hook::slaveRemoveExecutorHook() callback
VALUE slaveRemoveExecutorHook_wrapper(VALUE obj)
{
  VALUE result = rb_funcall(rb_cObject, rb_intern(SlaveRemoveExecutorHook), 1, obj);
  return result;
}

// map Ruby strings kv-pairs to mesos::Labels
// Ruby prototype for hash foreach closure is boggus and requires -fpermissive to compile.
static int unwrapLabels(VALUE key, VALUE value, VALUE arg)
{
  auto labels = (mesos::Labels*) arg;
  auto l = labels->add_labels();
  l->set_key(StringValueCStr(key));
  l->set_value(StringValueCStr(value));
  return ST_CONTINUE;
}

// map Ruby strings kv-pairs to mesos::Environment
// Ruby prototype for hash foreach closure is boggus and requires -fpermissive to compile.
static int unwrapEnv(VALUE name, VALUE value, VALUE arg)
{
  auto env = (mesos::Environment*) arg;
  auto e = env->add_variables();
  e->set_name(StringValueCStr(name));
  e->set_value(StringValueCStr(value));
  return ST_CONTINUE;
}

// map parts of mesos::TaskInfo into a Ruby hash structure
VALUE wrapTaskInfo(const mesos::TaskInfo& taskInfo)
{
  // create top level hash with required PB fields
  VALUE hash = rb_hash_new();
  hash_set(hash, "name", taskInfo.name());
  hash_set(hash, "task_id", taskInfo.task_id().value());
  hash_set(hash, "slave_id", taskInfo.slave_id().value());

  // add Labels as a strings hash and insert it into main hash as "labels"
  // e.g. in Ruby use: taskInfo["labels"]["foo"] = "bar"
  VALUE labels = rb_hash_new();
  if (taskInfo.has_labels()) {
    foreach (const mesos::Label& l, taskInfo.labels().labels()) {
      hash_set(labels, l.key(), l.has_value() ? l.value() : "");
    }
  }
  hash_set(hash, "labels", labels);

  return hash;
}

// map parts of mesos::ExecutorInfo into a Ruby hash structure
VALUE wrapExecutorInfo(const mesos::ExecutorInfo& executorInfo)
{
  // create top level hash with required PB fields
  VALUE hash = rb_hash_new();
  hash_set(hash, "name", executorInfo.name());

  // add command has a hash, TODO extract this in a method
  VALUE command = rb_hash_new();
  if (executorInfo.has_command()) {
    const mesos::CommandInfo& commandInfo = executorInfo.command();
    VALUE env = rb_hash_new();
    if (commandInfo.has_environment()) {
      foreach (const mesos::Environment::Variable& v, commandInfo.environment().variables()) {
        if (v.has_value()) { // Only one of `value` and `secret` must be set.
          hash_set(env, v.name(), v.value());
        }
      }
    }
    hash_set(command, "environment", env);
    VALUE user = commandInfo.has_user() ? rb_str_new_cstr(commandInfo.user().c_str())
                                        : rb_str_new_cstr("no_user_found");
    hash_set(command, "user", user);
  }
  hash_set(hash, "command", command);

  return hash;
}
}

////////////////////////////////////////////////////////////////////////////////
// build the RubyHook and load a Ruby script located by the 'script_path' parameter
RubyHook::RubyHook(const mesos::Parameters& parameters)
  : ruby(ScriptName)
{
  std::string scriptpath;
  foreach (const mesos::Parameter& parameter, parameters.parameter()) {
    if (parameter.has_key() && parameter.key() == ScriptPathParam && parameter.has_value()) {
      scriptpath = parameter.value();
    }
  }

  if (scriptpath.empty()) {
    throw std::invalid_argument("missing parameter " ScriptPathParam);
  }
  if (!ruby.load_script(scriptpath)) {
    throw std::runtime_error(ruby.handle_exception());
  }
}

// mesos::Hook callback on slaveExecutorEnvironmentDecorator
Result<mesos::Environment> RubyHook::slaveExecutorEnvironmentDecorator(
  const mesos::ExecutorInfo& executorInfo)
{
  synchronized (mutex) { // Ruby VM is not reentrant
    if (ruby.is_callback_defined(SlaveExecutorEnvironmentDecorator)) {
      // call Ruby in a protect env to avoid exception leakage
      int state = 0;
      VALUE executorInfoValue = wrapExecutorInfo(executorInfo); // map ExecutorInfo into strings kv-pair hash
      VALUE result = rb_protect(::slaveExecutorEnvironmentDecorator_wrapper, executorInfoValue, &state);
      if (state != 0) {
        return Error(ruby.handle_exception());
      }

      // fill in the Env from Ruby from return value; expect same structure as input
      if (isNonEmptyHash(result)) {
        VALUE ruby_cmd = rb_hash_lookup(result, rb_str_new_cstr("command"));
        if (isNonEmptyHash(ruby_cmd)) {
          VALUE ruby_env = rb_hash_lookup(ruby_cmd, rb_str_new_cstr("environment"));
          if (isNonEmptyHash(ruby_env)) {
            mesos::Environment env;
            // iterate over the hash and add env from kv-pairs
            // Ruby prototype for hash foreach closure is boggus and requires -fpermissive to compile.
            rb_hash_foreach(ruby_env, unwrapEnv, (VALUE)&env);
            return env; // will *replace* original env (i.e. not merge)
          }
        }
      }
    }
  }

  // if the Ruby method was not found or failed, return None which won't modify the original env
  return None();
}

// mesos::Hook callback on slaveRunTaskLabelDecorator()
Result<mesos::Labels> RubyHook::slaveRunTaskLabelDecorator(
  const mesos::TaskInfo& taskInfo,
  const mesos::ExecutorInfo& executorInfo,
  const mesos::FrameworkInfo& frameworkInfo,
  const mesos::SlaveInfo& slaveInfo)
{
  synchronized (mutex) { // Ruby VM is not reentrant
    if (ruby.is_callback_defined(SlaveRunTaskLabelDecorator)) {
      // call Ruby in a protect env to avoid exception leakage
      int state = 0;
      VALUE taskInfoValue = wrapTaskInfo(taskInfo); // map TaskInfo into strings kv-pair hash
      VALUE result = rb_protect(::slaveRunTaskLabelDecorator_wrapper, taskInfoValue, &state);
      if (state != 0) {
        return Error(ruby.handle_exception());
      }

      // fill in the Labels from Ruby from return value; expect same structure as input
      if (isNonEmptyHash(result)) {
        VALUE ruby_labels = rb_hash_lookup(result, rb_str_new_cstr("labels"));
        if (isNonEmptyHash(ruby_labels)) {
          mesos::Labels labels;
          // iterate over the hash and add labels from kv-pairs
          // Ruby prototype for hash foreach closure is boggus and requires -fpermissive to compile.
          rb_hash_foreach(ruby_labels, unwrapLabels, (VALUE)&labels);
          return labels; // will *replace* original labels (i.e. not merge)
        }
      }
    }
  }

  // if the Ruby method was not found or failed, return None which won't modify the original Labels
  return None();
}

// mesos::Hook callback on slaveRemoveExecutorHook()
Try<Nothing> RubyHook::slaveRemoveExecutorHook(
  const mesos::FrameworkInfo& frameworkInfo,
  const mesos::ExecutorInfo& executorInfo)
{
  synchronized (mutex) { // Ruby VM is not reentrant
    if (ruby.is_callback_defined(SlaveRemoveExecutorHook)) {
      // call Ruby in a protect env to avoid exception leakage
      int state = 0;
      VALUE execname = rb_str_new_cstr(executorInfo.name().c_str()); //TODO: pass more data?
      VALUE result = rb_protect(::slaveRemoveExecutorHook_wrapper, execname, &state);
      if (state != 0) {
        return Error(ruby.handle_exception());
      }
    }
  }

  return Nothing();
}
