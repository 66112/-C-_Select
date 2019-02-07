/*
 * 1.参数复杂,每次都要将输入参数重置,其输入和输出参数是同一个位图,最大监控的描述符有上限
 * 2.需要一个数组保存连接的文件描述符
 * 3.每次调用select,都需要把fd集合从用户态拷贝到内核态,开销很大
 * 4.每次调用select，都需要再重新遍历传进来的所有fd*/

#include <sys/select.h>
#include <iostream>
#include <vector>
#include <unistd.h>
class Selector
{
public:
    Selector()
    {}
    ~Selector()
    {}
    void InitSelector(int fd = 0)
    {
        _max_fd = fd;
        FD_ZERO(&_rd_fds);
    }
    void Set_fd(std::vector<int>& output)
    {
        for(size_t i = 0; i < output.size(); i++){
            FD_SET(output[i],&_rd_fds);
            if(output[i] > _max_fd){
                _max_fd = output[i];
            }
        }
    }
    bool Wait()
    {
        struct timeval tv;   //设置等待时间
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        int nfds = select(_max_fd + 1,&_rd_fds,NULL,NULL,&tv); //2秒内没人连，0 立即返回，NULL阻塞式等待
        if(nfds < 0){
            std::cout << "select error!!" << std::endl;
            return false;
        }
        if(nfds == 0){
            std::cout << "timeout!" << std::endl;
            return false;
        }
        return true;
    }
    bool Is_OK(int fd)
    {
        return FD_ISSET(fd,&_rd_fds);
    }
    bool Del(int i,std::vector<int>& output)
    {
        output.erase(output.begin()+i);      //删除,迭代器
        return true;
    }
private:
    fd_set _rd_fds;  //select下可读描述符的位图
    int _max_fd;     //最大文件描述符
};
