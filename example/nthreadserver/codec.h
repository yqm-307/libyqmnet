#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <assert.h>

//容器
enum Container
{
    INIT=0,
    DESTORY=1
};


//数据操作
enum CURD
{
    INSERT=0,
    EARES=1,
    FIND=2,
    SET=3
};



class codec
{
public:
    typedef std::vector<std::pair<CURD,int>> OrderList;
    void parseContainer()
    {
        
    }
    OrderList parseCURD(const char* data)
    {
        OrderList ret;
        int index = 0;
        while(*(data+index) != '\0')
        {
            //以@开头的一条命令
            if(*(data+index) == '@')
            {
                char c= *(data+(++index));
                CURD curd = (CURD)(atoi(&c));
                std::string tmp="";
                index+=2;    //跳过空格

                while(*(data+index) != ' ')
                {
                    if(*(data+index) == '\0') break;
                    tmp+=*(data+index++);
                }
                assert(!tmp.empty());
                ret.push_back(std::pair<CURD,int>(curd,atoi(tmp.c_str())));
            }
            else
                ++index;
        }
        return ret;
    }
private:
    
    std::mutex _lock;
};