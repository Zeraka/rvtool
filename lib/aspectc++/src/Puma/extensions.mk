#-------------------------------------------------
# extensions
#-------------------------------------------------


EXTENSIONS ?= gnuext winext acppext cc1xext
#EXTENSIONS ?= gnuext winext acppext cc1xext tracing matchexpr profiling


# No extensions
ifeq ($(EXTENSIONS),)
DONTWEAVE += \
	cpp/PreFileIncluder.cc \
	cpp/PreprocessorParser.cc \
	common/UnitManager.cc \
	scanner/CScanner.cc \
	parser/ccparser/CCBuilder.cc \
	parser/ccparser/CCNameLookup.cc \
	parser/ccparser/CCSemExpr.cc \
	parser/ccparser/CCSemantic.cc \
	parser/ccparser/CCSyntax.cc \
	parser/cparser/CBuilder.cc \
	parser/cparser/CSemExpr.cc \
	parser/cparser/CSyntax.cc \
	parser/cparser/CSemantic.cc \
	parser/cparser/CSemDeclSpecs.cc \
	parser/cparser/CProject.cc \
	parser/Syntax.cc \
	parser/Parser.cc \
	infos/CSemDatabase.cc
endif


# Win32/VisualC++ extensions
ifneq ($(findstring winext,$(EXTENSIONS)),)
ASPECTS += \
	aspects/WinIfExists.ah \
	aspects/WinImportHandler.ah \
	aspects/WinMacros.ah \
	aspects/WinAsm.ah \
	aspects/WinDeclSpecs.ah \
	aspects/WinMemberExplSpec.ah \
	aspects/WinTypeKeywords.ah \
	aspects/WinFriend.ah \
	aspects/WinKeywords.ah
endif


# AspectC++ extensions
ifneq ($(findstring acppext,$(EXTENSIONS)),)
ASPECTS += \
	aspects/ExtAC.ah \
	aspects/ExtACTree.ah

SOURCES += \
	aspects/ExtACTree.cc \
	parser/acparser/ACIntroducer_dummy.cc

DONTWEAVE += \
	aspects/ExtACTree.cc \
	parser/acparser/ACIntroducer_dummy.cc
endif


# GNU C/C++ extensions
ifneq ($(findstring gnuext,$(EXTENSIONS)),)
ASPECTS += \
	aspects/ExtGnu.ah \
	aspects/ExtGnuCTree.ah \
	aspects/ExtGnuCInfos.ah \
	aspects/ExtGnuCSemantic.ah \
	aspects/ExtGnuCSemExpr.ah \
	aspects/ExtGnuCSemDeclSpecs.ah \
	aspects/ExtGnuKeywords.ah
endif


# C++1X extensions
ifneq ($(findstring cc1xext,$(EXTENSIONS)),)
ASPECTS += \
	aspects/ExtCC1X.ah \
	aspects/ExtCC1XBuilderH.ah \
	aspects/ExtCC1XBuilderCC.ah \
	aspects/ExtCC1XSyntaxH.ah \
	aspects/ExtCC1XSyntaxCC.ah \
	aspects/ExtCC1XSemanticH.ah \
	aspects/ExtCC1XSemanticCC.ah

SOURCES += \
	aspects/CC1XTree.cc

DONTWEAVE += \
	aspects/CC1XTree.cc
endif


# #pragma once extension (if gnuext or winext is enabled)
ifneq ($(findstring gnuext,$(EXTENSIONS)),)
ASPECTS += \
	aspects/PragmaOnceUnitState.ah \
	aspects/PragmaOnce.ah
else
ifneq ($(findstring winext,$(EXTENSIONS)),)
ASPECTS += \
	aspects/PragmaOnceUnitState.ah \
	aspects/PragmaOnce.ah
endif
endif


# Grammar rule tracing
ifneq ($(findstring tracing,$(EXTENSIONS)),)
ASPECTS += \
	aspects/TraceSyntax.ah

SOURCES += \
	parser/Tracing.cc

DONTWEAVE += \
	parser/Tracing.cc
endif


# AST match expressions
ifneq ($(findstring matchexpr,$(EXTENSIONS)),)
ASPECTS += \
	aspects/CMatchSyntax.ah

SOURCES += \
	manip/CMatch.cc \
	manip/CMatchChecker.cc \
	manip/CSubMatch.cc \
	manip/CTreeMatcher.cc \
	manip/MatchCollector.cc

DONTWEAVE += \
	manip/CMatch.cc \
	manip/CMatchChecker.cc \
	manip/CSubMatch.cc \
	manip/CTreeMatcher.cc \
	manip/MatchCollector.cc

LEMSOURCES += \
	manip/CMatchParser.lem
endif


# Profiling
ifneq ($(findstring profiling,$(EXTENSIONS)),)
ASPECTS += \
	aspects/GenericProfiler.ah \
	aspects/Profiler.ah

FORCEWEAVE := \
	parser/ccparser/CCSemantic.cc \
	parser/ccparser/CCInstantiation.cc \
	parser/ccparser/CCNameLookup.cc \
	basics/SysCall.cc \
	scanner/CRecognizer.cc \
	scanner/CScanner.cc \
	cpp/PreprocessorParser.cc \
	parser/Parser.cc \
	manip/ManipCommander.cc

DONTWEAVE := $(filter-out $(FORCEWEAVE), $(DONTWEAVE))
endif
