

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -w")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/Echo)

add_executable(EchoServer EchoServer.cpp)
target_link_libraries(EchoServer  yqmnet)


add_executable(EchoClient EchoClient.cpp)
target_link_libraries(EchoClient  yqmnet)