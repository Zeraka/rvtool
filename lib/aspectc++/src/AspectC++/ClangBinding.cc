#include "clang/Basic/Version.h"

#if CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)
#elif CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2
#elif CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 6 && CLANG_VERSION_PATCHLEVEL == 2
#elif CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 7 && CLANG_VERSION_PATCHLEVEL == 1
#elif CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 8 && CLANG_VERSION_PATCHLEVEL == 0
#elif CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 9 && CLANG_VERSION_PATCHLEVEL == 1
#else
#warning "Invalid clang version used! Only 3.4, 3.4.2, 3.6.2, 3.7.1, 3.8.0, and 3.9.1 are supported by this code."
#endif

#include "ClangIntroSema.h"
#include "ClangWeaverBase.h"
using namespace clang;

// first define some helper macros to avoid code duplication

#ifdef __APPLE__
#define WRAP(x) x
#else
#define WRAP(x) __wrap_##x
#endif

#define GEN_SEMA_WRAPPER_WITH_RESULT(result_type, func_name, mangled, param_list, ...) \
  extern "C" result_type WRAP(mangled) param_list; \
  result_type WRAP(mangled) param_list { \
    ClangIntroSema &sema = (ClangIntroSema&)that; \
    return sema.func_name(__VA_ARGS__); \
  } \
  extern "C" result_type __real_##mangled param_list; \
  result_type Real##func_name param_list { \
    return __real_##mangled(that, __VA_ARGS__); \
  }

#define GEN_SEMA_WRAPPER_NO_RESULT(func_name, mangled, param_list, ...) \
  extern "C" void WRAP(mangled) param_list; \
  void WRAP(mangled) param_list { \
    ClangIntroSema &sema = (ClangIntroSema&)that; \
    sema.func_name(__VA_ARGS__); \
  } \
  extern "C" void __real_##mangled param_list; \
  void Real##func_name param_list { \
    __real_##mangled(that, __VA_ARGS__); \
  }

#define GEN_SEMA_WRAPPER_NO_RESULT_NO_ARG(func_name, mangled) \
  extern "C" void WRAP(mangled) (Sema &that); \
  void WRAP(mangled) (Sema &that) { \
    ClangIntroSema &sema = (ClangIntroSema&)that; \
    sema.func_name(); \
  } \
  extern "C" void __real_##mangled (Sema &that); \
  void Real##func_name (Sema &that) { \
    __real_##mangled(that); \
  }

// wrapper for Sema::ActOnStartCXXMemberDeclarations
GEN_SEMA_WRAPPER_NO_RESULT(ActOnStartCXXMemberDeclarations, _ZN5clang4Sema31ActOnStartCXXMemberDeclarationsEPNS_5ScopeEPNS_4DeclENS_14SourceLocationEbS5_, \
    (Sema &that, Scope *S, Decl *TagDecl, SourceLocation FinalLoc, bool IsFinalSpelledSealed, SourceLocation LBraceLoc), \
    S, TagDecl, FinalLoc, IsFinalSpelledSealed, LBraceLoc)

// wrapper for Sema::ActOnBaseSpecifiers
#if FRONTEND_CLANG >= 38
GEN_SEMA_WRAPPER_NO_RESULT(ActOnBaseSpecifiers, _ZN5clang4Sema19ActOnBaseSpecifiersEPNS_4DeclEN4llvm15MutableArrayRefIPNS_16CXXBaseSpecifierEEE, \
    (clang::Sema &that, clang::Decl *ClassDecl, clang::MutableArrayRef<clang::CXXBaseSpecifier *> Bases), \
    ClassDecl, Bases)
#else
GEN_SEMA_WRAPPER_NO_RESULT(ActOnBaseSpecifiers, _ZN5clang4Sema19ActOnBaseSpecifiersEPNS_4DeclEPPNS_16CXXBaseSpecifierEj, \
    (clang::Sema &that, clang::Decl *ClassDecl, clang::CXXBaseSpecifier **Bases, unsigned NumBases), \
    ClassDecl, Bases, NumBases)
#endif

// wrapper for Sema::ActOnStartOfFunctionDef
#if FRONTEND_CLANG >= 38
GEN_SEMA_WRAPPER_WITH_RESULT(clang::Decl*, ActOnStartOfFunctionDef, _ZN5clang4Sema23ActOnStartOfFunctionDefEPNS_5ScopeERNS_10DeclaratorEN4llvm15MutableArrayRefIPNS_21TemplateParameterListEEEPNS0_12SkipBodyInfoE, \
    (clang::Sema &that, clang::Scope *S, clang::Declarator &D, clang::MultiTemplateParamsArg TemplateParamLists, clang::Sema::SkipBodyInfo *SkipBody), S, D, TemplateParamLists, SkipBody)
