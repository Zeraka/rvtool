#-------------------------------------------------
# variables 
#-------------------------------------------------

ifneq ($(MINI),)

# libMiniPuma subset of Puma

SOURCES := \
	basics/DString.cc \
	basics/KeyboardSource.cc \
	basics/List.cc \
	basics/MemPool.cc \
	basics/Pipe.cc \
	basics/PipedCmd.cc \
	basics/RegComp.cc \
	basics/StrCol.cc \
	basics/StrHashTable.cc \
	basics/SysCall.cc \
	basics/TerminalSink.cc \
	basics/VerboseMgr.cc \
	basics/VoidPtrArray.cc \
	basics/WChar.cc

SOURCES += \
	common/Config.cc \
	common/ErrorCollector.cc \
	common/ErrorSeverity.cc \
	common/ErrorSink.cc \
	common/ErrorStream.cc \
	common/FilenameInfo.cc \
	common/OptsParser.cc \
	common/PathInfo.cc \
	common/PathIterator.cc \
	common/PathManager.cc \
	common/SimpleParser.cc

CSOURCES :=

ifeq ($(_TARGET),win32)
SOURCES += \
	basics/Heap.cc \
	win32/ptmalloc/sbrk.cc

CSOURCES += \
	win32/ptmalloc/ptmalloc.c
endif

DONTWEAVE := $(SOURCES) $(CSOURCES)

else

# full set of Puma source files

ASPECTS := \
	aspects/CCExprResolveCC.ah \
	aspects/CCExprResolveH.ah \
	aspects/CExprResolveCC.ah \
	aspects/CExprResolveH.ah \
	parser/SyntaxState.ah \
	parser/SyntaxBuilder.ah \
	parser/LookAhead.ah \
	parser/cparser/CBuilderExtension.ah \
	parser/cparser/CLookAhead.ah \
	parser/cparser/CSemBinding.ah \
	parser/ccparser/CCBuilderExtension.ah \
	parser/ccparser/CCLookAhead.ah \
	parser/ccparser/CCSemBinding.ah

SOURCES := \
	cpp/ImportHandler.cc \
	cpp/PreAnswer.cc \
	cpp/PreAssertionEvaluator.cc \
	cpp/PreMacro.cc \
	cpp/PreMacroExpander.cc \
	cpp/PreMacroManager.cc \
	cpp/PreParserState.cc \
	cpp/PrePredicate.cc \
	cpp/PrePredicateManager.cc \
	cpp/PrePrintVisitor.cc \
	cpp/PreTree.cc \
	cpp/PreTreeComposite.cc \
	cpp/PreFileIncluder.cc \
	cpp/PreprocessorParser.cc \
	common/OptsParser.cc \
	common/SimpleParser.cc \
	common/Config.cc \
	common/ErrorCollector.cc \
	common/ErrorSeverity.cc \
	common/ErrorSink.cc \
	common/ErrorStream.cc \
	common/FileUnit.cc \
	common/FilenameInfo.cc \
	common/PathInfo.cc \
	common/PathIterator.cc \
	common/PathManager.cc \
	common/Project.cc \
	common/Token.cc \
	common/TokenStream.cc \
	common/Unit.cc \
	common/MacroUnit.cc \
	common/UnitIterator.cc \
	common/UnitManager.cc \
	manip/ManipCommander.cc \
	manip/ManipController.cc \
	manip/Manipulators.cc \
	manip/RuleTable.cc \
	scanner/CScanner.cc \
	scanner/CLexer.cc \
	scanner/CCLexer.cc \
	basics/DString.cc \
	basics/KeyboardSource.cc \
	basics/List.cc \
	basics/MemPool.cc \
	basics/Pipe.cc \
	basics/PipedCmd.cc \
	basics/RegComp.cc \
	basics/StrCol.cc \
	basics/StrHashTable.cc \
	basics/SysCall.cc \
	basics/TerminalSink.cc \
	basics/VerboseMgr.cc \
	basics/VoidPtrArray.cc \
	basics/WChar.cc \
	basics/GPLHeader.cc \
	parser/ccparser/CCAssocScopes.cc \
	parser/ccparser/CCBuilder.cc \
	parser/ccparser/CCConvSeq.cc \
	parser/ccparser/CCConversion.cc \
	parser/ccparser/CCConversions.cc \
	parser/ccparser/CCNameLookup.cc \
	parser/ccparser/CCOverloading.cc \
	parser/ccparser/CCSemExpr.cc \
	parser/ccparser/CCSemVisitor.cc \
	parser/ccparser/CCSemantic.cc \
	parser/ccparser/CCSyntax.cc \
	parser/ccparser/instantiation/CCInstantiation.cc \
	parser/ccparser/instantiation/DeducedArgument.cc \
	parser/ccparser/instantiation/InstantiationCandidate.cc \
	parser/ccparser/instantiation/InstantiationParser.cc \
	parser/ccparser/instantiation/InstantiationSyntax.cc \
	parser/ccparser/instantiation/InstantiationSemantic.cc \
	parser/ccparser/instantiation/InstantiationTokenSource.cc \
	parser/ccparser/instantiation/InstantiationTokenProvider.cc \
	parser/cparser/CBuilder.cc \
	parser/cparser/CTree.cc \
	parser/cparser/CSemExpr.cc \
	parser/cparser/CSemDeclarator.cc \
	parser/cparser/CSyntax.cc \
	parser/cparser/CSemantic.cc \
	parser/cparser/CSemVisitor.cc \
	parser/cparser/CSemDeclSpecs.cc \
	parser/cparser/CProject.cc \
	parser/cparser/CPrintVisitor.cc \
	parser/cparser/CTranslationUnit.cc \
	parser/cparser/CUnit.cc \
	parser/cparser/Utf8.cc \
	parser/Syntax.cc \
	parser/Builder.cc \
	parser/CConstant.cc \
	parser/CVisitor.cc \
	parser/Parser.cc \
	parser/Semantic.cc \
	infos/CArgumentInfo.cc \
	infos/CAttributeInfo.cc \
	infos/CBaseClassInfo.cc \
	infos/CSemDatabase.cc \
	infos/CClassInfo.cc \
	infos/CEnumInfo.cc \
	infos/CEnumeratorInfo.cc \
	infos/CFunctionInfo.cc \
	infos/CFctInstance.cc \
	infos/CLabelInfo.cc \
	infos/CLocalScope.cc \
	infos/CMemberAliasInfo.cc \
	infos/CNamespaceInfo.cc \
	infos/CObjectInfo.cc \
	infos/CRecord.cc \
	infos/CScopeInfo.cc \
	infos/CSourceInfo.cc \
	infos/CStructure.cc \
	infos/CTemplateInfo.cc \
	infos/CTemplateInstance.cc \
	infos/CTemplateParamInfo.cc \
	infos/types/CTypeInfo.cc \
	infos/CTypedefInfo.cc \
	infos/CUnionInfo.cc \
	infos/CUsingInfo.cc

