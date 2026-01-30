#include <awin/native_access.hpp>
#include <awin/window.hpp>

void test_window()
{
    acul::events::dispatcher ed;
    awin::InitConfig config;
    config.events_dispatcher = &ed;

    awin::init_library(config);
#ifdef __unix__
    int backend_type = awin::native_access::get_backend_type();
    if (backend_type == WINDOW_BACKEND_WAYLAND) awin::native_access::enable_wayland_surface_placeholder();
#endif
    awin::Window window("Test Window", 640, 480);
    window.resize_limit({500, 400});

    bool resize_called = false;

    ed.bind_event(&resize_called, awin::event_id::resize, [&](awin::PosEvent &event) {
        if (event.window != &window || event.position.x <= 0 || event.position.y <= 0) return;
        resize_called = true;
        window.ready_to_close(true);
    });
    awin::update_events();

    while (!window.ready_to_close()) awin::poll_events();
    window.destroy();
    assert(resize_called && "Close event was not called");
    awin::destroy_library();
}