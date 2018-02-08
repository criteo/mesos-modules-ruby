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
    $ ./test_hook ../hook.rb
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
