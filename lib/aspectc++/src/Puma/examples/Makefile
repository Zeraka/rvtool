#-------------------------------------------------
# variables 
#-------------------------------------------------

SUBDIRS := $(shell find $(shell pwd) -maxdepth 1 -mindepth 1 -type d)
SUBDIRS := $(filter-out %/.svn %/CVS, $(SUBDIRS))
SUBDIRS := $(SUBDIRS)

#-------------------------------------------------
# rules
#-------------------------------------------------

all: $(SUBDIRS)
clean: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all clean $(SUBDIRS)
