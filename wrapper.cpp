#include "wrapper.hpp"
#include <iostream>

VALUE slaveRunTaskLabelDecorator_wrapper(VALUE obj)
{
  rb_funcall(rb_mKernel, rb_intern("slaveRunTaskLabelDecorator"), 1, obj);
  return Qtrue;
}

VALUE slaveRemoveExecutorHook_wrapper(VALUE obj)
{
  rb_funcall(rb_mKernel, rb_intern("slaveRemoveExecutorHook"), 1, obj);
  return Qtrue;
}

void handle_ruby_exception(int* state)
{
  VALUE lasterr = rb_errinfo();
  std::cerr << "caught interpreter error: " << RSTRING_PTR(rb_obj_as_string(lasterr)) << std::endl;
  rb_set_errinfo(Qnil);
  *state = 0;
}
