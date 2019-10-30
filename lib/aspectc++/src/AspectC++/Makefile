# ******************************
# *       S E T T I N G S      *
# ******************************

# AspectC++ supports alternative frontends: Puma or Clang
# It can be configured the 'make FRONTEND=...'. Default is Clang.
export FRONTEND ?= Clang

# PUMA root
PUMA    ?= ../Puma

# LLVM config command
LLVMCONF ?= llvm-config

# default target
TARGET  ?= linux-release

# include the PUMA configuration variables
# => set _TARGET, RELEASE, CFLAGS, CXXFLAGS, CPPFLAGS, etc.
include $(PUMA)/vars.mk

# check whether we compile for Windows: TARGET-Variable starts with 'win'
ifneq ($(filter win%,$(TARGET)),)
	WIN := yes
	EXT := .exe
endif

# libxml2 settings
LIBXML2_INC := `xml2-config --cflags` 
LIBXML2_LIB := `xml2-config --libs` 

# Check Clang version
ifneq ($(MAKECMDGOALS),clean)
  ifeq ($(FRONTEND),Clang)
    ifeq ($(shell which $(LLVMCONF)),)
      $(error FRONTEND is 'Clang', but LLVMCONF ($(LLVMCONF)) is not found)
    endif
    LLVM_VERSION := $(shell $(LLVMCONF) --version)
    LLVM_LIBDIR  := $(shell $(LLVMCONF) --libdir)
    ifeq ($(_TARGET),win_x86_64) # handle '\' in path name
      LLVM_LIBDIR := $(subst \,/,$(LLVM_LIBDIR))
    endif
    LLVM_LIBS :=  $(LLVM_LIBDIR)/libclang*.a
    # These functions need to be wrapped: parameters omitted to be more version independent
    LLVM_KEY_COUNT := 12
    LLVM_KEYS := _ZN5clang4Sema31ActOnStartCXXMemberDeclarations \
                 _ZN5clang4Sema33ActOnFinishCXXMemberSpecification \
                 _ZN5clang4Sema25ActOnEndOfTranslationUnit \
                 _ZN5clang4Sema19canSkipFunctionBody \
                 _ZN5clang4Sema24ActOnCXXMemberDeclarator \
                 _ZN5clang4Sema15ActOnDeclarator \
                 _ZN5clang12Preprocessor3Lex \
                 _ZN5clang12Preprocessor9PeekAhead \
                 _ZN5clang4Sema19ActOnBaseSpecifiers \
                 _ZN5clang4Sema8ActOnTag \
                 _ZN5clang4Sema22ActOnStartNamespaceDef \
                 _ZN5clang4Sema23ActOnStartOfFunctionDefEPNS_5ScopeERNS_10Declarator
    ifeq ($(_TARGET),macosx_x86_64) # on macOS the symbols have an additional '_'
      LLVM_KEYS := $(patsubst %, _%, $(LLVM_KEYS))
    endif
    # find the exact symbols using the keys in the clang libraries
    LLVM_SYMBOLS := $(foreach LLVM_KEY, $(LLVM_KEYS), $(filter _%, $(shell nm --defined-only $(LLVM_LIBS) | grep "T $(LLVM_KEY)")))
    # determine relevant interface variants
    LLVM_INTERFACE := 38 # latest is default
    ifneq ($(filter 3.4%, $(LLVM_VERSION)),)
      LLVM_INTERFACE := 34
    else
    ifneq ($(filter 3.5%, $(LLVM_VERSION))$(filter 3.6%, $(LLVM_VERSION)),)
      LLVM_INTERFACE := 35
    else
    ifneq ($(filter 3.7%, $(LLVM_VERSION)),)
      LLVM_INTERFACE := 37
    else
    ifneq ($(filter 3.8%, $(LLVM_VERSION))$(filter 3.9%, $(LLVM_VERSION)),)
      LLVM_INTERFACE := 38
    endif
    endif
    endif
    endif
    LLVM_SUPPORTED_VERSIONS := 3.4 3.4svn 3.4.2 3.6.2 3.7.1 3.8.0 3.9.1
    ifneq ($(filter $(LLVM_SUPPORTED_VERSIONS), $(LLVM_VERSION)),)
      $(info Clang version $(LLVM_VERSION): Fine)
    else
      $(warning Warning: UNSUPPORTED Clang version $(LLVM_VERSION) used)
    endif
    ifneq ($(LLVM_KEY_COUNT), $(words $(LLVM_SYMBOLS)))
      $(error Incompatible Clang version '$(LLVM_VERSION)': Not all wrapped symbols defined!)
    endif
    ifeq ($(_TARGET),macosx_x86_64) # on macOS the symbols have an additional '_'
      LLVM_SYMBOLS_SEMA := $(filter-out __ZN5clang12Preprocessor3LexERNS_5TokenE __ZN5clang12Preprocessor9PeekAheadEj, $(LLVM_SYMBOLS))
      LLVM_SYMBOLS_LEX  := $(filter __ZN5clang12Preprocessor3LexERNS_5TokenE __ZN5clang12Preprocessor9PeekAheadEj, $(LLVM_SYMBOLS))
    endif
  endif
