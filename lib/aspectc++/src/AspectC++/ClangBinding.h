// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2003  The 'ac++' developers (see aspectc.org)
//
// This program is free software;  you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this program; if not, write to the Free
// Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
// MA  02111-1307  USA

#ifndef __ClangBinding_h__
#define __ClangBinding_h__

// This module is needed to hook into the internal behavior of the Clang C++
// parser. Calls to a number of Sema::ActOn... functions are wrapped by means
// of the linker option --wrap=<symbol>. This method has a number restrictions:
//  - only works with gnu ld
//  - it does not work on MacOS X
//  - it only works if calls cannot be resolved by the compiler (an undefined
//    symbol references is needed by the linker)
// All that wouldn't be need if the ActOn...-functions of clang::Sema were
// virtual. We hope that this will be changed in the future.
// The implementation hooks into calls from clang::Parser to clang::Sema::ActOn*,
// cast the Sema-object reference to a ClangIntroSema-reference and calls the
// respective ActOn...-function. For ClangIntroSema this looks like the execution
// of a normal virtual function.
// In order to call the original ActOn-function, ClangIntroSema functions call
// Real..., which is provided by this module for all wrapped functions. Calling
// the respective base class method is not possible here, because due to the
// wrapping we would end up with an infinite loop.

namespace clang {
  class Sema;
  class Scope;
  class Decl;
}

#include "clang/Basic/SourceLocation.h"

// The following functions will be needed by the extended Sema class
void RealActOnStartCXXMemberDeclarations (clang::Sema &that, clang::Scope *S,
    clang::Decl *TagDecl, clang::SourceLocation FinalLoc,
    bool IsFinalSpelledSealed, clang::SourceLocation LBraceLoc);

#if FRONTEND_CLANG >= 38
void RealActOnBaseSpecifiers (clang::Sema &that, clang::Decl *ClassDecl,
    clang::MutableArrayRef<clang::CXXBaseSpecifier *> Bases);

#else
void RealActOnBaseSpecifiers (clang::Sema &that, clang::Decl *ClassDecl,
    clang::CXXBaseSpecifier **Bases, unsigned NumBases);
#endif

#if FRONTEND_CLANG >= 38
clang::Decl* RealActOnStartOfFunctionDef(clang::Sema &that, clang::Scope *S,
    clang::Declarator &D, clang::MultiTemplateParamsArg TemplateParamLists,
    clang::Sema::SkipBodyInfo *SkipBody);
#else
clang::Decl* RealActOnStartOfFunctionDef(clang::Sema &that, clang::Scope *S,
    clang::Declarator &D);
#endif

#if FRONTEND_CLANG >= 38
clang::Decl* RealActOnStartNamespaceDef (
    clang::Sema &that, clang::Scope *S, clang::SourceLocation InlineLoc, clang::SourceLocation NamespaceLoc,
    clang::SourceLocation IdentLoc, clang::IdentifierInfo *Ident, clang::SourceLocation LBrace,
    clang::AttributeList *AttrList, clang::UsingDirectiveDecl * &UsingDecl);
#else
clang::Decl* RealActOnStartNamespaceDef (
    clang::Sema &that, clang::Scope *S, clang::SourceLocation InlineLoc, clang::SourceLocation NamespaceLoc,
    clang::SourceLocation IdentLoc, clang::IdentifierInfo *Ident, clang::SourceLocation LBrace,
    clang::AttributeList *AttrList);
#endif

#if FRONTEND_CLANG >= 37
clang::Decl* RealActOnTag(
    clang::Sema &that, clang::Scope *S, unsigned TagSpec, clang::Sema::TagUseKind TUK, clang::SourceLocation KWLoc, clang::CXXScopeSpec &SS,
    clang::IdentifierInfo *Name, clang::SourceLocation NameLoc, clang::AttributeList *Attr, clang::AccessSpecifier AS,
    clang::SourceLocation ModulePrivateLoc, clang::MultiTemplateParamsArg TemplateParameterLists,
    bool &OwnedDecl, bool &IsDependent, clang::SourceLocation ScopedEnumKWLoc,
    bool ScopedEnumUsesClassTag, clang::TypeResult UnderlyingType, bool IsTypeSpecifier, clang::Sema::SkipBodyInfo *SkipBody);
#elif FRONTEND_CLANG >= 35
clang::Decl* RealActOnTag(
    clang::Sema &that, clang::Scope *S, unsigned TagSpec, clang::Sema::TagUseKind TUK, clang::SourceLocation KWLoc, clang::CXXScopeSpec &SS,
    clang::IdentifierInfo *Name, clang::SourceLocation NameLoc, clang::AttributeList *Attr, clang::AccessSpecifier AS,
    clang::SourceLocation ModulePrivateLoc, clang::MultiTemplateParamsArg TemplateParameterLists,
    bool &OwnedDecl, bool &IsDependent, clang::SourceLocation ScopedEnumKWLoc,
    bool ScopedEnumUsesClassTag, clang::TypeResult UnderlyingType, bool IsTypeSpecifier);
#else
clang::Decl* RealActOnTag(
    clang::Sema &that, clang::Scope *S, unsigned TagSpec, clang::Sema::TagUseKind TUK, clang::SourceLocation KWLoc, clang::CXXScopeSpec &SS,
    clang::IdentifierInfo *Name, clang::SourceLocation NameLoc, clang::AttributeList *Attr, clang::AccessSpecifier AS,
    clang::SourceLocation ModulePrivateLoc, clang::MultiTemplateParamsArg TemplateParameterLists,
    bool &OwnedDecl, bool &IsDependent, clang::SourceLocation ScopedEnumKWLoc,
    bool ScopedEnumUsesClassTag, clang::TypeResult UnderlyingType);
#endif

void RealActOnFinishCXXMemberSpecification (clang::Sema &that, clang::Scope* S,
    clang::SourceLocation RLoc, clang::Decl *TagDecl, clang::SourceLocation LBrac,
    clang::SourceLocation RBrac, clang::AttributeList *AttrList);

void RealActOnEndOfTranslationUnit(clang::Sema &that);

bool RealcanSkipFunctionBody(clang::Sema &that, clang::Decl *D);

clang::NamedDecl* RealActOnCXXMemberDeclarator(clang::Sema &that, clang::Scope *S,
    clang::AccessSpecifier AS, clang::Declarator &D,
    clang::MultiTemplateParamsArg TemplateParameterLists,
    clang::Expr *BitfieldWidth, const clang::VirtSpecifiers &VS,
    clang::InClassInitStyle InitStyle);

clang::Decl* RealActOnDeclarator(clang::Sema &that, clang::Scope *S, clang::Declarator &D);

// if macro_recorder points to a WeaverBase instance, each token that is
// return by clang::Preprocessor::Lex will also be passed to the recorder.
class WeaverBase;
extern WeaverBase *macro_recorder;

#endif // __ClangBinding_h__
