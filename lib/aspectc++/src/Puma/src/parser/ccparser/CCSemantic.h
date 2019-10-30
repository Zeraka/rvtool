// This file is part of PUMA.
// Copyright (C) 1999-2003  The PUMA developer team.
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

#ifndef __CCSemantic_h__
#define __CCSemantic_h__

#include "Puma/CSemantic.h"
#include "Puma/CCBuilder.h"
#include "Puma/CCSyntax.h"
#include "Puma/CLinkage.h"
#include "Puma/CProtection.h"
#include "Puma/Stack.h"
#include "Puma/CTree.h"
#include "Puma/CCNameLookup.h"

namespace Puma {


class CFunctionInfo;
class CRecord;
class CCSemDeclarator;
class CCInstantiation;
class CTemplateInfo;
class CT_DelayedParse;

class CCSemantic : public CSemantic {
  CCSyntax &_syntax;

protected:
  CRecord* _outermost_class_def;

  Stack<bool> _in_base_spec;
  Stack<bool> _in_nested_name;
  Stack<bool> _in_conv_type_id;
  Stack<const char*> _in_extern_decl;
  Stack<CProtection::Type> _protection;

public:
  // for parsing nested names
  CStructure *search_scope;  
  CStructure *last_search_scope;  
  CStructure *last_scope;
  bool dep_nested_name;

  // different kinds of member access operators; MAO_NONE shall remain 0!
  enum MemberAccessOperator { MAO_NONE = 0, MAO_PTS, MAO_DOT };

protected:
  // for looking up type-names and class-or-namespace-names
  CCNameLookup shared_nl;
  struct { const Token *tok; CStructure *scope; bool nested, nested_name, base_spec; } last_token_nl;

  // for parsing templates
  Stack<Syntax::State> _in_template_arg_list;
  Stack<Syntax::State> _in_template_param_list;
  Stack<bool> parsing_entity_name;
  Stack<bool> _in_expr;
  Stack<CTree*> _postfix_expr;
  Stack<MemberAccessOperator> _in_memb_access_expr;
  bool expect_template;
  bool parsing_fct_inst;
  bool parsing_class_inst;
  bool real_inst;
  CCInstantiation *tpl_inst;

  // last function body skip state
  Unit *_last_unit;
  bool _last_body_skipped;

public:
  CCSemantic (CCSyntax &, CCBuilder &);

  void init (CSemDatabase &, Unit &);
  void init (CSemDatabase &, Unit &, CStructure *, bool, bool, CCInstantiation *);

  CCInstantiation* Instantiation() const;

protected:
  virtual CCBuilder &builder () const;
  virtual CCSyntax &syntax () const;
  virtual void Delete ();

public:
  // control parse process
  virtual CTree *access_spec ();
  virtual CTree *typedef_name ();
  virtual CTree *class_name ();
  virtual CTree *enum_name ();
  virtual CTree *condition ();
  virtual CTree *colon_colon ();
  virtual CTree *decl_spec_seq1 ();
  virtual CTree *nested_name_spec ();
  virtual CTree *nested_name_spec1 ();
  virtual CTree *valid_id_expr ();
  virtual CTree *postfix_expr1 ();
  virtual CTree *postfix_expr2 ();
  virtual CTree *pseudo_dtor_name ();
  virtual CTree *simple_type_spec ();
  virtual CTree *direct_new_declarator ();
  virtual CTree *class_spec_err ();
  virtual CTree *member_decl1 ();
  virtual CTree *member_declarator_err ();
  virtual void declarator_id (bool);
  virtual CTree *finish_init_declarator ();
  virtual CTree *obj_decl (CTree *);
  virtual bool implicit_int ();

  // constant expression evaluation
  virtual CTree *init_declarator ();
  virtual CTree *const_expr ();
  virtual CTree *enumerator_def ();
  virtual CTree *pure_spec ();
  virtual CTree *const_init ();
  virtual CTree *template_param ();

  // add scope information to tree nodes
  virtual CTree *class_spec ();
  virtual CTree *ctor_init ();
  virtual CTree *handler ();
  virtual CTree *exception_spec ();

  // namespaces
  virtual CTree *introduce_orig_ns ();
  virtual CTree *introduce_ns_extension ();
  virtual CTree *introduce_unnamed_ns ();
  virtual CTree *introduce_ns_alias ();
  virtual CTree *namespace_def ();
  virtual CTree *original_ns_name ();
  virtual CTree *namespace_alias ();
  virtual CTree *using_directive ();
  virtual CTree *using_decl ();
  virtual CTree *access_decl (CTree* = 0);

