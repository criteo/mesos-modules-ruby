#ifndef __RUBYENGINE_HPP__
#define __RUBYENGINE_HPP__

#include <string>

class RubyEngine
{
  int state = 0;

public:
  RubyEngine(const std::string& name);

  ~RubyEngine();

  bool load_script(const std::string& scriptname);
};

#endif // __RUBYENGINE_HPP__
