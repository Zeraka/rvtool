#ifndef __ClangIntroSema_h__
#define __ClangIntroSema_h__

#include "clang/Sema/Sema.h"

class ClangIntroducer;

class ClangIntroSema : public clang::Sema {
  ClangIntroducer *_introducer;
  int _nested_tu_end;
  int _nested_base_specifiers;
  clang::SmallVector<clang::CXXBaseSpecifier *, 16> _BaseInfo;

public:
  ClangIntroSema(ClangIntroducer &introducer,
      clang::Preprocessor &pp, clang::ASTContext &ctxt,
      clang::ASTConsumer &consumer,
      clang::TranslationUnitKind TUKind = clang::TU_Complete,
      clang::CodeCompleteConsumer *CompletionConsumer = 0);
  ~ClangIntroSema ();

  clang::Scope *setCurScope(clang::Scope *new_scope);

  void ActOnStartCXXMemberDeclarations(clang::Scope *S, clang::Decl *TagDecl,
      clang::SourceLocation FinalLoc, bool IsFinalSpelledSealed,
      clang::SourceLocation LBraceLoc);

#if FRONTEND_CLANG >= 38
  void ActOnBaseSpecifiers (clang::Decl *ClassDecl,
      clang::MutableArrayRef<clang::CXXBaseSpecifier *> Bases);
#else
  void ActOnBaseSpecifiers (clang::Decl *ClassDecl,
      clang::CXXBaseSpecifier **Bases, unsigned NumBases);
#endif

#if FRONTEND_CLANG >= 38
  clang::Decl* ActOnStartOfFunctionDef(
      clang::Scope *S, clang::Declarator &D,
      clang::MultiTemplateParamsArg TemplateParamLists,
      clang::Sema::SkipBodyInfo *SkipBody);
#else
  clang::Decl* ActOnStartOfFunctionDef(
      clang::Scope *S, clang::Declarator &D);
#endif

#if FRONTEND_CLANG >= 38
  clang::Decl* ActOnStartNamespaceDef (
      clang::Scope *S, clang::SourceLocation InlineLoc, clang::SourceLocation NamespaceLoc,
      clang::SourceLocation IdentLoc, clang::IdentifierInfo *Ident, clang::SourceLocation LBrace,
      clang::AttributeList *AttrList, clang::UsingDirectiveDecl * &UsingDecl);
#else
  clang::Decl* ActOnStartNamespaceDef (
      clang::Scope *S, clang::SourceLocation InlineLoc, clang::SourceLocation NamespaceLoc,
      clang::SourceLocation IdentLoc, clang::IdentifierInfo *Ident, clang::SourceLocation LBrace,
      clang::AttributeList *AttrList);
#endif

#if FRONTEND_CLANG >= 37
  clang::Decl* ActOnTag (
      clang::Scope *S, unsigned TagSpec, clang::Sema::TagUseKind TUK, clang::SourceLocation KWLoc, clang::CXXScopeSpec &SS,
      clang::IdentifierInfo *Name, clang::SourceLocation NameLoc, clang::AttributeList *Attr, clang::AccessSpecifier AS,
      clang::SourceLocation ModulePrivateLoc, clang::MultiTemplateParamsArg TemplateParameterLists,
      bool &OwnedDecl, bool &IsDependent, clang::SourceLocation ScopedEnumKWLoc,
      bool ScopedEnumUsesClassTag, clang::TypeResult UnderlyingType, bool IsTypeSpecifier, clang::Sema::SkipBodyInfo *SkipBody);
#elif FRONTEND_CLANG >= 35
  clang::Decl* ActOnTag (
      clang::Scope *S, unsigned TagSpec, clang::Sema::TagUseKind TUK, clang::SourceLocation KWLoc, clang::CXXScopeSpec &SS,
      clang::IdentifierInfo *Name, clang::SourceLocation NameLoc, clang::AttributeList *Attr, clang::AccessSpecifier AS,
      clang::SourceLocation ModulePrivateLoc, clang::MultiTemplateParamsArg TemplateParameterLists,
      bool &OwnedDecl, bool &IsDependent, clang::SourceLocation ScopedEnumKWLoc,
      bool ScopedEnumUsesClassTag, clang::TypeResult UnderlyingType, bool IsTypeSpecifier);
#else
  clang::Decl* ActOnTag (
      clang::Scope *S, unsigned TagSpec, clang::Sema::TagUseKind TUK, clang::SourceLocation KWLoc, clang::CXXScopeSpec &SS,
      clang::IdentifierInfo *Name, clang::SourceLocation NameLoc, clang::AttributeList *Attr, clang::AccessSpecifier AS,
      clang::SourceLocation ModulePrivateLoc, clang::MultiTemplateParamsArg TemplateParameterLists,
      bool &OwnedDecl, bool &IsDependent, clang::SourceLocation ScopedEnumKWLoc,
      bool ScopedEnumUsesClassTag, clang::TypeResult UnderlyingType);
#endif

  void ActOnFinishCXXMemberSpecification (clang::Scope* S,
      clang::SourceLocation RLoc, clang::Decl *TagDecl, clang::SourceLocation LBrac,
      clang::SourceLocation RBrac, clang::AttributeList *AttrList);

  void ActOnEndOfTranslationUnit ();

  bool canSkipFunctionBody(clang::Decl *FctDecl);

  clang::NamedDecl *ActOnCXXMemberDeclarator(clang::Scope *S, clang::AccessSpecifier AS,
      clang::Declarator &D, clang::MultiTemplateParamsArg TemplateParameterLists,
      clang::Expr *BitfieldWidth, const clang::VirtSpecifiers &VS,
      clang::InClassInitStyle InitStyle);

  clang::Decl *ActOnDeclarator(clang::Scope *S, clang::Declarator &D);

};

#endif // __ClangIntroSema_h__
