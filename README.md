# App3d Window Library

**Awin** is a cross-platform windowing abstraction layer, developed as part of the **App3D** project.  
It provides a unified C++ API for creating and managing windows on Linux and Microsoft Windows.  
Awin is designed exclusively for Vulkan — no other graphics APIs are supported.

## Capabilities

- Cross-platform window creation and management.
- Native popup dialogs.
> [!NOTE]
> On Linux, popup dialogs require `zenity` or `kdialog` as message boxes backend.

## Integrations

- **AGRB** — provides a way to link `awin` windows with the rendering backend, enabling the creation of platform-specific presentation surfaces and the setup of required instance extensions for Vulkan.

## Supported Backends

- Win32 API
- X11
- Wayland

## Building

### Bundled submodules
The following dependencies are included as git submodules and must be checked out when cloning:

- [acbt](https://github.com/app3d-public/acbt)
- [acul](https://github.com/app3d-public/acul)
- [agrb](https://github.com/app3d-public/agrb) - Optional

### Supported OS:
- Linux
- Microsoft Windows

### Cmake options:
- `ENABLE_AGRB`: Enable `agrb` integration
- `USE_ASAN`: Enable address sanitizer
- `BUILD_TESTS`: Enable testing
- `ENABLE_COVERAGE`: Enable code coverage

## License
This project is licensed under the [MIT License](LICENSE).

## Contacts
For any questions or feedback, you can reach out via [email](mailto:wusikijeronii@gmail.com) or open a new issue.