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

#ifndef __CSyntax_h__
#define __CSyntax_h__

/** \file 
 *  Parser for the C programming language (C99). */

#include "Puma/Syntax.h"
#include "Puma/CBuilder.h"
#include "Puma/CSemantic.h"
#include "Puma/CTokens.h"

namespace Puma {


class CSyntax : public Syntax {
public:
  CSyntax (CBuilder &, CSemantic &);
  virtual void configure (Config &);

  enum Grammar { GRAMMAR_C, GRAMMAR_CPLUSPLUS };
  virtual Grammar grammar () const { return GRAMMAR_C; }

private:
  CBuilder &builder () const { return (CBuilder&)Syntax::builder (); }
  CSemantic &semantic () const { return (CSemantic&)Syntax::semantic (); }

protected:
  // Faster check for primitive types
  tokenset _prim_types;
  virtual void init_prim_types ();
  bool is_prim_type () { return _prim_types[look_ahead ()]; }

  // Faster check for cv qualifiers
  tokenset _cv_quals;
  virtual void init_cv_quals ();
  bool is_cv_qual () { return _cv_quals[look_ahead ()]; }

  // FIRST and FOLLOW sets
  tokenset _class_spec_1;

  // FIRST and FOLLOW initialization
  virtual void init_class_spec ();

  // result cache!
  Token *last_look_ahead_token;
  bool last_look_ahead_result;

  // Grammar rules

public:
  // A.1 Keywords

