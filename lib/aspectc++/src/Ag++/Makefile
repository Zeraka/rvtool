# ******************************
# *       S E T T I N G S      *
# ******************************


# PUMA root
PUMA    ?= ../Puma

# default target
TARGET  ?= linux-release

#AspectC++ source directory
AC_SRC_DIR := ../AspectC++

# include the PUMA configuration variables
# => set _TARGET, RELEASE, CFLAGS, CXXFLAGS, CPPFLAGS, etc.
include $(PUMA)/vars.mk

# determine which Puma library to use: libMiniPuma is preferred
PUMA_LIBS := $(notdir $(wildcard $(PUMA)/lib/$(TARGET)/lib*Puma.a))
ifneq ($(filter libMiniPuma.a, $(PUMA_LIBS)),)
	PUMA_LIB := MiniPuma
else
	PUMA_LIB := Puma
endif

#CXX  := /usr/bin/g++-3.3
#CXX := ag++  -p. -p../aspects --Xcompiler
#CXX := ag++ -v3 -p. --Xcompiler

# compiler settings
ifeq ($(_TARGET),win32)
        CXXFLAGS += -I $(PUMA)/src/win32/regex
	EXT :=.exe
	RES := mingw32-windres
endif

CXXFLAGS += $(CPPFLAGS) -I$(PUMA)/extern -I$(PUMA)/include
ifneq ($(_TARGET),macosx_x86_64)
	ifeq ($(SHARED),)
		LDFLAGS += -static
	endif

LDFLAGS += -Wl,-Bstatic -L$(PUMA)/lib/$(TARGET) -l$(PUMA_LIB)

ifneq ($(SHARED),)
LDFLAGS += -Wl,-Bdynamic
endif

else
	LDFLAGS += -L$(PUMA)/lib/$(TARGET) -l$(PUMA_LIB)
endif

ifneq ($(PROFILING),)
	LDFLAGS += -lc_p -lm_p 
	CXXFLAGS += -pg
endif

# ******************************
# *       S O U R C E S        *
# ******************************

BINBASE := bin
BINDIR := $(BINBASE)/$(TARGET)
PROG := $(BINDIR)/ag++$(EXT)

CCSOURCES := $(wildcard *.cc) 

OBJECTBASE := ObjFiles
OBJECTDIR := $(OBJECTBASE)/$(TARGET)
OBJECTS := $(addprefix $(OBJECTDIR)/,$(CCSOURCES:%.cc=%.o))

DEPBASE := DepFiles
DEPDIR := $(DEPBASE)/$(TARGET)
DEPS := $(addprefix $(DEPDIR)/,$(CCSOURCES:%.cc=%.d))

ifeq ($(_TARGET),win32)
RCFILES := win-ag++.rc
OBJECTS += $(addprefix $(OBJECTDIR)/,$(RCFILES:%.rc=%.o))
DEPS += $(addprefix $(DEPDIR)/,$(RCFILES:%.rc=%.d))
endif


DIRS := $(OBJECTDIR) $(DEPDIR) $(BINDIR)



# ******************************
# *     T A R G E T S          *
# ******************************

all: showtarget $(AC_SRC_DIR)/$(PROG)

strip: $(AC_SRC_DIR)/$(PROG)
	@$(STRIP) $<

showtarget:
	@echo "---"
	@echo "Making Ag++ for TARGET=$(TARGET)"
	@echo "  AC++     = $(ACXX) $(ACXXFLAGS) <objects> "
	@echo "  Compiler = $(CXX) $(CXXFLAGS)"
	@echo "  Linker   = $(CXX) $(CXXFLAGS) <objects> $(LDFLAGS)"
	@echo "---"

help: showtarget
	@echo 
	@echo 
	@echo "Make:" 
	@echo "  help:          show this help"
	@echo "  all (default): compile, link and copy"
	@echo "  copy:          copy executable into AspectC++ bin folder"
	@echo "  test :         run tests"
	@echo "  test_<XX>:     run test number <XX>"
	@echo "  clean:         remove intermediate files of current TARGET"
	@echo "  distclean:     clean all generated files and directories"
	@echo "---"

clean: 
	@echo -n "CLEAN    "
	rm -rf $(PROG) core core.*  $(DIRS)

distclean:
	rm -rf $(DEPBASE) $(OBJECTBASE) $(BINBASE)
 

test:	all
	@export TARGET=$(TARGET);export _TARGET=$(_TARGET);export EXT=$(EXT);cd tests;./run_tests.sh

test_%: all
	@export TARGET=$(TARGET);export _TARGET=$(_TARGET);export EXT=$(EXT);cd tests; ./run_tests.sh $@


.PHONY: all clean distclean showtarget

# ******************************
# *       R U L E S            *
# ******************************

$(PROG): $(OBJECTS)
	@echo "LD        $@"
	@mkdir -p $(BINDIR)
	@$(CXX) -o $@ $^ $(LDFLAGS)

$(AC_SRC_DIR)/$(PROG): $(PROG)
	@echo "COPY      $< to $@"
	@mkdir -p $(AC_SRC_DIR)/bin/$(TARGET)
	@cp $< $@

$(OBJECTDIR)/%.o : %.cc
	@echo "DEP       $(DEPDIR)/$(<:%.cc=%.d)."
	@mkdir -p $(DEPDIR)
	@$(CXX) $(CXXFLAGS) -MM -MP -MT $@ -MT $(DEPDIR)/$(<:%.cc=%.d) -MF $(DEPDIR)/$(<:%.cc=%.d) $< 
	@echo "CC        $@"
	@mkdir -p $(OBJECTDIR)
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

ifeq ($(_TARGET),win32)
$(OBJECTDIR)/%.o : %.rc
	@echo "DEP       $@."
	@mkdir -p $(DEPDIR)
	@$(CXX) $(CXXFLAGS) -MM -MP -MT $@ -MT $(DEPDIR)/$(<:%.cc=%.d) -MF $(DEPDIR)/$(<:%.cc=%.d) -xc++ $< 
	@echo "RES       $@"
	@mkdir -p $(OBJECTDIR)
	@$(RES) $(filter -D%,$(CXXFLAGS)) $< $@
endif

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
ifeq (,$(findstring help,$(MAKECMDGOALS)))
-include $(DEPS)
endif
endif

.SUFFIXES: .cc .o .h .rc .mk $(SUFFIXES)
