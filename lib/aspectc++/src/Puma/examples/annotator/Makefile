#-------------------------------------------------
# variables 
#-------------------------------------------------

PROG     := annotator
ROOT     := $(shell pwd)/../..
TARGET   ?= linux-release

OBJDIR   := obj
DEPDIR   := dep
GENDIR   := gen
INCDIR   := $(ROOT)/include $(ROOT)/extern $(GENDIR)
LIBDIR   := $(ROOT)/lib/$(TARGET)

ifeq ($(MAKELEVEL),0)

SOURCES  := $(wildcard *.ora) 
OBJECTS  := $(addprefix $(GENDIR)/,$(SOURCES:.ora=.ot))

else 

SOURCES  := $(wildcard *.cc) 
OBJECTS  := $(addprefix $(OBJDIR)/,$(SOURCES:.cc=.o))
DEPS     := $(addprefix $(DEPDIR)/,$(SOURCES:.cc=.d))

include $(ROOT)/vars.mk

CPPFLAGS += $(CXXFLAGS) $(addprefix -I,$(INCDIR))

endif

#-------------------------------------------------
# rules
#-------------------------------------------------

ifeq ($(MAKELEVEL),0)

all: $(OBJECTS)
	@$(MAKE)

else 

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
	export LD_LIBRARY_PATH=$(LIBDIR); \
	$(CXX) -o $@ $^ -L$(LIBDIR) $(CXXFLAGS) -Wl,-Bstatic -lPuma -Wl,-Bdynamic $(CPPFLAGS) 
#	$(CXX) -o $@ $^ -L$(LIBDIR) $(CXXFLAGS) -Wl,-Bdynamic -lPuma -Wl,-Bdynamic $(CPPFLAGS) 

endif

clean:
	@echo "Making $(PROG) clean..."
	@rm -f $(PROG)
	@rm -rf $(DEPDIR) $(OBJDIR) $(GENDIR)

#-------------------------------------------------
# targets
#-------------------------------------------------

ifeq ($(MAKELEVEL),0)

$(GENDIR)/%.ot : %.ora
	@echo "Generating $@..."
	@mkdir -p $(GENDIR)
	@$(ROOT)/tools/orange/orange $< >>  $@

else 

$(OBJDIR)/%.o : %.cc
	@echo "Compiling $(filter %.cc,$^)..."
	@mkdir -p $(OBJDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -g -o $@ $(filter %.cc,$^)

$(DEPDIR)/%.d : %.cc
	@echo "Generating dependencies for $(filter %.cc,$^)..."
	@mkdir -p $(DEPDIR)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MM $(filter %.cc,$^) | \
	sed -e "s/$*\.o/$(OBJDIR)\/& $(DEPDIR)\/$*.d/g" > $@

endif

.PHONY: clean

#-------------------------------------------------
# includes
#-------------------------------------------------

ifneq ($(MAKELEVEL),0)
ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif
endif
