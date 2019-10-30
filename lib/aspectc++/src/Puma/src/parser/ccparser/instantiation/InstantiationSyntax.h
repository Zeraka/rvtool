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

#ifndef __InstantiationSyntax_h__
#define __InstantiationSyntax_h__

// Parser for template instances

#include "Puma/CCSyntax.h"
#include "Puma/CTree.h"
#include "Puma/InstantiationTokenSource.h"
#include "Puma/InstantiationTokenProvider.h"

namespace Puma {


class InstantiationBuilder;
class InstantiationSemantic;

class InstantiationSyntax : public CCSyntax {
public:
  InstantiationSyntax (InstantiationBuilder &, InstantiationSemantic &);

  virtual void configure (Config &);

  InstantiationTokenSource &source () const;
  InstantiationTokenProvider &provider () const;

protected:
  InstantiationBuilder &builder () const;
  InstantiationSemantic &semantic () const;

protected: // Optimized parse way helper
  bool isRoot(int up) const;

  template<class NODE>
  bool is(int up) const;

protected: // Look-ahead
//  virtual bool is_fct_def ();
//  virtual bool is_nested_name ();
//  virtual bool is_class_def ();
//  virtual bool is_tpl_id ();
//  virtual bool is_tpl_declarator_id ();
//  virtual bool is_ptr_to_fct ();
//  virtual bool is_nested (State);
//  virtual bool is_ass_expr ();
//
//  virtual void init_prim_types ();
//  virtual void init_cv_quals ();
//
//  virtual void init_explicit_instantiation ();
//  virtual void init_explicit_specialization ();
//  virtual void init_access_spec ();
//  virtual void init_oper_fct_id ();
//  virtual void init_template_key ();
//  virtual void init_template_id ();
//  virtual void init_class_template_id ();

public: // Grammar rules

