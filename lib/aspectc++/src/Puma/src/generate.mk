ifeq ($(MAKELEVEL),0)
$(error Sorry, this makefile is not intended to be \
used directly. Please use the makefile in the toplevel \
directory of the PUMA source tree instead)
endif

#-------------------------------------------------
# variables 
#-------------------------------------------------

LEMOBJECTS := $(addprefix $(STEP1)/src/, \
	$(patsubst %.lem,%.cc, $(notdir $(LEMSOURCES))))
SRCTOCOPY  := $(SOURCES) 

FINDINCS    = $(wildcard $(dir)*.h) $(wildcard $(dir)*.inc)
SRCDIRS     = $(sort $(dir $(SOURCES)))
INCTOCOPY   = $(foreach dir,$(SRCDIRS),$(FINDINCS)) $(LEMOBJECTS:.cc=.h)

#-------------------------------------------------
# rules
#-------------------------------------------------

vpath %.lem $(dir $(LEMSOURCES))

all: $(LEMOBJECTS)
	@cp -p $(INCTOCOPY) $(STEP1)/inc/Puma/
	@cp -p $(SRCTOCOPY) $(STEP1)/src
ifneq ($(ASPECTS),)
	@cp -p $(ASPECTS)   $(STEP1)/aspects/Puma/
endif

#-------------------------------------------------
# targets
#-------------------------------------------------

$(STEP1)/src/%.cc : %.lem
	@echo "Generating parser from $<..."
	@$(ROOT)/tools/lemon/lemon -q $< $(STEP1)/src/; true

.PHONY: all
