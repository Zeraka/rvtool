ifeq ($(MAKELEVEL),0)
$(error Sorry, this makefile is not intended to be \
used directly. Please use the makefile in the toplevel \
directory of the PUMA source tree instead)
endif

#-------------------------------------------------
# variables 
#-------------------------------------------------

include $(ROOT)/vars.mk

ifeq ($(_TARGET),win32)
	CFLAGS := $(CFLAGS) -DHAVE_STRING_H -DSTDC_HEADERS
	vpath %.c  $(ROOT)/src/win32/regex
	vpath %.cc $(ROOT)/src/win32/ptmalloc
	vpath %.c  $(ROOT)/src/win32/ptmalloc
endif

OBJECTS    += $(notdir $(SOURCES:.cc=.o))
OBJECTS    += $(notdir $(CSOURCES:.c=.o))
OBJECTS    += $(notdir $(LEMSOURCES:.lem=.o))
OBJECTS    := $(addprefix $(OBJDIR)/, $(OBJECTS))

DEPDIR     := $(STEP2)/dep
DEPFILES   := $(addprefix $(DEPDIR)/, $(notdir $(SOURCES:.cc=.d)))

LIBRARY    := $(LIBDIR)/$(LIBNAME).a
# disable building of shared library, we link statically
# SHARED     := $(LIBDIR)/$(LIBNAME).so

LIBRARIES  := $(LIBRARY)
ifeq ($(_TARGET),linux)
LIBRARIES  := $(LIBRARIES) $(SHARED)
endif

CPPFLAGS   += -I $(EXTDIR) -I $(STEP2)/inc

#-------------------------------------------------
# rules
#-------------------------------------------------

vpath %.cc  $(STEP2)/src
vpath %.d   $(DEPDIR)
vpath %.h   $(INCDIR)/Puma
vpath %.o   $(OBJDIR)

all: showinfo $(OBJECTS) $(LIBDIR) $(LIBRARIES)

showinfo:
	@echo 
	@echo "---"
	@echo "Compiling PUMA for TARGET=$(TARGET)"
	@echo "  Compiler = $(CXX) $(CPPFLAGS) $(CXXFLAGS)"
	@echo "  Linker   = $(CXX) $(CXXFLAGS) <objects> $(LDFLAGS)"
	@echo "---"
	@echo 

$(LIBDIR):
	@mkdir -p $(LIBDIR)

$(LIBRARY): $(OBJECTS)
	@echo "Making the static library $(LIBRARY)..."
	@$(AR) rcs $(LIBRARY) $?

$(SHARED): $(OBJECTS)
	@echo "Making the shared library $(SHARED)..."
	@$(CXX) -shared -o $(SHARED) $(OBJECTS)

#-------------------------------------------------
# targets
#-------------------------------------------------

$(OBJDIR)/%.o : %.cc
	@echo "Compiling $(notdir $<)..."
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $(OBJDIR)/$(@F) -c $<

$(OBJDIR)/%.o : %.c
	@echo "Compiling $(notdir $<)..."
	@$(CC) $(CPPFLAGS) $(CFLAGS) -o $(OBJDIR)/$(@F) -c $<

$(DEPDIR)/%.d : %.cc
	@echo "Generating dependencies for $(notdir $<)..."
	@$(CXX) $(CPPFLAGS) -MM $(filter %.cc,$^) | \
	sed -e "s@$(*F)\.o@$(OBJDIR)/& $(DEPDIR)/$(*F).d@g" > $@


.PHONY: all showinfo

#-------------------------------------------------
# includes
#-------------------------------------------------

ifeq ($(MAKECMDGOALS),)
ifneq ($(DEPFILES),)
-include $(DEPFILES)
endif
endif