endif

# linker settings
ifneq ($(_TARGET),macosx_x86_64)
ifeq ($(SHARED),)
  LDFLAGS += -static
endif
ifeq ($(FRONTEND),Puma)
	LDFLAGS += -Wl,-Bstatic -L$(PUMA)/lib/$(TARGET) -lPuma 
ifneq ($(SHARED),)
	LDFLAGS += -Wl,-Bdynamic
endif
else # FRONTEND is Clang
	LDFLAGS += -L$(PUMA)/lib/$(TARGET) -lMiniPuma 
ifneq ($(SHARED),)
	LDFLAGS += #-Wl,-Bdynamic
endif
endif
else
ifeq ($(FRONTEND),Puma)
	LDFLAGS += -L$(PUMA)/lib/$(TARGET) -lPuma
else # FRONTEND is Clang
	LDFLAGS += -L$(PUMA)/lib/$(TARGET) -lMiniPuma
endif
endif

LDFLAGS += $(LIBXML2_LIB)
ifneq ($(FRONTEND),Puma)
ifneq ($(_TARGET),macosx_x86_64)
  LDFLAGS += -lclangRewriteFrontend -lclangRewriteCore -lclangFrontend -lclangSerialization -lclangDriver -lclangParse -lclangSema -lclangAnalysis -lclangEdit -lclangAST -lclangLex -lclangBasic -lLLVMAsmParser -lLLVMMCParser -lLLVMBitReader -lLLVMTransformUtils -lLLVMCore -lLLVMMC -lLLVMOption -lLLVMSupport
  LDFLAGS += $(patsubst %,-Xlinker --wrap=%,$(LLVM_SYMBOLS))
  LDFLAGS += `$(LLVMCONF) --ldflags`
else
# Mac OS X is special, we patch the clang libraries below.
  LDFLAGS += libclangSema.o libclangLex.o -lclangRewriteFrontend -lclangRewriteCore -lclangFrontend -lclangSerialization -lclangDriver -lclangParse -lclangAnalysis -lclangEdit -lclangAST -lclangBasic -lLLVMAsmParser -lLLVMMCParser -lLLVMBitReader -lLLVMTransformUtils -lLLVMCore -lLLVMMC -lLLVMOption -lLLVMSupport
  LDFLAGS += `$(LLVMCONF) --ldflags`
endif
ifneq ($(LLVM_VERSION),3.4)
ifneq ($(LLVM_VERSION),3.4svn)
ifneq ($(LLVM_VERSION),3.4.2)
  LDFLAGS := $(patsubst -lclangRewriteCore, -lclangRewrite, $(LDFLAGS))
  LDFLAGS += `$(LLVMCONF) --system-libs` # in 3.6 ldflags doesn't contain the system libs anymore
  ifneq ($(filter 3.9%, $(LLVM_VERSION)),)
    # Additional libraries needed:
    LDFLAGS += `$(LLVMCONF) --libs`
  endif
endif
endif
endif
endif

# compiler settings
ifeq ($(WIN),yes)
	RES := windres
	RCFILES := win-ac++.rc
endif

CPPFLAGS += -I$(PUMA)/extern -I$(PUMA)/include $(LIBXML2_INC)
ifeq ($(FRONTEND),Puma)
  CPPFLAGS += -DFRONTEND_PUMA
else
  FILTEROUT :=
ifeq ($(RELEASE),debug)
  # filter-out "-DNDEBUG" added by LLVMCONF, if LLVM is in built in release-mode
  FILTEROUT := -DNDEBUG
endif
  LLVM_CPPFLAGS := $(shell $(LLVMCONF) --cppflags)
  ifeq ($(WIN),yes)
    # llvm-config on Windows generates path name with '\' that mingw g++ can't handle properly
    LLVM_CPPFLAGS := $(subst \,/,$(LLVM_CPPFLAGS))
  endif
  CPPFLAGS += $(filter-out $(FILTEROUT),$(LLVM_CPPFLAGS))
  CPPFLAGS += -DFRONTEND_CLANG=$(LLVM_INTERFACE)
  CXXFLAGS += -fno-rtti -Wno-strict-aliasing # hide annoying warning in Clang header 
  CXXFLAGS += -std=gnu++11 # Clang uses C++11 features; "c++11" does not work -> WIN32 not defined on Windows!
endif

# profiling
ifneq ($(GPROF),)
	LDFLAGS := $(LDFLAGS) -lc_p -lm_p 
	CXXFLAGS += -pg
endif

ifneq ($(PROFILING),)
	CPPFLAGS += -DPROFILING
endif

# use libacmodel
LDFLAGS  += -LACModel/lib/$(TARGET) -lacmodel 


# ******************************
# *       S O U R C E S        *
# ******************************

BINDIR := bin/$(TARGET)

#programm to compile
PROG := $(BINDIR)/ac++$(EXT)

