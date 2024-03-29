/*


*/

#include <zmq.hpp>

#include <string>
#include <iostream>
#include <cstring>

#include "util-debug.hh"
#include "server.hpp"


/*
接收发送过来的消息，并把消息传送给automonitor.cc的字检测函数？
*/

int zmq_server(std::string addr)
{
    FuncBegin();

    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);

    socket.bind(addr);
    INFOPrint("Server has binded the address");

    while (true)
    {

        zmq::message_t request;

        socket.recv(&request);

        char *event = (char *)request.data();
        //VePrint(event);
        //对事件的处理,输出为新的日志

        sleep(0.3);

        // 返回消息给client
        zmq::message_t reply(5);
        memcpy(reply.data(), "World", 5);
        socket.send(reply);
    }
    FuncEnd();

    return 0;
}
//=====================================
//Test Units===========================
//=====================================
/*
int main(){
    std::string addr = "tcp://*:5555";

    //接收发送过来的日志，并且把消息传递给AutoMonitor??
    //"tcp://*:5555"

    zmq_server(addr);
}
*/