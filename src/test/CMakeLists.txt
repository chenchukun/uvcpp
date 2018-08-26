include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(testEventLoop "testEventLoop.cpp")
target_link_libraries(testEventLoop uvcpp_static uv)

add_executable(testTimeval "testTimeval.cpp")
target_link_libraries(testTimeval uvcpp_static uv)

add_executable(testAny "testAny.cpp")
target_link_libraries(testAny uvcpp_static uv)

add_executable(testEventLoopThread "testEventLoopThread.cpp")
target_link_libraries(testEventLoopThread uvcpp_static uv)

add_executable(testTcpServer "testTcpServer.cpp")
target_link_libraries(testTcpServer uvcpp_static uv)

add_executable(testThreadLocal "testThreadLocal.cpp")
target_link_libraries(testThreadLocal uvcpp_static uv)

add_executable(testTcpClient "testTcpClient.cpp")
target_link_libraries(testTcpClient uvcpp_static uv)

add_executable(testBuffer "testBuffer.cpp" "person.pb.cc")
target_link_libraries(testBuffer uvcpp_static uv protobuf)

add_executable(testCodec "testCodec.cpp" "person.pb.cc")
target_link_libraries(testCodec uvcpp_static uv protobuf)

add_executable(testQueue "testQueue.cpp")
target_link_libraries(testQueue uvcpp_static uv)

add_executable(testSingleton "testSingleton.cpp")
target_link_libraries(testSingleton uvcpp_static uv)

add_executable(testThreadPool "testThreadPool.cpp")
target_link_libraries(testThreadPool uvcpp_static uv)