

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -w")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/http)

add_executable(httpserver httpserver.cpp)
target_link_libraries(httpserver yqmnet)