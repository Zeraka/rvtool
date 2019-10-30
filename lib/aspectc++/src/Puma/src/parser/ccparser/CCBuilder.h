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

#ifndef __CCBuilder_h__
#define __CCBuilder_h__

#include "Puma/CBuilder.h"

namespace Puma {


class CCBuilder : public CBuilder {
public:
  CCBuilder () {}

  // functions to create several kinds of syntax trees

  // A.1 Keywords
  virtual CTree *namespace_name ();

  // A.4 Expression
  virtual CTree *prim_expr ();
  virtual CTree *id_expr ();
  virtual CTree *unqual_id ();
  virtual CTree *qual_id ();
  virtual CTree *nested_name_spec ();
  virtual CTree *nested_name_spec1 ();
  virtual CTree *class_or_ns_name ();
  virtual CTree *postfix_expr (CTree * = (CTree*)0);
  virtual CTree *postfix_expr2 ();
  virtual CTree *construct_expr ();
  virtual CTree *pseudo_dtor_name ();
  virtual CTree *type_trait_expr ();
  virtual CTree *new_expr ();
  virtual CTree *new_placement ();
  virtual CTree *direct_new_declarator ();
  virtual CTree *direct_new_declarator1 ();
  virtual CTree *new_init ();
  virtual CTree *delete_expr ();
  virtual CTree *pm_expr ();

  // A.5 Statements
  virtual CTree *condition (); 
  virtual CTree *decl_stmt (); 

  // A.6 Declarations
  virtual CTree *decl ();
  virtual CTree *simple_type_spec ();
  virtual CTree *elaborated_type_spec ();
  virtual CTree *linkage_spec ();

  // A.6.1 Namespaces
  virtual CTree *namespace_def ();
  virtual CTree *named_ns_def ();
  virtual CTree *orig_namespace_def ();
  virtual CTree *orig_namespace_def1 ();
  virtual CTree *namespace_body ();
  virtual CTree *ns_alias_def ();
  virtual CTree *qual_ns_spec ();
  virtual CTree *using_decl ();
  virtual CTree *using_directive ();

  // A.7 Declarators
  virtual CTree *declarator ();
  virtual CTree *declarator_id ();
  virtual CTree *param_init ();

  // A.8 Classes
  virtual CTree *class_head ();
  virtual CTree *class_head1 ();
  virtual CTree *class_spec ();
  virtual CTree *member_decl ();
  virtual CTree *member_decl1 ();
  virtual CTree *access_decl ();
  virtual CTree *member_declarator ();
  virtual CTree *pure_spec ();
  virtual CTree *const_init ();

  // A.9 Derived classes
  virtual CTree *base_clause ();
  virtual CTree *base_spec_list ();
  virtual CTree *base_spec ();
  virtual CTree *access_spec ();

  // A.10 Special member functions
  virtual CTree *conv_fct_id ();
  virtual CTree *ctor_init ();
  virtual CTree *mem_init_list ();
  virtual CTree *mem_init ();
  virtual CTree *mem_init_id ();

  // A.11 Overloading
  virtual CTree *oper_fct_id ();
  
  // A.12 Templates
  virtual CTree *template_decl (); 
  virtual CTree *template_param_list (); 
  virtual CTree *template_param (); 
  virtual CTree *type_param (); 
  virtual CTree *non_type_param (CTree * = (CTree*)0); 
  virtual CTree *template_id (); 
  virtual CTree *template_arg_list (); 
  virtual CTree *template_arg (); 
  virtual CTree *template_type_arg (); 
  virtual CTree *template_non_type_arg (); 
  virtual CTree *template_template_arg (); 
  virtual CTree *explicit_instantiation (); 
  virtual CTree *explicit_specialization (); 

  // A.13 Exception handling
  virtual CTree *try_block ();
  virtual CTree *fct_try_block ();
  virtual CTree *handler_seq ();
  virtual CTree *handler ();
  virtual CTree *exception_decl ();
  virtual CTree *throw_expr ();
  virtual CTree *exception_spec ();
  virtual CTree *type_id_list (); 

protected:
  void cleanup(CTree* tree);
  void Delete();
};


} // namespace Puma

#endif /* __CCBuilder_h__ */
