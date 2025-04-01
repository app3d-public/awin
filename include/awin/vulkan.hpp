#ifndef APP_WINDOW_VULKAN_H
#define APP_WINDOW_VULKAN_H

#include <acul/api.hpp>
#include <acul/gpu/device.hpp>
#include "window.hpp"

namespace awin
{
    namespace vulkan
    {
        class APPLIB_API CreateCtx final : public acul::gpu::device_create_ctx
        {
        public:
            CreateCtx(Window &window) : device_create_ctx(true), _window(window) {}

            [[nodiscard]] virtual vk::Result create_surface(vk::Instance &instance, vk::SurfaceKHR &surface,
                                                            vk::DispatchLoaderDynamic &loader) override;

            virtual acul::vector<const char *> get_window_extensions() override
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