  // templates
  virtual CTree *introduce_type_param ();
  virtual CTree *introduce_non_type_param ();
  virtual CTree *introduce_template_instance ();
  virtual CTree *template_param_list ();
  virtual CTree *template_decl ();
  virtual CTree *template_name (bool class_only = false);
  virtual CTree *template_key ();
  virtual CTree *explicit_instantiation ();
  virtual CTree *explicit_specialization ();

  // introduce names... add new entries to the class database
  virtual CTree *introduce_enum ();
  virtual CTree *introduce_enumerator ();
  virtual CTree *introduce_object (CTree* = 0);
  virtual CTree *introduce_named_type ();
  virtual CTree *introduce_function ();
  virtual CTree *introduce_parameter ();
  virtual CTree *introduce_class ();
  virtual CTree *introduce_member ();
  virtual CTree *introduce_tag ();
  virtual CTree *introduce_exception ();

public: // syntactic scope information
  void enter_param_decl_clause ();
  void enter_template_param_list (Syntax::State);
  void enter_base_spec ();
  void enter_class_def ();
  void enter_template_decl ();
  void enter_member_access (MemberAccessOperator);
  void enter_postfix_expr ();
  void enter_exception_spec ();
  void enter_template_arg_list (Syntax::State);
  void enter_conv_type_id ();
  void enter_entity_name (bool v = true);
  void enter_nested_name ();
  bool enter_expr ();
  void enter_extern_decl (bool block_decl);

  void leave_param_decl_clause ();
  void leave_template_param_list ();
  void leave_base_spec ();
  void leave_class_def (bool reject = false);
  void leave_member_access ();
  void leave_postfix_expr ();
  void leave_exception_spec ();
  void leave_template_arg_list ();
  void leave_conv_type_id ();
  void leave_entity_name ();
  void leave_nested_name ();
  bool leave_expr ();
  void leave_extern_decl ();

  bool in_base_spec () const;
  bool in_class_def () const;
  bool in_conv_type_id () const;
  bool in_nested_name () const;
  bool in_expr () const;
  bool in_extern_decl () const;
  bool is_entity_name () const;
  Syntax::State in_template_param_list () const;
  Syntax::State in_template_arg_list () const;
  virtual CProtection::Type protection () const;

  bool non_project_loc ();
  bool non_primary_loc ();
  bool skip_tpl_fct_bodies () const;
  bool reset_search_scope (bool = false);
  CStructure *array_delim (CStructure * = 0);
  CStructure *conv_fct_id (CStructure * = 0);
  CTree *postfix_expr ();

public: // class definition parsing
  virtual void parse_delayed (bool (CCSyntax::*)(), CTree *);
  virtual CTree *add_base_classes ();

protected:
  CTree* createDelayedParse (bool (CCSyntax::*)(), CTree *);
  void parseDelayed ();
  void instantiateDelayed (CT_DelayedParse*, CTree*);

  void enter_scope (CStructure *);
  void deleteContainer (int);
  void setQualName (CObjectInfo *, CObjectInfo *) const;
  CStructure *getQualifiedScope (CStructure *scope) const;

  CTypeInfo *resolveExpr (CTree *, CTree * = (CTree*)0) const;
  void calculateAddress (CTree *) const;

  CObjectInfo *getLastResolvedName (CT_SimpleName *) const;
  CStructure *getNameScope (CT_SimpleName *, bool = true) const;
  CStructure *getScope (CObjectInfo *obj) const;
  CStructure *getFriendScope (bool skip_tpl_scope = false) const;
  CStructure *getNonTplScope (bool consider_instance_scope = false) const;
  CClassInfo *getOutermostIncompleteParsedRecord ();
  Token* getCloseCurly ();

  void qualifiedScopeName (CT_SimpleName *, std::ostringstream &) const;
  bool isQualName (CT_SimpleName *) const;
  bool realTypedef (CObjectInfo *) const;
  bool equalType (CTypeInfo *, CTypeInfo *, bool = true) const;
  void lookupName (const Token*, CStructure *, bool);
  bool isClassOrEnum (CObjectInfo *) const;

  bool inTemplateDecl (CScopeInfo* scope) const;
  void introduceTemplate (CObjectInfo *, CTemplateInfo *, bool is_friend = false);
  void setTemplateScope (CCNameLookup &, CObjectInfo *, CTypeInfo *, CTemplateInfo *, bool);
  bool isSpecialization (CTree *) const;
  void setSpecializationName (CTree *, CTemplateInfo *) const;
  bool matchArguments (CT_TemplateName *n1, CT_TemplateName *n2) const;

  CLinkage::Type determine_linkage (CSemDeclSpecs *, SemObjType,
    CObjectInfo * = (CObjectInfo*)0) const;
  CStorage::Type determine_storage_class (CSemDeclSpecs *) const;
  void setLanguage (CObjectInfo *) const;
  void setProtection (CObjectInfo *) const;
  void linkFctDecls (CCNameLookup &, CObjectInfo *) const;
  void linkCtorDecls (CObjectInfo *) const;
  void addImplicitThisObject (CFunctionInfo *, CTypeInfo *, CTree *);

