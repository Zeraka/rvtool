/*
    该模块为消息接收模块, 接收发送过来的日志事件。
*/

#include <zmq.hpp>
#include <string>

#include <iostream>

#ifndef _WIN32
#include <unistd.h>

#else
#include <windows.h>

#define sleep(n)    Sleep(n)
#endif 




