#ifndef APP_WINDOW_VULKAN_H
#define APP_WINDOW_VULKAN_H

#include <agrb/device.hpp>
#include "../window.hpp"

namespace awin
{
    namespace integration
    {
        class APPLIB_API CreateCtx final : public agrb::device_present_ctx
        {
        public:
            CreateCtx(Window &window) : _window(window) {}

            [[nodiscard]] virtual vk::Result create_surface(vk::Instance &instance, vk::SurfaceKHR &surface,
                                                            vk::DispatchLoaderDynamic &loader) override;

            virtual void assign_instance_extensions(const acul::set<acul::string> &ext,
                                                    acul::vector<const char *> &dst) override;

        private:
            Window &_window;
        };
    } // namespace integration
} // namespace awin

#endif