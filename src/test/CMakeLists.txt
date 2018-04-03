include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(testEventLoop "testEventLoop.cpp")
target_link_libraries(testEventLoop uvcpp_static)

add_executable(testTimeval "testTimeval.cpp")
target_link_libraries(testTimeval uvcpp_static)

add_executable(testAny "testAny.cpp")
target_link_libraries(testAny uvcpp_static)