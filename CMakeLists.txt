cmake_minimum_required(VERSION 3.17)
project(awin
    VERSION 0.0.1
    LANGUAGES CXX C
    DESCRIPTION "App3d Window Library")

add_files(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/src")
if(ACUL_BUILD_MIN)
    list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan.cpp")
endif()

set(AWIN_LIB_NAME ${PROJECT_NAME}-${APP_SIMD_ARCH})
add_library(${AWIN_LIB_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${AWIN_LIB_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(${AWIN_LIB_NAME} PRIVATE -DAPPLIB_BUILD)
target_include_directories(${AWIN_LIB_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_link_libraries(${AWIN_LIB_NAME} PRIVATE ${ACUL_LIB_NAME})

if(USE_ASAN)
    target_link_options(${AWIN_LIB_NAME} PUBLIC -fsanitize=address -fno-omit-frame-pointer)
endif(USE_ASAN)

if(WIN32)
    gen_manifest_lib(${AWIN_LIB_NAME} ${PROJECT_VERSION})
endif()

set_target_properties(${AWIN_LIB_NAME}
    PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS YES
)

set(AWIN_LIB_NAME ${AWIN_LIB_NAME} PARENT_SCOPE)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()