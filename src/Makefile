CXX=g++ 
CFLAGS= -g
CXXFLAGS= 

all: automonitor

automonitor: automonitor.o cJSON.o CJsonObject.o ltl-parse.o \
            util-error.o 
	CXX  automonitor.o cJSON.o CJsonObject.o ltl-parse.o \
	util-error.o -o automonitor

automonitor.o: automonitor.cc automonitor.hh util-base.hh \
			util-debug.hh server.hpp parsehoa.hh ltl-parse.hh
	CXX -c automonitor.cc

cJSON.o: cJSON.c cJSON.h
	CXX -c cJSON.c

CJsonObject.o: CJsonObject.cpp CJsonObject.hpp 
	CXX -c CJsonObject.cpp

ltl-parse.o: ltl-parse.cc ltl-parse.hh util-error.hh CJsonObject.hpp
	CXX -c ltl-parse.cc

util-error.o: util-error.cc util-error.hh
	CXX -c util-error.cc

clean: 
	-rm main *.o
.PHONY: clean

sources=automonitor.cc cJSON.c CJsonObject.cpp ltl-parse.cc parsehoa.cc server.cpp util-error.cc

include $(sources:.c=.d)

