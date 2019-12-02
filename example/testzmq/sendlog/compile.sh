#!/bin/sh
g++ automonitor-client.cpp -o client -lzmq
g++ server.cpp -o server -lzmq

