#pragma once 
#include <string>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep(n) Sleep(n)

#endif

//int zmq_server(std::string addr);