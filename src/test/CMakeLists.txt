include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(testEventLoop "testEventLoop.cpp")
target_link_libraries(testEventLoop uvcpp_static)