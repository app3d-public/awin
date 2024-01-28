cmake_minimum_required(VERSION 3.17)
project(app3d-window
    VERSION 0.0.1
    LANGUAGES CXX C
    DESCRIPTION "App3d Window Lib")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fexperimental-library")
add_definitions(-D_LIBCPP_NO_VCRUNTIME)
add_definitions(-DWINVER=0x0A00)

if(NOT TARGET app3d-core)
    add_subdirectory("src/deps/internal/core")
endif()

add_library(tinyfiledialogs SHARED "src/deps/external/tinyfiledialogs/tinyfiledialogs.c")

if(WIN32)
    target_link_libraries(tinyfiledialogs PRIVATE comdlg32 ole32)
endif()

list(APPEND SOURCE_FILES
    "src/window/popup.cpp"
    "src/window/vulkan.cpp"
    "src/window/window.cpp"
)

if(WIN32)
    list(APPEND SOURCE_FILES
        "src/window/window_win32.cpp"
        "src/window/vulkan_win32.cpp"
    )
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:app3d-core,INCLUDE_DIRECTORIES>)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/deps/external/tinyfiledialogs/")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/")

target_link_libraries(${PROJECT_NAME} PRIVATE app3d-core tinyfiledialogs)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS YES
)