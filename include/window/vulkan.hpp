#ifndef APP_WINDOW_VULKAN_H
#define APP_WINDOW_VULKAN_H

#include <core/api.hpp>
#include <vulkan/vulkan.hpp>
#include "window.hpp"

namespace window
{
    namespace vulkan
    {
        extern struct BackendData
        {
            bool available;
            vk::DispatchLoaderDynamic *loader;
            bool KHR_surface;
            bool KHR_win32_surface;
            bool MVK_macos_surface;
            bool EXT_metal_surface;
            bool KHR_xlib_surface;
            bool KHR_xcb_surface;
            bool KHR_wayland_surface;
            DArray<std::string> extensitions;
        } bd;

        APPLIB_API DArray<std::string> getExtensionNames();

        APPLIB_API bool init(vk::DispatchLoaderDynamic *loader);

        [[nodiscard]] APPLIB_API vk::Result createWindowSurface(Window *window, vk::Instance instance, vk::SurfaceKHR &surface);

        APPLIB_API DArray<const char *> requiredInstanceExtensions();
    } // namespace vulkan
} // namespace window

#endif