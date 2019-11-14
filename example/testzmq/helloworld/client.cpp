#include <zmq.hpp>
#include <string>
#include <iostream>

int main()
{
    zmq::context_t context(1);//建立一个连接
    zmq::socket_t socket(context, ZMQ_REQ);//

    std::cout << "Connecting to hello world server..."<< std::endl;
    socket.connect("tcp://localhost:5555");//连接到该地址

    //做10次请求。
    for(int request_nbr = 0; request_nbr != 10; request_nbr++)
    {
        zmq::message_t request(5);//分配了空间大小
        memcpy(request.data(), "hello", 5);
        std::cout << "Sending Hello "<< request_nbr << "..."<<std::endl;

        socket.send(request);//发送请求
        std::cout << request.data() << std::endl;
        //
        zmq::message_t reply;//
        socket.recv(&reply);//接收回应

        std::cout << "Received World "<< request_nbr <<std::endl;
    }

    return 0;
}