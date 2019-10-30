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

#include "Puma/CSyntax.h"
#include "Puma/CStructure.h"
#include "Puma/CTokens.h"
#include "Puma/CTree.h"

namespace Puma {


void CSyntax::configure (Config &config) {
  Syntax::configure (config);
  init_prim_types ();
  init_cv_quals ();
}

void CSyntax::init_prim_types () {
  _prim_types.set (TOK_C_BOOL);
  _prim_types.set (TOK_CHAR);
  _prim_types.set (TOK_SHORT);
  _prim_types.set (TOK_INT);
  _prim_types.set (TOK_LONG);
  _prim_types.set (TOK_SIGNED);
  _prim_types.set (TOK_UNSIGNED);
  _prim_types.set (TOK_FLOAT);
  _prim_types.set (TOK_DOUBLE);
  _prim_types.set (TOK_WCHAR_T);
  _prim_types.set (TOK_VOID);
  _prim_types.set (TOK_UNKNOWN_T);
}
 
void CSyntax::init_cv_quals () {
  _cv_quals.set (TOK_CONST);
  _cv_quals.set (TOK_VOLATILE);
  _cv_quals.set (TOK_RESTRICT);
}

bool CSyntax::is_fct_def () {
  State s = token_provider->get_state ();
  bool result = false, braces = false, stop = false;
  Token *current;
  int token;

  while (!stop && (current = token_provider->current ())) {
    token = current->type ();

    // before function parameter list
    if (! braces) {
      switch (token) {
        // only declaration
        case TOK_SEMI_COLON:
        case TOK_COMMA:
        case TOK_ASSIGN:
        case TOK_TYPEDEF:
          stop = true;
          break;
        // structure definition in function return type
        case TOK_OPEN_CURLY:
          skip_curly_block ();
          break;
        // array delimiter in return type
        case TOK_OPEN_SQUARE:
          skip_block (TOK_OPEN_SQUARE, TOK_CLOSE_SQUARE);
          break;
        // function parameter list
        case TOK_OPEN_ROUND:
          skip_round_block ();
          braces = true;
          break;
        // names etc.
        default:
          token_provider->next ();
          locate_token ();
          break;
      }
    // after function parameter list
    } else {
      switch (token) {
        // only declaration
        case TOK_SEMI_COLON:
        case TOK_COMMA:
        case TOK_OPEN_ROUND:
          stop = true;
          break;
        // asm declaration specifier
        case TOK_ASM:
          skip ();
          skip_round_block ();
          break;
        // function body..
        case TOK_OPEN_CURLY:
          result = true;
          stop = true;
          break;
        // K&R function parameter declaration
        default:
        {
          static int stop_tokens[] = { TOK_SEMI_COLON, 0 };
          skip (stop_tokens, true);
          break;
        }
      }
    }
  }

  token_provider->set_state (s);
  return result;
}

bool CSyntax::is_ass_expr () {
  State s = token_provider->get_state ();
  Token *current = token_provider->current ();
  if (! current) {
    // This might look useless, but an aspect skips tokens in "current"
    token_provider->set_state (s);
    return false;
  }

  // return the last result if we check the same token
  if (last_look_ahead_token == current)
    return last_look_ahead_result;

  last_look_ahead_token = current;
  last_look_ahead_result = false;

  int token;

  bool stop = false;
  while (!stop && (current = token_provider->current ())) {
    token = current->type ();

    switch (token) {
      case TOK_SEMI_COLON:
      case TOK_COMMA:
      case TOK_CLOSE_CURLY:
      case TOK_CLOSE_ROUND:
      case TOK_CLOSE_SQUARE:
      case TOK_COLON:
      case TOK_QUESTION:
        stop = true;
        break;
      case TOK_OPEN_CURLY:
        skip_curly_block ();
        break;
      case TOK_OPEN_ROUND:
        skip_round_block ();
        break;
      case TOK_OPEN_SQUARE:
        skip_block (TOK_OPEN_SQUARE, TOK_CLOSE_SQUARE);
        break;
      case TOK_ASSIGN:
      case TOK_MUL_EQ:
      case TOK_DIV_EQ:
      case TOK_MOD_EQ:
      case TOK_ADD_EQ:
      case TOK_SUB_EQ:
      case TOK_RSH_EQ:
      case TOK_LSH_EQ:
      case TOK_AND_EQ:
      case TOK_XOR_EQ:
      case TOK_IOR_EQ:
        last_look_ahead_result = true;
        stop = true;
        break;
      default:
        skip ();
        break;
    }
  }

  token_provider->set_state (s);
  return last_look_ahead_result;
}

/*****************************************************************************/
/*                                                                           */
/*                             C   G r a m m a r                             */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* This grammar is derived from:                                             */
/*                                                                           */
/*                           International Standard                          */
/*                            ISO/IEC 9899-1999(E)                           */
/*                          Second edition 1999-04-01                        */
/*                          Programming languages - C                        */
/*                                                                           */
/*                                    and                                    */
/*                                                                           */
/*                           International Standard                          */
/*                            ISO/IEC 14882:1998(E)                          */
/*                          First edition 1998-09-01                         */
/*                         Programming languages - C++                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                           A . 1   K e y w o r d s                         */
/*                                                                           */
/*****************************************************************************/

bool CSyntax::TypedefName::parse (CSyntax &s) {
  // 1: ID
  return s.parse (TOK_ID);
}
bool CSyntax::typedef_name () { return TypedefName::parse (*this); }

bool CSyntax::PrivateName::parse (CSyntax &s) {
  return true;
}
bool CSyntax::private_name () { return PrivateName::parse (*this); }

/*****************************************************************************/
/*                                                                           */
/*               A . 2   L e x i c a l  c o n v e n t i o n s                */
/*                                                                           */
/*****************************************************************************/

bool CSyntax::Identifier::parse (CSyntax &s) {
  // 1: ID
  return s.parse (TOK_ID);
}
bool CSyntax::identifier () { return Identifier::parse (*this); }

bool CSyntax::Literal::parse (CSyntax &s) {
  // 1: cmpd_str
  // 1: TOK_INT_VAL  (e.g. 4711)
  // 1: TOK_ZERO_VAL (0)
  // 1: TOK_CHAR_VAL (e.g. 'x')
  // 1: TOK_FLT_VAL  (e.g. 3.14)
  static int literal_token[] = { TOK_INT_VAL, TOK_ZERO_VAL, TOK_CHAR_VAL, TOK_FLT_VAL, 0 };
  return CmpdStr::check (s) || s.parse (literal_token);
}
bool CSyntax::literal () { return Literal::parse (*this); }

bool CSyntax::CmpdStr::parse (CSyntax &s) {
  // 1+: str_literal...
  return seq<CSyntax, StrLiteral> (s);
}
bool CSyntax::cmpd_str () { return CmpdStr::parse(*this); }

bool CSyntax::StrLiteral::parse (CSyntax &s) {
  // 1: STRING_VAL
  return s.parse (TOK_STRING_VAL);
}
bool CSyntax::str_literal () { return StrLiteral::parse(*this); }

/*****************************************************************************/
/*                                                                           */
/*                   A . 3   B a s i c  c o n c e p t s                      */
/*                                                                           */
/*****************************************************************************/

bool CSyntax::TransUnit::parse (CSyntax &s) {
  // 1: decl_seq
  return DeclSeq::check (s);
}
bool CSyntax::trans_unit () { return TransUnit::parse(*this); }

/*****************************************************************************/
/*                                                                           */
/*                      A . 4   E x p r e s s i o n s                        */
/*                                                                           */
/*****************************************************************************/

bool CSyntax::PrimExpr::parse (CSyntax &s) {
  // 1: literal
  // 1: id_expr
  // 3: (  expr  )
  return (Literal::check (s) ||
           IdExpr::check (s) ||
           (s.parse (TOK_OPEN_ROUND) && Expr::check (s) && s.parse (TOK_CLOSE_ROUND)));
}
bool CSyntax::prim_expr () { return PrimExpr::parse (*this); }

bool CSyntax::IdExpr::parse (CSyntax &s) {
  // 1: ID
  return s.parse (TOK_ID);
}
bool CSyntax::id_expr () { return IdExpr::parse (*this); }

bool CSyntax::CmpdLiteral::parse (CSyntax &s) {
	  // 5 : (  type_id  )  {  }
	  // 6 : (  type_id  )  {  init_list  }
	  return (s.parse (TOK_OPEN_ROUND) && TypeId::check (s) &&
	          s.parse (TOK_CLOSE_ROUND) &&
	          s.parse (TOK_OPEN_CURLY) && s.opt (s.init_list()) &&
	          s.parse (TOK_CLOSE_CURLY));
}
bool CSyntax::cmpd_literal () { return CmpdLiteral::parse (*this); }

bool CSyntax::PostfixExpr::parse (CSyntax &s) {
  // 1 : cmpd_literal
  // 1+: cmpd_literal  postfix_expr1...
  // 1 : prim_expr
  // 1+: prim_expr  postfix_expr1...
  return ((CmpdLiteral::check (s) ||
           PrimExpr::check (s)) &&
          s.opt (seq<CSyntax, PostfixExpr1> (s)));
}
bool CSyntax::postfix_expr () { return PostfixExpr::parse (*this); }

bool CSyntax::PostfixExpr1::parse (CSyntax &s) {
  // 1: --
  // 1: ++
  // 2: .  identifier
  // 2: ->  identifier
  // 2: (  )
  // 3: (  expr_list  )
  // 3: [  expr  ]
  return (s.parse (TOK_OPEN_ROUND) ?
    (ExprList::check (s), s.parse (TOK_CLOSE_ROUND)) :
          s.parse (TOK_OPEN_SQUARE) ?
    (s.expr() && s.parse (TOK_CLOSE_SQUARE)) :
          (s.parse (TOK_DECR) || s.parse (TOK_INCR) ||
           ((s.parse (TOK_DOT) || s.parse (TOK_PTS)) &&
            Identifier::check (s))));
}
bool CSyntax::postfix_expr1 () { return PostfixExpr1::parse (*this); }

bool CSyntax::ExprList::parse (CSyntax &s) {
  // 1 : ass_expr
  // 3+: ass_expr  ,  ass_expr ...
  return s.list (&CSyntax::ass_expr, TOK_COMMA);
}
bool CSyntax::expr_list () { return ExprList::parse (*this); }

bool CSyntax::UnaryExpr::parse (CSyntax &s) {
  // 1: postfix_expr
  // 1: offsetof_expr
  // 2: --  unary_expr
  // 2: ++  unary_expr
  // 2: any_unary_op  cast_expr
  // 2: SIZEOF  unary_expr
  // 2: ALIGNOF  unary_expr
  // 4: SIZEOF  unary_expr1
  // 4: ALIGNOF  unary_expr1
  static int any_unary_op[] = { TOK_AND, TOK_MUL, TOK_PLUS, TOK_MINUS, 
                                TOK_TILDE, TOK_NOT, 0 };
  return (PostfixExpr::check (s) ||
          ((s.parse (TOK_DECR) || s.parse (TOK_INCR)) ?
     UnaryExpr::check (s) :
           s.parse (any_unary_op) ?
     CastExpr::check (s) :
           (s.parse (TOK_SIZEOF) || s.parse (TOK_ALIGNOF)) ?
     (UnaryExpr1::check (s) ||
      UnaryExpr::check (s)) :
           OffsetofExpr::check (s)));
}
bool CSyntax::unary_expr () { return UnaryExpr::parse (*this); }

bool CSyntax::UnaryExpr1::parse (CSyntax &s) {
  // 3: (  type_id  )
  return (s.parse (TOK_OPEN_ROUND) && TypeId::check (s) && s.parse (TOK_CLOSE_ROUND));
}
bool CSyntax::unary_expr1 () { return UnaryExpr1::parse (*this); }

bool CSyntax::CastExpr::parse (CSyntax &s) {
  // 1: cast_expr1
  // 1: unary_expr
  return (CastExpr1::check (s) || UnaryExpr::check (s));
}
bool CSyntax::cast_expr () { return CastExpr::parse (*this); }

bool CSyntax::CastExpr1::parse (CSyntax &s) {
  // 2+: cast_expr2... unary_expr
  return (seq<CSyntax, CastExpr2> (s) && UnaryExpr::check (s));
}
bool CSyntax::cast_expr1 () { return CastExpr1::parse (*this); }

bool CSyntax::CastExpr2::parse (CSyntax &s) {
  // 3: (  type_id  )
  return (s.parse (TOK_OPEN_ROUND) && TypeId::check (s) && s.parse (TOK_CLOSE_ROUND));
}
bool CSyntax::cast_expr2 () { return CastExpr2::parse (*this); }

bool CSyntax::OffsetofExpr::parse (CSyntax &s) {
  // 6: OFFSETOF  (  type_spec  ,  memb_designator  )
  return (s.parse (TOK_OFFSETOF) && s.parse (TOK_OPEN_ROUND) &&
          s.type_spec () && s.parse (TOK_COMMA) && MembDesignator::check (s) &&
          s.parse (TOK_CLOSE_ROUND));
}
bool CSyntax::offsetof_expr () { return OffsetofExpr::parse (*this); }

bool CSyntax::MembDesignator::parse (CSyntax &s) {
  // 1 : identifier
  // 2+: identifier  designator...
  return (Identifier::check (s) && s.opt (s.seq (&CSyntax::designator)));
}
bool CSyntax::memb_designator () { return MembDesignator::parse (*this); }

bool CSyntax::MulExpr::parse (CSyntax &s) {
  // 1 : cast_expr
  // 3+: cast_expr  any_mul_op  cast_expr ...
  static int any_mul_op[] = { TOK_MUL, TOK_DIV, TOK_MODULO, 0 };
  return list<CSyntax, CastExpr> (s, any_mul_op);
}
bool CSyntax::mul_expr () { return MulExpr::parse (*this); }

bool CSyntax::AddExpr::parse (CSyntax &s) {
  // 1 : mul_expr
  // 3+: mul_expr  any_add_op  mul_expr ...
  static int any_add_op[] = { TOK_PLUS, TOK_MINUS, 0 };
  return list<CSyntax, MulExpr> (s, any_add_op);
}
bool CSyntax::add_expr () { return AddExpr::parse (*this); }

bool CSyntax::ShiftExpr::parse (CSyntax &s) {
  // 1 : add_expr
  // 3+: add_expr  any_shift_op  add_expr ...
  static int any_shift_op[] = { TOK_LSH, TOK_RSH, 0 };
  return list<CSyntax, AddExpr> (s, any_shift_op);
}
bool CSyntax::shift_expr () { return ShiftExpr::parse (*this); }

bool CSyntax::RelExpr::parse (CSyntax &s) {
  // 1 : shift_expr
  // 3+: shift_expr  any_rel_op  shift_expr ...
  static int any_rel_op[] = { TOK_LESS, TOK_GREATER, TOK_LEQ, TOK_GEQ, 0 };
  return list<CSyntax, ShiftExpr> (s, any_rel_op);
}
bool CSyntax::rel_expr () { return RelExpr::parse (*this); }

bool CSyntax::EquExpr::parse (CSyntax &s) {
  // 1 : rel_expr
  // 3+: rel_expr  any_equ_op  rel_expr ...
  static int any_equ_op[] = { TOK_EQL, TOK_NEQ, 0 };
  return list<CSyntax, RelExpr> (s, any_equ_op);
}
bool CSyntax::equ_expr () { return EquExpr::parse (*this); }

bool CSyntax::AndExpr::parse (CSyntax &s) {
  // 1 : equ_expr
  // 3+: equ_expr  &  equ_expr ...
  return list<CSyntax, EquExpr> (s, TOK_AND);
}
bool CSyntax::and_expr () { return AndExpr::parse (*this); }

bool CSyntax::ExclOrExpr::parse (CSyntax &s) {
  // 1 : and_expr
  // 3+: and_expr  ^  and_expr ...
  return list<CSyntax, AndExpr> (s, TOK_ROOF);
}
bool CSyntax::excl_or_expr () { return ExclOrExpr::parse (*this); }

bool CSyntax::InclOrExpr::parse (CSyntax &s) {
  // 1 : excl_or_expr
  // 3+: excl_or_expr  |  excl_or_expr ...
  return list<CSyntax, ExclOrExpr> (s, TOK_OR);
}
bool CSyntax::incl_or_expr () { return InclOrExpr::parse (*this); }

bool CSyntax::LogAndExpr::parse (CSyntax &s) {
  // 1 : incl_or_expr
  // 3+: incl_or_expr  &&  incl_or_expr ...
  return list<CSyntax, InclOrExpr> (s, TOK_AND_AND);
}
bool CSyntax::log_and_expr () { return LogAndExpr::parse (*this); }

bool CSyntax::LogOrExpr::parse (CSyntax &s) {
  // 1 : log_and_expr
  // 3+: log_and_expr  ||  log_and_expr ...
  return list<CSyntax, LogAndExpr> (s, TOK_OR_OR);
}
bool CSyntax::log_or_expr () { return LogOrExpr::parse (*this); }

bool CSyntax::CondExpr::parse (CSyntax &s) {
  // 1: log_or_expr
  // 5: log_or_expr  ?  expr  :  cond_expr
  return (LogOrExpr::check (s) &&
          (s.parse (TOK_QUESTION) ? (Expr::check (s) &&
                                     s.parse (TOK_COLON) &&
                                     CondExpr::check (s)) : true));
}
bool CSyntax::cond_expr () { return CondExpr::parse (*this); }

bool CSyntax::AssExpr::parse (CSyntax &s) {
  // 1 : cond_expr
  // 2+: ass_expr1...  cond_expr
  return (seq<CSyntax, AssExpr1> (s), CondExpr::check (s));
}
bool CSyntax::ass_expr () { return AssExpr::parse (*this); }

bool CSyntax::AssExpr1::parse (CSyntax &s) {
  // 2: unary_expr  any_ass_op
  static int any_ass_op[] = { TOK_ASSIGN, TOK_MUL_EQ, TOK_DIV_EQ, TOK_MOD_EQ, 
                              TOK_ADD_EQ, TOK_SUB_EQ, TOK_RSH_EQ, TOK_LSH_EQ,
                              TOK_AND_EQ, TOK_XOR_EQ, TOK_IOR_EQ, 0 };
  return (UnaryExpr::check (s) && s.parse (any_ass_op));
}
bool CSyntax::ass_expr1 () { return AssExpr1::parse (*this); }

bool CSyntax::Expr::parse (CSyntax &s) {
  // 1 : ass_expr
  // 3+: ass_expr  ,  ass_expr ...
  return list<CSyntax, AssExpr> (s, TOK_COMMA);
}
bool CSyntax::expr () { return Expr::parse (*this); }

bool CSyntax::ConstExpr::parse (CSyntax &s) {
  // 1: cond_expr
  return CondExpr::check (s);
}
bool CSyntax::const_expr () { return ConstExpr::parse (*this); }

/*****************************************************************************/
/*                                                                           */
/*                      A . 5   S t a t e m e n t s                          */
/*                                                                           */
/*****************************************************************************/

bool CSyntax::Stmt::parse (CSyntax &s) {
  // 1: label_stmt
  // 1: expr_stmt
  // 1: cmpd_stmt
  // 1: select_stmt
  // 1: iter_stmt
  // 1: jump_stmt
  // 1: asm_def
  return (LabelStmt::check (s) ||
          ExprStmt::check (s) ||
          CmpdStmt::check (s) ||
          SelectStmt::check (s) ||
          IterStmt::check (s) ||
          JumpStmt::check (s) ||
          AsmDef::check (s));
}
bool CSyntax::stmt () { return Stmt::parse (*this); }

bool CSyntax::LabelStmt::parse (CSyntax &s) {
  // 3: identifier  :  stmt 
  // 3: DEFAULT  :  stmt
  // 4: CASE  const_expr  :  stmt
  return ((Identifier::check (s) ||
           s.parse (TOK_DEFAULT) || 
           (s.parse (TOK_CASE) && ConstExpr::check (s))) &&
          s.parse (TOK_COLON) && Stmt::check (s));
}
bool CSyntax::label_stmt () { return LabelStmt::parse (*this); }

bool CSyntax::ExprStmt::parse (CSyntax &s) {
  // 1: ;
  // 2: expr  ;
  return (Expr::check (s), s.parse (TOK_SEMI_COLON));
}
bool CSyntax::expr_stmt () { return ExprStmt::parse (*this); }

bool CSyntax::CmpdStmt::parse (CSyntax &s) {
  // 2: {  }
  // 3: {  stmt_seq  }
  if (! s.parse (TOK_OPEN_CURLY))
    return false;
  s.semantic ().enter_local_scope (); // enter local (block) scope
  return (StmtSeq::check (s), s.parse (TOK_CLOSE_CURLY));
}
bool CSyntax::cmpd_stmt () { return CmpdStmt::parse (*this); }

bool CSyntax::StmtSeq::parse (CSyntax &s) {
  // 1+: stmt..
  int skip[] = { TOK_SEMI_COLON, TOK_CLOSE_CURLY, 0 };
  int finish[] = { TOK_CLOSE_CURLY, 0 };

  if (! (BlockDecl::check (s) || catch_error<CSyntax, Stmt> (s,
         "invalid statement or declaration", finish, skip)))
    return false;

  while (BlockDecl::check (s) || catch_error<CSyntax, Stmt> (s,
         "invalid statement or declaration", finish, skip));
  return true;
}
bool CSyntax::stmt_seq () { return StmtSeq::parse (*this); }

bool CSyntax::SelectStmt::parse (CSyntax &s) {
  // 5: SWITCH  (  condition  )  sub_stmt
  // 5: IF  (  condition  )  sub_stmt
  // 7: IF  (  condition  )  sub_stmt  ELSE  sub_stmt
  bool is_if_stmt = s.parse (TOK_IF);
  if  (! ((is_if_stmt || s.parse (TOK_SWITCH)) && s.parse (TOK_OPEN_ROUND)))
    return false;
  s.semantic ().enter_local_scope (); // enter statement scope
  return (Condition::check (s) && s.parse (TOK_CLOSE_ROUND) && 
          SubStmt::check (s) &&
          ((is_if_stmt && s.parse (TOK_ELSE)) ? // 'else' part
           SubStmt::check (s) : true));
}
bool CSyntax::select_stmt () { return SelectStmt::parse (*this); }

bool CSyntax::SubStmt::parse (CSyntax &s) {
  // 1: stmt
  return Stmt::check (s); 
}
bool CSyntax::sub_stmt () { return SubStmt::parse (*this); }

bool CSyntax::Condition::parse (CSyntax &s) {
  // 1: expr
  return Expr::check (s);
}
bool CSyntax::condition () { return Condition::parse (*this); }

bool CSyntax::IterStmt::parse (CSyntax &s) {
  // 5: WHILE  (  condition  )  sub_stmt
  // 6: FOR  (  for_init_stmt  ;  )  sub_stmt
  // 7: DO  sub_stmt  WHILE  (  expr  )  ;
  // 7: FOR  (  for_init_stmt  ;  expr  )  sub_stmt
  // 7: FOR  (  for_init_stmt  condition  ;  )  sub_stmt
  // 8: FOR  (  for_init_stmt  condition  ;  expr  )  sub_stmt
  if (s.parse (TOK_DO)) 
    return (SubStmt::check (s) && 
            s.parse (TOK_WHILE) && s.parse (TOK_OPEN_ROUND) && 
            Expr::check (s) && s.parse (TOK_CLOSE_ROUND) &&
            s.parse (TOK_SEMI_COLON));
  s.semantic ().enter_local_scope (); // enter statement scope
  return ((s.parse (TOK_WHILE) ? 
           (s.parse (TOK_OPEN_ROUND) && Condition::check (s)) :
           (s.parse (TOK_FOR) && s.parse (TOK_OPEN_ROUND) && 
            ForInitStmt::check (s) && 
            s.opt (Condition::check (s)) && 
            s.parse (TOK_SEMI_COLON) && s.opt (Expr::check (s)))) &&
          s.parse (TOK_CLOSE_ROUND) && SubStmt::check (s));
}
bool CSyntax::iter_stmt () { return IterStmt::parse (*this); }

bool CSyntax::ForInitStmt::parse (CSyntax &s) {
  // 1: simple_decl
  // 1: expr_stmt
  return (SimpleDecl::check (s) || ExprStmt::check (s));
}
bool CSyntax::for_init_stmt () { return ForInitStmt::parse (*this); }

bool CSyntax::JumpStmt::parse (CSyntax &s) {
  // 2: BREAK  ;
  // 2: CONTINUE  ;
  // 2: RETURN  ;
  // 3: RETURN  expression  ;
  // 3: GOTO  identifier  ;
  return ((s.parse (TOK_BREAK) || 
           s.parse (TOK_CONTINUE) ||
           (s.parse (TOK_RETURN) && s.opt (Expr::check (s))) ||
           (s.parse (TOK_GOTO) && Identifier::check (s))) &&
          s.parse (TOK_SEMI_COLON));
}
bool CSyntax::jump_stmt () { return JumpStmt::parse (*this); }

/*****************************************************************************/
/*                                                                           */
/*                     A . 6   D e c l a r a t i o n s                       */
/*                                                                           */
/*****************************************************************************/

bool CSyntax::DeclSeq::parse (CSyntax &s) {
  // 1+: decl
  int skip[] = { TOK_SEMI_COLON, TOK_CLOSE_CURLY, 0 };
  int finish[] = { TOK_CLOSE_CURLY, 0 };

  if (! s.catch_error (&CSyntax::decl_check, "invalid declaration", finish, skip))
    return false;

  while (s.catch_error (&CSyntax::decl_check, "invalid declaration", finish, skip));
  return true;
}
bool CSyntax::decl_seq () { return DeclSeq::parse (*this); }

bool CSyntax::Decl::parse (CSyntax &s) {
  // 1: block_decl
  // 1: fct_def
  return (s.is_fct_def () ? s.fct_def () : (BlockDecl::check (s) || s.fct_def ()));
}
bool CSyntax::decl () { return Decl::parse (*this); }
bool CSyntax::decl_check () { return Decl::check (*this); }

bool CSyntax::BlockDecl::parse (CSyntax &s) {
  // 1: simple_decl
  return SimpleDecl::check (s);
}
bool CSyntax::block_decl () { return BlockDecl::parse (*this); }

bool CSyntax::SimpleDecl::parse (CSyntax &s) {
  // 1: ;
  // 2: decl_spec_seq  ;
  // 3: decl_spec_seq  init_declarator_list  ;
  if (s.parse (TOK_SEMI_COLON) ||
      ((DeclSpecSeq::check (s) || s.semantic ().implicit_int ()) &&
       ((! s.look_ahead (TOK_SEMI_COLON) &&
         InitDeclaratorList::check (s)),
        s.parse (TOK_SEMI_COLON)))) {
    s.semantic ().finish_decl ();
    return true;
  }
  return false;
}
bool CSyntax::simple_decl () { return SimpleDecl::parse (*this); }

bool CSyntax::DeclSpec::parse (CSyntax &s) {
  // 1: storage_class_spec
  // 1: type_spec
  // 1: fct_spec
  // 1: misc_spec
  return (StorageClassSpec::check (s) ||
          s.type_spec () ||
          FctSpec::check (s) ||
          MiscSpec::check (s));
}
bool CSyntax::decl_spec () { return DeclSpec::parse (*this); }

bool CSyntax::MiscSpec::parse (CSyntax &s) {
  // 1: TYPEDEF
  return s.parse (TOK_TYPEDEF);
}
bool CSyntax::misc_spec () { return MiscSpec::parse (*this); }

bool CSyntax::DeclSpecSeq::parse (CSyntax &s) {
  // 1+: decl_spec_seq1...
  return seq<CSyntax, DeclSpecSeq1> (s);
}
bool CSyntax::decl_spec_seq () { return DeclSpecSeq::parse (*this); }

bool CSyntax::DeclSpecSeq1::parse (CSyntax &s) {
  // 1: decl_spec
  return DeclSpec::check (s);
}
bool CSyntax::decl_spec_seq1 () { return DeclSpecSeq1::parse (*this); }

bool CSyntax::StorageClassSpec::parse (CSyntax &s) {
  // 1: storage_class_specs
  return (s.parse (TOK_EXTERN) ||
           s.parse (TOK_STATIC) ||
           s.parse (TOK_REGISTER) ||
           s.parse (TOK_AUTO));
}
bool CSyntax::storage_class_spec () { return StorageClassSpec::parse (*this); }

bool CSyntax::FctSpec::parse (CSyntax &s) {
  // 1: INLINE
  return s.parse (TOK_INLINE);
}
bool CSyntax::fct_spec () { return FctSpec::parse (*this); }

bool CSyntax::TypeSpec::parse (CSyntax &s) {
  // 1: simple_type_spec
  // 1: class_spec
  // 1: enum_spec
  // 1: elaborated_type_spec
  // 1: cv_qual
  return (SimpleTypeSpec::check (s) ||
          s.class_spec () ||
          EnumSpec::check (s) ||
          ElaboratedTypeSpec::check (s) ||
          CvQual::check (s));
}
bool CSyntax::type_spec () { return TypeSpec::parse (*this); }

bool CSyntax::SimpleTypeSpec::parse (CSyntax &s) {
  // 1: type_name
  // 1: prim_types
  return ((s.is_prim_type () && s.consume ()) || TypeName::check (s));
}
bool CSyntax::simple_type_spec () { return SimpleTypeSpec::parse (*this); }

bool CSyntax::TypeName::parse (CSyntax &s) {
  // 1: typedef_name
  return TypedefName::check (s);
}   
bool CSyntax::type_name () { return TypeName::parse (*this); }

bool CSyntax::ElaboratedTypeSpec::parse (CSyntax &s) {
  // 2: class_key  identifier
  // 2: enum_key  identifier
  return ((ClassKey::check (s) || EnumKey::check (s)) && Identifier::check (s));
}
bool CSyntax::elaborated_type_spec () { return ElaboratedTypeSpec::parse (*this); }

bool CSyntax::EnumKey::parse (CSyntax &s) {
  // 1: ENUM
  // single token already checked by lookahead!
  return s.consume ();
}
bool CSyntax::enum_key () { return EnumKey::parse (*this); }

bool CSyntax::EnumSpec::parse (CSyntax &s) {
  // 4: enum_spec1  {  enumerator_list  }
  return (EnumSpec1::check (s) && s.parse (TOK_OPEN_CURLY) &&
      EnumeratorList::check (s) && s.parse (TOK_CLOSE_CURLY));
}
bool CSyntax::enum_spec () { return EnumSpec::parse (*this); }

bool CSyntax::EnumSpec1::parse (CSyntax &s) {
  // 2: enum_key  identifier
  // 2: enum_key  private_name
  return (EnumKey::check (s) &&
          (Identifier::check (s) || PrivateName::check (s)) &&
          s.look_ahead (TOK_OPEN_CURLY));
}
bool CSyntax::enum_spec1 () { return EnumSpec1::parse (*this); }

bool CSyntax::EnumeratorList::parse (CSyntax &s) {
  // 1 : enumerator_def
  // 2 : enumerator_def  ,
  // 3+: enumerator_def  ,  enumerator_def ...
  // 4+: enumerator_def  ,  enumerator_def ...  , 
  return list<CSyntax, EnumeratorDef> (s, TOK_COMMA, true);
}
bool CSyntax::enumerator_list () { return EnumeratorList::parse (*this); }

bool CSyntax::EnumeratorDef::parse (CSyntax &s) {
  // 1: enumerator
  // 3: enumerator  =  const_expr
  return (Enumerator::check (s) &&
          (s.parse (TOK_ASSIGN) ? ConstExpr::check (s) : true));
}
bool CSyntax::enumerator_def () { return EnumeratorDef::parse (*this); }

bool CSyntax::Enumerator::parse (CSyntax &s) {
  // 1: identifier
  return Identifier::check (s);
}
bool CSyntax::enumerator () { return Enumerator::parse (*this); }

bool CSyntax::AsmDef::parse (CSyntax &s) {
  // 1: ASM  (  str_literal  )  ;
  return (s.parse (TOK_ASM) &&
          s.parse (TOK_OPEN_ROUND) && StrLiteral::check (s) &&
          s.parse (TOK_CLOSE_ROUND) && s.parse (TOK_SEMI_COLON));
}
bool CSyntax::asm_def () { return AsmDef::parse (*this); }

/*****************************************************************************/
/*                                                                           */
/*                     A . 7   D e c l a r a t o r s                         */
/*                                                                           */
/*****************************************************************************/

bool CSyntax::InitDeclaratorList::parse (CSyntax &s) {
  // 1 : init_declarator
  // 3+: init_declarator  ,  init_declarator ...
  return list<CSyntax, InitDeclarator> (s, TOK_COMMA);
}
bool CSyntax::init_declarator_list () { return InitDeclaratorList::parse (*this); }

bool CSyntax::InitDeclarator::parse (CSyntax &s) {
  // 1: init_declarator1
  // 2: init_declarator1 init
  return (InitDeclarator1::check (s) && s.opt (s.init ()));
}
bool CSyntax::init_declarator () { return InitDeclarator::parse (*this); }

// TODO: do we really need this rule? (init_declarator_ext has been removed!)
bool CSyntax::InitDeclarator1::parse (CSyntax &s) {
  // 1: declarator
  return Declarator::check (s);
}
bool CSyntax::init_declarator1 () { return InitDeclarator1::parse (*this); }

bool CSyntax::Declarator::parse (CSyntax &s) {
  // 1 : direct_declarator
  // 2+: ptr_operator..  direct_declarator
  return (s.opt (seq<CSyntax, PtrOperator> (s)) &&
		      DirectDeclarator::check (s));
}
bool CSyntax::declarator () { return Declarator::parse (*this); }

bool CSyntax::DirectDeclarator::parse (CSyntax &s) {
  // 1 : declarator_id
  // 2+: declarator_id  direct_declarator1...
  // 3 : (  declarator  ) 
  // 3+: (  declarator  )  direct_declarator1...
  return ((DeclaratorId::check (s) ||
           (s.parse (TOK_OPEN_ROUND) && Declarator::check (s) &&
            s.parse (TOK_CLOSE_ROUND) && ! s.look_ahead (TOK_DOT))) &&
          s.opt (s.seq (&CSyntax::direct_declarator1)));
}
bool CSyntax::direct_declarator () { return DirectDeclarator::parse (*this); }

bool CSyntax::DirectDeclarator1::parse (CSyntax &s) {
  // 3: [  array_delim  ]
  // 3: (  identifier_list  )
  // 3: (  param_decl_clause  )
  return (s.parse (TOK_OPEN_ROUND) ? 
          ((IdentifierList::check (s) || ParamDeclClause::check (s)) &&
           s.parse (TOK_CLOSE_ROUND)) : 
          (s.parse (TOK_OPEN_SQUARE) && 
           ArrayDelim::check (s) &&
           s.parse (TOK_CLOSE_SQUARE)));
}
bool CSyntax::direct_declarator1 () { return DirectDeclarator1::parse (*this); }

bool CSyntax::IdentifierList::parse (CSyntax &s) {
  // 1 : identifier
  // 3+: identifier  ,  identifier ...
  return list<CSyntax, Identifier> (s, TOK_COMMA);
}
bool CSyntax::identifier_list () { return IdentifierList::parse (*this); }

bool CSyntax::ArrayDelim::parse (CSyntax &s) {
  // 0:
  // 1: *
  // 1: ass_expr
  // 1: cv_qual_seq
  // 2: cv_qual_seq  *
  // 2: cv_qual_seq  ass_expr
  // 2: STATIC  ass_expr
  // 3: STATIC  cv_qual_seq  ass_expr
  // 3: cv_qual_seq  STATIC  ass_expr
  bool in_param_decl = s.semantic ().in_param_decl_clause (); 
  bool have_cv_qual = in_param_decl && CvQualSeq::check (s);
  bool is_static = in_param_decl && s.parse (TOK_STATIC);
  if (s.parse (TOK_MUL)) 
    return true;
  if (! have_cv_qual && is_static)
    CvQualSeq::check (s);
  return (AssExpr::check (s) || ! is_static);
}
bool CSyntax::array_delim () { return ArrayDelim::parse (*this); }

bool CSyntax::PtrOperator::parse (CSyntax &s) {
  // 1: *
  // 2: *  cv_qual_seq
  return (s.parse (TOK_MUL) && s.opt (CvQualSeq::check (s)));
}
bool CSyntax::ptr_operator () { return PtrOperator::parse (*this); }

bool CSyntax::CvQualSeq::parse (CSyntax &s) {
  // 1+: cv_qual...
  return seq<CSyntax, CvQual> (s);
}
bool CSyntax::cv_qual_seq () { return CvQualSeq::parse (*this); }

bool CSyntax::CvQual::parse (CSyntax &s) {
  // 1: cv_quals
  return s.is_cv_qual () && s.consume ();
}
bool CSyntax::cv_qual () { return CvQual::parse (*this); }

bool CSyntax::DeclaratorId::parse (CSyntax &s) {
  // 1: identifier
  return Identifier::check (s);
}
bool CSyntax::declarator_id () { return DeclaratorId::parse (*this); }

bool CSyntax::TypeId::parse (CSyntax &s) {
  // 2: type_spec_seq  abst_declarator
  // 2: type_spec_seq  private_name
  return TypeSpecSeq::check (s) &&
		  (s.abst_declarator () || PrivateName::check (s));
}
bool CSyntax::type_id () { return TypeId::parse (*this); }

bool CSyntax::TypeSpecSeq::parse (CSyntax &s) {
  // 1+: type_spec_seq1...
  return seq<CSyntax,TypeSpecSeq1> (s);
}
bool CSyntax::type_spec_seq() { return TypeSpecSeq::parse (*this); }

bool CSyntax::TypeSpecSeq1::parse (CSyntax &s) {
  // 1: type_spec
  return TypeSpec::check (s);
}
bool CSyntax::type_spec_seq1() { return TypeSpecSeq1::parse (*this); }

bool CSyntax::AbstDeclarator::parse (CSyntax &s) {
  // 1 : direct_abst_declarator
  // 1+: ptr_operator..
  // 2+: ptr_operator..  direct_abst_declarator
  return (DirectAbstDeclarator::check (s) ||
          (seq<CSyntax, PtrOperator> (s) &&
           s.opt (DirectAbstDeclarator::check (s))));
}
bool CSyntax::abst_declarator () { return AbstDeclarator::parse (*this); }

bool CSyntax::DirectAbstDeclarator::parse (CSyntax &s) {
  // 1 : direct_abst_declarator1
  // 2+: direct_abst_declarator1  direct_abst_declarator1...
  // 3 : ( abst_declarator )
  // 3+: ( abst_declarator )  direct_abst_declarator1...
  return ((DirectAbstDeclarator1::check (s) ||
           (s.parse (TOK_OPEN_ROUND) && 
            AbstDeclarator::check (s) &&
            s.parse (TOK_CLOSE_ROUND))) &&  
          s.opt (seq<CSyntax, DirectAbstDeclarator1> (s))); 
}
bool CSyntax::direct_abst_declarator () { return DirectAbstDeclarator::parse (*this); }

bool CSyntax::DirectAbstDeclarator1::parse (CSyntax &s) {
  // 2: [  ]
  // 3: [  *  ]
  // 3: [  ass_expr  ]
  // 3: (  param_decl_clause  )
  return (s.parse (TOK_OPEN_ROUND) ?  
          (ParamDeclClause::check (s), s.parse (TOK_CLOSE_ROUND)) : 
          (s.parse (TOK_OPEN_SQUARE) && 
           (AssExpr::check (s) || s.opt (s.parse (TOK_MUL))) &&
           s.parse (TOK_CLOSE_SQUARE)));
}
bool CSyntax::direct_abst_declarator1 () { return DirectAbstDeclarator1::parse (*this); }

bool CSyntax::ParamDeclClause::parse (CSyntax &s) {
  // 0:
  // 1: param_decl_list
  // 3: param_decl_list  ... 
  return s.opt (s.param_decl_list () && s.opt (s.parse (TOK_ELLIPSIS)));
}
bool CSyntax::param_decl_clause () { return ParamDeclClause::parse (*this); }

bool CSyntax::ParamDeclList::parse (CSyntax &s) {
  // 1 : param_decl
  // 2 : param_decl  ,
  // 3+: param_decl  ,  param_decl ...
  // 4+: param_decl  ,  param_decl ...  , 
  return s.list (&CSyntax::param_decl, TOK_COMMA, true);
}
bool CSyntax::param_decl_list () { return ParamDeclList::parse (*this); }

CTree *CSyntax::rule_param_decl () { 
  // 1: param_decl1 
  if (! ParamDecl1::check (*this))
    return (CTree*)0;
  semantic ().finish_decl ();
  return builder ().param_decl ();
}

bool CSyntax::param_decl () {
  return parse (&CSyntax::rule_param_decl);
}

bool CSyntax::ParamDecl1::parse (CSyntax &s) {
  // 2: decl_spec_seq  abst_declarator
  // 2: decl_spec_seq  declarator
  // 2: decl_spec_seq  private_name
  return (DeclSpecSeq::check (s) &&
          (Declarator::check (s) ||
           AbstDeclarator::check (s) ||
           PrivateName::check (s)));
}
bool CSyntax::param_decl1 () { return ParamDecl1::parse (*this); }

//CTree *CSyntax::rule_param_decl2 () { 
//  // 1: declarator
//  semantic ().begin_param_check (); // ambiguity check
//  Declarator::check (*this);
//  return semantic ().finish_param_check ();
//}
//
//bool CSyntax::param_decl2 () {
//  return parse (&CSyntax::rule_param_decl2);
//}

CTree *CSyntax::rule_fct_def () { 
  // 2: declarator  fct_body
  // 3: decl_spec_seq  declarator  fct_body
  // 4: decl_spec_seq  declarator  arg_decl_seq  fct_body
  if (! ((DeclSpecSeq::check (*this) || semantic ().implicit_int ()) &&
         Declarator::check (*this))) {
    semantic ().finish_declarator ();
    return (CTree*)0;
  }
  // introduce name and check scope; functions shall be 
  // defined only in namespace or class scope
  if (! semantic ().introduce_function ())
    return (CTree*)0;
  return (arg_decl_seq (), fct_body ()) ? 
    semantic ().finish_fct_def () : (CTree*)0;
}

bool CSyntax::fct_def () {
  return parse (&CSyntax::rule_fct_def);
}

CTree *CSyntax::rule_arg_decl_seq () { 
  // 1+: simple_decl
  semantic ().enter_arg_decl_seq ();
  bool success = (seq<CSyntax, SimpleDecl> (*this));
  semantic ().leave_arg_decl_seq ();
  return success ?
    semantic ().arg_decl_seq () : (CTree*)0;
}

bool CSyntax::arg_decl_seq () {
  return parse (&CSyntax::rule_arg_decl_seq);
}

CTree *CSyntax::rule_fct_body () { 
  // 1: cmpd_stmt
  return CmpdStmt::check (*this) ? builder ().fct_body () : (CTree*)0;
}

bool CSyntax::fct_body () {
  return parse (&CSyntax::rule_fct_body);
}

CTree *CSyntax::rule_init () { 
  // 2: =  init_clause 
  return (parse (TOK_ASSIGN) && init_clause ()) ?
    builder ().init () : (CTree*)0;
}

bool CSyntax::init () {
  return parse (&CSyntax::rule_init);
}

CTree *CSyntax::rule_init_clause () { 
  // 1: ass_expr
  // 3: {  init_list  }
  return (AssExpr::check (*this) ||
          (parse (TOK_OPEN_CURLY) && 
           init_list () && 
           parse (TOK_CLOSE_CURLY))) ?
    builder ().init_clause () : (CTree*)0;
}

bool CSyntax::init_clause () {
  return parse (&CSyntax::rule_init_clause);
}

CTree *CSyntax::rule_init_list () { 
  // 1 : init_list_item
  // 2 : init_list_item  ,
  // 3+: init_list_item  ,  init_list_item ...
  // 4+: init_list_item  ,  init_list_item ...  ,
  return list (&CSyntax::init_list_item, TOK_COMMA, true) ? 
    builder ().init_list () : (CTree*)0;
}

bool CSyntax::init_list () {
  return parse (&CSyntax::rule_init_list);
}

CTree *CSyntax::rule_init_list_item () { 
  // 1: init_clause
  // 2: designation  init_clause
  return (designation (), init_clause ()) ? 
    builder ().init_list_item () : (CTree*)0;
}

bool CSyntax::init_list_item () {
  return parse (&CSyntax::rule_init_list_item);
}

CTree *CSyntax::rule_designation () { 
  // 2+: designator...  =
  return (seq (&CSyntax::designator) && parse (TOK_ASSIGN)) ? 
    builder ().designation () : (CTree*)0;
}

bool CSyntax::designation () {
  return parse (&CSyntax::rule_designation);
}

CTree *CSyntax::rule_designator () { 
  // 2: .  identifier
  // 3: [  const_expr  ]
  return (parse (TOK_OPEN_SQUARE) ? 
          (ConstExpr::check (*this) && parse (TOK_CLOSE_SQUARE)) :
          (parse (TOK_DOT) && identifier ())) ? 
    builder ().designator () : (CTree*)0;
}

bool CSyntax::designator () {
  return parse (&CSyntax::rule_designator);
}

/*****************************************************************************/
/*                                                                           */
/*                          A . 8   C l a s s e s                            */
/*                                                                           */
/*****************************************************************************/

void CSyntax::init_class_spec () {
  init_class_head ();
  _class_spec_1 = _class_head_1;
}

CTree *CSyntax::rule_class_spec () { 
  // 3: class_head  {  }
  // 4: class_head  {  member_spec  }
  if (!ClassHead::check (*this))
    return (CTree*)0; 
  semantic ().reenter_class_scope ();
  consume (); // opening bracket
  return ((!look_ahead (TOK_CLOSE_CURLY) && member_spec ()),
          parse (TOK_CLOSE_CURLY)) ? 
    semantic ().class_spec () : (CTree*)0;
}

bool CSyntax::class_spec () {
  return predict_1 (_class_spec_1) && parse (&CSyntax::rule_class_spec);
}

bool CSyntax::ClassHead::parse (CSyntax &s) {
  // 1: class_key
  // 2: class_key  identifier
  return (ClassKey::check (s) &&
          (Identifier::check (s) || PrivateName::check (s)) &&
          s.look_ahead (TOK_OPEN_CURLY));
}
bool CSyntax::class_head () { return ClassHead::parse (*this); }

bool CSyntax::ClassKey::parse (CSyntax &s) {
  // 1: STRUCT
  // 1: UNION
  // single token alread checked by lookahead!
  return s.consume ();
}
bool CSyntax::class_key () { return ClassKey::parse (*this); }

CTree *CSyntax::rule_member_spec () { 
  // 1+: member_decl...
  int skip[] = { /*TOK_SEMI_COLON,*/ TOK_CLOSE_CURLY, 0 };
  int finish[] = { TOK_CLOSE_CURLY, 0 };

  if (! catch_error (&CSyntax::member_decl, 
    "invalid member declaration", finish, skip))
    return (CTree*)0;

  while (catch_error (&CSyntax::member_decl, 
    "invalid member declaration", finish, skip));
  return builder ().member_spec ();
}

bool CSyntax::member_spec () {
  return parse (&CSyntax::rule_member_spec);
}

CTree *CSyntax::rule_member_decl () { 
  // 3: type_spec_seq  member_declarator_list  ;
  if (! (TypeSpecSeq::check (*this) && member_declarator_list () &&
         parse (TOK_SEMI_COLON)))
    return (CTree*)0;
  semantic ().finish_decl ();
  return builder ().member_decl (); 
}

bool CSyntax::member_decl () {
  return parse (&CSyntax::rule_member_decl);
}

CTree *CSyntax::rule_member_declarator_list () { 
  // 1 : member_declarator
  // 3+: member_declarator  ,  member_declarator ...
  return list (&CSyntax::member_declarator, TOK_COMMA) ? 
    builder ().member_declarator_list () : (CTree*)0;
}

bool CSyntax::member_declarator_list () {
  return parse (&CSyntax::rule_member_declarator_list);
}

CTree *CSyntax::rule_member_declarator () { 
  // 1: declarator
  // 3: declarator  :  const_expr
  // 3: private_name  :  const_expr
  bool have_declarator = Declarator::check (*this);
  semantic ().finish_declarator ();
  return (look_ahead (TOK_COLON) ?
            ((have_declarator || PrivateName::check (*this)) &&
           parse (TOK_COLON) && ConstExpr::check (*this)) :
          have_declarator) ? 
    semantic ().introduce_member () : (CTree*)0;
}

bool CSyntax::member_declarator () {
  return parse (&CSyntax::rule_member_declarator);
}

/*****************************************************************************/
/*                                                                           */
/*                   A . 9   D e r i v e d  c l a s s e s                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*        A . 1 0   S p e c i a l  m e m b e r  f u n c t i o n s            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                     A . 1 1   O v e r l o a d i n g                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                       A . 1 2   T e m p l a t e s                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                A . 1 3  E x c e p t i o n  h a n d l i n g                */
/*                                                                           */
/*****************************************************************************/


} // namespace Puma
