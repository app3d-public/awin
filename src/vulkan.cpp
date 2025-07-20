#include <awin/native_access.hpp>
#include <awin/vulkan.hpp>
#ifdef _WIN32
    #include "win32_pd.hpp"
#else
    #include "wayland/platform.hpp"
    #include "x11/platform.hpp"
#endif

namespace awin
{
    namespace vulkan
    {
        void CreateCtx::assign_instance_extensions(const acul::set<acul::string> &ext, acul::vector<const char *> &dst)
        {
            dst.push_back(vk::KHRSurfaceExtensionName);
#ifdef _WIN32
            dst.push_back(vk::KHRWin32SurfaceExtensionName);
#else
            int backend = native_access::get_backend_type();
            if (backend == WINDOW_BACKEND_X11)
            {
    #ifdef VK_USE_PLATFORM_XCB_KHR
                auto it = ext.find(vk::KHRXcbSurfaceExtensionName);
                if (it != ext.end())
                {
                    platform::x11::ctx.xlib.xcb.is_extension_present = true;
                    dst.push_back(vk::KHRXcbSurfaceExtensionName);
                }
    #endif
    #ifdef VK_USE_PLATFORM_XLIB_KHR
                dst.push_back(vk::KHRXlibSurfaceExtensionName);
    #endif
            }
    #ifdef VK_USE_PLATFORM_WAYLAND_KHR
            else if (backend == WINDOW_BACKEND_WAYLAND)
                dst.push_back(vk::KHRWaylandSurfaceExtensionName);
    #endif
#endif
        }

#ifndef _WIN32
    #ifdef VK_USE_PLATFORM_XCB_KHR
        static vk::Result create_xcb_surface(vk::Instance &instance, vk::SurfaceKHR &surface,
                                             vk::DispatchLoaderDynamic &loader, Window &window)
        {
            xcb_connection_t *connection = platform::x11::ctx.xlib.xcb.XGetXCBConnection(platform::x11::ctx.display);
            if (!connection)
            {
                LOG_ERROR("X11: Failed to retrieve XCB connection");
                return vk::Result::eErrorExtensionNotPresent;
            }
            vk::XcbSurfaceCreateInfoKHR info;
            info.setConnection(connection).setWindow(native_access::get_x11_window_handle(window));
            surface = instance.createXcbSurfaceKHR(info, nullptr, loader);
            return vk::Result::eSuccess;
        }
    #endif
    #ifdef VK_USE_PLATFORM_XLIB_KHR
        static vk::Result create_xlib_surface(vk::Instance &instance, vk::SurfaceKHR &surface,
                                              vk::DispatchLoaderDynamic &loader, Window &window)
        {
            vk::XlibSurfaceCreateInfoKHR info;
            info.setDpy(platform::x11::ctx.display).setWindow(native_access::get_x11_window_handle(window));
            surface = instance.createXlibSurfaceKHR(info, nullptr, loader);
            return vk::Result::eSuccess;
        }
    #endif
#endif

        vk::Result CreateCtx::create_surface(vk::Instance &instance, vk::SurfaceKHR &surface,
                                             vk::DispatchLoaderDynamic &loader)
        {
            try
            {
#ifdef _WIN32
                vk::Win32SurfaceCreateInfoKHR info;
                info.setHinstance(platform::ctx.instance).setHwnd(native_access::get_hwnd(_window));
                surface = instance.createWin32SurfaceKHR(info, nullptr, loader);
                return vk::Result::eSuccess;
#else
                int backend = native_access::get_backend_type();
                if (backend == WINDOW_BACKEND_X11)
                {
    #ifdef VK_USE_PLATFORM_XCB_KHR
                    if (platform::x11::ctx.xlib.xcb.is_extension_present && platform::x11::ctx.xlib.xcb.handle)
                        return create_xcb_surface(instance, surface, loader, _window);
    #endif
    #ifdef VK_USE_PLATFORM_XLIB_KHR
                    return create_xlib_surface(instance, surface, loader, _window);
    #else
                    return vk::Result::eErrorExtensionNotPresent;
    #endif
                }
                else if (backend == WINDOW_BACKEND_WAYLAND)
                {
    #ifdef VK_USE_PLATFORM_WAYLAND_KHR
                    vk::WaylandSurfaceCreateInfoKHR info;
                    info.setDisplay(platform::wayland::ctx.display)
                        .setSurface(native_access::get_wayland_surface(_window));
                    surface = instance.createWaylandSurfaceKHR(info, nullptr, loader);
                    return vk::Result::eSuccess;
    #endif
                }
                return vk::Result::eErrorExtensionNotPresent;
#endif
            }
            catch (const vk::SystemError &e)
            {
                return static_cast<vk::Result>(e.code().value());
            }
            catch (const std::exception &e)
            {
                return vk::Result::eErrorInitializationFailed;
            }
        }
    } // namespace vulkan
} // namespace awin