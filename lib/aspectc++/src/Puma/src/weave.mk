#ifeq ($(MAKELEVEL),0)
#$(error Sorry, this makefile is not intended to be \
#used directly. Please use the makefile in the toplevel \
#directory of the PUMA source tree instead)
#endif

#-------------------------------------------------
# variables 
#-------------------------------------------------

include $(ROOT)/vars.mk

HEADERS   := $(shell find $(STEP1)/inc -name "*")
HEADERSDEP:= $(STEP2)/.inc

ACOBJECTS := $(notdir $(filter-out $(DONTWEAVE), $(SOURCES)))
ACOBJECTS += $(notdir $(LEMSOURCES:.lem=.cc))
ACOBJECTS := $(addprefix $(STEP2)/src/, $(ACOBJECTS))

CPPFLAGS  += -I $(EXTDIR) -I $(STEP1)/inc -I $(STEP1)/aspects

ACFILES   := $(addprefix $(STEP1)/aspects/Puma/, $(notdir $(ASPECTS)))
ACDEPS    := $(addprefix --include , $(ACFILES))
ACFLAGS   += $(addprefix -a , $(ACFILES))
#ACFLAGS   += $(CPPFLAGS) -I $(STEP1)/src -r $(GENDIR)/puma.acp
ACFLAGS   += $(CPPFLAGS) -I $(STEP1)/src
ACFLAGS   += --config $(CONFIG) --gen_size_type __SIZE_TYPE__ -k

DEPDIR    := $(STEP1)/dep
DEPFILES  := $(addprefix $(DEPDIR)/, $(notdir $(ACOBJECTS:.cc=.d)))

#-------------------------------------------------
# targets
#-------------------------------------------------

vpath %.cc  $(STEP1)/src
vpath %.d   $(DEPDIR)

ifneq ($(ASPECTS),)
all: showinfo dontweave $(ACOBJECTS) $(HEADERSDEP)
else
all: dontweave
	@cp -pr $(STEP1)/inc/Puma $(STEP2)/inc
	@cp -pr $(STEP1)/inc/Puma $(INCDIR)
endif

prof:
	@echo Profiling enabled

showinfo:
	@echo 
	@echo "---"
	@echo "Weaving PUMA for TARGET=$(TARGET)"
	@echo "  Weaver = $(AC) $(ACFLAGS)"
	@echo "---"
	@echo 

ifeq ($(DONTWEAVE),)
dontweave:
	@echo "Weaving into ALL files"
else
dontweave:
	@echo "Copying sources not to weave..."
	@cp -p $(DONTWEAVE) $(STEP2)/src
endif

#-------------------------------------------------
# rules
#-------------------------------------------------

$(STEP2)/src/%.cc : %.cc
	@echo "Weaving aspects into $(notdir $<)..."
	@$(AC) $(ACFLAGS) -p $(STEP1) -c $< -o $@
	@echo "$@: \\" > $(addprefix $(DEPDIR)/, $(notdir $(<:.cc=.d)))
	@grep "^#line" $@ | grep -v $(STEP2) | grep -v "<ac" | grep -v "\.ah" | \
	 awk '{print $$3}' | sed -e "s#\"##g" | sort | uniq |\
	 sed -e "s/$$/ \\\/g" >> $(addprefix $(DEPDIR)/, $(notdir $(<:.cc=.d)))
	@echo $(ACFILES) >> $(addprefix $(DEPDIR)/, $(notdir $(<:.cc=.d)))


$(HEADERSDEP): $(HEADERS) $(ACFILES)
	@echo "Generating library header files..."
	@touch $(HEADERSDEP)
	@cd $(STEP1) && $(AC) $(ACFLAGS) -i -p. -d$(STEP2)
	@cp -p $(STEP2)/aspects/Puma/*.ah $(STEP2)/inc/Puma
	@cp -pr $(STEP2)/inc/Puma $(INCDIR)


#-------------------------------------------------
# special targets
#-------------------------------------------------

.PHONY: all dontweave config showinfo

#-------------------------------------------------
# includes
#-------------------------------------------------

ifeq ($(MAKECMDGOALS),)
ifneq ($(DEPFILES),)
-include $(DEPFILES)
endif
endif