#else
GEN_SEMA_WRAPPER_WITH_RESULT(clang::Decl*, ActOnStartOfFunctionDef, _ZN5clang4Sema23ActOnStartOfFunctionDefEPNS_5ScopeERNS_10DeclaratorE, \
    (clang::Sema &that, clang::Scope *S, clang::Declarator &D), S, D)
#endif

// wrapper for Sema::ActOnStartNamespaceDef
#if FRONTEND_CLANG >= 38
GEN_SEMA_WRAPPER_WITH_RESULT(clang::Decl*, ActOnStartNamespaceDef, _ZN5clang4Sema22ActOnStartNamespaceDefEPNS_5ScopeENS_14SourceLocationES3_S3_PNS_14IdentifierInfoES3_PNS_13AttributeListERPNS_18UsingDirectiveDeclE, \
    (clang::Sema &that, clang::Scope *S, clang::SourceLocation InlineLoc, clang::SourceLocation NamespaceLoc,
    clang::SourceLocation IdentLoc, clang::IdentifierInfo *Ident, clang::SourceLocation LBrace, clang::AttributeList *AttrList, clang::UsingDirectiveDecl * &UsingDecl), \
    S, InlineLoc, NamespaceLoc, IdentLoc, Ident, LBrace, AttrList, UsingDecl)
#else
GEN_SEMA_WRAPPER_WITH_RESULT(clang::Decl*, ActOnStartNamespaceDef, _ZN5clang4Sema22ActOnStartNamespaceDefEPNS_5ScopeENS_14SourceLocationES3_S3_PNS_14IdentifierInfoES3_PNS_13AttributeListE, \
    (clang::Sema &that, clang::Scope *S, clang::SourceLocation InlineLoc, clang::SourceLocation NamespaceLoc,
    clang::SourceLocation IdentLoc, clang::IdentifierInfo *Ident, clang::SourceLocation LBrace, clang::AttributeList *AttrList), \
    S, InlineLoc, NamespaceLoc, IdentLoc, Ident, LBrace, AttrList)
#endif

// wrapper for Decl *Sema::ActOnTag
#if FRONTEND_CLANG >= 37
GEN_SEMA_WRAPPER_WITH_RESULT(clang::Decl*, ActOnTag, \
    _ZN5clang4Sema8ActOnTagEPNS_5ScopeEjNS0_10TagUseKindENS_14SourceLocationERNS_12CXXScopeSpecEPNS_14IdentifierInfoES4_PNS_13AttributeListENS_15AccessSpecifierES4_N4llvm15MutableArrayRefIPNS_21TemplateParameterListEEERbSH_S4_bNS_12ActionResultINS_9OpaquePtrINS_8QualTypeEEELb0EEEbPNS0_12SkipBodyInfoE, \
    (clang::Sema &that, clang::Scope *S, unsigned TagSpec, clang::Sema::TagUseKind TUK, clang::SourceLocation KWLoc, clang::CXXScopeSpec &SS, \
    clang::IdentifierInfo *Name, clang::SourceLocation NameLoc, clang::AttributeList *Attr, clang::AccessSpecifier AS, \
    clang::SourceLocation ModulePrivateLoc, clang::MultiTemplateParamsArg TemplateParameterLists, \
    bool &OwnedDecl, bool &IsDependent, clang::SourceLocation ScopedEnumKWLoc, \
    bool ScopedEnumUsesClassTag, clang::TypeResult UnderlyingType, bool IsTypeSpecifier, clang::Sema::SkipBodyInfo *SkipBody), \
    S, TagSpec, TUK, KWLoc, SS, Name, NameLoc, Attr, AS, ModulePrivateLoc, TemplateParameterLists, \
    OwnedDecl, IsDependent, ScopedEnumKWLoc, ScopedEnumUsesClassTag, UnderlyingType, IsTypeSpecifier, SkipBody)
