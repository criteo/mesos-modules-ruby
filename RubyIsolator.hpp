#ifndef __RUBY_ISOLATOR_MODULE_HPP__
#define __RUBY_ISOLATOR_MODULE_HPP__

#include "RubyEngine.hpp"

#include <mesos/mesos.hpp>
#include <mesos/slave/isolator.hpp>

constexpr const char * PrepareCallbackName = "isolator_prepare";
constexpr const char * CleanupCallbackName = "isolator_cleanup";

namespace criteo {
  namespace mesos {

    class RubyIsolator : public ::mesos::slave::Isolator
    {
      RubyEngine ruby;
      std::mutex mutex;

    public:

      RubyIsolator(const ::mesos::Parameters& parameters);
      virtual ~RubyIsolator();

      virtual process::Future<Option<::mesos::slave::ContainerLaunchInfo>> prepare(
          const ::mesos::ContainerID& containerId,
          const ::mesos::slave::ContainerConfig& containerConfig);

      virtual process::Future<Nothing> cleanup(
          const ::mesos::ContainerID& containerId);

    };
  }
}

#endif
