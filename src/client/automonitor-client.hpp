#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <zmq.hpp>

using namespace std;

#define MSGLENGTH 128 //The zmq message length.

class zmqMessageClient
{

private:
    string address;
    int msgLength;


    zmq::socket_t * client_socket;
    zmq::context_t * client_context;
    zmq::message_t * client_request;
    zmq::message_t * client_reply;
public:
    zmqMessageClient(string addr);
    ~zmqMessageClient(){};

    int creatZmqContext();
    int deleteZmqContext();

    int sendBufferToZmq(string str);
    int sendBufferToZmq(string str, zmq::socket_t& socket); //Send the message to server.
    int Creat_zmq_client(const char *filename, zmq::socket_t& socket);
};
