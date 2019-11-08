#include <zmq.hpp>
#include <string>
#include <iostream> //读取文件
#include <fstream>  //读取文件

#include "client.hpp"
#include "util-debug.hh"
/*
改为发送生成的日志，然后把日志发送给服务端，
保留main函数，可作为单独的消息发送机制;
也可嵌入被检测系统中，提供相关API.
*/


int main()
{
    std::string addr = "tcp://localhost:5555";
    const char* filename = "event.log";

    


}
/*
功能： 将消息放入zmq中去
*/
static int Str_to_zmq(std::string str)
{

}



/*
功能： 读取文件, 合成序列，然后发送消息给zmq
*/
static int read_log_file(std::ifstream &file, const char *filename)
{
    //打开文件
    file.open(filename, std::ios::in); //只读该文件
    int i = 0;
    std::string line;
    if(file.is_open() == 0)
    {
        INFOPrint("The file is not opened");

        return ERROR;
    }

    INFOPrint("Now read the file");
    //读取文件
    while (i<10000)
    {
        //一行行打印出消息
        if(getline(file, line) == nullptr)
        {
            
        }
        //INFOPrint(line);
        
        //将line放入zmq中去。
        Str_to_zmq();
        i++;//调试 i 行

        //合成消息格式
        
        //发送消息

        //等待回复。如果收到回复，如果没有收到回复，或者收到了错误信息，则停止发送，并且报错。
    }
    file.close(); //关闭文件
}

//将日志格式化
/*
将日志变为一定的格式

*/


/*
功能： 建立一个zmq连接, 并且读取文件
*/
int Creat_zmq_client(std::string addr, const char* filename)
{
    
    zmq::context_t context(1);//建立一个连接
    zmq::socket_t socket(context, ZMQ_REQ);//

    std::cout << "Connecting to Monitor's server..."<< std::endl;

    //判断输入的地址的格式
    socket.connect(addr);//连接到该地址

    //做10次请求。
    for(int request_nbr = 0; request_nbr != 10; request_nbr++)
    {
        zmq::message_t request(5);//分配了空间大小
        
        memcpy(request.data(), "hello", 5);
        std::cout << "Sending Hello "<< request_nbr << "..."<<std::endl;

        socket.send(request);//发送请求

        //
        zmq::message_t reply;//
        socket.recv(&reply);//接收回应

        std::cout << "Received World "<< request_nbr <<std::endl;
    }
    

    //test
    std::ifstream file_;
    std::ifstream &file = file_;

    read_log_file(file, filename);
    return 0;
}