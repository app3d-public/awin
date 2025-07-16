cmake_minimum_required(VERSION 3.17)
project(awin
    VERSION 0.0.1
    LANGUAGES CXX C
    DESCRIPTION "App3d Window Library")

if(PROJECT_IS_TOP_LEVEL)
    include(cmake/utils.cmake)
    add_subdirectory(modules/acul)
endif()

add_files(${PROJECT_NAME} RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/src")

if(ACUL_BUILD_MIN)
    list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan.cpp")
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DAPPLIB_BUILD)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_link_libraries(${PROJECT_NAME} PRIVATE acul)

if(USE_ASAN)
    target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address -fno-omit-frame-pointer)
endif(USE_ASAN)

if(WIN32)
    gen_manifest_lib(${PROJECT_NAME} ${PROJECT_VERSION})
elseif(UNIX AND NOT APPLE) # Generate by wayland-scanner
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/wayland/generators")
endif()

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS YES
)

if(BUILD_TESTS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE AWIN_TEST_BUILD)
    enable_testing()
    add_subdirectory(tests)
endif()