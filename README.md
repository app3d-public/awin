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
- `BUILD_TESTS`: Enable testing
- `ENABLE_COVERAGE`: Enable code coverage
- `AWIN_WIN32_APP_SDK`: Enable Windows App SDK support in the Win32 backend
- `AWIN_WIN32_APP_SDK_BOOTSTRAP`: Enable bootstrap support for unpackaged Win32 applications

### Win32 and Windows App SDK

On Windows, `awin` is built around the Win32 backend by default.  
It can also create and manage windows through the Windows App SDK integration path when enabled at build time.

- `AWIN_WIN32_APP_SDK=ON` enables App SDK support in `awin`
- `AWIN_WIN32_APP_SDK_BOOTSTRAP=ON` adds support for unpackaged bootstrap initialization

At runtime, this is selected through `InitConfig::platform_flags`:

- `AWIN_PLATFORM_WIN32_APP_SDK` enables the App SDK window path
- `AWIN_PLATFORM_WIN32_APP_SDK_BOOTSTRAP` enables unpackaged bootstrap

Notes:

- For unpackaged applications, bootstrap support must be enabled both:
  - at build time with `AWIN_WIN32_APP_SDK_BOOTSTRAP=ON`
  - at runtime with `AWIN_PLATFORM_WIN32_APP_SDK_BOOTSTRAP`
- For packaged applications, bootstrap is not required
- `AWIN_PLATFORM_WIN32_APP_SDK_BOOTSTRAP` requires `AWIN_PLATFORM_WIN32_APP_SDK`

## License
This project is licensed under the [MIT License](LICENSE).

## Contacts
For any questions or feedback, you can reach out via [email](mailto:wusikijeronii@gmail.com) or open a new issue.
