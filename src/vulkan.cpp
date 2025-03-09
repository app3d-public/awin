#include <cassert>
#include <vulkan/vulkan.hpp>
#include <window/vulkan.hpp>

namespace awin
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
                info.setHinstance(platform::ctx.instance).setHwnd(platform::native_access::getHWND(_window));
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
} // namespace awin