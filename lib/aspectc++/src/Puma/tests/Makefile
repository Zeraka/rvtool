ifeq ($(MAKELEVEL),0)
$(error Sorry, this makefile is not intended to be \
used directly. Please use the makefile in the toplevel \
directory of the Puma source tree instead)
endif

SHELL = /bin/bash
export PUMA_CONFIG ?= $(CONFIG)

# fixed bugs
FIXED = misc bug129 bug138 bug96 bug130 bug108 bug94 bug149 \
        bug164 bug153 bug166 bug156 bug180 bug182 bug320 bug344 \
        bug269 bug259 bug248 bug218 bug203 bug350 bug273 bug326 \
        bug332 bug340 bug339 bug337 bug345 bug346 bug137 bug328 \
        bug333 bug363 bug2 bug5 bug6

# open bugs
OPEN = bug100 bug10 bug102 bug187 bug249 bug5 bug145 \
       bug188 bug342 bug61 bug189 bug338 \
       bug325 bug347 bug232 \
       bug303 bug289 bug287 bug275 bug348 bug343 bug274 \
       bug264 bug258 bug220

STDOUT_FILE := stdout.txt
STDERR_FILE := stderr.txt
ERROR_FILE  := error.txt

all:
	@(\
	([ ! -e ../examples/parser/parser ] && echo "Release build of Puma/examples/parser required for testing!") || \
	$(MAKE) run "TESTS=$(FIXED)" \
	)

open:
	@(\
	([ ! -e ../examples/parser/parser ] && echo "Release build of Puma/examples/parser required for testing!") || \
	$(MAKE) run "TESTS=$(OPEN)" \
	)

run: $(addsuffix .parse, $(TESTS))
	@if [ -f $(ERROR_FILE) ];then \
		echo -e "\n\n"; \
		echo "+---------+"; \
		echo "|ERRORS:  |"; \
		echo "+---------+"; \
		cat $(ERROR_FILE); \
	 else \
		echo -e "\n\n"; \
	 	echo "All tests OK"; \
	 fi;\
	 rm -f $(STDOUT_FILE) $(STDERR_FILE) $(ERROR_FILE)

ref:  $(addsuffix .ref, $(TESTS))

refclean:
	@rm -f */parse.ref

outclean:
	@rm -f */parse.out

clean: $(addsuffix .clean, $(FIXED)) $(addsuffix .clean, $(OPEN))

%.parse: %
	@unset LANG; \
	exec 6>&1; \
	exec 7>&2; \
	exec >& $</parse.out; \
	$(MAKE) -s -C $< ; \
	exec 1>&6 6>&-; \
	exec 2>&7 7>&-; \
	exec 6>&1; \
	exec 7>&2; \
	exec > $(STDOUT_FILE); \
	exec 2> $(STDERR_FILE); \
	diff $</parse.out $</parse.ref; \
	RET=$$?; \
	exec 1>&6 6>&-; \
	exec 2>&7 7>&-; \
	if [ $$RET == 0 ];then \
		echo -n "."; \
	else \
		ERR_FLAG=D; \
	 	echo -n "[$$ERR_FLAG:$<]"; \
		echo -ne "\n-----------------------------------------------------------------------------\n" >> $(ERROR_FILE); \
		echo  -n "TEST: $<" >> $(ERROR_FILE); \
		echo -ne "\n-----------------------------------------------------------------------------\n" >> $(ERROR_FILE); \
		echo -e "STDOUT:\n--------" >> $(ERROR_FILE); \
		cat $(STDOUT_FILE) >> $(ERROR_FILE); \
		echo   >> $(ERROR_FILE); \
		echo -e "STDERR:\n--------" >> $(ERROR_FILE); \
		cat $(STDERR_FILE) >> $(ERROR_FILE); \
		echo   >> $(ERROR_FILE); \
	fi;

%.ref: %
	@$(MAKE) -s EXT="$(EXT)" -C $< ref; 

%.clean: %
	@cd $< && $(MAKE) clean

.PHONY: all clean ref  
