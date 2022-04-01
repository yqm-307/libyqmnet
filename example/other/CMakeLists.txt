
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/othertest)

add_executable(testThread threadtest.cpp )
target_link_libraries(testThread  tinymuduo)

add_executable(ThreadPool threadpool.cpp)
target_link_libraries(ThreadPool tinymuduo)
