Mesos Modules Ruby
------------------

This repository is a collection of **experimental** mesos modules to call ruby scripts implementing hooks.

This project is not battle-tested, use it at your own risk.

Build Instructions
------------------

```shell
    $ export MESOS_SOURCE_DIR=[ directory of Mesos source code, e.g. ~/repos/mesos/ ]
    $ export MESOS_BUILD_DIR=[ directory where Mesos was BUILT if different from $MESOS_SOURCE_DIR/build ]
    $ # export PKG_CONFIG_PATH=[ probably the same directory or ${installdir}/lib/pkgconfig ]
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ ./test_rmodules ../tests/mesos_modules.rb
```

Scripting Documentation
-----------------------

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


Deployement & Configuration
------------------------------

This libraries' modules supports the parameter `script_path`, which shall be unique for the library.
In reality the first one process shall win (no other `script_path` will be taken into account).

This is done on purpose to make it explicit that all modules (hook and isolator) will run in the same 
ruby context.

Example of json configuration file in examples sub-folder.
Don't forget to add `com_criteo_mesos_RubyIsolator` to the list of slave's activated isolators
(--isolation="...,com_criteo_mesos_RubyIsolator") 

 
