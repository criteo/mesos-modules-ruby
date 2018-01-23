$ export MESOS_BUILD_DIR={ directory where Mesos was **built**, e.g. ~/repos/mesos/build }  
$ export PKG_CONFIG_PATH={ probably the same directory or *installdir*/lib/pkgconfig }  
$ mkdir build  
$ cd build  
$ cmake ..  
$ make  
$ ./ruby_hook ../hook.rb
