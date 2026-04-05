cmake_minimum_required(VERSION 3.17)

if(NOT DEFINED ACBT_PROJECT_LOADED)
    include(cmake/project.cmake)
endif()

project(awin
    VERSION 0.0.1
    LANGUAGES CXX C
    DESCRIPTION "App3d Window Library")

if(NOT DEFINED ACBT_LOADED)
    include(cmake/utils.cmake)
endif()

if(NOT DEFINED AWIN_WIN32_APP_SDK)
    option(AWIN_WIN32_APP_SDK "Enable Windows App SDK support in awin on Win32" OFF)
endif()

if(NOT DEFINED AWIN_WIN32_APP_SDK_BOOTSTRAP)
    option(AWIN_WIN32_APP_SDK_BOOTSTRAP "Enable Windows App SDK bootstrap support for unpackaged Win32 builds" OFF)
endif()

set(AWIN_WIN32_APP_SDK_FOUNDATION_VERSION "1.8.260222000" CACHE STRING
    "Version of Microsoft.WindowsAppSDK.Foundation to install from NuGet")
set(AWIN_WIN32_APP_SDK_INTERACTIVE_VERSION "1.8.260125001" CACHE STRING
    "Version of Microsoft.WindowsAppSDK.InteractiveExperiences to install from NuGet")

string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\." awin_windows_app_sdk_version_match
    "${AWIN_WIN32_APP_SDK_FOUNDATION_VERSION}")
if(NOT awin_windows_app_sdk_version_match)
    message(FATAL_ERROR
        "Failed to parse Windows App SDK version '${AWIN_WIN32_APP_SDK_FOUNDATION_VERSION}'")
endif()
set(awin_windows_app_sdk_version_major "${CMAKE_MATCH_1}")
set(awin_windows_app_sdk_version_minor "${CMAKE_MATCH_2}")

