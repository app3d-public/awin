cmake_minimum_required(VERSION 3.17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

set(TEST_LIBRARIES ${AWIN_LIB_NAME} ${ACUL_LIB_NAME})
set(TEST_SOURCES "")

add_test_files(awin window window.cpp)
# add_test_files(awin popup popup.cpp)

if(ENABLE_COVERAGE)
    add_test_coverage(awin)
endif()