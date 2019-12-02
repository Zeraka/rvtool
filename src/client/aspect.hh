/*
该头文件提供了对被插装系统的日志宏函数，可生成格式化日志
必须先安装AspectC++
*/

#ifndef __ASPECT_H__
#define __ASPECT_H__

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <sstream>
#include <time.h>

#include "automonitor-client.hpp"
/*color*/
#define RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"            /* Magenta */
#define CYAN "\033[36m"               /* Cyan */
#define WHITE "\033[37m"              /* White */
#define BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m"   /* Bold White */
//end
using namespace std;
std::mutex mtx;
/*
mycout
*/
/*
char *errorfilename = "error.log";
ofstream errorcout(errorfilename);
*/
int fileIsExisted = -1;
int eventid = 0;
int socket_connect_state = -1;

typedef struct AspectState
{

} AspectState;
/*
char* getDate()
{
    time_t timep;
    time(&timep);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y_%m_%d_%H_%M_%S", localtime(&timep));
    return tmp;
}
*/
#define EVENTLOG(mycout) std::ofstream mycout;

#define CREATELOGFILEBYDATE(filename)                                       \
    {                                                                       \
        std::stringstream sstream;                                          \
        time_t timep;                                                       \
        time(&timep);                                                       \
        char tmp[64];                                                       \
        strftime(tmp, sizeof(tmp), "%Y_%m_%d_%H_%M_%S", localtime(&timep)); \
        sstream << filename << "_" << tmp << std::endl;                     \
        std::cout << sstream.str();                                         \
    }

//#define ADDATE(filename)  std::cout<<filename+GETDATE()<<std::endl;

/*Use this To build a zeromq socket*/
#define CREATSOCKET()          \
    zmq::context_t context(1); \
    zmq::socket_t socket(context, ZMQ_REQ);

#define CHECKSOCKETCONNECT(socket)                                   \
    {                                                                \
        if (socket_connect_state == -1)                              \
        {                                                            \
            socket.connect(addr);                                    \
            socket_connect_state = 0;                                \
            cout << "is connected? -" << socket.connected() << endl; \
            if (socket.connected() == false)                         \
            {                                                        \
                cout << "socket is not connected\n"                  \
                     << "Check out the network\n";                   \
            }                                                        \
        }                                                            \
        if (socket.connected() == false)                             \
        {                                                            \
            socket.connect(addr);                                    \
            cout << "socket is not connected\n"                      \
                 << "Check out the network\n";                       \
        }                                                            \
    }
/*Get the Config from Server's yaml*/
#define GetConfigFromServer() \
    {                         \
    }
#define DATE_FORMAT "%Y-%m-%d-%H:%M:%s"

#define TimeStamp_str(tstr)                                         \
    {                                                               \
        time_t timep;                                               \
        time(&timep);                                               \
        char tmp[64]={'\0'};                                               \
        strftime(tmp, sizeof(tmp), DATE_FORMAT, localtime(&timep)); \
        tstr = tmp;                                                 \
    }
#define TimeStamp_Num(num)

#define AOPLogger(id, eventName, mycout)            \
    {                                               \
        std::string tstr;                           \
        TimeStamp_str(tstr);                        \
        mycout << "{\"eventId\":" << id << ","      \
               << "\"eventName\":"                  \
               << "\"" << eventName << "\","        \
               << "\"fileName\":"                   \
               << "\"" << tjp->filename() << "\""   \
               << ","                               \
               << "\"line\":" << tjp->line() << "," \
               << "\"eventTime\":"                  \
               << "\"" << tstr << "\""              \
               << "}" << std::endl;                 \
    }

#define AOPLogger_ID_ADD(id, eventName, mycout)     \
    {                                               \
        std::string tstr;                           \
        TimeStamp_str(tstr);                        \
        mycout << "{\"eventId\":" << id << ","      \
               << "\"eventName\":"                  \
               << "\"" << eventName << "\","        \
               << "\"fileName\":"                   \
               << "\"" << tjp->filename() << "\""   \
               << ","                               \
               << "\"line\":" << tjp->line() << "," \
               << "\"eventTime\":"                  \
               << "\"" << tstr << "\""              \
               << "}" << std::endl;                 \
        id++;                                       \
    }

