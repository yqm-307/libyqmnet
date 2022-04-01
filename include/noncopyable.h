#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H


namespace net
{

class noncopyable
{
public:
    noncopyable(noncopyable& non) = delete;
    void operator=(noncopyable& non) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};
}
#endif