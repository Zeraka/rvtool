
g++ -g -std=c++14 -I/usr/local/include automonitor.cc  \
	cJSON.c	util-error.cc ltl-parse.cc \
	CJsonObject.cpp	  util-base.cc				\
	solidity.cc	util-parse.cc	\
	-L/usr/local/lib -lspot -lbddx -lzmq -lyaml-cpp -lgvc -lcgraph -o automonitor

