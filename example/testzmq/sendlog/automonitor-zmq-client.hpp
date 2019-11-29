#pragma once

#include <iostream>
#include <fstream>


int read_log_file(std::ifstream& file,const char* filename);
int Creat_zmq_client(std::string addr, const char* filename);