CSOURCES :=

DONTWEAVE := \
	cpp/ImportHandler.cc \
	cpp/PreAnswer.cc \
	cpp/PreAssertionEvaluator.cc \
	cpp/PreMacro.cc \
	cpp/PreMacroExpander.cc \
	cpp/PreMacroManager.cc \
	cpp/PreParserState.cc \
	cpp/PrePredicate.cc \
	cpp/PrePredicateManager.cc \
	cpp/PreTree.cc \
	cpp/PreTreeComposite.cc \
	common/OptsParser.cc \
	common/Config.cc \
	common/SimpleParser.cc \
	common/ErrorCollector.cc \
	common/ErrorSeverity.cc \
	common/ErrorSink.cc \
	common/ErrorStream.cc \
	common/FileUnit.cc \
	common/FilenameInfo.cc \
	common/PathInfo.cc \
	common/PathIterator.cc \
	common/PathManager.cc \
	common/Project.cc \
	common/Token.cc \
	common/TokenStream.cc \
	common/Unit.cc \
	common/MacroUnit.cc \
	common/UnitIterator.cc \
	manip/ManipCommander.cc \
	manip/ManipController.cc \
	manip/Manipulators.cc \
	manip/RuleTable.cc \
	basics/DString.cc \
	basics/KeyboardSource.cc \
	basics/List.cc \
	basics/MemPool.cc \
	basics/Pipe.cc \
	basics/PipedCmd.cc \
	basics/RegComp.cc \
	basics/StrCol.cc \
	basics/StrHashTable.cc \
	basics/SysCall.cc \
	basics/TerminalSink.cc \
	basics/VerboseMgr.cc \
	basics/VoidPtrArray.cc \
	basics/WChar.cc \
	basics/GPLHeader.cc \
	parser/ccparser/CCAssocScopes.cc \
	parser/ccparser/CCConvSeq.cc \
	parser/ccparser/CCConversion.cc \
	parser/ccparser/CCConversions.cc \
	parser/ccparser/CCOverloading.cc \
	parser/ccparser/instantiation/CCInstantiation.cc \
	parser/ccparser/instantiation/DeducedArgument.cc \
	parser/ccparser/instantiation/InstantiationCandidate.cc \
	parser/ccparser/instantiation/InstantiationSemantic.cc \
	parser/ccparser/instantiation/InstantiationTokenSource.cc \
	parser/ccparser/instantiation/InstantiationTokenProvider.cc \
	parser/cparser/CSemDeclarator.cc \
	parser/cparser/CTranslationUnit.cc \
	parser/cparser/CUnit.cc \
	parser/cparser/Utf8.cc \
	parser/Builder.cc \
	parser/CConstant.cc \
	parser/CVisitor.cc \
	parser/Semantic.cc \
	infos/CArgumentInfo.cc \
	infos/CAttributeInfo.cc \
	infos/CBaseClassInfo.cc \
	infos/CClassInfo.cc \
	infos/CEnumInfo.cc \
	infos/CEnumeratorInfo.cc \
	infos/CFctInstance.cc \
	infos/CLabelInfo.cc \
	infos/CLocalScope.cc \
	infos/CMemberAliasInfo.cc \
	infos/CNamespaceInfo.cc \
	infos/CObjectInfo.cc \
	infos/CRecord.cc \
	infos/CScopeInfo.cc \
	infos/CSourceInfo.cc \
	infos/CStructure.cc \
	infos/CTemplateInfo.cc \
	infos/CTemplateInstance.cc \
	infos/CTemplateParamInfo.cc \
	infos/types/CTypeInfo.cc \
	infos/CTypedefInfo.cc \
	infos/CUnionInfo.cc \
	infos/CUsingInfo.cc

LEMSOURCES := \
	cpp/PreExprParser.lem \
	cpp/PreParser.lem

ifeq ($(_TARGET),win32)
SOURCES += \
	basics/Heap.cc \
	win32/ptmalloc/sbrk.cc

CSOURCES += \
	win32/ptmalloc/ptmalloc.c
endif

DONTWEAVE += \
	basics/Heap.cc \
	win32/ptmalloc/ptmalloc.c \
	win32/ptmalloc/sbrk.cc

include extensions.mk

endif
