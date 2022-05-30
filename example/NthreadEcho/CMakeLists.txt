
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -w")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/NthEcho)


add_executable(nthEchoServer nthEchoServer.cpp)
target_link_libraries(nthEchoServer  yqmnet)

add_executable(nthEchoClient nthEchoClient.cpp)
target_link_libraries(nthEchoClient  yqmnet)