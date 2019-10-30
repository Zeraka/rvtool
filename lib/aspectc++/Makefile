ACFLAGS ?= -v1 -k
ACXX    ?= $(PWD)/ag++
export ACXX

ACTESTS = $(filter-out Makefile%, $(filter-out CVS,$(notdir $(wildcard $(PWD)/examples/*))))

.PHONY: $(ACTESTS)

all: testrun

test: $(addsuffix .make, $(ACTESTS))

testrun: test $(addsuffix .run, $(ACTESTS))

clean: $(addsuffix .clean, $(ACTESTS))

%.make: %
	make -C examples/$<

%.run: %
	make -C examples/$< run

%.clean: %
	make -C examples/$< clean
