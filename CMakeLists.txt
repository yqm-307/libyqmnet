cmake_minimum_required(VERSION 2.6) #最低版本号2.6
project(yqmnet C CXX)               #工程名和工程语言


#自定义变量 环境选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -w")




set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)   #可执行文件目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)      #库输出路径

include_directories(${PROJECT_BINARY_DIR})
set(INCLUDE_FILE
    include/Acceptor.h
    include/Buffer.h
    include/CallBack.h 
    include/Channel.h 
    include/Connector.h 
    include/Epoller.h
    include/EventLoop.h
    include/IPAddress.h
    include/Logger.h
    include/noncopyable.h
    include/MyLocker.h
    include/TcpClient.h
    include/TcpConnection.h
    include/TcpServer.h
    include/TcpServerSingle.h
    include/Thread.h
    include/ThreadPool.h
    include/Timer.h
    include/TimerQueue.h
    include/Timestamp.h
    include/TimeTask.h
)

set(SRC_FILE
    src/Acceptor.cpp 
    src/Buffer.cpp 
    src/Channel.cpp 
    src/Connector.cpp 
    src/Epoller.cpp 
    src/EventLoop.cpp 
    src/IPAddress.cpp 
    src/Logger.cpp 
    src/TcpClient.cpp 
    src/TcpConnection.cpp
    src/TcpServer.cpp 
    src/TcpServerSingle.cpp 
    src/Thread.cpp 
    include/ThreadPool.h
    src/Timer.cpp 
    src/TimerQueue.cpp 
)

add_library(yqmnet SHARED ${SRC_FILE} ${INCLUDE_FILE})  #源文件目录
target_link_libraries(yqmnet pthread)

install(TARGETS yqmnet DESTINATION lib)

install(FILES ${INCLUDE_FILE} DESTINATION include)


add_subdirectory(example)#分支目录


