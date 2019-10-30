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

#ifndef __ClangSyntacticContext_h__
#define __ClangSyntacticContext_h__

// STL includes
#include <string>
#include <set>
using namespace std;

// clang includes
#include "clang/Basic/Version.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"
#include "clang/Lex/Lexer.h"
#include "clang/Parse/Parser.h"

// AspectC++ includes
#include "WeaverBase.h"

class ClangSyntacticContext {
  const clang::Decl *_decl;

public:
  ClangSyntacticContext(const clang::Decl *d = 0) : _decl(d) {}
  bool operator < (const ClangSyntacticContext &that) const { return _decl < that._decl; }

  bool is_anon_union_member () const {
    if (const clang::FieldDecl *fd = llvm::dyn_cast<clang::FieldDecl>(_decl))
      return fd->getParent ()->isAnonymousStructOrUnion ();
    return false;
  }

  ClangSyntacticContext get_anon_union_context () const {
    assert (is_anon_union_member ());
    return ClangSyntacticContext (llvm::cast<clang::FieldDecl>(_decl)->getParent ());
  }

  bool is_array () const {
    if (const clang::ValueDecl *vd = llvm::dyn_cast<clang::ValueDecl>(_decl))
      return vd->getType ().getTypePtr ()->isArrayType ();
    return false;
  }

  string name () const {
    if (const clang::NamedDecl *nd = llvm::dyn_cast<clang::NamedDecl>(_decl))
      return nd->getNameAsString ();
    return "";
  }

  string get_as_string (int indentation = 0) const {
    std::string s;
    llvm::raw_string_ostream os(s);
    _decl->print(os, indentation);
    return s;
  }

  string type () const;

  bool is_definition () const {
    if (const clang::FunctionDecl *fd = llvm::dyn_cast<clang::FunctionDecl>(_decl))
      return fd->isThisDeclarationADefinition();
    return false;
  }

  bool has_assigned_scope () const {
    return _decl->isOutOfLine ();
  }

  // returns 0 if not in an extern "C" { ... } block, otherwise (result >= 1) the
  // result is the depth of nested extern "C" blocks
  int is_in_extern_c_block () const;

  bool is_in_class_scope () const {
    return _decl->getLexicalDeclContext ()->isRecord();
  }

  bool is_in_namespace () const {
    return _decl->getLexicalDeclContext ()->isNamespace();
  }

  bool is_in_project () const {
    return true; // FIXME!
  }

  string protection () const {
    string result;
    switch (_decl->getAccess()) {
    case clang::AS_public:    result = "public";    break;
    case clang::AS_private:   result = "private";   break;
    case clang::AS_protected: result = "protected"; break;
    case clang::AS_none:      result = "";
    }
    return result;
  }

  const WeavePos &wrapper_pos (WeaverBase &wb) const {
    // First case: this is a call in a variable declaration:
    const clang::VarDecl *vd = clang::dyn_cast<clang::VarDecl>(_decl);
    if (vd)
      return wb.weave_pos(vd->getLocStart (), WeavePos::WP_BEFORE);
    // Second case: a function ...
    clang::SourceLocation loc = llvm::cast<clang::FunctionDecl>(_decl)->getLocStart();
    const clang::LinkageSpecDecl *lsd =
        llvm::dyn_cast<clang::LinkageSpecDecl>(llvm::cast<clang::FunctionDecl>(_decl)->getDeclContext ());
    if (lsd && !lsd->hasBraces ())
      loc = lsd->getLocStart ();

    if (_decl->hasAttrs()) {
      clang::SourceManager &source_manager = wb.getRewriter ().getSourceMgr();
      const clang::AttrVec &attrs = _decl->getAttrs();

      for(const clang::Attr *attr : attrs) {
        AnnotationMap::const_iterator i = wb.get_annotation_map().find(attr->getLocation());
        if (i != wb.get_annotation_map().end() &&
            source_manager.isBeforeInTranslationUnit(i->second.seqBegin, loc))
          loc = i->second.seqBegin;
      }
    }

    return wb.weave_pos(loc, WeavePos::WP_BEFORE);
  }

  const WeavePos &fct_body_start_pos (WeaverBase &wb) const {
    clang::SourceLocation loc = llvm::cast<clang::FunctionDecl>(_decl)->getBody()->getLocStart();
    return wb.weave_pos(loc, WeavePos::WP_BEFORE);
  }

  const WeavePos &fct_body_end_pos (WeaverBase &wb) const {
    clang::SourceLocation loc = llvm::cast<clang::FunctionDecl>(_decl)->getLocEnd();
    return wb.weave_pos(loc, WeavePos::WP_BEFORE);
  }

  const WeavePos &objdecl_start_pos (WeaverBase &wb) const {
    clang::SourceLocation loc = _decl->getLocStart();
    return wb.weave_pos(loc, WeavePos::WP_BEFORE);
  }

  const WeavePos &objdecl_end_pos (WeaverBase &wb) const {
    clang::SourceLocation loc = _decl->getLocEnd();
    if (llvm::isa<clang::FieldDecl>(_decl)) {
      const clang::LangOptions &options    = wb.getRewriter ().getLangOpts ();
      clang::SourceManager &source_manager = wb.getRewriter ().getSourceMgr();
      loc = clang::Lexer::getLocForEndOfToken (loc, 0, source_manager, options);
    }

    return wb.weave_pos(loc, WeavePos::WP_AFTER);
  }

  bool ends_with_return () const {
    const clang::FunctionDecl *fd = llvm::cast<clang::FunctionDecl>(_decl);
    if (clang::Stmt *body = fd->getBody()) {
      clang::Stmt::child_iterator child = body->child_begin();
      clang::Stmt::child_iterator end = body->child_end();
      if (child == end)
        return false; // no children

      // Iterate to the last Stmt.
#if CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)
      while (llvm::next(child) != end)
        ++child;
#else // C++ 11 interface
      while (std::next(child) != end)
        ++child;
#endif

      // Check if it's a return.
      return llvm::isa<clang::ReturnStmt>(*child);
    }
    return false;
  }

  string result_type (const string &name = "") const;

  unsigned int args () const {
    return llvm::cast<clang::FunctionDecl>(_decl)->getNumParams();
  }

  string arg_name (unsigned int no) const {
    const clang::FunctionDecl *fd = llvm::cast<clang::FunctionDecl>(_decl);
    const clang::ParmVarDecl *pd = fd->getParamDecl(no);
    return pd->getNameAsString ();
  }

  string arg_type (unsigned int no, const string &name = "") const;

  string qualified_scope () const {
    string result;
    if (_decl->isOutOfLine ()) {
      const clang::NamedDecl *ND =
          llvm::dyn_cast<clang::NamedDecl>(_decl->getDeclContext ());
      if (ND) result = ND->getQualifiedNameAsString();
    }
    return result;
  }

  string syntactical_scope () const {
    string result;
    const clang::NamedDecl *ND =
        llvm::dyn_cast<clang::NamedDecl>(_decl->getLexicalDeclContext ());
    if (ND) ND->getQualifiedNameAsString();
    return result;
  }

  // TODO: still needed? yes, in CodeWeaver
  const clang::Decl *decl () const { return _decl; }
};

#endif // __ClangSyntacticContext_h__


