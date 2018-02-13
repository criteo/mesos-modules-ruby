## Mesos Modules Ruby

This repository is a collection of **experimental** mesos modules to call ruby scripts implementing hooks.

This project is not battle-tested, use it at your own risk.

## Build Instructions

```shell
    $ export MESOS_BUILD_DIR=[ directory where Mesos was BUILT, e.g. ~/repos/mesos/build ]
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ ./test_rmodules ../tests/mesos_modules.rb
```

## Scripting Documentation

Example available in ./tests/mesos_modules.rb

### Ruby based Mesos Hook

RubyHook is currently expecting the following API from the attached Ruby script:

```ruby
    def slaveRunTaskLabelDecorator taskinfo
        ...             # read and modify taskinfo fields or
        ...             # taskinfo sub-classes, esp. Labels
        return taskinfo # mandatory!
    end
    
    def slaveRemoveExecutorHook execinfo
        ...
        # no return value
    end
```

Both `taskinfo` and `execinfo` are hashes filled from equivalent C++ classes.
For instance, you can get the name of a task with `taskinfo["name"]` and access
the labels kv-pairs through `taskinfo["labels"]` as a string-string hash. 

### Ruby based Mesos Isolator

Current implemenation supports the prepare and cleanup callback of the mesos isolator
For details on the Isolator interface, please check directly the source code:
 https://github.com/apache/mesos/blob/master/include/mesos/slave/isolator.hpp

```ruby
  def isolator_prepare params
    # Params contains the container_id, user,
    # and environment (pre-launch w/o for example the actual mesos-slave inject environment!)

    return {"pre_exec_commands" => [{"value" => "touch /tmp/rb_isolator"}]} # pre exec commands
  end

  def isolator_cleanup params
    # params: hash containing a container_id
  end
```

## Deployement & Configuration

This library supports a single source Ruby script, for both supported module interfaces: hook and isolation.
The hook and isolation modules both supports the `script_path` parameter, and shall be pointing to the same script.
The library will, on purpose, only take into account the first `script_path` processed.

This is done on purpose to make it explicit that all modules (hook and isolator) will run in the same 
ruby context (single vm/process).

Example of json configuration file in examples sub-folder.
Don't forget to add `com_criteo_mesos_RubyIsolator` to the list of slave's activated isolators
(--isolation="...,com_criteo_mesos_RubyIsolator") 
