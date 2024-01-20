#include <window/types.hpp>

namespace window
{
    namespace _internal
    {
        WindowEvents gWindowEvents(nullptr);
    }
    
    void WindowBase::inputKey(io::Key key, io::KeyPressState action, io::KeyMode mods)
    {
        if (+key >= 0 && key <= io::Key::kLast)
        {
            bool repeated{false};
            if (action == io::KeyPressState::release && _keys[+key] == io::KeyPressState::release)
                return;
            if (action == io::KeyPressState::press && _keys[+key] == io::KeyPressState::press)
                repeated = true;
            _keys[+key] = action;
            if (repeated)
                action = io::KeyPressState::repeat;
        }
        _internal::emitWindowEvent(_internal::gWindowEvents.keyInputEvents, "window:keyInput", this, key, action, mods);
    }
} // namespace window