  // A.1 Keywords
  virtual bool class_name ();
  virtual bool enum_name ();
  virtual bool template_name ();
  virtual bool class_template_name ();
  virtual bool namespace_name ();
  virtual bool typedef_name ();
//  virtual bool original_ns_name ();
//  virtual bool namespace_alias ();
//
//  // A.2 Lexical conventions
//  virtual bool literal ();
//
//  // A.4 Expression
//  virtual bool prim_expr ();
//  virtual bool id_expr ();
//  virtual bool qual_id ();
//  virtual bool unqual_id ();
//  virtual bool colon_colon ();
//  virtual bool nested_name_spec ();
//  virtual bool nested_name_spec1 ();
//  virtual bool class_or_ns_name ();
//  virtual bool postfix_expr ();
//  virtual bool postfix_expr1 ();
//  virtual bool postfix_expr2 ();
//  virtual bool construct_expr ();
//  virtual bool pseudo_dtor_name ();
//  virtual bool unary_expr ();
//  virtual bool type_trait_expr ();
//  virtual bool new_expr ();
//  virtual bool new_placement ();
//  virtual bool new_type_id ();
//  virtual bool new_declarator ();
//  virtual bool direct_new_declarator ();
//  virtual bool direct_new_declarator1 ();
//  virtual bool new_init ();
//  virtual bool delete_expr ();
//  virtual bool pm_expr ();
//  virtual bool mul_expr ();
//  virtual bool rel_expr ();
//  virtual bool cond_expr ();
//  virtual bool ass_expr ();
//  virtual bool ass_expr1 ();
//  virtual bool const_expr ();
//  virtual bool stmt ();
//  virtual bool stmt_seq ();
//  virtual bool sub_stmt ();
//  virtual bool condition ();
//  virtual bool condition1 ();
//  virtual bool condition2 ();
//  virtual bool decl_stmt ();
//
//  // A.6 Declarations
//  virtual bool decl ();
//  virtual bool decl_check ();
//  virtual bool block_decl ();
//  virtual bool simple_decl ();
//  virtual bool decl_spec_seq1 ();
//  virtual bool misc_spec ();
//  virtual bool storage_class_spec ();
//  virtual bool fct_spec ();
//  virtual bool simple_type_spec ();
//  virtual bool type_name ();
//  virtual bool elaborated_type_spec ();
//  virtual bool enumerator_list ();
//  virtual bool enumerator_def ();
//  virtual bool linkage_spec ();
//
//  // A.6.1 Namespaces
//  virtual bool namespace_def ();
//  virtual bool named_ns_def ();
//  virtual bool original_ns_def ();
//  virtual bool original_ns_def1 ();
//  virtual bool extension_ns_def ();
//  virtual bool extension_ns_def1 ();
//  virtual bool unnamed_ns_def ();
//  virtual bool unnamed_ns_def1 ();
//  virtual bool namespace_body ();
//  virtual bool ns_alias_def ();
//  virtual bool qual_ns_spec ();
//  virtual bool using_decl ();
//  virtual bool using_directive ();
//
//  // A.7 Declarators
//  virtual bool init_declarator ();
//  virtual bool direct_declarator1 ();
//  virtual bool array_delim ();
//  virtual bool ptr_operator ();
//  virtual bool declarator_id ();
//  virtual bool direct_abst_declarator ();
//  virtual bool param_decl_clause ();
//  virtual bool param_decl ();
//  virtual bool param_init ();
//  virtual bool fct_def ();
//  virtual bool skipped_fct_body ();
//  virtual bool fct_body ();
//  virtual bool init ();
//  virtual bool init_clause ();
//
//  // A.8 Classes
//  virtual bool class_spec ();
//  virtual bool class_head ();
//  virtual bool class_head1 ();
//  virtual bool member_decl ();
//  virtual bool member_decl1 ();
//  virtual bool access_decl ();
//  virtual bool member_declarator ();
//  virtual bool pure_spec ();
//  virtual bool const_init ();
//
//  // A.9 Derived classes
//  virtual bool base_clause ();
//  virtual bool base_spec_list ();
//  virtual bool base_spec ();
//  virtual bool access_spec ();
//
//  // A.10 Special member functions
//  virtual bool conv_fct_id ();
//  virtual bool conv_type_id ();
//  virtual bool conv_declarator ();
//  virtual bool ctor_init ();
//  virtual bool mem_init_list ();
//  virtual bool mem_init ();
//  virtual bool mem_init_id ();
//
//  // A.11 Overloading
//  virtual bool oper_fct_id ();
//
//  // A.12 Templates
//  virtual bool template_key ();
//  virtual bool template_decl ();
//  virtual bool member_template_decl ();
//  virtual bool template_param_list ();
//  virtual bool template_param ();
//  virtual bool type_param ();
//  virtual bool non_type_param ();
  virtual bool template_id ();
  virtual bool class_template_id ();
//  virtual bool template_arg_list ();
//  virtual bool template_arg ();
//  virtual bool explicit_instantiation ();
//  virtual bool explicit_specialization ();

  // Special rules for template parameter instantiation
  bool type_param_without_init();
  CTree *rule_type_param_without_init();
  bool non_type_param_without_init();
  CTree *rule_non_type_param_without_init();

//  // A.13 Exception handling
//  virtual bool try_block ();
//  virtual bool fct_try_block ();
//  virtual bool handler_seq ();
//  virtual bool handler ();
//  virtual bool exception_decl ();
//  virtual bool throw_expr ();
//  virtual bool exception_spec ();
//  virtual bool type_id_list ();
};


template<class NODE>
inline bool InstantiationSyntax::is(int up) const {
//  CTree* node = provider().node();
  CTree* node = (CTree*)provider().current_context();
  for (int i = 0; i < up && node; i++)
    node = node->Parent();
  return node ? node->NodeName() == NODE::NodeId() : false;
}


} // namespace Puma

#endif /* __InstantiationSyntax_h__ */
