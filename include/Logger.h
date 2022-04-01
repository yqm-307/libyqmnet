#ifndef LOGGER_H
#define LOGGER_H

#include <queue>
#include <thread>
#include <atomic>
#include <memory>
#include <condition_variable>   //mutex
#include "noncopyable.h"
#include <functional>

namespace  net
{
#ifndef LOG_LEVEL
#define LOG_LEVEL -1
#endif

enum LOGLEVEL
{
    LOG_TRACE=0,        //跟踪点
    LOG_DEBUG,          //调试
    LOG_INFO,           //消息
    LOG_WARN,           //警告
    LOG_ERROR,          //错误
    LOG_FATAL,          //致命错误
};

static const char* LeveL[6]{
    " [TRACE] ",
    " [DEBUG] ",
    " [INFO] ",
    " [WARN] ",
    " [ERROR] ",
    " [FATAL] ",
};

//缓冲日志
class Logger : noncopyable
{
public:
    static Logger* GetInstance(std::string name = "./log");
    void Log(LOGLEVEL level,const std::string& log);
    static void SetFileName(std::string name);
private:
    Logger(std::string);
    ~Logger();
    bool Dequeue(std::string& str);
    void Enqueue(std::string log);
private:
    std::queue<std::string> _queue;
    std::thread* _writeThread;       //不断dequeue
    std::mutex _mutex;
    std::string filename;           //文件名可配置
    std::function<void ()>  work;
    int _openfd;                    //文件
};

std::string format(const char* fmt, ...);



#define TRACE(fmt, ...)     net::Logger::GetInstance()->Log(net::LOG_TRACE, net::format(fmt,##__VA_ARGS__))
#define DEBUG(fmt, ...)     net::Logger::GetInstance()->Log(net::LOG_DEBUG, net::format(fmt,##__VA_ARGS__))
#define INFO(fmt, ...)      net::Logger::GetInstance()->Log(net::LOG_INFO,  net::format(fmt,##__VA_ARGS__))
#define WARN(fmt, ...)      net::Logger::GetInstance()->Log(net::LOG_WARN,  net::format(fmt,##__VA_ARGS__))
#define ERROR(fmt, ...)     net::Logger::GetInstance()->Log(net::LOG_ERROR, net::format(fmt,##__VA_ARGS__))
#define FATAL(fmt, ...)     net::Logger::GetInstance()->Log(net::LOG_FATAL, net::format(fmt,##__VA_ARGS__))


}


#endif
