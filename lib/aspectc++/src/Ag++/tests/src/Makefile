
ACXX := ../../../AspectC++/bin/linux-release/ag++ 
OBJ_DIR := obj

SRC := $(wildcard *.cc)
OBJ := $(SRC:%.cc=$(OBJ_DIR)/%.o)
AH := $(wildcard *.ah)

PROG := test

all: $(PROG)


clean:
	rm -rf *.acc $(OBJ_DIR) $(PROG) puma.config

$(PROG): $(OBJ)
	$(ACXX) -v 2 -o $@ $+

$(OBJ_DIR)/%.o:%.cc $(AH)
	@mkdir -p $(OBJ_DIR)
	$(ACXX) -v 2 -o $@ -c $<
