ifeq ($(TARGET),mingw)
	CC  := mingw32-gcc
	CXX := mingw32-g++
	CFLAGS := -O6 -DHAVE_GETPAGESIZE

OBJECTS = ptmalloc.o sbrk.o

all: $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.cc
	$(CXX) $(CFLAGS) -o $@ -c $<

else

all:

endif

clean:
	rm -f *.o *~
