#!/bin/sh
g++ automonitor-client.cpp ../util-error.cc ../util-debug.hh -o client -lzmq
#g++ server.cpp -o server -lzmq

