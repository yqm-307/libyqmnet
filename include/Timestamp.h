#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <chrono>
#include "Config.h"

namespace net
{
using std::chrono::system_clock;
using namespace std::literals::chrono_literals;

//chrono相关 类型定义（简化）
typedef std::chrono::nanoseconds   Nanosecond;      //纳秒
typedef std::chrono::microseconds  Microsecond;     //微秒
typedef std::chrono::milliseconds  Millisecond;     //毫秒
typedef std::chrono::seconds       Second;
typedef std::chrono::minutes       Minute;
typedef std::chrono::hours         Hour;
typedef std::chrono::time_point
            <system_clock,Nanosecond>  Timestamp;  //时间戳

namespace clock
{

//获取当前时间戳
inline Timestamp now()
{ return system_clock::now(); }

inline Timestamp nowAfter(Nanosecond interval)
{ return now() + interval; }

inline Timestamp nowBefore(Nanosecond interval)
{ return now() - interval; }

inline time_t utcms(Timestamp&& ts)
{
    return ts.time_since_epoch().count()/1000/1000;
}

//月份
inline time_t day(Timestamp&& ts= now())
{
    time_t tt = system_clock::to_time_t(ts);
    tm utc_tm = *gmtime(&tt);
    return utc_tm.tm_mday;
}
inline time_t month(Timestamp&& ts=now())
{
    time_t s = utcms(std::move(ts))/1000;    //ms
    return (s%(31556952)/(2629746));
}
inline time_t hour(Timestamp&& ts=now())
{
    time_t s = utcms(std::move(ts))/1000;    //ms
    return (s%(86400))/(3600);
}
inline time_t year(Timestamp&& ts=now())
{
    time_t tt = system_clock::to_time_t(ts);
    tm utc_tm = *gmtime(&tt);
    return 1900+utc_tm.tm_year;

}

inline time_t minute(Timestamp&& ts=now())
{
    time_t tt = system_clock::to_time_t(ts);
    tm utc_tm = *gmtime(&tt);
    return utc_tm.tm_min;
}
inline time_t second(Timestamp&& ts=now())
{
    time_t tt = system_clock::to_time_t(ts);
    tm utc_tm = *gmtime(&tt);
    return utc_tm.tm_sec;
}
inline time_t millisecond(Timestamp&& ts=now())
{
    time_t ms = utcms(std::move(ts));
    return (ms%1000);
}
}

}

#endif