# sources; front-end specific files for the wrong frontend are filtered out
CCSOURCES := $(wildcard *.cc)
ifeq ($(FRONTEND),Puma)
CCSOURCES := $(filter-out Clang%.cc, $(CCSOURCES))
else
CCSOURCES := $(filter-out Puma%.cc, $(CCSOURCES))
endif

OBJECTDIR := ObjFiles/$(TARGET)
OBJECTS := $(addprefix $(OBJECTDIR)/,$(CCSOURCES:.cc=.o) $(RCFILES:.rc=.o))

DEPDIR := DepFiles/$(TARGET)
DEPS := $(addprefix $(DEPDIR)/,$(CCSOURCES:.cc=.d))

DIRS := $(OBJECTDIR) $(DEPDIR) $(BINDIR)

# ******************************
# *  E X A M P L E  / T E S T  *
# ******************************

# tool selection
export AC := $(shell pwd)/$(PROG)
MAKE ?= make

# ac++ settings for tests
ACFLAGS := -k -v 9

ACEXAMPLES = coverage helloworld modules profiling singleton threads


.PHONY: $(ACEXAMPLES) 


# ******************************
# *      T A R G E T S         *
# ******************************

all: libacmodel showtarget $(PROG)
#.NOTPARALLEL: all

strip: $(PROG)
	@$(STRIP) $<

showtarget:
	@echo "---"
	@echo "Making AspectC++ for TARGET=$(TARGET)"
	@echo "  Compiler = $(CXX) $(CPPFLAGS) $(CXXFLAGS)"
	@echo "  Linker   = $(CXX) $(CXXFLAGS) <objects> $(LDFLAGS)"
	@echo "---"


clean:  testclean exampleclean
	@echo Making it clean.
	@$(MAKE) -C ACModel clean
	@rm -rf  core core.* *~ $(PROG) $(OBJECTDIR) $(DEPDIR) $(ERROR_FILE)

cleanall:	doxygen-clean clean

test: $(PROG)$(EXT) testall

example: $(PROG)$(EXT) $(addsuffix .example_make, $(ACEXAMPLES))

examplerun: example $(addsuffix .example_run, $(ACEXAMPLES))

exampleclean:
	@rm -rf examples/*-out
	@rm -rf examples/*.acp

doxygen:
	@echo "Making AspectC++ documentation with doxygen"
	@doxygen doc/doxygen.conf && echo "=> see doc/doxygen/html/index.html"

doxygen-clean:
	@rm -rf doc/doxygen

.PHONY: all test clean cleanall example examplerun exampleclean \
        showtarget doxygen doxygen-clean libacmodel



# ******************************
# *       R U L E S            *
# ******************************




$(DIRS) :
	@mkdir -p $@

libacmodel :
	@$(MAKE) -C ACModel

$(PROG): $(OBJECTS) | libacmodel
# For clang on macosx_x86_64 we create relinked objects with the patched symbols.
ifneq ($(FRONTEND),Puma)
ifeq ($(_TARGET),macosx_x86_64)
	@echo "Creating patched clang libraries ..."
	@$(LD) -r `$(LLVMCONF) --libdir`/libclangSema.a -o libclangSema.o -all_load \
          $(foreach SYM, $(LLVM_SYMBOLS_SEMA), -alias $(SYM) ___real$(SYM) -unexported_symbol $(SYM))
	@$(LD) -r `$(LLVMCONF) --libdir`/libclangLex.a -o libclangLex.o -all_load \
          $(foreach SYM, $(LLVM_SYMBOLS_LEX), -alias $(SYM) ___real$(SYM) -unexported_symbol $(SYM))
endif
endif
	@echo "Linking $@."
	@mkdir -p $(BINDIR)
	@$(CXX) -o $@ $(CXXFLAGS) $^ $(LDFLAGS)


$(OBJECTDIR)/%.o : %.cc
	@echo Making dependency file $(DEPDIR)/$(<:%.cc=%.d).
	@mkdir -p $(DEPDIR)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MM -MP -MT $(OBJECTDIR)/$(<:%.cc=%.o) -MT $(DEPDIR)/$(<:%.cc=%.d) $< > $(DEPDIR)/$(<:%.cc=%.d)
	@echo Making object file $@.
	@mkdir -p $(OBJECTDIR)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

# -Wold-style-cast to find classic C casts "a = (A*)b;"

ifeq ($(WIN),yes)
$(OBJECTDIR)/%.o : %.rc
	@echo Compiling Windows resource file
	@mkdir -p $(OBJECTDIR)
	@$(RES) $(filter -D%,$(CPPFLAGS)) $< $@

endif

test%: 
	$(MAKE) -C tests -s  $*

%.example_make: %
	cd examples/$< ; \
	$(AC) $(ACFLAGS) -I. -p . -d ../$<-out \
	-r ../$<.acp ; \
	cp Makefile ../$<-out/Makefile; \
	cd ../$<-out; g++ -o $< *.cc -lpthread

%.example_run:	%.example_make
	@echo ""
	@echo ---- Running $* ----
	@cd examples/$*-out; `find . -type f -perm -700`

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif
