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

#ifndef __CCSyntax_h__
#define __CCSyntax_h__

// Parser for the C++ programming language

#include "Puma/CSyntax.h"

namespace Puma {


class CCSemantic;
class CCBuilder;
class CStructure;

class CCSyntax : public CSyntax {
  friend class CCSemantic;
  int _skip_bodies;
  
  enum {
    SKIP_BODIES_NONE               = 0x0,  // don't skip function bodies
    SKIP_BODIES_ALL                = 0x1,  // skip all function bodies
    SKIP_BODIES_TPL                = 0x2,  // skip function bodies of templates
    SKIP_BODIES_NON_PRJ            = 0x4,  // skip bodies of non-project functions
    SKIP_BODIES_NON_PRIM           = 0x8   // skip bodies in non-primary files
  };

public:
  CCSyntax (CCBuilder &, CCSemantic &);
  
  virtual Grammar grammar () const { return GRAMMAR_CPLUSPLUS; }

  virtual void configure (Config &);
  
  void config_skip_fct_body (bool s) { 
    if (s) _skip_bodies |= SKIP_BODIES_ALL; 
    else   _skip_bodies ^= (_skip_bodies & SKIP_BODIES_ALL); 
  }

protected:
  CCBuilder &builder () const;
  CCSemantic &semantic () const;

  // TODO: no idea why this can't be introduce by CCLookAhead.ah
  // Specific look-ahead functions
  virtual bool is_fct_def ();
  virtual bool is_nested_name ();
  virtual bool is_class_def ();
  virtual bool is_tpl_id ();
  virtual bool is_tpl_declarator_id ();
  virtual bool is_ptr_to_fct ();
  virtual bool is_nested (State);
  virtual bool is_ass_expr ();

  virtual void init_prim_types ();
  virtual void init_cv_quals ();
  
  virtual void init_explicit_instantiation ();
  virtual void init_explicit_specialization ();
  virtual void init_access_spec ();
  virtual void init_oper_fct_id ();
  virtual void init_template_key ();
  virtual void init_template_id ();
  virtual void init_class_template_id ();

  // Grammar rules
public:

