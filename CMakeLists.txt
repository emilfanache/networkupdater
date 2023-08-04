cmake_minimum_required(VERSION 3.20)

project(network_updater_cplpusplus)
set (CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 871ed52d350214a034f6ef8a3b8f51c5ce1bd400) # The commit hash for 1.9.0. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

#get googletest
include(FetchContent)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.13.0)
#googletest_POPULATED
#googletest_SOURCE_DIR
#googletest_BUILD_DIR
if (NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BUILD_DIR})
endif()

FetchContent_GetProperties(googletest)

file(GLOB TEST_SOURCES "${CMAKE_SOURCE_DIR}/test/network_updater_test.cpp"
                       "${CMAKE_SOURCE_DIR}/test/http_test_server.cpp")

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/network_updater.cpp")
add_library(main_lib STATIC ${SOURCES})
target_link_libraries(main_lib PRIVATE cpr::cpr)

add_executable(network_updater main.cpp)
target_link_libraries(network_updater main_lib)

add_executable(test_runner ${TEST_SOURCES})
target_link_libraries(test_runner main_lib gtest_main gmock_main)

add_subdirectory(test)
