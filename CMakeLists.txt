cmake_minimum_required(VERSION 3.20)

project(network_updater_cplpusplus)
set (CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 871ed52d350214a034f6ef8a3b8f51c5ce1bd400) # The commit hash for 1.9.0. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)


file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/network_updater.cpp")
add_library(main_lib STATIC ${SOURCES})
target_link_libraries(main_lib PRIVATE cpr::cpr)

add_executable(network_updater main.cpp)
target_link_libraries(network_updater main_lib)


add_subdirectory(test)
