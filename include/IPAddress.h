#ifndef IPADDRESS_H
#define IPADDRESS_H

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

namespace net
{

class IPAddress
{
public:
    IPAddress(std::string ip,int port);
    //给服务器初始化使用
    explicit
    IPAddress(int port, int opt = INADDR_ANY);
    IPAddress() = default;
    ~IPAddress(){};

    void set(std::string ip,int port);
    void set(sockaddr_in addr);
    std::string GetIP() const;
    int GetPort() const;
    const struct sockaddr* getsockaddr() const 
    {   return reinterpret_cast<const sockaddr*>(&_addr);}
    const socklen_t getsocklen() const
    {   return sizeof(_addr);}
    std::string GetIPPort() const;
    //char* StringToCstr(std::string ip);
private:
    struct sockaddr_in _addr;
    std::string _ip;
    int _port;
};


}
#endif