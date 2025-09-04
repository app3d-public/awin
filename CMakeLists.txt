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

if(ENABLE_AGRB)
    target_link_libraries(${PROJECT_NAME} PUBLIC agrb)
endif()

if(USE_ASAN)
    target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address -fno-omit-frame-pointer)
endif(USE_ASAN)

if(WIN32)
    gen_manifest_lib(lib${PROJECT_NAME} ${PROJECT_VERSION})
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