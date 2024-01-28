#include <cassert>
#include <vulkan/vulkan.hpp>
#include <window/vulkan.hpp>

namespace window
{
    namespace vulkan
    {
        BackendData bd = {.available = false,
                          .KHR_surface = false,
                          .KHR_win32_surface = false,
                          .MVK_macos_surface = false,
                          .EXT_metal_surface = false,
                          .KHR_xlib_surface = false,
                          .KHR_xcb_surface = false,
                          .KHR_wayland_surface = false};

        bool init(vk::DispatchLoaderDynamic *loader)
        {
            if (bd.available)
                return true;
            vk::Result err;
            u32 count;
            bd.loader = loader;
            err = vk::enumerateInstanceExtensionProperties(nullptr, &count, nullptr, *bd.loader);
            if (err != vk::Result::eSuccess)
                return false;
            Array<vk::ExtensionProperties> extensions(count);
            err = vk::enumerateInstanceExtensionProperties(nullptr, &count, extensions.data(), *bd.loader);
            if (err != vk::Result::eSuccess)
                return false;
            for (const auto &extension : extensions)
            {
                if (strcmp(extension.extensionName, "VK_KHR_surface") == 0)
                    bd.KHR_surface = true;
                else if (strcmp(extension.extensionName, "VK_KHR_win32_surface") == 0)
                    bd.KHR_win32_surface = true;
                else if (strcmp(extension.extensionName, "VK_MVK_macos_surface") == 0)
                    bd.MVK_macos_surface = true;
                else if (strcmp(extension.extensionName, "VK_EXT_metal_surface") == 0)
                    bd.EXT_metal_surface = true;
                else if (strcmp(extension.extensionName, "VK_KHR_xlib_surface") == 0)
                    bd.KHR_xlib_surface = true;
                else if (strcmp(extension.extensionName, "VK_KHR_xcb_surface") == 0)
                    bd.KHR_xcb_surface = true;
                else if (strcmp(extension.extensionName, "VK_KHR_wayland_surface") == 0)
                    bd.KHR_wayland_surface = true;
            }
            bd.available = true;
            bd.extensitions = getExtensionNames();
            return true;
        }

        Array<const char *> requiredInstanceExtensions()
        {
            assert(bd.available);
            Array<const char *> result;
            for (auto &extension : bd.extensitions)
                result.push_back(extension.c_str());
            return result;
        }
    } // namespace vulkan
} // namespace window