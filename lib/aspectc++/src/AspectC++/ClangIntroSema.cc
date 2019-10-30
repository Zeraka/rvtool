
// FIXME: This is a hack to give ClangIntroSema access rights to the current scope
// This has to be included FIRST!
#define private protected
#include "clang/Sema/Sema.h"
#undef private
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Sema/SemaDiagnostic.h"

#include "ClangIntroSema.h"
#include "ClangIntroducer.h"
#include "ClangBinding.h"
#include "ACProject.h"
#include "ModelBuilder.h"

using namespace clang;

// remove all attributes that have namespace "AC::" in its name from the
// attribute list 'in' and put them into 'hidden'
void hideACAttrs(clang::AttributeList * &in, clang::AttributeList *&hidden) {
  clang::AttributeList *prev = 0, *al = in;
  while (al) {
    clang::AttributeList * next = al->getNext();
    if (al->hasScope() && al->getScopeName()->getName() != "gnu" &&
        al->getScopeName()->getName() != "clang") {
//      std::cout << "  hide " << al->getName()->getNameStart() << std::endl;
      if (prev)
        prev->setNext(al->getNext());
      else
        in = al->getNext();
      al->setNext(hidden);
      hidden = al;
    }
    else
      prev = al;
    al = next;
  }
}

// add all attributes from 'hidden' into 'out'. 'hidden' is empty afterwards
void restoreACAttrs(clang::AttributeList * &out, clang::AttributeList *&hidden) {
  clang::AttributeList *al = out;
  // special handling of empty list first
  if (!al) {
    out = hidden;
  }
  else {
    // find the end of the list
    while (al->getNext()) {
      al = al->getNext();
    }
    // append the hidden list
    al->setNext(hidden);
  }
  // make sure the hidden list is empty after this shifting of attributes
  hidden = 0;
}

void injectAttrs(clang::Decl *decl, clang::ASTContext &context, const clang::AttributeList * al) {
  while (al) {
//    std::cout << "  injecting " << al->getName()->getNameStart() << std::endl;
    clang::AnnotateAttr *attr = ::new (context) clang::AnnotateAttr(al->getRange(), context,
        al->getName()->getName(), 0 /* spelling list index of AnnotateAttr is always 0 */);
    decl->addAttr(attr);
    al = al->getNext();
  }
}

ClangIntroSema::ClangIntroSema(ClangIntroducer &introducer,
      Preprocessor &pp, ASTContext &ctxt, ASTConsumer &consumer,
      TranslationUnitKind TUKind, CodeCompleteConsumer *CompletionConsumer) :
        Sema (pp, ctxt, consumer, TUKind, CompletionConsumer),
        _introducer (&introducer), _nested_tu_end(0), _nested_base_specifiers (0) {
}

ClangIntroSema::~ClangIntroSema() {
}

Scope *ClangIntroSema::setCurScope(Scope *new_scope) {
  Scope *old_scope = CurScope;
  CurScope = new_scope;
  return old_scope;
}

void ClangIntroSema::ActOnStartCXXMemberDeclarations(Scope *S, Decl *TagDecl,
    SourceLocation FinalLoc, bool IsFinalSpelledSealed,
    SourceLocation LBraceLoc) {
  _introducer->class_start(TagDecl, LBraceLoc);
  RealActOnStartCXXMemberDeclarations(*this, S, TagDecl, FinalLoc,
      IsFinalSpelledSealed, LBraceLoc);
}

#if FRONTEND_CLANG >= 38
void ClangIntroSema::ActOnBaseSpecifiers (Decl *ClassDecl,
    clang::MutableArrayRef<clang::CXXBaseSpecifier *> Bases) {
  for (unsigned n = 0; n < Bases.size(); n++)
    _BaseInfo.push_back(Bases[n]);
  if (_nested_base_specifiers == 0) {
    _nested_base_specifiers++;
    _introducer->base_specs_end(ClassDecl);
    _nested_base_specifiers--;
    RealActOnBaseSpecifiers(*this, ClassDecl, _BaseInfo);
    _BaseInfo.clear ();
  }
}
#else
void ClangIntroSema::ActOnBaseSpecifiers (Decl *ClassDecl,
    CXXBaseSpecifier **Bases, unsigned NumBases) {
  for (unsigned n = 0; n < NumBases; n++)
    _BaseInfo.push_back(Bases[n]);
  if (_nested_base_specifiers == 0) {
    _nested_base_specifiers++;
    _introducer->base_specs_end(ClassDecl);
    _nested_base_specifiers--;
    RealActOnBaseSpecifiers(*this, ClassDecl, _BaseInfo.data (), _BaseInfo.size ());
    _BaseInfo.clear ();
  }
}
#endif

