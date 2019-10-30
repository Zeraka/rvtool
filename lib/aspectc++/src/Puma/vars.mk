#-------------------------------------------------
# variables 
#-------------------------------------------------

ifeq ($(filter %-release,$(TARGET)),)

RELEASE := debug

else

RELEASE := release
endif

_TARGET := $(patsubst %-release,%,$(TARGET))

ifeq ($(_TARGET),win32)

CXX   := mingw32-g++
CC    := mingw32-gcc
AR    := mingw32-ar
STRIP := mingw32-strip
CPPFLAGS += -I $(ROOT)/src/win32/regex

else

CXX   ?= g++
CC    ?= gcc
AR    ?= ar
STRIP ?= strip

endif


# if this is a release build, set the appropriate (specific) flags
ifeq ($(RELEASE),debug)

CFLAGS := -O0 -ggdb3 -fno-inline -fno-default-inline

else

CFLAGS   ?= -O2 -ggdb3
CPPFLAGS += -DNDEBUG

endif

# standard settings
ACFLAGS += $(AC_OPTFLAGS)
CFLAGS   += -Wall -pipe -Wno-deprecated-declarations
CXXFLAGS += $(CFLAGS) # -fno-rtti -fno-exceptions
CPPFLAGS += $(CPP_OPTFLAGS) -std=gnu++11
