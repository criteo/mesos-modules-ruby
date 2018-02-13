#ifndef __RUBYENGINE_HPP__
#define __RUBYENGINE_HPP__

#include <ruby.h>
#include <string>
#include <mutex>
#include <mesos/mesos.hpp>

const std::string PARAM_SCRIPT_PATH = "script_path";

namespace criteo {
  namespace mesos {

    namespace helpers{
      void hash_set(VALUE hash, const std::string& key, const std::string& value);
      void hash_set(VALUE hash, const std::string& key, VALUE value);
      bool is_non_empty_hash(VALUE hash);
      bool is_non_empty_array(VALUE array);
    }

    // Manages basic initialization of Ruby's engine
    class RubyEngine
    {
      int state = 0;

    public:
      RubyEngine(const std::string& name);
      ~RubyEngine();

      bool load_script(const std::string& scriptname);
      bool is_callback_defined(const std::string& symbol) const;

      std::string handle_exception();
    };

    // This class is a singleton,
    // to avoid multiple initializations
    // from multiple modules.
    //
    // Goal is too enforce a single common ruby source script
    // for all support modules declared here.
    // TODO: Support Ruby based declaration of the modules
    //
    // /!\ First set of mesos module paramater wins!
    //
    class RubyEngineSingleton : public RubyEngine
    {
      bool started = false;
      std::mutex m_mutex;

    public:
      static RubyEngineSingleton& getInstance();
      void start(const ::mesos::Parameters& parameters);
      std::mutex* mutex();

    private:
      RubyEngineSingleton();

    public:
      // remove capability to copy (by mistake) the object
      RubyEngineSingleton(RubyEngineSingleton const&) = delete;
      void operator=(RubyEngineSingleton const&)   = delete;
    };

  } // criteo.mesos
}
#endif // __RUBYENGINE_HPP__
