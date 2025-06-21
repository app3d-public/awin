#include <awin/vulkan.hpp>
#ifndef _WIN32
    #include "x11/platform.hpp"
    #include "x11/window.hpp"
#endif

namespace awin
{
    namespace vulkan
    {
        void CreateCtx::init_extensions(const acul::set<acul::string> &ext, acul::vector<const char *> &dst)
        {
            dst.push_back(vk::KHRSurfaceExtensionName);
#ifdef _WIN32
            dst.push_back(vk::KHRWin32SurfaceExtensionName);
#else
            int backend = platform::native_access::get_backend_type();
            if (backend == WINDOW_BACKEND_X11)
            {
                auto it = ext.find(vk::KHRXcbSurfaceExtensionName);
                if (it != ext.end())
                {
                    platform::x11::ctx.xlib.xcb.is_extension_present = true;
                    dst.push_back(vk::KHRXcbSurfaceExtensionName);
                }
                dst.push_back(vk::KHRXlibSurfaceExtensionName);
            }
            else if (backend == WINDOW_BACKEND_WAYLAND)
                dst.push_back(vk::KHRWaylandSurfaceExtensionName);
#endif
        }

        static vk::Result create_xcb_surface(vk::Instance &instance, vk::SurfaceKHR &surface,
                                             vk::DispatchLoaderDynamic &loader, Window &window)
        {
            xcb_connection_t *connection = platform::x11::ctx.xlib.xcb.XGetXCBConnection(platform::x11::ctx.display);
            if (!connection)
            {
                LOG_ERROR("X11: Failed to retrieve XCB connection");
                return vk::Result::eErrorExtensionNotPresent;
            }
            auto *x11_data = (platform::x11::X11WindowData *)platform::native_access::get_window_data(window);
            vk::XcbSurfaceCreateInfoKHR info;
            info.setConnection(connection).setWindow(x11_data->window);
            surface = instance.createXcbSurfaceKHR(info, nullptr, loader);
            return vk::Result::eSuccess;
        }

        static vk::Result create_xlib_surface(vk::Instance &instance, vk::SurfaceKHR &surface,
                                              vk::DispatchLoaderDynamic &loader, Window &window)
        {
            auto *x11_data = (platform::x11::X11WindowData *)platform::native_access::get_window_data(window);
            vk::XlibSurfaceCreateInfoKHR info;
            info.setDpy(platform::x11::ctx.display).setWindow(x11_data->window);
            surface = instance.createXlibSurfaceKHR(info, nullptr, loader);
            return vk::Result::eSuccess;
        }

        vk::Result CreateCtx::create_surface(vk::Instance &instance, vk::SurfaceKHR &surface,
                                             vk::DispatchLoaderDynamic &loader)
        {
            try
            {
#ifdef _WIN32
                vk::Win32SurfaceCreateInfoKHR info;
                info.setHinstance(platform::ctx.instance).setHwnd(platform::native_access::get_hwnd(_window));
                surface = instance.createWin32SurfaceKHR(info, nullptr, loader);
                return vk::Result::eSuccess;
#else
                int backend = platform::native_access::get_backend_type();
                if (backend == WINDOW_BACKEND_X11)
                {
                    if (platform::x11::ctx.xlib.xcb.is_extension_present && platform::x11::ctx.xlib.xcb.handle)
                        return create_xcb_surface(instance, surface, loader, _window);
                    return create_xlib_surface(instance, surface, loader, _window);
                }
                // else if (backend == WINDOW_BACKEND_WAYLAND) // todo
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