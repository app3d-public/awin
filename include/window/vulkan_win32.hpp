#ifndef APP_WINDOW_VULKAN_WIN32_H
#define APP_WINDOW_VULKAN_WIN32_H

#include <core/std/array.hpp>
#include <string>
#include <vulkan/vulkan.hpp>
#include "types.hpp"

namespace window
{
    namespace _internal
    {
        Array<std::string> getExtensionNames();

        vk::Result createSurfaceNative(WindowBase *window, vk::Instance instance, vk::SurfaceKHR &surface);
    } // namespace _internal
} // namespace window

#endif