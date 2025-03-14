#ifndef APP_WINDOW_VULKAN_H
#define APP_WINDOW_VULKAN_H

#include <core/api.hpp>
#include <backend/device.hpp>
#include <vulkan/vulkan.hpp>
#include "window.hpp"

namespace awin
{
    namespace vulkan
    {

        class APPLIB_API CreateCtx final : public DeviceCreateCtx
        {
        public:
            CreateCtx(Window &window) : DeviceCreateCtx(true), _window(window) {}

            [[nodiscard]] virtual vk::Result createSurface(vk::Instance &instance, vk::SurfaceKHR &surface,
                                                           vk::DispatchLoaderDynamic &loader) override;

            virtual acul::vector<const char *> getWindowExtensions() override
            {
#ifdef _WIN32
                return {vk::KHRSurfaceExtensionName, vk::KHRWin32SurfaceExtensionName};
#else
    #error "Unsupported platform"
#endif
            }

        private:
            Window &_window;
        };
    } // namespace vulkan
} // namespace awin

#endif