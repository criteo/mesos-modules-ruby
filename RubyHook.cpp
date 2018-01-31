#include "RubyHook.hpp"
#include <stdexcept>

// undef already defined macros to reduce warnings
#undef NORETURN
#undef UNREACHABLE
#include <ruby.h>

#define ScriptName                 "RubyHook"
#define ScriptPathParam            "script_path"
#define SlaveRunTaskLabelDecorator "slaveRunTaskLabelDecorator"
#define SlaveRemoveExecutorHook    "slaveRemoveExecutorHook"

extern "C" {
// unprotected call in mesos::Hook::slaveRunTaskLabelDecorator() callback
VALUE slaveRunTaskLabelDecorator_wrapper(VALUE obj)
{
  VALUE result = rb_funcall(rb_cObject, rb_intern(SlaveRunTaskLabelDecorator), 1, obj);
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
}

// map parts of mesos::TaskInfo into a Ruby hash structure
VALUE wrapTaskInfo(const mesos::TaskInfo& taskInfo)
{
  // create top level hash with required PB fields
  VALUE hash = rb_hash_new();
  rb_hash_aset(hash, rb_str_new_cstr("name"), rb_str_new_cstr(taskInfo.name().c_str()));
  rb_hash_aset(hash, rb_str_new_cstr("task_id"), rb_str_new_cstr(taskInfo.task_id().value().c_str()));
  rb_hash_aset(hash, rb_str_new_cstr("slave_id"), rb_str_new_cstr(taskInfo.slave_id().value().c_str()));

  // add Labels as a strings hash and insert it into main hash as "labels"
  // e.g. in Ruby use: taskInfo["labels"]["foo"] = "bar"
  VALUE labels = rb_hash_new();
  if (taskInfo.has_labels()) {
    foreach (const mesos::Label& l, taskInfo.labels().labels()) {
      rb_hash_aset(labels, rb_str_new_cstr(l.key().c_str()), rb_str_new_cstr(l.value().c_str()));
    }
  }
  rb_hash_aset(hash, rb_str_new_cstr("labels"), labels);

  return hash;
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
      if (!NIL_P(result) && RB_TYPE_P(result, T_HASH) && RHASH_SIZE(result) > 0) {
        VALUE ruby_labels = rb_hash_lookup(result, rb_str_new_cstr("labels"));
        if (!NIL_P(ruby_labels) && RB_TYPE_P(ruby_labels, T_HASH) && RHASH_SIZE(ruby_labels) > 0) {
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