function(awin_windows_app_sdk_init)
    if(NOT WIN32)
        return()
    endif()

    get_property(is_initialized GLOBAL PROPERTY AWIN_WINDOWS_APP_SDK_INITIALIZED)
    if(is_initialized)
        return()
    endif()

    set(awin_windows_app_sdk_root_dir "${CMAKE_BINARY_DIR}/windows-app-sdk")
    set(awin_windows_app_sdk_packages_dir "${awin_windows_app_sdk_root_dir}/packages")
    set(awin_windows_app_sdk_generated_dir "${awin_windows_app_sdk_root_dir}/include-gen")

    acbt_nuget_install_package(
        PACKAGE_NAME "Microsoft.WindowsAppSDK.Foundation"
        PACKAGE_VERSION "${AWIN_WIN32_APP_SDK_FOUNDATION_VERSION}"
        OUTPUT_DIRECTORY "${awin_windows_app_sdk_packages_dir}")
    acbt_nuget_install_package(
        PACKAGE_NAME "Microsoft.WindowsAppSDK.InteractiveExperiences"
        PACKAGE_VERSION "${AWIN_WIN32_APP_SDK_INTERACTIVE_VERSION}"
        OUTPUT_DIRECTORY "${awin_windows_app_sdk_packages_dir}")

    set(awin_windows_app_sdk_foundation_dir
        "${awin_windows_app_sdk_packages_dir}/Microsoft.WindowsAppSDK.Foundation.${AWIN_WIN32_APP_SDK_FOUNDATION_VERSION}")
    set(awin_windows_app_sdk_interactive_dir
        "${awin_windows_app_sdk_packages_dir}/Microsoft.WindowsAppSDK.InteractiveExperiences.${AWIN_WIN32_APP_SDK_INTERACTIVE_VERSION}")

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(ARM64|arm64|aarch64)$")
            set(awin_windows_app_sdk_arch "arm64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(ARM64EC|arm64ec)$")
            set(awin_windows_app_sdk_arch "arm64ec")
        else()
            set(awin_windows_app_sdk_arch "x64")
        endif()
    else()
        set(awin_windows_app_sdk_arch "x86")
    endif()

    file(GLOB awin_windows_app_sdk_metadata_candidates
        LIST_DIRECTORIES true
        "${awin_windows_app_sdk_interactive_dir}/metadata/*")
    list(FILTER awin_windows_app_sdk_metadata_candidates INCLUDE REGEX ".*/[0-9][0-9.]*$")
    list(SORT awin_windows_app_sdk_metadata_candidates COMPARE NATURAL ORDER DESCENDING)
    list(LENGTH awin_windows_app_sdk_metadata_candidates awin_windows_app_sdk_metadata_count)
    if(awin_windows_app_sdk_metadata_count EQUAL 0)
        message(FATAL_ERROR
            "No metadata version directories were found under "
            "${awin_windows_app_sdk_interactive_dir}/metadata")
    endif()
    list(GET awin_windows_app_sdk_metadata_candidates 0 awin_windows_app_sdk_metadata_base)

    set(awin_windows_app_sdk_foundation_winmd
        "${awin_windows_app_sdk_foundation_dir}/metadata/Microsoft.Windows.Foundation.winmd")
    set(awin_windows_app_sdk_graphics_winmd
        "${awin_windows_app_sdk_metadata_base}/Microsoft.Graphics.winmd")
    set(awin_windows_app_sdk_ui_winmd
        "${awin_windows_app_sdk_metadata_base}/Microsoft.UI.winmd")
    set(awin_windows_app_sdk_bootstrap_dll
        "${awin_windows_app_sdk_foundation_dir}/runtimes/win-${awin_windows_app_sdk_arch}/native/Microsoft.WindowsAppRuntime.Bootstrap.dll")
    acbt_get_windows_dll_file_version(
        "${awin_windows_app_sdk_bootstrap_dll}"
        awin_windows_app_sdk_bootstrap_version)

    foreach(required_path IN ITEMS
        "${awin_windows_app_sdk_foundation_dir}/include/MddBootstrap.h"
        "${awin_windows_app_sdk_interactive_dir}/include/Microsoft.UI.Interop.h"
        "${awin_windows_app_sdk_foundation_dir}/lib/native/${awin_windows_app_sdk_arch}/Microsoft.WindowsAppRuntime.lib"
        "${awin_windows_app_sdk_foundation_dir}/lib/native/${awin_windows_app_sdk_arch}/Microsoft.WindowsAppRuntime.Bootstrap.lib"
        "${awin_windows_app_sdk_foundation_winmd}"
        "${awin_windows_app_sdk_graphics_winmd}"
        "${awin_windows_app_sdk_ui_winmd}")
        if(NOT EXISTS "${required_path}")
            message(FATAL_ERROR "Required Windows App SDK file was not found: ${required_path}")
        endif()
    endforeach()

    if(NOT TARGET awin_windows_app_sdk_headers)
        acbt_add_cppwinrt_target(
            TARGET_NAME awin_windows_app_sdk_headers
            OUTPUT_DIR "${awin_windows_app_sdk_generated_dir}"
            INPUT_WINMD "${awin_windows_app_sdk_ui_winmd}"
            REF_WINMDS
                "${awin_windows_app_sdk_graphics_winmd}"
                "${awin_windows_app_sdk_foundation_winmd}")
    endif()

    set_property(GLOBAL PROPERTY AWIN_WINDOWS_APP_SDK_INITIALIZED TRUE)
    set_property(GLOBAL PROPERTY AWIN_WINDOWS_APP_SDK_FOUNDATION_DIR "${awin_windows_app_sdk_foundation_dir}")
    set_property(GLOBAL PROPERTY AWIN_WINDOWS_APP_SDK_INTERACTIVE_DIR "${awin_windows_app_sdk_interactive_dir}")
    set_property(GLOBAL PROPERTY AWIN_WINDOWS_APP_SDK_GENERATED_DIR "${awin_windows_app_sdk_generated_dir}")
    set_property(GLOBAL PROPERTY AWIN_WINDOWS_APP_SDK_ARCH "${awin_windows_app_sdk_arch}")
    set_property(GLOBAL PROPERTY AWIN_WINDOWS_APP_SDK_BOOTSTRAP_DLL "${awin_windows_app_sdk_bootstrap_dll}")
    set_property(GLOBAL PROPERTY AWIN_WINDOWS_APP_SDK_BOOTSTRAP_VERSION "${awin_windows_app_sdk_bootstrap_version}")
endfunction()

function(awin_windows_app_sdk_get_property property_name out_var)
    get_property(property_value GLOBAL PROPERTY "${property_name}")
    if(NOT property_value)
        message(FATAL_ERROR "AWIN Windows App SDK property '${property_name}' is not initialized")
    endif()
    set(${out_var} "${property_value}" PARENT_SCOPE)
endfunction()

function(awin_target_enable_windows_app_sdk target_name)
    set(options BOOTSTRAP)
    cmake_parse_arguments(ARG "${options}" "" "" ${ARGN})

    if(NOT WIN32)
        return()
    endif()

    if(NOT TARGET "${target_name}")
        message(FATAL_ERROR "awin_target_enable_windows_app_sdk(): target '${target_name}' does not exist")
    endif()

    awin_windows_app_sdk_init()

    awin_windows_app_sdk_get_property(AWIN_WINDOWS_APP_SDK_FOUNDATION_DIR awin_windows_app_sdk_foundation_dir)
    awin_windows_app_sdk_get_property(AWIN_WINDOWS_APP_SDK_INTERACTIVE_DIR awin_windows_app_sdk_interactive_dir)
    awin_windows_app_sdk_get_property(AWIN_WINDOWS_APP_SDK_GENERATED_DIR awin_windows_app_sdk_generated_dir)
    awin_windows_app_sdk_get_property(AWIN_WINDOWS_APP_SDK_ARCH awin_windows_app_sdk_arch)

    add_dependencies(${target_name} awin_windows_app_sdk_headers)

    target_include_directories(${target_name} PRIVATE
        "${awin_windows_app_sdk_foundation_dir}/include"
        "${awin_windows_app_sdk_interactive_dir}/include"
        "${awin_windows_app_sdk_generated_dir}")

    target_link_directories(${target_name} PRIVATE
        "${awin_windows_app_sdk_foundation_dir}/lib/native/${awin_windows_app_sdk_arch}")

    target_link_libraries(${target_name} PRIVATE
        dwmapi
        gdi32
        ole32
        oleaut32
        runtimeobject
        user32
        Microsoft.WindowsAppRuntime)

    target_compile_definitions(${target_name} PUBLIC
        AWIN_WIN32_APP_SDK_ENABLED
        AWIN_WIN32_APP_SDK_VERSION_MAJOR=${awin_windows_app_sdk_version_major}
        AWIN_WIN32_APP_SDK_VERSION_MINOR=${awin_windows_app_sdk_version_minor})

    if(ARG_BOOTSTRAP)
        target_link_libraries(${target_name} PRIVATE Microsoft.WindowsAppRuntime.Bootstrap)
        target_compile_definitions(${target_name} PUBLIC AWIN_WIN32_APP_SDK_BOOTSTRAP_ENABLED)
    endif()
