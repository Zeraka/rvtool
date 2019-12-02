/*
该demo的功能是，当服务器接收到

*/

#include <zmq.hpp>

#include <string>
#include <iostream>
#include <cstring>

#include "util-debug.hh"
#include "server.hpp"
#include <iostream>
#include <fstream>

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep(n) Sleep(n)

#endif
/*
接收发送过来的消息，并把消息传送给automonitor.cc的字检测函数？
*/

int main()
{
    std::string addr = "tcp://*:25555";

    //接收发送过来的日志，并且把消息传递给AutoMonitor??
    //"tcp://*:5555"

    zmq_server(addr);
}

static int interceptCString(char *str, char n)
{
    char *p = str;

    while (*p != '\0')
    {
        if (*p == n)
        {
            *(p + 1) = '\0';
            break;
        }
        else
        {
            p = p + 1;
        }
    }
}

int zmq_server(std::string addr)
{
    FuncBegin();

    //Creat the log file
    std::string filename = "accept.log";

    std::ofstream mycout(filename);
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);

    socket.bind(addr);
    INFOPrint("Server has binded the address");

    while (true)
    {

        zmq::message_t request;

        socket.recv(&request);

        char *event = (char *)request.data();
        //interceptCString(event, '}');
        //std::cout<<event;
        //VePrint(event);
        //把事件送入检测函数里面去？
        mycout << event;
        INFOPrint("has accepted a event");

        // 返回消息给client
        zmq::message_t reply(5);
        memcpy(reply.data(), "World", 5);
        socket.send(reply);
    }
    FuncEnd();

    return 0;
}