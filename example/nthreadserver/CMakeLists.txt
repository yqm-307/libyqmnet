

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -w")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/nthread)

add_executable(nthclient nthclient.cpp)
target_link_libraries(nthclient yqmnet)

add_executable(nthserver nthserver.cpp)
target_link_libraries(nthserver yqmnet)