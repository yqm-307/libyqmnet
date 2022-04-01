
add_compile_options(
    -g
    -std=c++14
)

add_executable(EchoClient EchoClient.cpp )
target_link_libraries(EchoClient  tinymuduo)

add_executable(EchoServer EchoServer.cpp)
target_link_libraries(EchoServer  tinymuduo)

add_subdirectory(other)