  // A.1 Keywords
  struct ClassName {
    static inline bool check (CCSyntax &s) { return s.class_name (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool class_name ();

  struct EnumName {
    static inline bool check (CCSyntax &s) { return s.enum_name (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool enum_name ();

  struct TemplateName {
    static inline bool check (CCSyntax &s) { return s.template_name (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool template_name ();

  struct ClassTemplateName {
    static inline bool check (CCSyntax &s) { return s.class_template_name (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool class_template_name ();

  struct NamespaceName {
    static inline bool check (CCSyntax &s) { return s.namespace_name (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool namespace_name ();

  struct OriginalNsName {
    static inline bool check (CCSyntax &s) { return s.original_ns_name (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool original_ns_name ();

  struct NamespaceAlias {
    static inline bool check (CCSyntax &s) { return s.namespace_alias (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool namespace_alias ();
  
  // A.2 Lexical conventions
  struct Literal {
    static inline bool check (CCSyntax &s) { return s.literal (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool literal ();

  // A.4 Expression
  struct PrimExpr {
    static inline bool check (CCSyntax &s) { return s.prim_expr (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool prim_expr ();

  struct IdExpr {
    static inline bool check (CCSyntax &s) { return s.id_expr (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool id_expr ();

  struct QualId {
    static inline bool check (CCSyntax &s) { return s.qual_id (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool qual_id ();

  struct UnqualId {
    static inline bool check (CCSyntax &s) { return s.unqual_id (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool unqual_id ();

  struct ColonColon {
    static inline bool check (CCSyntax &s) { return s.colon_colon (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool colon_colon ();

  struct NestedNameSpec {
    static inline bool check (CCSyntax &s) { return s.nested_name_spec (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool nested_name_spec ();

  struct NestedNameSpec1 {
    static inline bool check (CCSyntax &s) { return s.nested_name_spec1 (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool nested_name_spec1 ();

  struct ClassOrNsName {
    static inline bool check (CCSyntax &s) { return s.class_or_ns_name (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool class_or_ns_name ();

  struct PostfixExpr {
    static inline bool check (CCSyntax &s) { return s.postfix_expr (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool postfix_expr ();

  struct PostfixExpr1 {
    static inline bool check (CCSyntax &s) { return s.postfix_expr1 (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool postfix_expr1 ();

  struct PostfixExpr2 {
    static inline bool check (CCSyntax &s) { return s.postfix_expr2 (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool postfix_expr2 ();

  struct ConstructExpr {
    static inline bool check (CCSyntax &s) { return s.construct_expr (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool construct_expr ();

  struct PseudoDtorName {
    static inline bool check (CCSyntax &s) { return s.pseudo_dtor_name (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool pseudo_dtor_name ();

  struct UnaryExpr {
    static inline bool check (CCSyntax &s) { return s.unary_expr (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool unary_expr ();

  struct TypeTraitExpr {
    static inline bool check (CCSyntax &s) { return s.type_trait_expr (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool type_trait_expr ();

  struct NewExpr {
    static inline bool check (CCSyntax &s) { return s.new_expr (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool new_expr ();

  struct NewPlacement {
    static inline bool check (CCSyntax &s) { return s.new_placement (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool new_placement ();

  struct NewTypeId {
    static inline bool check (CCSyntax &s) { return s.new_type_id (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool new_type_id ();

  struct NewDeclarator {
    static inline bool check (CCSyntax &s) { return s.new_declarator (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool new_declarator ();

  struct DirectNewDeclarator {
    static inline bool check (CCSyntax &s) { return s.direct_new_declarator (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool direct_new_declarator ();

  struct DirectNewDeclarator1 {
    static inline bool check (CCSyntax &s) { return s.direct_new_declarator1 (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool direct_new_declarator1 ();
  
  struct NewInit {
    static inline bool check (CCSyntax &s) { return s.new_init (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool new_init ();

  struct DeleteExpr {
    static inline bool check (CCSyntax &s) { return s.delete_expr (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool delete_expr ();

  struct PmExpr {
    static inline bool check (CCSyntax &s) { return s.pm_expr (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool pm_expr ();

  struct MulExpr {
    static inline bool check (CCSyntax &s) { return s.mul_expr (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool mul_expr ();

  struct RelExpr {
    static inline bool check (CCSyntax &s) { return s.rel_expr (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool rel_expr ();

  struct CondExpr {
    static inline bool check (CCSyntax &s) { return s.cond_expr (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool cond_expr ();

  struct AssExpr {
    static inline bool check (CCSyntax &s) { return s.ass_expr (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool ass_expr ();

  struct AssExpr1 {
    static inline bool check (CCSyntax &s) { return s.ass_expr1 (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool ass_expr1 ();

  struct ConstExpr {
    static inline bool check (CCSyntax &s) { return s.const_expr (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool const_expr ();

  // A.5 Statements
  struct Stmt {
    static inline bool check (CCSyntax &s) { return s.stmt (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool stmt ();

  struct StmtSeq {
    static inline bool check (CCSyntax &s) { return s.stmt_seq (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool stmt_seq ();

  struct SubStmt {
    static inline bool check (CCSyntax &s) { return s.sub_stmt (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool sub_stmt ();

  struct Condition {
    static inline bool check (CCSyntax &s) { return s.condition (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool condition ();

  struct Condition1 {
    static inline bool check (CCSyntax &s) { return s.condition1 (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool condition1 ();

  struct Condition2 {
    static inline bool check (CCSyntax &s) { return s.condition2 (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool condition2 ();

  struct DeclStmt {
    static inline bool check (CCSyntax &s) { return s.decl_stmt (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool decl_stmt ();

  // A.6 Declarations
  
  struct Decl {
    static inline bool check (CCSyntax &s) { return s.decl (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool decl ();
  // helper function, which is needed, because ac++ can't weave in templates :-(
  virtual bool decl_check ();

  struct BlockDecl {
    static inline bool check (CCSyntax &s) { return s.block_decl (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool block_decl ();

  struct SimpleDecl {
    static inline bool check (CCSyntax &s) { return s.simple_decl (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool simple_decl ();

  struct DeclSpecSeq1 {
    static inline bool check (CCSyntax &s) { return s.decl_spec_seq1 (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool decl_spec_seq1 ();

  struct MiscSpec {
    static inline bool check (CCSyntax &s) { return s.misc_spec (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool misc_spec ();

  struct StorageClassSpec {
    static inline bool check (CCSyntax &s) { return s.storage_class_spec (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool storage_class_spec ();

  struct FctSpec {
    static inline bool check (CCSyntax &s) { return s.fct_spec (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool fct_spec ();

  struct SimpleTypeSpec {
    static inline bool check (CCSyntax &s) { return s.simple_type_spec (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool simple_type_spec ();

  struct TypeName {
    static inline bool check (CCSyntax &s) { return s.type_name (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool type_name ();

  struct ElaboratedTypeSpec {
    static inline bool check (CCSyntax &s) { return s.elaborated_type_spec (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool elaborated_type_spec ();

  struct EnumeratorList {
    static inline bool check (CCSyntax &s) { return s.enumerator_list (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool enumerator_list ();

  struct EnumeratorDef {
    static inline bool check (CCSyntax &s) { return s.enumerator_def (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool enumerator_def ();

  struct LinkageSpec {
    static inline bool check (CCSyntax &s) { return s.linkage_spec (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool linkage_spec ();

  // A.6.1 Namespaces
  struct NamespaceDef {
    static inline bool check (CCSyntax &s) { return s.namespace_def (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool namespace_def ();

  struct NamedNsDef {
    static inline bool check (CCSyntax &s) { return s.named_ns_def (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool named_ns_def ();

  struct OriginalNsDef {
    static inline bool check (CCSyntax &s) { return s.original_ns_def (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool original_ns_def ();

  struct OriginalNsDef1 {
    static inline bool check (CCSyntax &s) { return s.original_ns_def1 (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool original_ns_def1 ();

  struct ExtensionNsDef {
    static inline bool check (CCSyntax &s) { return s.extension_ns_def (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool extension_ns_def ();

  struct ExtensionNsDef1 {
    static inline bool check (CCSyntax &s) { return s.extension_ns_def1 (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool extension_ns_def1 ();

  struct UnnamedNsDef {
    static inline bool check (CCSyntax &s) { return s.unnamed_ns_def (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool unnamed_ns_def ();

  struct UnnamedNsDef1 {
    static inline bool check (CCSyntax &s) { return s.unnamed_ns_def1 (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool unnamed_ns_def1 ();

  struct NamespaceBody {
    static inline bool check (CCSyntax &s) { return s.namespace_body (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool namespace_body ();

  struct NsAliasDef {
    static inline bool check (CCSyntax &s) { return s.ns_alias_def (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool ns_alias_def ();

  struct QualNsSpec {
    static inline bool check (CCSyntax &s) { return s.qual_ns_spec (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool qual_ns_spec ();

  struct UsingDecl {
    static inline bool check (CCSyntax &s) { return s.using_decl (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool using_decl ();

  struct UsingDirective {
    static inline bool check (CCSyntax &s) { return s.using_directive (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool using_directive ();

  // A.7 Declarators
  struct InitDeclarator {
    static inline bool check (CCSyntax &s) { return s.init_declarator (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool init_declarator ();

  struct DirectDeclarator1 {
    static inline bool check (CCSyntax &s) { return s.direct_declarator1 (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool direct_declarator1 ();

  struct ArrayDelim {
    static inline bool check (CCSyntax &s) { return s.array_delim (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool array_delim ();

  struct PtrOperator {
    static inline bool check (CCSyntax &s) { return s.ptr_operator (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool ptr_operator ();

  struct DeclaratorId {
    static inline bool check (CCSyntax &s) { return s.declarator_id (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool declarator_id ();

  struct DirectAbstDeclarator {
    static inline bool check (CCSyntax &s) { return s.direct_abst_declarator (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool direct_abst_declarator ();

  struct ParamDeclClause {
    static inline bool check (CCSyntax &s) { return s.param_decl_clause (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool param_decl_clause ();

  CTree * rule_param_decl ();
  virtual bool param_decl ();
  CTree * rule_param_init ();
  virtual bool param_init ();
  CTree * rule_fct_def ();
  virtual bool fct_def ();
  CTree * rule_skipped_fct_body ();
  virtual bool skipped_fct_body ();
  CTree * rule_fct_body ();
  virtual bool fct_body ();
  CTree * rule_init ();
  virtual bool init ();
  CTree * rule_init_clause ();
  virtual bool init_clause ();

  // A.8 Classes
  CTree *rule_class_spec ();
  virtual bool class_spec ();

  struct ClassHead {
    static inline bool check (CCSyntax &s) { return s.class_head (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool class_head ();

  struct ClassHead1 {
    static inline bool check (CCSyntax &s) { return s.class_head1 (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool class_head1 ();

  CTree * rule_member_decl ();
  virtual bool member_decl ();
  CTree * rule_member_decl1 ();
  virtual bool member_decl1 ();
  CTree * rule_access_decl ();
  virtual bool access_decl ();
  CTree * rule_member_declarator ();
  virtual bool member_declarator ();
  CTree * rule_pure_spec ();
  virtual bool pure_spec ();
  CTree * rule_const_init ();
  virtual bool const_init ();

  // A.9 Derived classes
  CTree * rule_base_clause ();
  virtual bool base_clause ();
  CTree * rule_base_spec_list ();
  virtual bool base_spec_list ();
  CTree * rule_base_spec ();
  virtual bool base_spec ();
  CTree * rule_access_spec ();
  virtual bool access_spec ();

  // A.10 Special member functions
  struct ConvFctId {
    static inline bool check (CCSyntax &s) { return s.conv_fct_id (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool conv_fct_id ();

  struct ConvTypeId {
    static inline bool check (CCSyntax &s) { return s.conv_type_id (); }
    static inline bool parse (CCSyntax &);
  };
  virtual bool conv_type_id ();

  CTree * rule_conv_declarator ();
  virtual bool conv_declarator ();
  CTree * rule_ctor_init ();
  virtual bool ctor_init ();
  CTree * rule_mem_init_list ();
  virtual bool mem_init_list ();
  CTree * rule_mem_init ();
  virtual bool mem_init ();
  CTree * rule_mem_init_id ();
  virtual bool mem_init_id ();

  // A.11 Overloading
  CTree *rule_oper_fct_id ();
  virtual bool oper_fct_id ();

  // A.12 Templates
  CTree * rule_template_key (); 
  virtual bool template_key (); 
  CTree * rule_template_decl (); 
  virtual bool template_decl (); 
  CTree * rule_member_template_decl (); 
  virtual bool member_template_decl (); 
  CTree * rule_template_param_list (); 
  virtual bool template_param_list (); 
  CTree * rule_template_param (); 
  virtual bool template_param (); 
  CTree * rule_type_param (); 
  virtual bool type_param (); 
  CTree * rule_non_type_param (); 
  virtual bool non_type_param (); 
  CTree *rule_template_id (); 
  virtual bool template_id ();
  CTree *rule_class_template_id (); 
  virtual bool class_template_id ();
  CTree * rule_template_arg_list (); 
  virtual bool template_arg_list (); 
  CTree * rule_template_arg (); 
  virtual bool template_arg (); 
  CTree * rule_template_type_arg (); 
  virtual bool template_type_arg (); 
  CTree * rule_template_non_type_arg (); 
  virtual bool template_non_type_arg (); 
  CTree * rule_template_template_arg (); 
  virtual bool template_template_arg (); 
  CTree * rule_explicit_instantiation (); 
  virtual bool explicit_instantiation (); 
  CTree * rule_explicit_specialization (); 
  virtual bool explicit_specialization (); 

  // A.13 Exception handling
  CTree * rule_try_block ();
  virtual bool try_block ();
  CTree * rule_fct_try_block ();
  virtual bool fct_try_block ();
  CTree * rule_handler_seq ();
  virtual bool handler_seq ();
  CTree * rule_handler ();
  virtual bool handler ();
  CTree * rule_exception_decl ();
  virtual bool exception_decl ();
  CTree * rule_throw_expr ();
  virtual bool throw_expr ();
  CTree * rule_exception_spec ();
  virtual bool exception_spec ();
  CTree * rule_type_id_list (); 
  virtual bool type_id_list (); 

protected:
  void skip_param_init ();
  void skip_ctor_init ();
  void skip_fct_body ();
  void skip_fct_try_block ();
  void skip_const_expr ();
  void skip_const_init ();

protected:
  struct SearchScope { CStructure *scope, *last_scope; bool dep; };
  void get_search_scope (SearchScope &);
  void set_search_scope (SearchScope &);
};


} // namespace Puma

#endif /* __CCSyntax_h__ */
