#include <cassert>
#include <vulkan/vulkan.hpp>
#include <window/vulkan.hpp>
#ifdef _WIN32
    #include <window/platform_win32.hpp>
#else
    #error "Unsupported platform"
#endif

namespace window
{
    namespace vulkan
    {
        vk::Result CreateCtx::createSurface(vk::Instance &instance, vk::SurfaceKHR &surface,
                                            vk::DispatchLoaderDynamic &loader)
        {
            try
            {
#ifdef _WIN32
                vk::Win32SurfaceCreateInfoKHR info;
                auto accessBridge = _window.accessBridge();
                info.setHinstance(accessBridge.global()).setHwnd(accessBridge.hwnd());
                surface = instance.createWin32SurfaceKHR(info, nullptr, loader);
                return vk::Result::eSuccess;
#else
    #error "Unsupported platform"
#endif
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