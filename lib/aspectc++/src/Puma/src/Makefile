ifeq ($(MAKELEVEL),0)
$(error Sorry, this makefile is not intended to be \
used directly. Please use the makefile in the toplevel \
directory of the PUMA source tree instead)
endif

#-------------------------------------------------
# variables 
#-------------------------------------------------

AGXX    ?= $(subst ac++,ag++,$(AC))

include $(ROOT)/vars.mk

#-------------------------------------------------
# rules
#-------------------------------------------------

all: weave compile_after_weave

generate: config dirs
	@+$(MAKE) -f generate.mk

weave: generate
	@+$(MAKE) -f weave.mk

compile_after_weave: dirs weave
	@+$(MAKE) -f compile.mk

compile: dirs
	@+$(MAKE) -f compile.mk

dirs:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(INCDIR)
	@mkdir -p $(STEP1)/src
	@mkdir -p $(STEP1)/dep
	@mkdir -p $(STEP1)/aspects/Puma
	@mkdir -p $(STEP1)/inc/Puma
	@mkdir -p $(STEP2)/src
	@mkdir -p $(STEP2)/dep
	@mkdir -p $(STEP2)/inc/Puma

config:
ifneq ($(ASPECTS),)
	@echo "Generating ac++ config file..."
	@mkdir -p $(dir $(CONFIG))
	@$(AGXX) --gen_config --c_compiler $(CXX) -o $(CONFIG)
	@perl -i -pe "s/^-I [\"]?\.[\"]?$$//g" $(CONFIG) 
endif

clean:
	@echo "Making sources clean..."
	@rm -rf $(GENDIR)

distclean: 
	@echo "Removing library, headers, and other generated files..."
	@rm -rf $(LIBDIR) $(INCDIR) $(ROOT)/gen-*

install:
	# nothing to do for install

dry:
	@+$(MAKE) -f weave.mk showinfo
	@+$(MAKE) -f compile.mk showinfo

#-------------------------------------------------
# targets
#-------------------------------------------------

.PHONY: all clean dirs generate weave compile dry install distclean config compile_after_weave