endfunction()

function(awin_target_copy_windows_app_sdk_bootstrap target_name)
    if(NOT WIN32)
        return()
    endif()

    if(NOT TARGET "${target_name}")
        message(FATAL_ERROR "awin_target_copy_windows_app_sdk_bootstrap(): target '${target_name}' does not exist")
    endif()

    awin_windows_app_sdk_init()
    awin_windows_app_sdk_get_property(AWIN_WINDOWS_APP_SDK_BOOTSTRAP_DLL awin_windows_app_sdk_bootstrap_dll)

    acbt_get_target_output_dir("${target_name}" target_output_dir)
    set(bootstrap_output_path "${target_output_dir}/Microsoft.WindowsAppRuntime.Bootstrap.dll")

    normalize_variable_name("${target_name}_windows_app_sdk_bootstrap" bootstrap_copy_target_name)

    add_custom_command(
        OUTPUT "${bootstrap_output_path}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${target_output_dir}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${awin_windows_app_sdk_bootstrap_dll}"
            "${bootstrap_output_path}"
        DEPENDS
            "${awin_windows_app_sdk_bootstrap_dll}"
        COMMENT "Copying Windows App Runtime bootstrap DLL for ${target_name}"
        VERBATIM
    )

    add_custom_target(${bootstrap_copy_target_name} ALL
        DEPENDS "${bootstrap_output_path}")
    add_dependencies(${target_name} ${bootstrap_copy_target_name})
endfunction()

option(ENABLE_AGRB "Enable AGRB Integration" ON)

if(NOT TARGET acul)
    add_subdirectory(modules/acul)
endif()

if (ENABLE_AGRB AND NOT TARGET agrb)
    add_subdirectory(modules/agrb)
endif()

add_files(${PROJECT_NAME} RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/src")

if(NOT ENABLE_AGRB)
    list(REMOVE_ITEM AWIN_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/integration/agrb.cpp")
endif()

add_library(${PROJECT_NAME} SHARED ${AWIN_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DAPPLIB_BUILD)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_link_libraries(${PROJECT_NAME} PRIVATE acul)

set(AWIN_MANIFEST_DEPS lib${PROJECT_NAME}:${PROJECT_VERSION})

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DWIN32_LEAN_AND_MEAN -DNOMINMAX -DWINVER=0x0A00)
endif()

if(WIN32 AND AWIN_WIN32_APP_SDK)
    if(AWIN_WIN32_APP_SDK_BOOTSTRAP)
        awin_target_enable_windows_app_sdk(awin BOOTSTRAP)
        awin_target_copy_windows_app_sdk_bootstrap(awin)
    else()
        awin_target_enable_windows_app_sdk(awin)
    endif()
endif()

if(ENABLE_AGRB)
    target_link_libraries(${PROJECT_NAME} PUBLIC agrb)
endif()

if(WIN32)
    if(AWIN_WIN32_APP_SDK AND AWIN_WIN32_APP_SDK_BOOTSTRAP)
        awin_windows_app_sdk_init()
        awin_windows_app_sdk_get_property(AWIN_WINDOWS_APP_SDK_BOOTSTRAP_VERSION awin_windows_app_sdk_bootstrap_version)
        gen_manifest_lib(Microsoft.WindowsAppRuntime.Bootstrap ${awin_windows_app_sdk_bootstrap_version})
        gen_manifest_lib(lib${PROJECT_NAME} ${PROJECT_VERSION}
            Microsoft.WindowsAppRuntime.Bootstrap:${awin_windows_app_sdk_bootstrap_version})
    else()
        gen_manifest_lib(lib${PROJECT_NAME} ${PROJECT_VERSION})
    endif()
elseif(UNIX AND NOT APPLE) # Generate by wayland-scanner
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/wayland/generators")
endif()

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    CXX_EXTENSIONS YES
)

if(BUILD_TESTS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE AWIN_TEST_BUILD)
    enable_testing()
    add_subdirectory(tests)
endif()

set(AWIN_MANIFEST_DEPS "${AWIN_MANIFEST_DEPS}" PARENT_SCOPE)