#if FRONTEND_CLANG >= 38
clang::Decl* ClangIntroSema::ActOnStartOfFunctionDef(
    clang::Scope *S, clang::Declarator &D,
    clang::MultiTemplateParamsArg TemplateParamLists,
    clang::Sema::SkipBodyInfo *SkipBody) {
//  std::cout << "ActOnStartOfFunctionDef!" << std::endl;
  clang::AttributeList *declspec_attrs = 0, *declarator_attrs = 0;
  hideACAttrs (D.getMutableDeclSpec().getAttributes().getListRef(), declspec_attrs);
  hideACAttrs (D.getAttrListRef(), declarator_attrs);
  clang::Decl *decl = RealActOnStartOfFunctionDef(*this, S, D, TemplateParamLists, SkipBody);
  injectAttrs(decl, this->getASTContext(), declspec_attrs);
  injectAttrs(decl, this->getASTContext(), declarator_attrs);
//  decl->dump();
  clang::FunctionDecl *fdecl = clang::dyn_cast<clang::FunctionDecl>(decl);
  if (fdecl)
    _introducer->function_start(fdecl);
  restoreACAttrs(D.getMutableDeclSpec().getAttributes().getListRef(), declspec_attrs);
  restoreACAttrs(D.getAttrListRef(), declarator_attrs);
  return decl;
}
#else
clang::Decl* ClangIntroSema::ActOnStartOfFunctionDef(
    clang::Scope *S, clang::Declarator &D) {
//  std::cout << "ActOnStartOfFunctionDef!" << std::endl;
  clang::AttributeList *declspec_attrs = 0, *declarator_attrs = 0;
  hideACAttrs (D.getMutableDeclSpec().getAttributes().getListRef(), declspec_attrs);
  hideACAttrs (D.getAttrListRef(), declarator_attrs);
  clang::Decl *decl = RealActOnStartOfFunctionDef(*this, S, D);
  injectAttrs(decl, this->getASTContext(), declspec_attrs);
  injectAttrs(decl, this->getASTContext(), declarator_attrs);
//  decl->dump();
  clang::FunctionDecl *fdecl = clang::dyn_cast<clang::FunctionDecl>(decl);
  if (fdecl)
    _introducer->function_start(fdecl);
  restoreACAttrs(D.getMutableDeclSpec().getAttributes().getListRef(), declspec_attrs);
  restoreACAttrs(D.getAttrListRef(), declarator_attrs);
  return decl;
}
#endif

#if FRONTEND_CLANG >= 38
clang::Decl* ClangIntroSema::ActOnStartNamespaceDef (
    clang::Scope *S, clang::SourceLocation InlineLoc, clang::SourceLocation NamespaceLoc,
    clang::SourceLocation IdentLoc, clang::IdentifierInfo *Ident, clang::SourceLocation LBrace,
    clang::AttributeList *AttrList, clang::UsingDirectiveDecl * &UsingDecl)  {
//    std::cout << "ActOnNamespaceDef!" << std::endl;
    clang::AttributeList *hidden_attrs = 0;
    hideACAttrs (AttrList, hidden_attrs);
    clang::Decl *decl = RealActOnStartNamespaceDef(*this, S, InlineLoc, NamespaceLoc, IdentLoc, Ident, LBrace, AttrList, UsingDecl);
    injectAttrs(decl, this->getASTContext(), hidden_attrs);
  //  decl->dump();
    restoreACAttrs(AttrList, hidden_attrs);
    return decl;
}
#else
clang::Decl* ClangIntroSema::ActOnStartNamespaceDef (
    clang::Scope *S, clang::SourceLocation InlineLoc, clang::SourceLocation NamespaceLoc,
    clang::SourceLocation IdentLoc, clang::IdentifierInfo *Ident, clang::SourceLocation LBrace,
    clang::AttributeList *AttrList)  {
//    std::cout << "ActOnNamespaceDef!" << std::endl;
    clang::AttributeList *hidden_attrs = 0;
    hideACAttrs (AttrList, hidden_attrs);
    clang::Decl *decl = RealActOnStartNamespaceDef(*this, S, InlineLoc, NamespaceLoc, IdentLoc, Ident, LBrace, AttrList);
    injectAttrs(decl, this->getASTContext(), hidden_attrs);
  //  decl->dump();
    restoreACAttrs(AttrList, hidden_attrs);
    return decl;
}
#endif

