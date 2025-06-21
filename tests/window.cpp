#include <acul/log.hpp>
#include <awin/window.hpp>

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
    awin::Window window("Test Window", 640, 480);

    window.resize_limit({500, 400});

    bool resize_called = false;

    ed.bind_event(&resize_called, awin::event_id::Resize, [&](awin::PosEvent &event) {
        if (event.window != &window || event.position.x <= 0 || event.position.y <= 0) return;
        resize_called = true;
        awin::push_empty_event();
        window.ready_to_close(true);
    });
    awin::update_events();

    while (!window.ready_to_close()) awin::poll_events();
    assert(resize_called && "Close event was not called");
    awin::destroy_library();
}