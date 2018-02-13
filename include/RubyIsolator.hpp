#ifndef __RUBY_ISOLATOR_MODULE_HPP__
#define __RUBY_ISOLATOR_MODULE_HPP__

#include "RubyEngine.hpp"

#include <ruby.h>
#include <mesos/mesos.hpp>
#include <mesos/slave/isolator.hpp>
#include <mesos/module/isolator.hpp>

using ::mesos::slave::ContainerLaunchInfo;

namespace criteo {
  namespace mesos {

    constexpr const char * PrepareCallbackName = "isolator_prepare";
    constexpr const char * CleanupCallbackName = "isolator_cleanup";

    class RubyIsolator : public ::mesos::slave::Isolator
    {
    public:

      RubyIsolator(const ::mesos::Parameters& parameters);

      virtual process::Future<Option<ContainerLaunchInfo>> prepare(
          const ::mesos::ContainerID& containerId,
          const ::mesos::slave::ContainerConfig& containerConfig) override;

      virtual process::Future<Nothing> cleanup(
          const ::mesos::ContainerID& containerId) override;

    private:

      bool rb2ContainerLaunchInfo(ContainerLaunchInfo& cli, VALUE ruby_value) const;

    };
   }
 }

 // Callback used by registration below...
static mesos::slave::Isolator* createRubyIsolator(const ::mesos::Parameters& parameters)
{
  try {
    return new criteo::mesos::RubyIsolator(parameters);
  } catch (const std::exception& e) {
    LOG(ERROR) << "RubyHook error: " << e.what();
    return nullptr; // already wrapped in a Try<> at calling site
  }
}

// Declaration of the isolator
mesos::modules::Module<mesos::slave::Isolator> com_criteo_mesos_RubyIsolator(
    MESOS_MODULE_API_VERSION,
    MESOS_VERSION,
    "Criteo Mesos",
    "mesos@criteo.com",
    "Ruby isolator module",
    nullptr,
    createRubyIsolator);

#endif //__RUBY_ISOLATOR_MODULE_HPP__
