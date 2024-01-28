#ifndef APP_WINDOW_VULKAN_H
#define APP_WINDOW_VULKAN_H

#include <core/std/array.hpp>
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
            Array<std::string> extensitions;
        } bd;

        Array<std::string> getExtensionNames();

        bool init(vk::DispatchLoaderDynamic *loader);

        [[nodiscard]] vk::Result createWindowSurface(Window *window, vk::Instance instance, vk::SurfaceKHR &surface);

        Array<const char *> requiredInstanceExtensions();
    } // namespace vulkan
} // namespace window

#endif