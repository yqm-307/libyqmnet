#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

int main()
{
    char a[] = "hello world!";
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    inet_aton("127.0.0.1",&addr.sin_addr);


    socklen_t len = sizeof(addr);
    int ret = connect(sockfd,(struct sockaddr*)&addr,len);

    if(ret == -1)
    {
        printf("连接失败\n");
    }else
    {
        printf("连接成功\n");

        int ret = write(sockfd,a,sizeof(a));
        if(ret < 0)
            printf("write error !\n");
        else
            printf("成功向 %d 写入\n",sockfd);
    }
    while(1);
    ::close(sockfd);
}