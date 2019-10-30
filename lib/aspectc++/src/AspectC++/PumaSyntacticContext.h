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

#ifndef __PumaSyntacticContext_h__
#define __PumaSyntacticContext_h__

// Puma includes
#include "Puma/CAttributeInfo.h"
#include "Puma/CArgumentInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CTree.h"
#include "Puma/CProject.h"
#include "Puma/CSemDatabase.h"

// AspectC++ includes
#include "WeaverBase.h"
#include "IntroductionUnit.h"

class PumaSyntacticContext {
  Puma::CObjectInfo *_obj;

  // return LinkageSpec node for extern "C" <decl> like declarations
  static Puma::CTree *linkage_adjust (Puma::CT_Decl *decl) {
    Puma::CT_LinkageSpec *linkage = decl->Linkage ();
    return (!linkage || linkage->isList ()) ? decl : linkage;
  }

  Puma::Token *wrapper_token () const {
    Puma::CTree *tree = _obj->Tree ();
    assert (tree);
    Puma::CTree *pos = 0;
    if (tree->NodeName() == Puma::CT_InitDeclarator::NodeId()) {
      // move the generated declaration in front of the original declaration
      Puma::CT_ObjDecl *objdecl = ((Puma::CT_InitDeclarator*)tree)->ObjDecl ();
      // handle extern "C" declarations
      pos = linkage_adjust (objdecl);
    }
    if (tree->NodeName() == Puma::CT_FctDef::NodeId()) {
      pos = linkage_adjust((Puma::CT_FctDef*)tree);
    }
    assert (pos);
    return pos->token ();
  }

  static void print_tree (ostream &out, Puma::CTree *node, bool expand_implicit_calls = false);

public:
  PumaSyntacticContext (Puma::CObjectInfo *o = 0) : _obj (o) {}
  bool operator < (const PumaSyntacticContext &that) const { return _obj < that._obj; }

  void set_object (Puma::CObjectInfo *o) { _obj = o; }

  string name () const { return _obj->Name ().c_str (); }

  string get_as_string (int indentation = 0) const {
    ostringstream os;
    while (indentation-- >= 0)
      os << "  ";
    Puma::CTree *tree = _obj->Tree ();
    if (tree->NodeName() == Puma::CT_InitDeclarator::NodeId()) {
      print_tree (os, ((Puma::CT_InitDeclarator*)tree)->ObjDecl ()->DeclSpecs ());
      os << " ";
    }
    print_tree (os, tree);
    return os.str ();
  }

  string type () const {
    stringstream str;
    _obj->TypeInfo()->TypeText(str, "", true, true);
    return str.str();
  }

  bool is_anon_union_member () const {
    return _obj->Scope ()->isUnion ();
  }

  bool is_array () const {
    return _obj->TypeInfo ()->TypeArray ();
  }

  PumaSyntacticContext get_anon_union_context () const {
    assert (is_anon_union_member ());
    return PumaSyntacticContext (_obj->Scope ());
  }

  bool is_definition () const {
    if (_obj->FunctionInfo ())
      return _obj->FunctionInfo ()->isFctDef ();
    return false;
  }

  bool has_assigned_scope () const {
    return _obj->AssignedScope ();
  }

  // returns 0 if not in an extern "C" { ... } block, otherwise (result >= 1) the
  // result is the depth of nested extern "C" blocks
  int is_in_extern_c_block () const;

  bool is_in_class_scope () const {
    return _obj->ClassScope () && (_obj->Scope() == _obj->ClassScope());
  }

  bool is_in_namespace () const {
    return _obj->Scope ()->isNamespace();
  }

  bool is_in_project () const {
    if (_obj->isBuiltin())
      return false;
    Puma::Unit *unit = _obj->SourceInfo ()->SrcUnit ();
    if (!_obj->SemDB ()->Project ()->isBelow (unit) &&
        !IntroductionUnit::cast (unit))
      return false;
    return true;
  }

  string protection () const {
    string result;
    switch (_obj->Protection()) {
    case Puma::CProtection::PROT_PUBLIC:    result = "public";    break;
    case Puma::CProtection::PROT_PRIVATE:   result = "private";   break;
    case Puma::CProtection::PROT_PROTECTED: result = "protected"; break;
    case Puma::CProtection::PROT_NONE:      result = "";
    }
    return result;
  }

