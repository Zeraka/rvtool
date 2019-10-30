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

#ifndef __CBuilder_h__
#define __CBuilder_h__

#include "Puma/Builder.h"

namespace Puma {


class CBuilder : public Builder {
public:
  CBuilder () {}

public:
  // functions to create several kinds of syntax trees

  // A.1 Keywords
  virtual CTree *simple_name ();

  // A.2 Lexical conventions
  virtual CTree *literal ();
  virtual CTree *cmpd_str ();
  virtual CTree *str_literal ();

  // A.3 Basic concepts
  virtual CTree *trans_unit ();

  // A.4 Expression
  virtual CTree *prim_expr ();
  virtual CTree *cmpd_literal ();
  virtual CTree *postfix_expr ();
  virtual CTree *postfix_expr1 ();
  virtual CTree *expr_list ();
  virtual CTree *unary_expr ();
  virtual CTree *unary_expr1 ();
  virtual CTree *cast_expr ();
  virtual CTree *cast_expr1 ();
  virtual CTree *cast_expr2 ();
  virtual CTree *offsetof_expr ();
  virtual CTree *memb_designator ();
  virtual CTree *mul_expr ();
  virtual CTree *add_expr ();
  virtual CTree *shift_expr ();
  virtual CTree *rel_expr ();
  virtual CTree *equ_expr ();
  virtual CTree *and_expr ();
  virtual CTree *excl_or_expr ();
  virtual CTree *incl_or_expr ();
  virtual CTree *log_and_expr ();
  virtual CTree *log_or_expr ();
  virtual CTree *cond_expr ();
  virtual CTree *ass_expr ();
  virtual CTree *ass_expr1 ();
  virtual CTree *expr ();
  virtual CTree *const_expr ();

  // A.5 Statements
  virtual CTree *stmt ();
  virtual CTree *label_stmt ();
  virtual CTree *expr_stmt ();
  virtual CTree *cmpd_stmt ();
  virtual CTree *stmt_seq ();
  virtual CTree *select_stmt ();
  virtual CTree *sub_stmt ();
  virtual CTree *condition (CTree * = (CTree*)0); 
  virtual CTree *iter_stmt ();
  virtual CTree *for_init_stmt ();
  virtual CTree *jump_stmt ();

  // A.6 Declarations
  virtual CTree *decl_seq ();
  virtual CTree *decl ();
  virtual CTree *block_decl ();
  virtual CTree *simple_decl ();
  virtual CTree *decl_spec ();
  virtual CTree *misc_spec ();
  virtual CTree *decl_spec_seq ();
  virtual CTree *decl_spec_seq1 ();
  virtual CTree *storage_class_spec ();
  virtual CTree *fct_spec ();
  virtual CTree *type_spec ();
  virtual CTree *simple_type_spec ();
  virtual CTree *type_name ();
  virtual CTree *elaborated_type_spec ();
  virtual CTree *enum_key ();
  virtual CTree *enum_spec ();
  virtual CTree *enum_spec1 ();
  virtual CTree *enumerator_list ();
  virtual CTree *enumerator_def ();
  virtual CTree *enumerator ();
  virtual CTree *asm_def ();

  // A.7 Declarators
  virtual CTree *init_declarator_list ();
  virtual CTree *init_declarator ();
  virtual CTree *init_declarator1 ();
  virtual CTree *declarator ();
  virtual CTree *direct_declarator ();
  virtual CTree *direct_declarator1 ();
  virtual CTree *identifier_list ();
  virtual CTree *array_delim ();
  virtual CTree *ptr_operator ();
  virtual CTree *cv_qual_seq ();
  virtual CTree *cv_qual ();
  virtual CTree *declarator_id ();
  virtual CTree *type_id ();
  virtual CTree *abst_declarator ();
  virtual CTree *direct_abst_declarator ();
  virtual CTree *direct_abst_declarator1 ();
  virtual CTree *param_decl_clause ();
  virtual CTree *param_decl_list ();
  virtual CTree *param_decl ();
  virtual CTree *param_decl1 ();
  virtual CTree *fct_def ();
  virtual CTree *arg_decl_seq ();
  virtual CTree *fct_body ();
  virtual CTree *init ();
  virtual CTree *init_clause ();
  virtual CTree *init_list ();
  virtual CTree *init_list_item ();
  virtual CTree *designation ();
  virtual CTree *designator ();

  // A.8 Classes
  virtual CTree *class_spec ();
  virtual CTree *class_head ();
  virtual CTree *class_key ();
  virtual CTree *member_spec ();
  virtual CTree *member_decl ();
  virtual CTree *member_declarator_list ();
  virtual CTree *member_declarator ();

protected:
  // helper functions
  CTree *lr_bin_expr ();
  CTree *prim_ds ();
};


} // namespace Puma

#endif /* __CBuilder_h__ */
