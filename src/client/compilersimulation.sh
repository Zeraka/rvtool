#!/bin/sh
ag++ -g test.cpp EventLoop.cpp automonitor-client.cpp EventLoop.hpp  -o test -lpthread -lzmq 
