#include <zmq.hpp>
#include <string>
#include <cstring>
#include <iostream> //读取文件
#include <fstream>  //读取文件

#include "client.hpp"
#include "util-debug.hh"

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
int Creat_zmq_client(std::string addr, const char* filename)
{
    FuncBegin();
    
    zmq::context_t context(1);//建立一个连接
    zmq::socket_t socket(context, ZMQ_REQ);//

    std::cout << "Connecting to Monitor's server..."<< std::endl;

    //判断输入的地址的格式
    socket.connect(addr);//连接到该地址

    std::ifstream file_;
    std::ifstream& file = file_;

    
    std::string line_;
    std::string& line = line_;

    //打开log文件， 读取日志，
    file.open(filename, std::ios::in);//ios::in的意思是如果文件不存在，则打开出错
    INFOPrint("open log file");

    if(file.is_open() == 0)
    {
        INFOPrint("The file is not opened");
        return ERROR;
    }

    //每当读取一次，就发送一次消息给另一端,先发送一个消息给它
    //发送消息

    //1000次请求， 
    for(int request_nbr = 0; request_nbr != 100; request_nbr++)
    {
        
        //读取文件,判断流的异常
        getline(file, line);
        zmq::message_t request(6);
        
        //VePrint(line.c_str())
        memcpy(request.data(), (void*)(line.c_str()), 6);

        //VePrint((char*)request.data());
        socket.send(request);
        VePrint(line);
        zmq::message_t reply;
        socket.recv(&reply);

        //VePrint((char*)reply.data());
        if((char*)reply.data() == nullptr)
        {
            INFOPrint("Reply failed");
            return ERROR;
        }

        if( strncmp((char*)reply.data(), "200", 3) == 0)
        {
            INFOPrint("Checked out ERROR");//检测到时序错误，就会自动停止系统。
            return ERROR;
        }

    }
    
    file.close();

    FuncEnd();
    return 0;
}


//================================================
//Test Unit//=====================================
//================================================

int main()
{
    std::string addr = "tcp://localhost:25555";
    const char* filename = "event.log";


    Creat_zmq_client(addr, filename);


}
