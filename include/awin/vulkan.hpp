#ifndef APP_WINDOW_VULKAN_H
#define APP_WINDOW_VULKAN_H

#include <acul/api.hpp>
#include <acul/gpu/device.hpp>
#include "window.hpp"

namespace awin
{
    namespace vulkan
    {
        class APPLIB_API CreateCtx final : public acul::gpu::device::create_ctx
        {
        public:
            CreateCtx(Window &window) : create_ctx(true), _window(window) {}

            [[nodiscard]] virtual vk::Result create_surface(vk::Instance &instance, vk::SurfaceKHR &surface,
                                                            vk::DispatchLoaderDynamic &loader) override;

            virtual void init_extensions(const acul::set<acul::string> &ext, acul::vector<const char *> &dst) override;
        private:
            Window &_window;
        };
    } // namespace vulkan
} // namespace awin

#endif