#elif FRONTEND_CLANG >= 35
GEN_SEMA_WRAPPER_WITH_RESULT(clang::Decl*, ActOnTag, \
    _ZN5clang4Sema8ActOnTagEPNS_5ScopeEjNS0_10TagUseKindENS_14SourceLocationERNS_12CXXScopeSpecEPNS_14IdentifierInfoES4_PNS_13AttributeListENS_15AccessSpecifierES4_N4llvm15MutableArrayRefIPNS_21TemplateParameterListEEERbSH_S4_bNS_12ActionResultINS_9OpaquePtrINS_8QualTypeEEELb0EEEb, \
    (clang::Sema &that, clang::Scope *S, unsigned TagSpec, clang::Sema::TagUseKind TUK, clang::SourceLocation KWLoc, clang::CXXScopeSpec &SS, \
    clang::IdentifierInfo *Name, clang::SourceLocation NameLoc, clang::AttributeList *Attr, clang::AccessSpecifier AS, \
    clang::SourceLocation ModulePrivateLoc, clang::MultiTemplateParamsArg TemplateParameterLists, \
    bool &OwnedDecl, bool &IsDependent, clang::SourceLocation ScopedEnumKWLoc, \
    bool ScopedEnumUsesClassTag, clang::TypeResult UnderlyingType, bool IsTypeSpecifier), \
    S, TagSpec, TUK, KWLoc, SS, Name, NameLoc, Attr, AS, ModulePrivateLoc, TemplateParameterLists, \
    OwnedDecl, IsDependent, ScopedEnumKWLoc, ScopedEnumUsesClassTag, UnderlyingType, IsTypeSpecifier)
#else
GEN_SEMA_WRAPPER_WITH_RESULT(clang::Decl*, ActOnTag, \
    _ZN5clang4Sema8ActOnTagEPNS_5ScopeEjNS0_10TagUseKindENS_14SourceLocationERNS_12CXXScopeSpecEPNS_14IdentifierInfoES4_PNS_13AttributeListENS_15AccessSpecifierES4_N4llvm15MutableArrayRefIPNS_21TemplateParameterListEEERbSH_S4_bNS_12ActionResultINS_9OpaquePtrINS_8QualTypeEEELb0EEE, \
    (clang::Sema &that, clang::Scope *S, unsigned TagSpec, clang::Sema::TagUseKind TUK, clang::SourceLocation KWLoc, clang::CXXScopeSpec &SS, \
    clang::IdentifierInfo *Name, clang::SourceLocation NameLoc, clang::AttributeList *Attr, clang::AccessSpecifier AS, \
    clang::SourceLocation ModulePrivateLoc, clang::MultiTemplateParamsArg TemplateParameterLists, \
    bool &OwnedDecl, bool &IsDependent, clang::SourceLocation ScopedEnumKWLoc, \
    bool ScopedEnumUsesClassTag, clang::TypeResult UnderlyingType), \
    S, TagSpec, TUK, KWLoc, SS, Name, NameLoc, Attr, AS, ModulePrivateLoc, TemplateParameterLists, \
    OwnedDecl, IsDependent, ScopedEnumKWLoc, ScopedEnumUsesClassTag, UnderlyingType)
#endif

// wrapper for Sema::ActOnFinishCXXMemberSpecification
GEN_SEMA_WRAPPER_NO_RESULT(ActOnFinishCXXMemberSpecification, \
    _ZN5clang4Sema33ActOnFinishCXXMemberSpecificationEPNS_5ScopeENS_14SourceLocationEPNS_4DeclES3_S3_PNS_13AttributeListE, \
    (clang::Sema &that, clang::Scope* S, clang::SourceLocation RLoc, Decl *TagDecl, \
        clang::SourceLocation LBrac, clang::SourceLocation RBrac, clang::AttributeList *AttrList), \
        S, RLoc, TagDecl, LBrac, RBrac, AttrList)

// wrapper for Sema::ActOnEndOfTranslationUnit
GEN_SEMA_WRAPPER_NO_RESULT_NO_ARG(ActOnEndOfTranslationUnit, _ZN5clang4Sema25ActOnEndOfTranslationUnitEv)

// wrapper for Sema::canSkipFunctionBody
GEN_SEMA_WRAPPER_WITH_RESULT(bool, canSkipFunctionBody, _ZN5clang4Sema19canSkipFunctionBodyEPNS_4DeclE, \
    (clang::Sema &that, clang::Decl *D), D)

