#!/bin/sh
g++ client.cpp -o client -lzmq
g++ server.cpp -o server -lzmq