#if FRONTEND_CLANG >= 37
clang::Decl* ClangIntroSema::ActOnTag (
    clang::Scope *S, unsigned TagSpec, clang::Sema::TagUseKind TUK, clang::SourceLocation KWLoc, clang::CXXScopeSpec &SS,
    clang::IdentifierInfo *Name, clang::SourceLocation NameLoc, clang::AttributeList *Attr, clang::AccessSpecifier AS,
    clang::SourceLocation ModulePrivateLoc, clang::MultiTemplateParamsArg TemplateParameterLists,
    bool &OwnedDecl, bool &IsDependent, clang::SourceLocation ScopedEnumKWLoc,
    bool ScopedEnumUsesClassTag, clang::TypeResult UnderlyingType, bool IsTypeSpecifier, clang::Sema::SkipBodyInfo *SkipBody) {
//  std::cout << "ActOnTag!" << std::endl;
  clang::AttributeList *hidden_attrs = 0;
  hideACAttrs (Attr, hidden_attrs);
  clang::Decl *decl = RealActOnTag(*this, S, TagSpec, TUK, KWLoc, SS, Name, NameLoc, Attr, AS, ModulePrivateLoc, TemplateParameterLists,
      OwnedDecl, IsDependent, ScopedEnumKWLoc, ScopedEnumUsesClassTag, UnderlyingType, IsTypeSpecifier, SkipBody);
  injectAttrs(decl, this->getASTContext(), hidden_attrs);
//  decl->dump();
  restoreACAttrs(Attr, hidden_attrs);
  return decl;
}
#elif FRONTEND_CLANG >= 35
clang::Decl* ClangIntroSema::ActOnTag (
    clang::Scope *S, unsigned TagSpec, clang::Sema::TagUseKind TUK, clang::SourceLocation KWLoc, clang::CXXScopeSpec &SS,
    clang::IdentifierInfo *Name, clang::SourceLocation NameLoc, clang::AttributeList *Attr, clang::AccessSpecifier AS,
    clang::SourceLocation ModulePrivateLoc, clang::MultiTemplateParamsArg TemplateParameterLists,
    bool &OwnedDecl, bool &IsDependent, clang::SourceLocation ScopedEnumKWLoc,
    bool ScopedEnumUsesClassTag, clang::TypeResult UnderlyingType, bool IsTypeSpecifier) {
//  std::cout << "ActOnTag!" << std::endl;
  clang::AttributeList *hidden_attrs = 0;
  hideACAttrs (Attr, hidden_attrs);
  clang::Decl *decl = RealActOnTag(*this, S, TagSpec, TUK, KWLoc, SS, Name, NameLoc, Attr, AS, ModulePrivateLoc, TemplateParameterLists,
      OwnedDecl, IsDependent, ScopedEnumKWLoc, ScopedEnumUsesClassTag, UnderlyingType, IsTypeSpecifier);
  injectAttrs(decl, this->getASTContext(), hidden_attrs);
//  decl->dump();
  restoreACAttrs(Attr, hidden_attrs);
  return decl;
}
#else
clang::Decl* ClangIntroSema::ActOnTag (
    clang::Scope *S, unsigned TagSpec, clang::Sema::TagUseKind TUK, clang::SourceLocation KWLoc, clang::CXXScopeSpec &SS,
    clang::IdentifierInfo *Name, clang::SourceLocation NameLoc, clang::AttributeList *Attr, clang::AccessSpecifier AS,
    clang::SourceLocation ModulePrivateLoc, clang::MultiTemplateParamsArg TemplateParameterLists,
    bool &OwnedDecl, bool &IsDependent, clang::SourceLocation ScopedEnumKWLoc,
    bool ScopedEnumUsesClassTag, clang::TypeResult UnderlyingType) {
//  std::cout << "ActOnTag!" << std::endl;
  clang::AttributeList *hidden_attrs = 0;
  hideACAttrs (Attr, hidden_attrs);
  clang::Decl *decl = RealActOnTag(*this, S, TagSpec, TUK, KWLoc, SS, Name, NameLoc, Attr, AS, ModulePrivateLoc, TemplateParameterLists,
      OwnedDecl, IsDependent, ScopedEnumKWLoc, ScopedEnumUsesClassTag, UnderlyingType);
  injectAttrs(decl, this->getASTContext(), hidden_attrs);
//  decl->dump();
  restoreACAttrs(Attr, hidden_attrs);
  return decl;
}
#endif

void ClangIntroSema::ActOnFinishCXXMemberSpecification (Scope* S,
    SourceLocation RLoc, Decl *TagDecl, SourceLocation LBrac,
    SourceLocation RBrac, AttributeList *AttrList) {
  _introducer->class_end (TagDecl, RBrac);
  RealActOnFinishCXXMemberSpecification(*this, S, RLoc, TagDecl, LBrac, RBrac,
      AttrList);
}

