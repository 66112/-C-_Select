#include <iostream>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <string.h>
#include "Select.hpp"
using namespace std;
class Sock
{
public:
    Sock():_fd(-1)
    {}
    bool Socket()
    {
        _fd = socket(AF_INET,SOCK_STREAM,0);
        if(_fd < 0){
            std::cout << "socket error!" <<std::endl;
            return false;
        }
        return true;
    }
    bool Solve_TIME_WAIT()
    {
        int opt = 1;
        int ret = setsockopt(_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
        if(ret < 0){
            std::cout << "setsockopt error!" << std::endl;
            return false;
        }
        return true;
    }
    bool Bind(const uint16_t& port_)
    {
        struct sockaddr_in local;
        bzero(&local,sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(port_);
        local.sin_addr.s_addr = INADDR_ANY;
        int ret = bind(_fd,(struct sockaddr*)&local,sizeof(local));
        if(ret < 0){
            std::cout << "bind error!" << std::endl;
            return false;
        }
        return true;
    }
    bool Listen(int num)
    {
        int ret = listen(_fd,num);
        if(ret < 0){
            std::cout << "listen error!" << std::endl;
            return false;
        }
        return true;
    }
    int Accept(struct sockaddr_in* pclient = NULL,socklen_t* plen = NULL)
    {
        int new_sock = accept(_fd,(struct sockaddr*)pclient,plen); 
        if(new_sock < 0){
            std::cout << "accept error!" << std::endl;
            return -1;
        }
        std::cout << "New Client Connect!" << std::endl;
        return new_sock;
    }
    bool Recv(struct sockaddr_in& client)
    {
        char buf[1024] = {0};
        int ret = recv(_fd,buf,sizeof(buf)-1,0);
        if(ret < 0){
            std::cout << "recv error" << std::endl;
            return false;
        }
        else if(ret == 0){
            std::cout << "client quit!" << std::endl;
            return false;
        }
        std::cout << "ip: " << inet_ntoa(client.sin_addr) << " "\
            << ntohs(client.sin_port) << " # " << buf << std::endl;
        return true;
    }
    bool Send(int fd,char* msg)
    {
        send(fd,msg,strlen(msg),0);
        return true;
    }
    int GetListenfd()
    {
        return _fd;
    }
    void Close()
    {
        close(_fd);
    }
    ~Sock()
    {}
private:
    int _fd;
};
class Server
{
public:
    Server(const uint16_t port):_port(port)
    {}
    bool InitServer()
    {
        return _Listen_sock.Socket()
             &&_Listen_sock.Solve_TIME_WAIT()
             &&_Listen_sock.Bind(_port)
             &&_Listen_sock.Listen(5);
    }
    void StartServer()
    {
        int listen_fd = _Listen_sock.GetListenfd(); //拿到监听描述符
        std::vector<int> out_put;
        out_put.push_back(listen_fd);
        Selector _selector;       //定义
        while(1){
            _selector.InitSelector(listen_fd);        //监视位图清零,将listen_fd设为最大
            _selector.Set_fd(out_put);
            bool ret = _selector.Wait(); 
            if(!ret) continue;
            sockaddr_in client;
            for(size_t i = 0; i < out_put.size(); i++){
                //判断文件描述符是否就绪
                if(!_selector.Is_OK(out_put[i])) continue;
                //就绪描述符是监听描述符
                if(out_put[i] == listen_fd)
                {
                    socklen_t len = sizeof(client);
                    int new_sock = _Listen_sock.Accept(&client,&len);
                    out_put.push_back(new_sock);
                }
                //就绪描述符是客户端连接
                else{
                    char buf[1024] = {0};
                    int ret = recv(out_put[i],buf,sizeof(buf)-1,0);
                    if(ret > 0){
                        std::cout << "ip: "<< inet_ntoa(client.sin_addr)\
                        << " port: " << ntohs(client.sin_port) \
                        << " client# " << buf << std::endl;
                        std::string answer = "干啥？";
                        send(out_put[i],answer.c_str(),answer.size(),0);
                    }
                    else{
                        close(out_put[i]);
                        _selector.Del(i,out_put);
                        std::cout << "ip: "<< inet_ntoa(client.sin_addr)\
                        << " port: " << ntohs(client.sin_port) \
                        << " ByeBye!" << std::endl;
                    }
                }    
            }   
        }
    }
    ~Server()
    {}
private:
    uint16_t _port;    
    Sock _Listen_sock;
};
