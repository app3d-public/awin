#ifndef APP_WINDOW_VULKAN_H
#define APP_WINDOW_VULKAN_H

#include <vulkan/vulkan.hpp>
#include "core/std/array.hpp"
#include "window/types.hpp"

namespace window
{
    namespace _internal
    {
        extern struct BackendData
        {
            bool available;
            vk::DispatchLoaderDynamic* loader;
            bool KHR_surface;
            bool KHR_win32_surface;
            bool MVK_macos_surface;
            bool EXT_metal_surface;
            bool KHR_xlib_surface;
            bool KHR_xcb_surface;
            bool KHR_wayland_surface;
            Array<std::string> extensitions;
        } bd;

    } // namespace _internal

    bool initVulkan(vk::DispatchLoaderDynamic* loader);

    [[nodiscard]] vk::Result createWindowSurface(WindowBase *window, vk::Instance instance, vk::SurfaceKHR &surface);

    Array<const char *> requiredInstanceExtensions();
} // namespace window

#endif