file(GLOB SRVSRC "${CMAKE_SOURCE_DIR}/test/http_test_server.cpp")
add_library(main_server STATIC ${SRVSRC})

add_executable(http_test_server main.cpp)
target_link_libraries(http_test_server main_server)


add_custom_command(TARGET http_test_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:http_test_server> ${PROJECT_BINARY_DIR}/http_test_server
    COMMENT "Created ${PROJECT_BINARY_DIR}/http_test_server"
)