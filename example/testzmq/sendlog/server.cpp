/*
该demo的功能是，当服务器接收到

*/

#include<zmq.hpp>

#include<string>
#include<iostream>
#include<cstring>
#ifndef _WIN32
#include<unistd.h>
#else
#include <windows.h>
#define sleep(n)    Sleep(n)

#endif

int main(){
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
        printf("asf\n\n");

    socket.bind("tcp://*:5555");//绑定5555端口
        printf("21asf\n\n");

    while(true){
        zmq::message_t request;

        //等待client的请求
        socket.recv(&request);//接受请求
        std::cout << "Recevied Hello" << std::endl;

        sleep(1);

        // 返回消息给client
        zmq::message_t reply(5);
        memcpy(reply.data(), "World", 5);
        socket.send(reply);
    }
    printf("qwef\n\n");
    return 0;
}