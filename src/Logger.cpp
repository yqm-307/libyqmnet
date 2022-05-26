#include "../include/Logger.h"
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

using namespace net;

//#define YNET_LOG_BUFFER




Logger* Logger::GetInstance(std::string name)
{
    static Logger* _instance = new Logger(name);
    return _instance;
}

#ifdef YNET_LOG_BUFFER
Logger::Logger(std::string name)
    :_pendingwriteindex(0),
    _nowindex(0),
    _nowsize(ARRAY_NUM)
{
    for(int i=0;i<ARRAY_NUM;++i)
    {
        _buffers.push_back(std::pair<int,char*>((i+1)%ARRAY_NUM,new char[ARRAY_SIZE]));
    }
    

    filename = name;
    _openfd = open(filename.c_str(),O_RDWR|O_CREAT|O_APPEND,S_IRWXU);  //读写打开文件
    work = [this](){
        while(1)
        {
            std::unique_lock<std::mutex> loc(_condlock);
            while(!hasfulled()) //
                _cond.wait(loc);
            const char* log = GetFullArray();
            //确保所有数据可以写入
            int fullnum = ARRAY_SIZE;
            int re=0;
            while(fullnum!=0)
            {
                re = write(_openfd,log,ARRAY_SIZE);
                assert(re>=0); 
                fullnum-=re;     
            }

        }
    };
    _writeThread = new std::thread(work);
}
#else
Logger::Logger(std::string name)
{
    filename = name;
    _openfd = open(filename.c_str(),O_RDWR|O_CREAT|O_APPEND,S_IRWXU);  //读写打开文件
    work = [this](){
        while(1)
        {
            static std::string line="";
            if(this->_queue.empty())
                continue;
            this->Dequeue(line);                //取
            write(this->_openfd,line.c_str(),line.size());  //写
        }
    };
    _writeThread = new std::thread(work);
}
#endif



Logger::~Logger()
{
#ifdef YNET_LOG_BUFFER
    next(); //写入所有数据
#endif
    close(_openfd); //关闭文件描述符
}





#ifdef YNET_LOG_BUFFER
//写入缓冲
void Logger::Enqueue(std::string log)
{
    std::lock_guard<std::mutex> lock(_mutex);

    int log_remain = log.size();    //日志剩余
    int wd = 0;                     //已写
    const char* logc = log.c_str(); 

    while(log_remain != 0)
    {
        int worklen = strlen(workarray());          
        int gap = ARRAY_SIZE-strlen(workarray());   //当前数组可写入
        if(log_remain > gap)
        {
            log_remain-=gap;
            strncpy(workarray()+worklen,logc+wd,gap);
            wd+=gap;
            next();
        }
        else
        {
            strncpy(workarray()+worklen,logc+wd,log_remain);
            log_remain=0;
        }
    }
}

const char* Logger::GetFullArray()
{
    const char* ret = _buffers[_pendingwriteindex].second;
    _pendingwriteindex = _buffers[_pendingwriteindex].first;
    return ret;
}


void Logger::next()
{
    //是否需要扩张
    if((_nowindex+1)%_nowsize == _pendingwriteindex)
    {//扩张
        int nextnext = _buffers[_nowindex].first;
        _buffers.push_back(std::pair<int,char*>(nextnext,new char[ARRAY_SIZE]));
        _buffers[_nowindex].first = _nowsize;
        _nowsize++;
        _nowindex = _buffers[_nowindex].first;
    }
    else//正常移动
    {
        _nowindex = _buffers[_nowindex].first;  //下一个节点
        memset(workarray(),'\0',ARRAY_SIZE);
    }
    _cond.notify_all();
}

void Logger::nextPending()
{
}
#else

void Logger::Enqueue(std::string log)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push(log);
}

bool Logger::Dequeue(std::string& str)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(_queue.size()<=0)
        return false;
    str = _queue.front();   //取队首
    _queue.pop();           //出队
    return true;
}

#endif



void Logger::Log(LOGLEVEL level ,const std::string str)
{
    if(LOG_LEVEL > level)
        return;
    //char log[128];
    char log[1024];
    int index = 0;


    //
	auto now = std::chrono::system_clock::now();
	//通过不同精度获取相差的毫秒数
	uint64_t dis_millseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
		- std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() * 1000;
	time_t tt = std::chrono::system_clock::to_time_t(now);
	tm* tm_time = localtime(&tt);

    snprintf(log, 35, "[%4d%02d%02d %02d:%02d:%02d.%06ld]",
                    tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
                    tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, (int)dis_millseconds);
    

    switch (level)
    {
    case LOGLEVEL::LOG_TRACE :
        strcpy(log+strlen(log),LeveL[0]);
        break;
    case LOGLEVEL::LOG_DEBUG :
        strcpy(log+strlen(log),LeveL[1]);
        break;
    case LOGLEVEL::LOG_INFO :
        strcpy(log+strlen(log),LeveL[2]);
        break;
    case LOGLEVEL::LOG_WARN :
        strcpy(log+strlen(log),LeveL[3]);
        break;
    case LOGLEVEL::LOG_ERROR :
        strcpy(log+strlen(log),LeveL[4]);
        break;
    case LOGLEVEL::LOG_FATAL :
        strcpy(log+strlen(log),LeveL[5]);
        break;
    default:
        break;
    }
    
    strcpy(log+strlen(log),str.c_str());
    strcpy(log+strlen(log),"\n");
    Enqueue(log);
}



std::string net::format(const char* fmt, ...)
{
    char        data[128];
    size_t      i = 0;
    va_list     ap;

    va_start(ap, fmt);
    vsnprintf(data + i, 128 - i, fmt, ap);
    va_end(ap);

    return std::string(data);
}