/*拥有互斥锁
Print the event log to mycout.
*/
#define AOPLogger_mutex(id, eventName, mycout) \
    {                                          \
        mtx.lock();                            \
        AOPLogger(id, eventName, mycout);      \
        mtx.unlock();                          \
    }

// Here are functions about Send buffer to zmq Server.
/**/

#define AOPLoggerToBufferNFile(id, eventName, mycout, addr)           \
    do                                                                \
    {                                                                 \
        mtx.lock();                                                   \
        if (socket_connect_state == -1)                               \
        {                                                             \
            socket.connect(addr);                                     \
            socket_connect_state = 0;                                 \
            cout << "is connected? ：" << socket.connected() << endl; \
            if (socket.connected() == false)                          \
            {                                                         \
                cout << "socket is not connected\n"                   \
                     << "Check out the network\n";                    \
            }                                                         \
        }                                                             \
        if (socket.connected() == false)                              \
        {                                                             \
            socket.connect(addr);                                     \
            cout << "socket is not connected\n"                       \
                 << "Check out the network\n";                        \
        }                                                             \
        zmq::message_t reply;                                         \
        stringstream sstream;                                         \
        AOPLogger_ID_ADD(id, eventName, sstream);                     \
        AOPLogger(id, eventName, mycout);                             \
        string str = sstream.str();                                   \
        zmq::message_t request(str.length());                         \
        sstream.clear();                                              \
        cout << str;                                                  \
        memcpy(request.data(), (void *)(str.c_str()), str.length());  \
        socket.send(request);                                         \
        socket.recv(&reply);                                          \
        mtx.unlock();                                                 \
    } while (0);

#define AOPLoggerToBufferNewFile(id, eventName, addr, mycout)                   \
    do                                                                          \
    {                                                                           \
        mtx.lock();                                                             \
        string filename;                                                        \
        if (fileIsExisted == -1)                                                \
        {                                                                       \
            std::cout << "Ceate a new log file." << std::endl;                  \
            std::stringstream sstream;                                          \
            time_t timep;                                                       \
            time(&timep);                                                       \
            char tmp[64];                                                       \
            strftime(tmp, sizeof(tmp), "%Y_%m_%d_%H_%M_%S", localtime(&timep)); \
            sstream << filename << "_" << tmp << std::endl;                     \
            std::cout << "Created event.log" << sstream.str();                  \
            filename = "event.log" + sstream.str();                             \
            mycout.open(filename, ios::app);                                    \
            fileIsExisted = 0;                                                  \
        }                                                                       \
        if (socket_connect_state == -1)                                         \
        {                                                                       \
            socket.connect(addr);                                               \
            socket_connect_state = 0;                                           \
            cout << "is connected? ：" << socket.connected() << endl;           \
            if (socket.connected() == false)                                    \
            {                                                                   \
                cout << "socket is not connected\n"                             \
                     << "Check out the network\n";                              \
            }                                                                   \
        }                                                                       \
        if (socket.connected() == false)                                        \
        {                                                                       \
            socket.connect(addr);                                               \
            cout << "socket is not connected\n"                                 \
                 << "Check out the network\n";                                  \
        }                                                                       \
        zmq::message_t reply;                                                   \
        stringstream sstream;                                                   \
        AOPLogger_ID_ADD(id, eventName, sstream);                               \
        AOPLogger(id, eventName, mycout);                                       \
        string str = sstream.str();                                             \
        zmq::message_t request(str.length());                                   \
        sstream.clear();                                                        \
        cout << str << "str.length=" << str.length() << std::endl;              \
        memcpy(request.data(), (void *)(str.c_str()), str.length());            \
        socket.send(request.data(), str.length());                              \
        socket.recv(&reply);                                                    \
        mtx.unlock();                                                           \
    } while (0);
#endif
