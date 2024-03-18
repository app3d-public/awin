#include <window/vulkan.hpp>
#include <window/platform_win32.hpp>

namespace window
{
    namespace vulkan
    {
        Array<std::string> getExtensionNames()
        {
            if (!bd.KHR_surface || !bd.KHR_win32_surface)
                return {};

            return {vk::KHRSurfaceExtensionName, vk::KHRWin32SurfaceExtensionName};
        }

        [[nodiscard]] vk::Result createWindowSurface(Window *window, vk::Instance instance, vk::SurfaceKHR &surface)
        {
            if (!bd.available)
                return vk::Result::eErrorInitializationFailed;
            if (bd.extensitions.empty())
                return vk::Result::eErrorExtensionNotPresent;
            try
            {
                VkResult err;
                vk::Win32SurfaceCreateInfoKHR info;
                auto accessBridge = window->accessBridge();
                info.setHinstance(accessBridge.instance()).setHwnd(accessBridge.hwnd());
                surface = instance.createWin32SurfaceKHR(info, nullptr, *bd.loader);
                return vk::Result::eSuccess;
            }
            catch (const vk::SystemError &e)
            {
                return static_cast<vk::Result>(e.code().value());
            }
            catch (const std::runtime_error &e)
            {
                return vk::Result::eErrorInitializationFailed;
            }
        }
    } // namespace vulkan
} // namespace window