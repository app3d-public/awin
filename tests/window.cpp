#include <acul/log.hpp>
#include <awin/window.hpp>
#include "awin/types.hpp"

void test_window()
{
    acul::events::dispatcher ed;
    acul::task::service_dispatch sd;
    auto log_service = acul::alloc<acul::log::log_service>();
    sd.register_service(log_service);
    log_service->level = acul::log::level::Trace;
    auto *app_logger = log_service->add_logger<acul::log::console_logger>("app");
    log_service->level = acul::log::level::Trace;
    app_logger->set_pattern("%(message)\n");
    log_service->default_logger = app_logger;

    awin::init_library(&ed);
    awin::Window window("Test Window", 800, 600, awin::WindowFlagBits::Decorated);
    const int width = 32;
    const int height = 32;
    acul::vector<u32> pixels(width * height);
    u32 red = 0xFF0000FF;
    std::fill(pixels.begin(), pixels.end(), red);
    awin::Image im;
    im.dimenstions = {width, height};
    im.pixels = pixels.data();
    awin::set_window_icon(window, {im});

    window.title("New Title");
    LOG_INFO("Title: %s", window.title().c_str());

    // window.resize_limit(500, 400);

    bool resize_called = false;

    int dummy = 0;
    ed.bind_event(&dummy, awin::event_id::KeyInput, [&window](const awin::KeyInputEvent &e) {
        LOG_INFO("Key: %d, mods: %d, action: %d", e.key, +e.mods, e.action);
        if (e.key == awin::io::Key::A)
        {
            auto pos = window.position();
            LOG_INFO("position: %d %d", pos.x, pos.y);
        }
        if (e.key == awin::io::Key::S)
        {
            window.position({300, 400});
        }
    });

    ed.bind_event(&resize_called, awin::event_id::Resize, [&](awin::PosEvent &event) {
        // if (event.window != &window || event.position.x <= 0 || event.position.y <= 0) return;
        LOG_INFO("Resize: %d %d window: %p %p", event.position.x, event.position.y, event.window, &window);
        // resize_called = true;
        // awin::push_empty_event();
        // window.ready_to_close(true);
    });
    awin::update_events();

    while (!window.ready_to_close()) awin::poll_events();
    // assert(resize_called && "Close event was not called");
    window.destroy();
    awin::destroy_library();
}