  void declareNamespaceStd ();
  void declareImplicitFcts ();
  void declareSpecialMembers ();
  CFunctionInfo *createMethod (const char *, CRecord *, CTypeInfo *);
  bool isRefToClass (CTypeInfo *, CRecord *) const;

  bool isBaseClass (CClassInfo *, CClassInfo *) const;
  void addBaseClasses (CClassInfo *, CT_ClassDef *);
  void addBaseClass (CClassInfo *, CT_BaseSpec *);

  virtual bool check_function_lookup (CCNameLookup &, CStructure *, 
   CSemDeclSpecs *, CCSemDeclarator &, CTree *, bool);
  virtual bool check_member_lookup (CCNameLookup &, CStructure *, 
   CSemDeclSpecs *, CCSemDeclarator &, CTree *, bool &, bool);
  virtual bool check_object_lookup (CCNameLookup &, CStructure *, 
   CSemDeclSpecs *, CCSemDeclarator &, CTree *, bool &, CObjectInfo *&);
};

inline CCBuilder &CCSemantic::builder () const 
 { return (CCBuilder&) CSemantic::builder (); }
inline CCSyntax &CCSemantic::syntax () const 
 { return (CCSyntax&) _syntax; }

inline bool CCSemantic::in_class_def () const 
 { return _outermost_class_def; }

inline bool CCSemantic::in_base_spec () const 
 { return _in_base_spec.top (); }
inline void CCSemantic::enter_base_spec () 
 { _in_base_spec.push (true); }
inline void CCSemantic::leave_base_spec ()
 { _in_base_spec.pop (); }

inline bool CCSemantic::in_nested_name () const 
 { return _in_nested_name.top (); }
inline void CCSemantic::enter_nested_name () 
 { _in_nested_name.push (true); dep_nested_name = false; }
inline void CCSemantic::leave_nested_name ()
 { _in_nested_name.pop (); }

inline bool CCSemantic::in_conv_type_id () const 
 { return _in_conv_type_id.top (); }
inline void CCSemantic::enter_conv_type_id () 
 { _in_conv_type_id.push (true); }
inline void CCSemantic::leave_conv_type_id ()
 { _in_conv_type_id.pop (); }

inline bool CCSemantic::in_expr () const 
 { return _in_expr.top (); }
inline bool CCSemantic::enter_expr () 
 { _in_expr.push (true); return true; }
inline bool CCSemantic::leave_expr ()
 { _in_expr.pop (); return true; }

inline void CCSemantic::enter_member_access (MemberAccessOperator mao)
 { _in_memb_access_expr.push (mao); }
inline void CCSemantic::leave_member_access ()
 { _in_memb_access_expr.pop (); }
inline void CCSemantic::enter_postfix_expr ()
 { _postfix_expr.push (0); _in_memb_access_expr.push (MAO_NONE); }
inline void CCSemantic::leave_postfix_expr ()
 { _postfix_expr.pop (); _in_memb_access_expr.pop (); }

inline void CCSemantic::setProtection (CObjectInfo *info) const 
 { info->Protection (_protection.top ()); }
inline CProtection::Type CCSemantic::protection () const
 { return _protection.top (); }
 
inline void CCSemantic::enter_template_arg_list (Syntax::State state) 
 { _in_template_arg_list.push (state); enter_postfix_expr (); }
inline void CCSemantic::leave_template_arg_list () 
 { _in_template_arg_list.pop (); leave_postfix_expr (); }
inline Syntax::State CCSemantic::in_template_arg_list () const 
 { return _in_template_arg_list.top (); }
inline void CCSemantic::leave_template_param_list () 
 { _in_template_param_list.pop (); }
inline Syntax::State CCSemantic::in_template_param_list () const 
 { return _in_template_param_list.top (); }

inline void CCSemantic::enter_entity_name (bool v) 
 { parsing_entity_name.push (v); }
inline void CCSemantic::leave_entity_name ()
 { parsing_entity_name.pop (); }
inline bool CCSemantic::is_entity_name () const
 { return parsing_entity_name.top (); }

inline void CCSemantic::leave_extern_decl () 
 { if (_in_extern_decl.top ()) delete[] _in_extern_decl.top (); _in_extern_decl.pop (); }
inline bool CCSemantic::in_extern_decl () const
 { return _in_extern_decl.top (); }

inline CCInstantiation* CCSemantic::Instantiation() const
 { return tpl_inst; }


} // namespace Puma

#endif /* __CCSemantic_h__ */
