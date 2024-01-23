#include <cassert>
#include <vulkan/vulkan.hpp>
#include <window/vulkan.hpp>
#ifdef _WIN32
    #include <window/vulkan_win32.hpp>
#else
    #error "Unsupported platform"
#endif

namespace window
{
    namespace _internal
    {
        BackendData bd = {.available = false,
                          .KHR_surface = false,
                          .KHR_win32_surface = false,
                          .MVK_macos_surface = false,
                          .EXT_metal_surface = false,
                          .KHR_xlib_surface = false,
                          .KHR_xcb_surface = false,
                          .KHR_wayland_surface = false};
    } // namespace _internal

    bool initVulkan(vk::DispatchLoaderDynamic* loader)
    {
        if (_internal::bd.available)
            return true;
        vk::Result err;
        u32 count;
        _internal::bd.loader = loader;
        err = vk::enumerateInstanceExtensionProperties(nullptr, &count, nullptr, *_internal::bd.loader);
        if (err != vk::Result::eSuccess)
            return false;
        Array<vk::ExtensionProperties> extensions(count);
        err = vk::enumerateInstanceExtensionProperties(nullptr, &count, extensions.data(), *_internal::bd.loader);
        if (err != vk::Result::eSuccess)
            return false;
        for (auto &extension : extensions)
        {
            if (strcmp(extension.extensionName, "VK_KHR_surface") == 0)
                _internal::bd.KHR_surface = true;
            else if (strcmp(extension.extensionName, "VK_KHR_win32_surface") == 0)
                _internal::bd.KHR_win32_surface = true;
            else if (strcmp(extension.extensionName, "VK_MVK_macos_surface") == 0)
                _internal::bd.MVK_macos_surface = true;
            else if (strcmp(extension.extensionName, "VK_EXT_metal_surface") == 0)
                _internal::bd.EXT_metal_surface = true;
            else if (strcmp(extension.extensionName, "VK_KHR_xlib_surface") == 0)
                _internal::bd.KHR_xlib_surface = true;
            else if (strcmp(extension.extensionName, "VK_KHR_xcb_surface") == 0)
                _internal::bd.KHR_xcb_surface = true;
            else if (strcmp(extension.extensionName, "VK_KHR_wayland_surface") == 0)
                _internal::bd.KHR_wayland_surface = true;
        }
        _internal::bd.available = true;
        _internal::bd.extensitions = _internal::getExtensionNames();
        return true;
    }

    vk::Result createWindowSurface(WindowBase *window, vk::Instance instance, vk::SurfaceKHR &surface)
    {
        if (!_internal::bd.available)
            return vk::Result::eErrorInitializationFailed;
        if (_internal::bd.extensitions.empty())
            return vk::Result::eErrorExtensionNotPresent;
        return _internal::createSurfaceNative(window, instance, surface);
    }

    Array<const char *> requiredInstanceExtensions()
    {
        assert(_internal::bd.available);
        Array<const char *> result;
        for (auto &extension : _internal::bd.extensitions)
            result.push_back(extension.c_str());
        return result;
    }
} // namespace window