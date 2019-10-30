#-------------------------------------------------
# variables 
#-------------------------------------------------

PROG     := cparser

ROOT     ?= $(shell pwd)/../..
TARGET   ?= linux-release

INCDIR   := $(ROOT)/include
EXTDIR   := $(ROOT)/extern
LIBDIR   := $(ROOT)/lib/$(TARGET)
OBJDIR   := obj
DEPDIR   := dep

SOURCES  := $(wildcard *.cc) 
OBJECTS  := $(addprefix $(OBJDIR)/,$(SOURCES:.cc=.o))
DEPS     := $(addprefix $(DEPDIR)/,$(SOURCES:.cc=.d))

include $(ROOT)/vars.mk

CPPFLAGS := $(CPPFLAGS) -I$(INCDIR) -I$(EXTDIR)
#-L$(LIBDIR)

#-------------------------------------------------
# rules
#-------------------------------------------------

all: showinfo $(PROG)

showinfo:
	@echo 
	@echo "---"
	@echo "Compiling $(PROG) for TARGET=$(TARGET)"
	@echo "  Compiler = $(CXX) $(CPPFLAGS) $(CXXFLAGS)"
	@echo "  Linker   = $(CXX) $(CXXFLAGS) <objects> $(LDFLAGS)"
	@echo "---"
	@echo 

$(PROG): $(OBJECTS)
	@echo "Linking $@..."
	@export LD_LIBRARY_PATH=$(LIBDIR); \
	$(CXX) -o $@ $^ -L$(LIBDIR) $(CXXFLAGS) -Wl,-Bstatic -lPuma -Wl,-Bdynamic $(CPPFLAGS) 
#	$(CXX) -o $@ $^ -L$(LIBDIR) $(CXXFLAGS) -Wl,-Bdynamic -lPuma -Wl,-Bdynamic $(CPPFLAGS) 

clean:
	@echo "Making $(PROG) clean..."
	@rm -f $(PROG)
	@rm -rf $(DEPDIR) $(OBJDIR)

#-------------------------------------------------
# targets
#-------------------------------------------------

$(OBJDIR)/%.o : %.cc
	@echo "Compiling $(filter %.cc,$^)..."
	@mkdir -p $(OBJDIR)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -g -o $@ $(filter %.cc,$^)

$(DEPDIR)/%.d : %.cc
	@echo "Generating dependencies for $(filter %.cc,$^)..."
	@mkdir -p $(DEPDIR)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MM $(filter %.cc,$^) | \
	sed -e "s/$*\.o/$(OBJDIR)\/& $(DEPDIR)\/$*.d/g" > $@

.PHONY: clean

#-------------------------------------------------
# includes
#-------------------------------------------------

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif
