#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <zmq.hpp>
#include <memory>

using namespace std;

#define MSGLENGTH 128 //The zmq message length.
#define TESTCLIENT 0




int sendBufferToZmq(string str); //Send the message to server.
int sendLoggerToZmq(const char *filename, zmq::socket_t & socket);

void test_automonitor_client_Creat_zmq_client();