#if 0 // these wrapper are hopefully not needed
// wrapper for Sema::ActOnFunctionDeclarator
GEN_SEMA_WRAPPER_WITH_RESULT(clang::NamedDecl*, ActOnFunctionDeclarator, \
    _ZN5clang4Sema23ActOnFunctionDeclaratorEPNS_5ScopeERNS_10DeclaratorEPNS_11DeclContextEPNS_14TypeSourceInfoERNS_12LookupResultEN4llvm15MutableArrayRefIPNS_21TemplateParameterListEEERb, \
    (clang::Sema &that, clang::Scope* S, clang::Declarator& D, \
        clang::DeclContext* DC, clang::TypeSourceInfo *TInfo, clang::LookupResult &Previous, \
        clang::MultiTemplateParamsArg TemplateParamLists, bool &AddToScope), \
        S, D, DC, TInfo, Previous, TemplateParamLists, AddToScope)

// wrapper for void Sema::ProcessDeclAttributeList(Scope *S, Decl *D, const AttributeList *AL, bool IncludeCXX11Attributes = true);
GEN_SEMA_WRAPPER_NO_RESULT(ProcessDeclAttributeList, _ZN5clang4Sema24ProcessDeclAttributeListEPNS_5ScopeEPNS_4DeclEPKNS_13AttributeListEb, \
    (clang::Sema &that, clang::Scope *S, clang::Decl *D, const clang::AttributeList *AL, bool IncludeCXX11Attributes), \
    S, D, AL, IncludeCXX11Attributes)
#endif

// wrapper for NamedDecl *Sema::ActOnCXXMemberDeclarator
GEN_SEMA_WRAPPER_WITH_RESULT(clang::NamedDecl*, ActOnCXXMemberDeclarator, \
    _ZN5clang4Sema24ActOnCXXMemberDeclaratorEPNS_5ScopeENS_15AccessSpecifierERNS_10DeclaratorEN4llvm15MutableArrayRefIPNS_21TemplateParameterListEEEPNS_4ExprERKNS_14VirtSpecifiersENS_16InClassInitStyleE, \
    (clang::Sema &that, clang::Scope *S, clang::AccessSpecifier AS, clang::Declarator &D, \
        clang::MultiTemplateParamsArg TemplateParameterLists, clang::Expr *BitfieldWidth, const clang::VirtSpecifiers &VS, \
        clang::InClassInitStyle InitStyle), S, AS, D, TemplateParameterLists, BitfieldWidth, VS, InitStyle)

// wrapper for Sema::ActOnDeclarator
GEN_SEMA_WRAPPER_WITH_RESULT(clang::Decl*, ActOnDeclarator, _ZN5clang4Sema15ActOnDeclaratorEPNS_5ScopeERNS_10DeclaratorE, \
    (clang::Sema &that, clang::Scope *S, clang::Declarator &D), S, D)


// Special wrapper for Preprocessor::Lex
WeaverBase *macro_recorder = 0;
int in_peek_ahead = 0;
extern "C" void __real__ZN5clang12Preprocessor3LexERNS_5TokenE(clang::Preprocessor &that, clang::Token &Result);
extern "C" void WRAP(_ZN5clang12Preprocessor3LexERNS_5TokenE)(clang::Preprocessor &that, clang::Token &Result);
void WRAP(_ZN5clang12Preprocessor3LexERNS_5TokenE)(clang::Preprocessor &that, clang::Token &Result) {
  static int lex_nested = 0;
  lex_nested++;
  if (macro_recorder && in_peek_ahead == 0 && lex_nested == 1)
    macro_recorder->lex_and_filter_attr(that, Result);
  else {
//    cout << "real instead of filtered" << endl;
    __real__ZN5clang12Preprocessor3LexERNS_5TokenE(that, Result);
  }
  lex_nested--;
  if (!lex_nested && macro_recorder) {
    macro_recorder->handle_token (that, Result);
  }
}

extern "C" const clang::Token &__real__ZN5clang12Preprocessor9PeekAheadEj(clang::Preprocessor &that, unsigned N);
extern "C" const clang::Token & WRAP(_ZN5clang12Preprocessor9PeekAheadEj)(clang::Preprocessor &that, unsigned N);
const clang::Token & WRAP(_ZN5clang12Preprocessor9PeekAheadEj)(clang::Preprocessor &that, unsigned N) {
  in_peek_ahead++;
//  cout << "peek " << N << endl;
  const clang::Token &result = __real__ZN5clang12Preprocessor9PeekAheadEj(that, N);
//  cout << "peek end " << N << endl;
  in_peek_ahead--;
  return result;
}
