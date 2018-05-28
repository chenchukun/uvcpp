include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(testEventLoop "testEventLoop.cpp")
target_link_libraries(testEventLoop uvcpp_static)

add_executable(testTimeval "testTimeval.cpp")
target_link_libraries(testTimeval uvcpp_static)

add_executable(testAny "testAny.cpp")
target_link_libraries(testAny uvcpp_static)

add_executable(testEventLoopThread "testEventLoopThread.cpp")
target_link_libraries(testEventLoopThread uvcpp_static)

add_executable(testTcpServer "testTcpServer.cpp")
target_link_libraries(testTcpServer uvcpp_static)

add_executable(testThreadLocal "testThreadLocal.cpp")
target_link_libraries(testThreadLocal uvcpp_static)

add_executable(testTcpClient "testTcpClient.cpp")
target_link_libraries(testTcpClient uvcpp_static)

add_executable(testBuffer "testBuffer.cpp" "person.pb.cc")
target_link_libraries(testBuffer uvcpp_static)