  const WeavePos &wrapper_pos (WeaverBase &wb) const {
    Puma::Token *tok = wrapper_token ();
    return wb.weave_pos(tok, WeavePos::WP_BEFORE);
  }

  Puma::Token *error_pos () const {
    return _obj->Tree()->token();
  }

  const WeavePos &fct_body_start_pos (WeaverBase &wb) const {
    Puma::CTree *tree = _obj->Tree ();
    assert (tree && tree->NodeName() == Puma::CT_FctDef::NodeId());
    Puma::Token *tok = ((Puma::CT_FctDef*)tree)->Body()->token ();
    return wb.weave_pos(tok, WeavePos::WP_BEFORE);
  }

  const WeavePos &fct_body_end_pos (WeaverBase &wb) const {
    Puma::CTree *tree = _obj->Tree ();
    assert (tree && tree->NodeName() == Puma::CT_FctDef::NodeId());
    Puma::Token *tok = ((Puma::CT_FctDef*)tree)->Body()->end_token ();
    return wb.weave_pos(tok, WeavePos::WP_BEFORE);
  }

  const WeavePos &objdecl_start_pos (WeaverBase &wb) const {
    Puma::CTree *tree = _obj->Tree ();
    if (tree->NodeName() == Puma::CT_InitDeclarator::NodeId())
      tree = ((Puma::CT_InitDeclarator*)tree)->ObjDecl ();
    Puma::Token *tok = tree->token ();
    return wb.weave_pos(tok, WeavePos::WP_BEFORE);
  }

  const WeavePos &objdecl_end_pos (WeaverBase &wb) const {
    Puma::CTree *tree = _obj->Tree ();
    Puma::Token *tok = tree->end_token ();
    return wb.weave_pos(tok, WeavePos::WP_AFTER);
  }

  bool ends_with_return () const {
    assert (_obj->FunctionInfo() && _obj->Tree ()->NodeName () == Puma::CT_FctDef::NodeId());
    Puma::CT_FctDef *fctdef = (Puma::CT_FctDef*)_obj->Tree ();
    Puma::CTree *last_stmt = 0;
    int entries = fctdef->Body()->Entries();
    if (entries > 0)
      last_stmt = fctdef->Body()->Entry (entries - 1);
    return (last_stmt && last_stmt->NodeName() == Puma::CT_ReturnStmt::NodeId());
  }

  // search the set of actually *used* arguments (needed only for constructor
  // wrapper functions).
  void search_used_args (set<string> &args, Puma::CTree *tree = 0) const;

  string result_type (const string &name = "") const {
    Puma::CFunctionInfo *func = _obj->FunctionInfo();
    assert(func);
    Puma::CTypeInfo *type = func->isConversion () ? func->ConversionType () :
                                              func->TypeInfo ()->BaseType ();
    ostringstream type_str;
    type->TypeText (type_str, name.c_str (), true, true);
    return type_str.str ();
  }

  unsigned int args () const {
    Puma::CFunctionInfo *func = _obj->FunctionInfo();
    assert(func);
    return func->Arguments();
  }

  string arg_name (unsigned int no) const {
    Puma::CFunctionInfo *func = _obj->FunctionInfo();
    assert(func);
    Puma::CArgumentInfo *arg = func->Argument(no);
    return arg->isAnonymous() ? "" : arg->Name ().c_str();
  }

  string arg_type (unsigned int no, const string &name = "") const {
    Puma::CFunctionInfo *func = _obj->FunctionInfo();
      assert(func);
    ostringstream type_str;
    Puma::CTypeInfo *type = func->Argument(no)->TypeInfo();
    type->TypeText(type_str, name.c_str (), true, true);
    return type_str.str ();
  }

  string qualified_scope () const {
    if (_obj->QualifiedScope())
      return _obj->QualifiedScope()->QualName();
    return "";
  }

  string syntactical_scope () const {
    return _obj->Scope()->QualName();
  }

  // TODO: still needed?
  Puma::CObjectInfo *object () const { return _obj; }
};

#endif // __PumaSyntacticContext_h__