  struct TypedefName {
    static inline bool check (CSyntax &s) { return s.typedef_name (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool typedef_name ();

  struct PrivateName {
    static inline bool check (CSyntax &s) { return s.private_name (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool private_name ();

  // A.2 Lexical conventions

  struct Identifier {
    static inline bool check (CSyntax &s) { return s.identifier (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool identifier ();
  
  struct Literal {
    static inline bool check (CSyntax &s) { return s.literal (); }
    static bool parse (CSyntax &);
  };
  virtual bool literal ();
  
  struct CmpdStr {
    static inline bool check (CSyntax &s) { return s.cmpd_str (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool cmpd_str ();
  
  struct StrLiteral {
    static inline bool check (CSyntax &s) { return s.str_literal (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool str_literal ();
  
  // A.3 Basic concepts 

  struct TransUnit {
    static inline bool check (CSyntax &s) { return s.trans_unit (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool trans_unit ();

  // A.4 Expression
  struct PrimExpr {
    static inline bool check (CSyntax &s) { return s.prim_expr (); }
    static bool parse (CSyntax &);
  };
  virtual bool prim_expr ();

  struct IdExpr {
    static inline bool check (CSyntax &s) { return s.id_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool id_expr ();

  struct CmpdLiteral {
    static inline bool check (CSyntax &s) { return s.cmpd_literal (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool cmpd_literal ();

  struct PostfixExpr {
    static inline bool check (CSyntax &s) { return s.postfix_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool postfix_expr ();

  struct PostfixExpr1 {
    static inline bool check (CSyntax &s) { return s.postfix_expr1 (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool postfix_expr1 ();

  struct ExprList {
    static inline bool check (CSyntax &s) { return s.expr_list (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool expr_list ();

  struct UnaryExpr {
    static inline bool check (CSyntax &s) { return s.unary_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool unary_expr ();

  struct UnaryExpr1 {
    static inline bool check (CSyntax &s) { return s.unary_expr1 (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool unary_expr1 ();

  struct CastExpr {
    static inline bool check (CSyntax &s) { return s.cast_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool cast_expr ();

  struct CastExpr1 {
    static inline bool check (CSyntax &s) { return s.cast_expr1 (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool cast_expr1 ();

  struct CastExpr2 {
    static inline bool check (CSyntax &s) { return s.cast_expr2 (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool cast_expr2 ();

  struct OffsetofExpr {
    static inline bool check (CSyntax &s) { return s.offsetof_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool offsetof_expr ();

  struct MembDesignator {
    static inline bool check (CSyntax &s) { return s.memb_designator (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool memb_designator ();

  struct MulExpr {
    static inline bool check (CSyntax &s) { return s.mul_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool mul_expr ();

  struct AddExpr {
    static inline bool check (CSyntax &s) { return s.add_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool add_expr ();

  struct ShiftExpr {
    static inline bool check (CSyntax &s) { return s.shift_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool shift_expr ();

  struct RelExpr {
    static inline bool check (CSyntax &s) { return s.rel_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool rel_expr ();

  struct EquExpr {
    static inline bool check (CSyntax &s) { return s.equ_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool equ_expr ();

  struct AndExpr {
    static inline bool check (CSyntax &s) { return s.and_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool and_expr ();

  struct ExclOrExpr {
    static inline bool check (CSyntax &s) { return s.excl_or_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool excl_or_expr ();

  struct InclOrExpr {
    static inline bool check (CSyntax &s) { return s.incl_or_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool incl_or_expr ();

  struct LogAndExpr {
    static inline bool check (CSyntax &s) { return s.log_and_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool log_and_expr ();

  struct LogOrExpr {
    static inline bool check (CSyntax &s) { return s.log_or_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool log_or_expr ();

  struct CondExpr {
    static inline bool check (CSyntax &s) { return s.cond_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool cond_expr ();

  struct AssExpr {
    static inline bool check (CSyntax &s) { return s.ass_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool ass_expr ();

  struct AssExpr1 {
    static inline bool check (CSyntax &s) { return s.ass_expr1 (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool ass_expr1 ();

  struct Expr {
    static inline bool check (CSyntax &s) { return s.expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool expr ();

  struct ConstExpr {
    static inline bool check (CSyntax &s) { return s.const_expr (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool const_expr ();

  // A.5 Statements

  struct Stmt {
    static inline bool check (CSyntax &s) { return s.stmt (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool stmt ();

  struct LabelStmt {
    static inline bool check (CSyntax &s) { return s.label_stmt (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool label_stmt ();

  struct ExprStmt {
    static inline bool check (CSyntax &s) { return s.expr_stmt (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool expr_stmt ();

  struct CmpdStmt {
    static inline bool check (CSyntax &s) { return s.cmpd_stmt (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool cmpd_stmt ();

  struct StmtSeq {
    static inline bool check (CSyntax &s) { return s.stmt_seq (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool stmt_seq ();

  struct SelectStmt {
    static inline bool check (CSyntax &s) { return s.select_stmt (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool select_stmt ();

  struct SubStmt {
    static inline bool check (CSyntax &s) { return s.sub_stmt (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool sub_stmt ();

  struct Condition {
    static inline bool check (CSyntax &s) { return s.condition (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool condition ();

  struct IterStmt {
    static inline bool check (CSyntax &s) { return s.iter_stmt (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool iter_stmt ();
  
  struct ForInitStmt {
    static inline bool check (CSyntax &s) { return s.for_init_stmt (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool for_init_stmt ();
  
  struct JumpStmt {
    static inline bool check (CSyntax &s) { return s.jump_stmt (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool jump_stmt ();
      
  // A.6 Declarations

  struct DeclSeq {
    static inline bool check (CSyntax &s) { return s.decl_seq (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool decl_seq ();

  struct Decl {
    static inline bool check (CSyntax &s) { return s.decl (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool decl ();
  // helper function, which is needed, because ac++ can't weave in templates :-(
  virtual bool decl_check ();

  struct BlockDecl {
    static inline bool check (CSyntax &s) { return s.block_decl (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool block_decl ();

  struct SimpleDecl {
    static inline bool check (CSyntax &s) { return s.simple_decl (); }
    static bool parse (CSyntax &);
  };
  virtual bool simple_decl ();

  struct DeclSpec {
    static inline bool check (CSyntax &s) { return s.decl_spec (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool decl_spec ();

  struct DeclSpecSeq {
    static inline bool check (CSyntax &s) { return s.decl_spec_seq (); }
    static bool parse (CSyntax &);
  };
  virtual bool decl_spec_seq ();

  struct DeclSpecSeq1 {
    static inline bool check (CSyntax &s) { return s.decl_spec_seq1 (); }
    static bool parse (CSyntax &);
  };
  virtual bool decl_spec_seq1 ();

  struct MiscSpec {
    static inline bool check (CSyntax &s) { return s.misc_spec (); }
    static bool parse (CSyntax &);
  };
  virtual bool misc_spec ();

  struct StorageClassSpec {
    static inline bool check (CSyntax &s) { return s.storage_class_spec (); }
    static bool parse (CSyntax &);
  };
  virtual bool storage_class_spec ();

  struct FctSpec {
    static inline bool check (CSyntax &s) { return s.fct_spec (); }
    static bool parse (CSyntax &);
  };
  virtual bool fct_spec ();

  struct TypeSpec {
    static inline bool check (CSyntax &s) { return s.type_spec (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool type_spec ();

  struct SimpleTypeSpec {
    static inline bool check (CSyntax &s) { return s.simple_type_spec (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool simple_type_spec ();

  struct TypeName {
    static inline bool check (CSyntax &s) { return s.type_name (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool type_name ();

  struct ElaboratedTypeSpec {
    static inline bool check (CSyntax &s) { return s.elaborated_type_spec (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool elaborated_type_spec ();

  struct EnumKey {
    static inline bool check (CSyntax &s) { return s.enum_key (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool enum_key ();

  struct EnumSpec {
    static inline bool check (CSyntax &s) { return s.enum_spec (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool enum_spec ();

  struct EnumSpec1 {
    static inline bool check (CSyntax &s) { return s.enum_spec1 (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool enum_spec1 ();

  struct EnumeratorList {
    static inline bool check (CSyntax &s) { return s.enumerator_list (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool enumerator_list ();

  struct EnumeratorDef {
    static inline bool check (CSyntax &s) { return s.enumerator_def (); }
    static bool parse (CSyntax &);
  };
  virtual bool enumerator_def ();

  struct Enumerator {
    static inline bool check (CSyntax &s) { return s.enumerator (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool enumerator ();

  struct AsmDef {
    static inline bool check (CSyntax &s) { return s.asm_def (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool asm_def ();

  // A.7 Declarators
  struct InitDeclaratorList {
    static inline bool check (CSyntax &s) { return s.init_declarator_list (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool init_declarator_list ();

  struct InitDeclarator {
    static inline bool check (CSyntax &s) { return s.init_declarator (); }
    static bool parse (CSyntax &);
  };
  virtual bool init_declarator ();

  struct InitDeclarator1 {
    static inline bool check (CSyntax &s) { return s.init_declarator1 (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool init_declarator1 ();

  struct Declarator {
    static inline bool check (CSyntax &s) { return s.declarator (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool declarator ();

  struct DirectDeclarator {
    static inline bool check (CSyntax &s) { return s.direct_declarator (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool direct_declarator ();

  struct DirectDeclarator1 {
    static inline bool check (CSyntax &s) { return s.direct_declarator1 (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool direct_declarator1 ();

  struct IdentifierList {
    static inline bool check (CSyntax &s) { return s.identifier_list (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool identifier_list ();

  struct ArrayDelim {
    static inline bool check (CSyntax &s) { return s.array_delim (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool array_delim ();

  struct PtrOperator {
    static inline bool check (CSyntax &s) { return s.ptr_operator (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool ptr_operator ();

  struct CvQualSeq {
    static inline bool check (CSyntax &s) { return s.cv_qual_seq (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool cv_qual_seq ();

  struct CvQual {
    static inline bool check (CSyntax &s) { return s.cv_qual (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool cv_qual ();

  struct DeclaratorId {
    static inline bool check (CSyntax &s) { return s.declarator_id (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool declarator_id ();

  struct TypeId {
    static inline bool check (CSyntax &s) { return s.type_id (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool type_id ();

  struct TypeSpecSeq {
    static inline bool check (CSyntax &s) { return s.type_spec_seq (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool type_spec_seq ();

  struct TypeSpecSeq1 {
    static inline bool check (CSyntax &s) { return s.type_spec_seq1 (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool type_spec_seq1 ();

  struct AbstDeclarator {
    static inline bool check (CSyntax &s) { return s.abst_declarator (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool abst_declarator ();

  struct DirectAbstDeclarator {
    static inline bool check (CSyntax &s) { return s.direct_abst_declarator (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool direct_abst_declarator ();

  struct DirectAbstDeclarator1 {
    static inline bool check (CSyntax &s) { return s.direct_abst_declarator1 (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool direct_abst_declarator1 ();

  struct ParamDeclClause {
    static inline bool check (CSyntax &s) { return s.param_decl_clause (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool param_decl_clause ();

  struct ParamDeclList {
    static inline bool check (CSyntax &s) { return s.param_decl_list (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool param_decl_list ();

  CTree * rule_param_decl ();
  virtual bool param_decl ();
  
  struct ParamDecl1 {
    static inline bool check (CSyntax &s) { return s.param_decl1 (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool param_decl1 ();

//  CTree * rule_param_decl2 ();
//  virtual bool param_decl2 ();
  CTree * rule_fct_def ();
  virtual bool fct_def ();
  CTree * rule_arg_decl_seq ();
  virtual bool arg_decl_seq ();
  CTree * rule_fct_body ();
  virtual bool fct_body ();
  CTree * rule_init ();
  virtual bool init ();
  CTree * rule_init_clause ();
  virtual bool init_clause ();
  CTree * rule_init_list ();
  virtual bool init_list ();
  CTree * rule_init_list_item ();
  virtual bool init_list_item ();
  CTree * rule_designation ();
  virtual bool designation ();
  CTree * rule_designator ();
  virtual bool designator ();

  // A.8 Classes
  CTree * rule_class_spec ();
  virtual bool class_spec ();

  struct ClassHead {
    static inline bool check (CSyntax &s) { return s.class_head (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool class_head ();

  struct ClassKey {
    static inline bool check (CSyntax &s) { return s.class_key (); }
    static inline bool parse (CSyntax &);
  };
  virtual bool class_key ();

  CTree * rule_member_spec ();
  virtual bool member_spec ();
  CTree * rule_member_decl ();
  virtual bool member_decl ();
  CTree * rule_member_declarator_list ();
  virtual bool member_declarator_list ();
  CTree * rule_member_declarator ();
  virtual bool member_declarator ();

protected:
  virtual bool is_fct_def ();
  virtual bool is_ass_expr ();

  virtual void handle_directive ();
};

inline CSyntax::CSyntax (CBuilder &b, CSemantic &s) : Syntax (b, s),
 last_look_ahead_token (0),
 last_look_ahead_result (false)
 {}

inline void CSyntax::handle_directive ()
 { Syntax::handle_directive (); }


} // namespace Puma

#endif /* __CSyntax_h__ */
