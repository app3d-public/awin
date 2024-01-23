#include <cassert>
#include <stdexcept>
#include <window/vulkan_win32.hpp>
// include platform specific headers first
#include <window/vulkan.hpp>
#include <window/window_win32.hpp>

namespace window
{
    namespace _internal
    {
        Array<std::string> getExtensionNames()
        {
            if (!bd.KHR_surface || !bd.KHR_win32_surface)
                return {};

            return {"VK_KHR_surface", "VK_KHR_win32_surface"};
        }

        vk::Result createSurfaceNative(WindowBase *window, vk::Instance instance, vk::SurfaceKHR &surface)
        {
            try
            {
                Win32Window *win32Window = static_cast<Win32Window *>(window);
                VkResult err;
                vk::Win32SurfaceCreateInfoKHR info;
                info.setHinstance(pd.instance).setHwnd(win32Window->nativeHandle());
                assert(bd.loader->vkCreateWin32SurfaceKHR != nullptr);
                surface = instance.createWin32SurfaceKHR(info, nullptr, *bd.loader);
                return vk::Result::eSuccess;
            }
            catch (vk::SystemError &e)
            {
                return static_cast<vk::Result>(e.code().value());
            }
            catch (std::runtime_error &e)
            {
                return vk::Result::eErrorInitializationFailed;
            }
        }
    } // namespace _internal
} // namespace window