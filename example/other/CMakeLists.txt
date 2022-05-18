

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -w")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/othertest)

add_executable(testThread threadtest.cpp )
target_link_libraries(testThread  yqmnet)

add_executable(ThreadPool threadpool.cpp)
target_link_libraries(ThreadPool yqmnet)

add_executable(timerqueue timerqueue.cpp)
target_link_libraries(timerqueue yqmnet)