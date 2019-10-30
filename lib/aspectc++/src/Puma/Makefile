#-------------------------------------------------
# variables 
#-------------------------------------------------

ROOT    ?= $(shell pwd)
TARGET  ?= linux-release
AC      ?= ac++
PREFIX  ?= /usr/local 

include vars.mk
include files.mk

ifneq ($(MINI),)
LIBNAME := libMiniPuma
else
LIBNAME := libPuma
endif

LIBDIR  := $(ROOT)/lib/$(TARGET)
GENDIR  := $(ROOT)/gen-$(RELEASE)

INCDIR  := $(ROOT)/include
EXTDIR  := $(ROOT)/extern
OBJDIR  := $(GENDIR)/obj-$(_TARGET)
STEP1   := $(GENDIR)/step1
STEP2   := $(GENDIR)/step2

CONFIG  := $(GENDIR)/puma.cfg
ACNEWREPO ?= $(GENDIR)/acpp.repo

# exported variables
export GENDIR INCDIR EXTDIR LIBNAME LIBDIR OBJDIR STEP1 STEP2
export ROOT TARGET AC DONTWEAVE CONFIG ACNEWREPO
export LEMSOURCES SOURCES CSOURCES ASPECTS 

# suppress reporting directory changes
MAKE := $(MAKE) --no-print-directory

#-------------------------------------------------
# rules
#-------------------------------------------------

all: showinfo src

generate: showinfo src

weave: showinfo src

compile_after_weave: showinfo src

compile: showinfo src

dry: showinfo
	@cd $(ROOT)/src && $(MAKE) $(MAKECMDGOALS)

showinfo:
	@echo 
	@echo "---"
	@echo "Making PUMA for TARGET=$(TARGET)"
	@echo "  Extensions = $(EXTENSIONS)"
	@echo "---"
	@echo 

tools:
	@cd $(ROOT)/tools && $(MAKE)

src: tools
	@cd $(ROOT)/src && $(MAKE) $(MAKECMDGOALS)

examples: tools
	@$(MAKE) -C $(ROOT)/src
	@cd $(ROOT)/examples && $(MAKE)

doc:
	@echo "Generating Puma reference manual..."
	@doxygen doc/RefMan/doxygen.conf && echo "=> see doc/RefMan/html/index.html"

install: src
	@echo "Installing library and headers..."
	@mkdir -p $(PREFIX)/lib
	@cp -pv $(LIBDIR)/$(LIBNAME).* $(PREFIX)/lib
	@mkdir -p $(PREFIX)/include/Puma
	@cp -pv $(INCDIR)/Puma/*.*h $(PREFIX)/include/Puma

uninstall: 
	@echo "Uninstalling library and headers..."
	@rm -fv $(PREFIX)/lib/$(LIBNAME).*
	@rm -rfv $(PREFIX)/include/Puma

clean: tools-clean
	@cd $(ROOT)/src && $(MAKE) clean

tools-clean:
	@cd $(ROOT)/tools && $(MAKE) -s clean

examples-clean:
	@cd $(ROOT)/examples && $(MAKE) -s clean

docs-clean:
	@echo "Making docs clean..."
	@rm -rf $(ROOT)/doc/RefMan/html

cleanall: test-clean examples-clean tools-clean docs-clean
	@echo "Making completely clean..."
	@rm -rf $(ROOT)/lib
	@rm -rf $(ROOT)/gen-*

distclean: clean examples-clean docs-clean
	@cd $(ROOT)/src && $(MAKE) distclean

libclean:
	@echo "Removing library and object files..."
	@rm -f $(LIBDIR)/$(LIBNAME).*
	@rm -rf $(OBJDIR)
	@rm -rf $(STEP2)/dep

test: test-all

test-%:
	@echo "Making tests $*..."
	@$(MAKE) -C tests -s $*

#-------------------------------------------------
# targets
#-------------------------------------------------

.PHONY: all tools src install examples distclean cleanall \
clean tools-clean examples-clean libclean weave compile uninstall doc
