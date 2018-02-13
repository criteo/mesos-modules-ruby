#ifndef __RUBY_ISOLATOR_MODULE_HPP__
#define __RUBY_ISOLATOR_MODULE_HPP__

#include "RubyEngine.hpp"

#include <ruby.h>
#include <mesos/mesos.hpp>
#include <mesos/slave/isolator.hpp>

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

#endif
