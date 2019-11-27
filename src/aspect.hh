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

std::mutex mtx; 
/*
mycout
*/
#define AOP_Logger(id, eventName, mycout)           \
    {                                               \
        mycout << "{\"eventId\":" << id << ","      \
               << "\"eventName\":"                  \
               << "\"" << eventName << "\","        \
               << "\"fileName\":"                   \
               << "\"" << tjp->filename() << "\""   \
               << ","                               \
               << "\"line\":" << tjp->line() << "," \
               << "\"eventTime\":"                  \
               << "\"" << __TIME__ << "\""          \
               << "}" << std::endl;                 \
        id++;                                       \
    }

/*拥有互斥锁*/
#define AOP_Logger_mutex(id, eventName, mycout) \
    {                                           \
        mtx.lock();                             \
        AOP_Logger(id, eventName, mycout);      \
        mtx.unlock();                           \
    }

#endif
