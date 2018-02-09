#ifndef __RUBYENGINE_HPP__
#define __RUBYENGINE_HPP__

#include <string>
#include <mesos/mesos.hpp>

const std::string PARAM_SCRIPT_PATH = "script_path";

namespace criteo {
  namespace mesos {

    class RubyEngine
    {
      int state = 0;

    public:
      RubyEngine(const std::string& name);

      ~RubyEngine();

      bool load_script(const std::string& scriptname);

      bool is_callback_defined(const std::string& symbol);

      std::string handle_exception();
    };

    // This class is a singleton,
    // to avoid multiple initializations
    // from multiple modules
    //
    // /!\ First set of mesos module paramater wins!
    //
    class UniqueRubyEngine : public RubyEngine
    {
      bool started = false;
      std::mutex m_mutex;

    public:
      static UniqueRubyEngine& getInstance();
      void start(const ::mesos::Parameters& parameters);
      std::mutex * mutex();

    private:
      UniqueRubyEngine();

    public:
      // remove capability to copy (by mistake) the object
      UniqueRubyEngine(UniqueRubyEngine const&) = delete;
      void operator=(UniqueRubyEngine const&)   = delete;
    };

  } // criteo.mesos
}
#endif // __RUBYENGINE_HPP__
