# ******************************
# *       S E T T I N G S      *
# ******************************

# PUMA root
PUMA    ?= ../../Puma

# default target
TARGET  ?= linux-release

# include the PUMA configuration variables
# => set _TARGET, RELEASE, CFLAGS, CXXFLAGS, CPPFLAGS, etc.
include $(PUMA)/vars.mk

# libxml2 settings
ifeq ($(_TARGET),win32)
	LIBXML2_INC := `mingw32-xml2-config --cflags` 
else
	LIBXML2_INC := `xml2-config --cflags` 
endif

CPPFLAGS += -I$(PUMA)/extern $(LIBXML2_INC)

# quick hack: filter-out no-rtti
CXXFLAGS := $(filter-out -fno-rtti, $(CXXFLAGS))

# ******************************
# *          I N F O S         *
# ******************************

$(info ---)
$(info Making ACModel library for TARGET=$(TARGET))
$(info   Compiler = $(CXX) $(CPPFLAGS) $(CXXFLAGS))
$(info ---)
  
# ******************************
# *       S O U R C E S        *
# ******************************

# library file
LIBDIR := lib/$(TARGET)
LIB := $(LIBDIR)/libacmodel.a

CCSOURCES := $(wildcard *.cc) 

OBJECTDIR := ObjFiles/$(TARGET)
OBJECTS := $(addprefix $(OBJECTDIR)/,$(CCSOURCES:.cc=.o))

DEPDIR := DepFiles/$(TARGET)
DEPS := $(addprefix $(DEPDIR)/,$(CCSOURCES:.cc=.d))

DIRS := $(OBJECTDIR) $(DEPDIR) $(LIBDIR)

# ******************************
# *      T A R G E T S         *
# ******************************

all: $(LIB)

clean:
	@echo Making it clean.
	@rm -rf core core.* *~ lib DepFiles ObjFiles

# ******************************
# *       R U L E S            *
# ******************************

$(LIB): $(OBJECTS) 
	@echo "Creating library $@."
	@mkdir -p $(LIBDIR)
	@$(AR) rcs $@  $(OBJECTS)

$(OBJECTDIR)/%.o : %.cc 
	@echo Making dependency file $@.
	@mkdir -p $(DEPDIR)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MM -MP -MT $(OBJECTDIR)/$(<:%.cc=%.o) -MT $(DEPDIR)/$(<:%.cc=%.d) $< > $(DEPDIR)/$(<:%.cc=%.d)
	@echo Making object file $@.
	@mkdir -p $(OBJECTDIR)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

.phony: all clean dirs showtarget
