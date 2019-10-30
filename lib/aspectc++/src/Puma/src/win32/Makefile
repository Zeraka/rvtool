#-------------------------------------------------
# variables 
#-------------------------------------------------

SUBDIRS := $(shell find $(shell pwd) -type d -maxdepth 1 -mindepth 1)
SUBDIRS := $(filter-out %/.svn, $(SUBDIRS))
SUBDIRS := $(SUBDIRS)

#-------------------------------------------------
# rules
#-------------------------------------------------

all: $(SUBDIRS)
clean: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all clean $(SUBDIRS)
