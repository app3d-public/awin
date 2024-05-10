cmake_minimum_required(VERSION 3.17)
project(app3d-window
    VERSION 0.0.1
    LANGUAGES CXX C
    DESCRIPTION "App3d Window Lib")

add_definitions(-DWINVER=0x0A00)

if(NOT TARGET app3d-core)
    add_subdirectory("src/deps/internal/core")
endif()

add_library(tinyfiledialogs SHARED "src/deps/external/tinyfiledialogs/tinyfiledialogs.c")
target_compile_options(tinyfiledialogs PRIVATE)

if(WIN32)
    target_link_libraries(tinyfiledialogs PRIVATE comdlg32 ole32)
endif()

add_files(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/src/window")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_include_directories(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:app3d-core,INCLUDE_DIRECTORIES>)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/deps/external/tinyfiledialogs/")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/")

target_link_libraries(${PROJECT_NAME} PRIVATE app3d-core tinyfiledialogs)

if(USE_ASAN)
    target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(tinyfiledialogs PUBLIC -fsanitize=address -fno-omit-frame-pointer)
endif(USE_ASAN)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS YES
)