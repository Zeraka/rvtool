#include <zmq.hpp>
#include <string>
#include <cstring>
#include <iostream> //读取文件
#include <fstream>  //读取文件

#include "automonitor-client.hpp"
#include "util-debug.hh"


using namespace std;


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
int sendLoggerToZmq(const char *filename, zmq::socket_t &socket, string addr)
{
    FuncBegin();

    //zmq::context_t context(1);              //建立一个连接
    //zmq::socket_t socket(context, ZMQ_REQ); //
    //socket_ptr socket = zmqMessageClient::socket;
    std::cout << "Connecting to Monitor's server..." << std::endl;
    socket.connect(addr);
    //判断输入的地址的格式
    //socket->connect(zmqMessageClient::address); //连接到该地址

    std::ifstream file_;
    std::ifstream &file = file_;

    std::string line_;
    std::string &line = line_;

    //打开log文件， 读取日志，
    file.open(filename, std::ios::in); //ios::in的意思是如果文件不存在，则打开出错
    INFOPrint("open log file" << filename);

    if (file.is_open() == 0)
    {
        INFOPrint("The file is not opened" << filename);
        return ERROR;
    }
    else
    {
        INFOPrint("Open the file:" << filename);
    }

    //每当读取一次，就发送一次消息给另一端,先发送一个消息给它
    //发送消息
    zmq::message_t request(MSGLENGTH);
    zmq::message_t reply;

    /*Judge the connection is established or not*/

    //请求到末尾
    //for(int request_nbr = 0; request_nbr != 100; request_nbr++)
    while (!file.eof()) //判断是否读至文件末尾
    {
        //读取文件,判断流的异常
        getline(file, line);
        //VePrint(line.length());
        int length = line.length();
        //VePrint(line.c_str())
        if (length >= MSGLENGTH)
        {
            INFOPrint("The zmq::message_t instance's MSGLENGTH is not enough");
            return ERROR;
        }
        memcpy(request.data(), (void *)(line.c_str()), length);
        //VePrint((char*)request.data());

        try
        {
            socket.send(request);
        }
        catch (zmq::error_t error)
        {
            INFOPrint("Error: socket_send() in line: " << __LINE__ << " in" << __FILE__);
        }

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
/*
int zmqMessageClient::sendBufferToZmq(string str,)
{
    //zmqMessageClient::request= make_shared<zmq::message_t>(str.length());
    //zmq::message_t request(zmqMessageClient::msgLength);
    memcpy(zmqMessageClient::request->data(), (void *)(str.c_str()), str.length());

    //zmqMessageClient::socket->send(requ;

    zmqMessageClient::socket->send(*zmqMessageClient::request);
    zmq::message_t reply;

    zmqMessageClient::socket->recv(&reply);

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

    return 0;
}
*/
//================================================
//Test Unit//=====================================
//================================================

void test_automonitor_client_Creat_zmq_client()
{
    string addr = "tcp://localhost:25555";
    const char *filename = "event.log";
    //zmqMessageClient zmmsg(addr);
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);
    socket.connect(addr);

    sendLoggerToZmq(filename, socket, addr);
}

#if TESTCLIENT == 0
int main()
{
    test_automonitor_client_Creat_zmq_client();
}
#endif
