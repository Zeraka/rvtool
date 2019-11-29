#include <zmq.hpp>
#include <string>
#include <cstring>
#include <iostream> //读取文件
#include <fstream>  //读取文件

#include "automonitor-client.hpp"
#include "../util-debug.hh"
#include "../util-error.hh"
#include "../util-base.hh"

using namespace std;

zmqMessageClient::zmqMessageClient(string addr)
{
    address = addr;
    msgLength = MSGLENGTH;
}

int zmqMessageClient::creatZmqContext()
{
    client_context = new zmq::context_t(1);
    client_socket = new zmq::socket_t(*client_context, ZMQ_REP);
    client_request = new zmq::message_t(msgLength);
    client_reply = new zmq::message_t;

    return 0;
}
int zmqMessageClient::deleteZmqContext()
{

    AMFree(client_context);
    AMFree(client_reply);
    AMFree(client_socket);
    AMFree(client_request);
}
/*
改为发送生成的日志，然后把日志发送给服务端，
保留main函数，可作为单独的消息发送机制;
也可嵌入被检测系统中，提供相关API.
*/

//将日志格式化
/*
将日志变为一定的格式

*/

/*
读取文件的测试
*/

/*
功能： 建立一个zmq连接, 并且读取文件
*/
int zmqMessageClient::Creat_zmq_client(const char *filename, zmq::socket_t &socket)
{
    FuncBegin();

    //zmq::context_t context(1);              //建立一个连接
    //zmq::socket_t socket(context, ZMQ_REQ); //

    std::cout << "Connecting to Monitor's server..." << std::endl;

    //判断输入的地址的格式
    socket.connect(zmqMessageClient::address); //连接到该地址

    std::ifstream file_;
    std::ifstream &file = file_;

    std::string line_;
    std::string &line = line_;

    //打开log文件， 读取日志，
    file.open(filename, std::ios::in); //ios::in的意思是如果文件不存在，则打开出错
    INFOPrint("open log file");

    if (file.is_open() == 0)
    {
        INFOPrint("The file is not opened");
        return ERROR;
    }

    //每当读取一次，就发送一次消息给另一端,先发送一个消息给它
    //发送消息
    zmq::message_t request(msgLength);
    zmq::message_t reply;
    //请求到末尾
    //for(int request_nbr = 0; request_nbr != 100; request_nbr++)
    while (!file.eof()) //判断是否读至文件末尾
    {

        //读取文件,判断流的异常
        getline(file, line);
        //VePrint(line.length());
        int length = line.length();
        //VePrint(line.c_str())
        if (length >= msgLength)
        {
            INFOPrint("The zmq::message_t instance's MSGLENGTH is not enough");
            return ERROR;
        }
        memcpy(request.data(), (void *)(line.c_str()), length);

        //VePrint((char*)request.data());
        socket.send(request);
        VePrint(line);

        socket.recv(&reply);

        //VePrint((char*)reply.data());
        if ((char *)reply.data() == nullptr)
        {
            INFOPrint("Reply failed");
            return ERROR;
        }

        if (strncmp((char *)reply.data(), "200", 3) == 0)
        {
            INFOPrint("Checked out ERROR"); //检测到时序错误，就会自动停止系统。
            return ERROR;
        }
        if (strncmp((char *)reply.data(), "300", 3) == 0)
        {
            INFOPrint("Parse JSON ERROR");
            return ERROR;
        }

        memset((char *)reply.data(), 0, sizeof((char *)reply.data()));
    }

    file.close();

    FuncEnd();
    return 0;
}
/*
brief\ send string buffer to zmq server.
Param str\ That needed to be sent to server.
*/
int zmqMessageClient::sendBufferToZmq(string str, zmq::socket_t &socket)
{
    zmq::message_t request(str.length());

    memcpy(request.data(), (void *)(str.c_str()), str.length());

    socket.send(request);
    zmq::message_t reply;

    socket.recv(&reply);

    //VePrint((char*)reply.data());
    if ((char *)reply.data() == nullptr)
    {
        INFOPrint("Reply failed");
        return ERROR;
    }

    if (strncmp((char *)reply.data(), "200", 3) == 0)
    {
        INFOPrint("Checked out ERROR"); //检测到时序错误，就会自动停止系统。
        return ERROR;
    }
    if (strncmp((char *)reply.data(), "300", 3) == 0)
    {
        INFOPrint("Parse JSON ERROR");
        return ERROR;
    }
}

//================================================
//Test Unit//=====================================
//================================================

int test()
{
    string addr = "tcp://localhost:25555";
    const char *filename = "event.log";
    zmqMessageClient zmmsg(addr);
    //zmqMessage zmmsg2 = zmqMessage(addr);
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);

    zmmsg.Creat_zmq_client(filename, socket);
}

#if TESTCLIENT == 1
int main()
{
    test();
}
#endif