void ClangIntroSema::ActOnEndOfTranslationUnit () {
  if (_nested_tu_end == 0) {
    _nested_tu_end++;
    _introducer->tunit_end ();
    _nested_tu_end--;
    // workaround: clang warns if skipped inline functions are used. In our case
    // this happens with inline functions that are not part of the project.
    // To avoid annoying messages they are suppressed here.
    getDiagnostics().setSeverity(diag::warn_undefined_inline, diag::Severity::Ignored, SourceLocation());
    RealActOnEndOfTranslationUnit(*this);
//    getASTContext().getTranslationUnitDecl ()->dump();

  }
}

bool ClangIntroSema::canSkipFunctionBody(Decl *FctDecl) {
  clang::SourceManager &sm = _introducer->_ci->getSourceManager();
  clang::PresumedLoc PL = sm.getPresumedLoc(FctDecl->getLocation());
  llvm::StringRef Name = PL.getFilename();
  llvm::StringRef BufferName = sm.getBufferName(FctDecl->getLocation());
  ACProject &project = _introducer->get_model_builder().get_project();
  bool in_project = (BufferName.startswith("<intro") ||
      (!Name.empty() && (Name.equals("<ac>") || project.isBelow(Name.str().c_str()))));
//  cout << Name.str() << " " << BufferName.str() << " " << in_project << endl;
  return !in_project && RealcanSkipFunctionBody(*this, FctDecl);
}


clang::NamedDecl *ClangIntroSema::ActOnCXXMemberDeclarator(clang::Scope *S, clang::AccessSpecifier AS,
    clang::Declarator &D, clang::MultiTemplateParamsArg TemplateParameterLists,
    clang::Expr *BitfieldWidth, const clang::VirtSpecifiers &VS,
    clang::InClassInitStyle InitStyle){
//  std::cout << "ActOnCXXMemberDeclarator!" << std::endl;
  clang::AttributeList *declspec_attrs = 0, *declarator_attrs = 0;
  hideACAttrs (D.getMutableDeclSpec().getAttributes().getListRef(), declspec_attrs);
  hideACAttrs (D.getAttrListRef(), declarator_attrs);
  clang::NamedDecl *ndecl = RealActOnCXXMemberDeclarator(*this, S, AS, D, TemplateParameterLists,
      BitfieldWidth, VS, InitStyle);
//  std::cout << "  name " << ndecl->getNameAsString() << std::endl;
  injectAttrs(ndecl, this->getASTContext(), declspec_attrs);
  injectAttrs(ndecl, this->getASTContext(), declarator_attrs);
  clang::DeclaratorDecl *ddecl = clang::dyn_cast<clang::DeclaratorDecl>(ndecl);
  clang::FunctionDecl *fdecl = clang::dyn_cast<clang::FunctionDecl>(ndecl);
  if (fdecl)
    _introducer->function_start(fdecl);
  else if (ddecl)
    _introducer->declarator_start(ddecl);
  //  ndecl->dump();
  restoreACAttrs(D.getMutableDeclSpec().getAttributes().getListRef(), declspec_attrs);
  restoreACAttrs(D.getAttrListRef(), declarator_attrs);
  return ndecl;
}

clang::Decl *ClangIntroSema::ActOnDeclarator(clang::Scope *S, clang::Declarator &D) {
//  std::cout << "ActOnDeclarator!" << std::endl;
  clang::AttributeList *declspec_attrs = 0, *declarator_attrs = 0;
  hideACAttrs (D.getMutableDeclSpec().getAttributes().getListRef(), declspec_attrs);
  hideACAttrs (D.getAttrListRef(), declarator_attrs);
  clang::Decl *decl = RealActOnDeclarator(*this, S, D);
  injectAttrs(decl, this->getASTContext(), declspec_attrs);
  injectAttrs(decl, this->getASTContext(), declarator_attrs);
  clang::DeclaratorDecl *ddecl = clang::dyn_cast<clang::DeclaratorDecl>(decl);
  clang::FunctionDecl *fdecl = clang::dyn_cast<clang::FunctionDecl>(decl);
  if (fdecl) // must be check first, because a FunctionDecl is also a DeclaratorDecl
    _introducer->function_start(fdecl);
  else if (ddecl)
    _introducer->declarator_start(ddecl);
  restoreACAttrs(D.getMutableDeclSpec().getAttributes().getListRef(), declspec_attrs);
  restoreACAttrs(D.getAttrListRef(), declarator_attrs);
  return decl;
}

