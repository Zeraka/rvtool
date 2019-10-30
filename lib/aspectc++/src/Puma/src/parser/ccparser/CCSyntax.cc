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

#include "Puma/CCSyntax.h"
#include "Puma/CCSemantic.h"
#include "Puma/CCBuilder.h"
#include "Puma/CTokens.h"
#include "Puma/CTree.h"

namespace Puma {


CCSyntax::CCSyntax (CCBuilder &b, CCSemantic &s) : 
  CSyntax (b, s), _skip_bodies (SKIP_BODIES_NONE) {
}

CCBuilder &CCSyntax::builder () const { 
  return (CCBuilder&)Syntax::builder (); 
}

CCSemantic &CCSyntax::semantic () const { 
  return (CCSemantic&)Syntax::semantic (); 
}

void CCSyntax::configure (Config &config) {
  // configure the C syntax
  CSyntax::configure (config);
  
  // configure C++ syntax options
  if (config.Option ("--pseudo-instances"))
    _skip_bodies |= SKIP_BODIES_TPL;
  if (config.Option ("--skip-bodies-all"))
    _skip_bodies |= SKIP_BODIES_ALL;
  else if (config.Option ("--skip-bodies-tpl"))
    _skip_bodies |= SKIP_BODIES_TPL;
  else if (config.Option ("--skip-bodies-non-prj"))
    _skip_bodies |= SKIP_BODIES_NON_PRJ;
  else if (config.Option ("--skip-bodies-non-prim"))
    _skip_bodies |= SKIP_BODIES_NON_PRIM;
}

void CCSyntax::init_prim_types () {
  _prim_types.set (TOK_BOOL);
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

void CCSyntax::init_cv_quals () {
  // CSyntax::init_cv_quals is not called, because TOK_RESTRICT is not wanted!
  _cv_quals.set (TOK_CONST);
  _cv_quals.set (TOK_VOLATILE);
}


void CCSyntax::get_search_scope (SearchScope &sc) {
  sc.scope      = semantic ().search_scope;
  sc.last_scope = semantic ().last_search_scope;
  sc.dep        = semantic ().dep_nested_name;
}

void CCSyntax::set_search_scope (SearchScope &sc) {
  semantic ().search_scope      = sc.scope;
  semantic ().last_search_scope = sc.last_scope;
  semantic ().dep_nested_name   = sc.dep;
}

void CCSyntax::skip_fct_try_block () {
  static int stop[] = { TOK_OPEN_ROUND, 0 };
  skip_ctor_init ();
  skip_fct_body ();
  
  // skip handler sequence
  while (look_ahead (TOK_CATCH)) {
    skip (stop, false);    // CATCH
    skip_round_block ();   // (...)
    skip_curly_block ();   // {...}
  }
}

void CCSyntax::skip_ctor_init () {
  static int stop_tokens[] = { TOK_OPEN_CURLY, 0 };
  skip (stop_tokens, false);
}

void CCSyntax::skip_fct_body () {
  skip_curly_block ();
}

void CCSyntax::skip_param_init () {
  static int stop_tokens[] = { TOK_COMMA, TOK_CLOSE_ROUND, TOK_ELLIPSIS, 0 };
  skip (stop_tokens, false);
}

void CCSyntax::skip_const_expr () {
  static int stop_tokens[] = { TOK_COMMA, TOK_SEMI_COLON, 0 };
  skip (stop_tokens, false);
}

void CCSyntax::skip_const_init () {
  static int stop_tokens[] = { TOK_COMMA, TOK_SEMI_COLON, 0 };
  skip (stop_tokens, false);
}

bool CCSyntax::is_fct_def () {
  State s = token_provider->get_state ();
  bool result = false, braces = false, stop = false;
  Token *current;
  int token;

  while (!stop && (current = token_provider->current ())) {
    token = current->type ();

    // function body
    if (braces && (token == TOK_OPEN_CURLY || token == TOK_COLON)) {
      result = true;
      break;
    } else if (!braces && token == TOK_ASSIGN) {
      break;
    // class definition in function return type
    } else if (!braces && token == TOK_OPEN_CURLY) {
      skip_curly_block ();
    } else switch (token) {
      // only declaration
      case TOK_SEMI_COLON:
      case TOK_COMMA:
      case TOK_TYPEDEF:
      // TODO: checking the AspectC++ tokens here is really bad style
      case TOK_ADVICE:
      case TOK_POINTCUT:
      case TOK_SLICE:
        stop = true;
        break;
      // operator name
      case TOK_OPERATOR:
        skip (TOK_OPEN_ROUND, false);
        break;
      // function `try' block, function body..
      case TOK_TRY:
        result = true;
        stop = true;
        break;
      // array delimiter
      case TOK_OPEN_SQUARE:
        skip_block (TOK_OPEN_SQUARE, TOK_CLOSE_SQUARE);
        break;
      // template-id
      case TOK_LESS:
        skip_block (TOK_LESS, TOK_GREATER);
        break;
      // function parameter list
      case TOK_OPEN_ROUND:
        skip_round_block ();
        braces = true;
        break;
      // throw expression
      case TOK_THROW:
        skip ();
        skip_round_block (); 
        break;
      // asm declaration specifier
      case TOK_ASM:
        skip ();
        skip_round_block ();
        break;
      // names etc.
      default:
        skip ();
        break;
    }
  }

  token_provider->set_state (s);
  return result;
}

bool CCSyntax::is_nested (State first) {
  int token, braces, curlies, squares;
  Token *current;

  if (! first)
    return false;

  State s = token_provider->get_state ();
  token_provider->set_state (first);

  braces = curlies = squares = 0;
  while ((current = token_provider->current ())) {
    token = current->type ();

    switch (token) {
      case TOK_OPEN_ROUND: braces++; break;
      case TOK_CLOSE_ROUND: braces--; break;
      case TOK_OPEN_CURLY: curlies++; break;
      case TOK_CLOSE_CURLY: curlies--; break;
      case TOK_OPEN_SQUARE: squares++; break;
      case TOK_CLOSE_SQUARE: squares--; break;
      default: break;
    }

    if (token_provider->get_state () == s)
      break;

    skip ();
  }

  token_provider->set_state (s);
  return (braces + curlies + squares) > 0;
}

bool CCSyntax::is_nested_name () {
  // result cache!
  static Token *last_token = 0;
  static bool last_result = false;

  State s = token_provider->get_state ();
  Token *current = token_provider->current ();
  if (! current) {
    token_provider->set_state (s);
    return false;
  }

  // return the last result if we check the same token
  if (last_token == current)
    return last_result;

  last_token  = current;
  last_result = false;
  int token, depth = 0;

  // keyword `template'?
  if (current->type () == TOK_TEMPLATE)
    skip ();

  // identifier?
  current = token_provider->current ();
  if (current && current->type () == TOK_ID) {
    skip ();

    // template id?
    if (token_provider->current () &&
        token_provider->current ()->type () == TOK_LESS) {
      skip ();

      bool stop = false;
      while (!stop && (current = token_provider->current ())) {
        token = current->type ();

        switch (token) {
          case TOK_SEMI_COLON:
          case TOK_CLOSE_ROUND:
          case TOK_CLOSE_SQUARE:
          case TOK_CLOSE_CURLY:
            stop = true;
            break;
          case TOK_OPEN_ROUND:
            skip_round_block ();
            continue;
          case TOK_OPEN_CURLY:
            skip_curly_block ();
            continue;
          case TOK_OPEN_SQUARE:
            skip_block (TOK_OPEN_SQUARE, TOK_CLOSE_SQUARE);
            continue;
          case TOK_LESS:
            depth++;
            break;
          case TOK_GREATER:
            if (depth == 0) {
              skip ();
              stop = true;
            } else {
              depth--;
            }
          default:
            break;
        }

        if (!stop)
          skip ();
      }
    }

    // scope operator => nested name
    if (token_provider->current () &&
        token_provider->current ()->type () == TOK_COLON_COLON)
      last_result = true;
  }

  token_provider->set_state (s);
  return last_result;
}

bool CCSyntax::is_class_def () {
  // result cache!
  static Token *last_token = 0;
  static bool last_result = false;

  State s = token_provider->get_state ();
  Token *current = token_provider->current ();
  if (! current) {
    token_provider->set_state (s);
    return false;
  }

  // return the last result if we check the same token
  if (last_token == current)
    return last_result;

  last_token = current;
  last_result = false;
  int token;

  bool stop = false;
  while (!stop && (current = token_provider->current ())) {
    token = current->type ();

    switch (token) {
      case TOK_SEMI_COLON:
        stop = true;
        break;
      case TOK_OPEN_CURLY:
        last_result = true;
        stop = true;
        break;
      case TOK_OPEN_ROUND:
        skip_round_block ();
        continue;
      case TOK_OPEN_SQUARE:
        skip_block (TOK_OPEN_SQUARE, TOK_CLOSE_SQUARE);
        continue;
      default:
        break;
    }

    if (!stop)
      skip ();
  }

  token_provider->set_state (s);
  return last_result;
}

bool CCSyntax::is_tpl_id () {
  int token, depth = 0, roundDepth = 0, curlyDepth = 0, squareDepth = 0;
  State s = token_provider->get_state ();
  Token *current;

  while ((current = token_provider->current ())) {
    token = current->type ();

    switch (token) {
      case TOK_LESS: depth++; break;
      case TOK_GREATER: depth--; break;
      case TOK_OPEN_ROUND: roundDepth++; break;
      case TOK_CLOSE_ROUND: roundDepth--; break;
      case TOK_OPEN_CURLY: curlyDepth++; break;
      case TOK_CLOSE_CURLY: curlyDepth--; break;
      case TOK_OPEN_SQUARE: squareDepth++; break;
      case TOK_CLOSE_SQUARE: squareDepth--; break;
      default: break;
    }

    if ((depth == 0 || roundDepth < 0 || squareDepth < 0 || curlyDepth < 0) ||
        (token == TOK_SEMI_COLON && curlyDepth == 0)) {
      break;
    }

    skip ();
  }

  token_provider->set_state (s);
  return depth == 0;
}

// find out if a qualified name is a template declarator id
// rather than a declaration specifier, this is used to 
// decide when to instantiate a template id
bool CCSyntax::is_tpl_declarator_id () {
  // doesn't matter in parameter declaration
  if (semantic().in_param_decl_clause() ||
      semantic().in_template_param_list()) {
    return false;
  }

  // hack: This is save here (and not later), because an aspect might
  // skip tokens while executing token_provider->current().
  State s = token_provider->get_state ();

  // only consider names
  if (! token_provider->current () ||
      token_provider->current ()->type() != TOK_ID) {
    token_provider->set_state (s); // see comment above
    return false;
  }

  bool result = false, tpl_id = false;

  while (token_provider->current ()) {
    int token = token_provider->current ()->type ();

    // match names only
    if (token != TOK_ID) {
      // operator name is a declarator id
      result = (token == TOK_OPERATOR);
      break;
    }
    // skip name
    skip();

    // skip template ids
    if (token_provider->current () &&
        token_provider->current ()->type () == TOK_LESS &&
        is_tpl_id ()) {
      tpl_id = true;
      skip_block (TOK_LESS, TOK_GREATER);
    }

    // nested name, go on to next name part
    if (token_provider->current () &&
        token_provider->current ()->type () == TOK_COLON_COLON) {
      skip();
      continue;
    }

    // not a nested name, now check for declarator id
    token = token_provider->current () ? token_provider->current ()->type () : 0;
    if (token == TOK_OPEN_ROUND) {
      if (is_ptr_to_fct ()) {
        // pointer to function or reference to function
        break;
      } else {
        skip_round_block ();
      }
      token = token_provider->current () ? token_provider->current ()->type () : 0;
      // not a function or array pointer decl?
      result = (token != TOK_OPEN_ROUND && token != TOK_OPEN_SQUARE);
    } else {
      result = (token == TOK_ASSIGN || token == TOK_SEMI_COLON || token == TOK_OPEN_SQUARE);
    }
    break;
  }

  token_provider->set_state (s);
  return tpl_id && result;
}

bool CCSyntax::is_ptr_to_fct () {
  State s = token_provider->get_state ();
  bool result = false;

  // skip all open parentheses
  while (token_provider->current () && token_provider->current ()->type () == TOK_OPEN_ROUND) {
    skip();
  }

  if (token_provider->current ()) {
    // expect * or &
    result = token_provider->current ()->type () == TOK_MUL ||
             token_provider->current ()->type () == TOK_AND;
  }

  token_provider->set_state (s);
  return result;
}

bool CCSyntax::is_ass_expr () {
  State s = token_provider->get_state ();
  Token *current = token_provider->current ();
  if (! current) {
    token_provider->set_state (s);
    return false;
  }

  // return the last result if we check the same token
  if (last_look_ahead_token == current)
    return last_look_ahead_result;

  last_look_ahead_token = current;
  last_look_ahead_result = false;

  int token;
  bool id_found = false;

  bool stop = false;
  while (!stop && (current = token_provider->current ())) {
    token = current->type ();

    if (token == TOK_ID) {
      id_found = true;
      skip ();
      continue;
    }

    if (id_found && token == TOK_LESS) {
      //      skip_block (TOK_LESS, TOK_GREATER);
      last_look_ahead_result = true;
      stop = true;
      break;
    } else switch (token) {
      case TOK_SEMI_COLON:
      case TOK_COMMA:
      case TOK_CLOSE_CURLY:
      case TOK_CLOSE_ROUND:
      case TOK_CLOSE_SQUARE:
      case TOK_COLON:
      case TOK_QUESTION:
      case TOK_WHILE:
      case TOK_DO:
      case TOK_FOR:
      case TOK_IF:
      case TOK_ELSE:
      case TOK_SWITCH:
      case TOK_CASE:
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

    if (!stop)
      id_found = false;
  }

  token_provider->set_state (s);
  return last_look_ahead_result;
}

/*****************************************************************************/
/*                                                                           */
/*                           C + +   G r a m m a r                           */
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

bool CCSyntax::ClassName::parse (CCSyntax &s) {
  // 1: template_id
  // 1: ID
  return s.template_id () || s.parse (TOK_ID); // TODO: should be TemplateId::check(s)
}
bool CCSyntax::class_name () { return ClassName::parse (*this); }

bool CCSyntax::EnumName::parse (CCSyntax &s) {
  // 1: ID
  return s.parse (TOK_ID);
}
bool CCSyntax::enum_name () { return EnumName::parse (*this); }

bool CCSyntax::TemplateName::parse (CCSyntax &s) {
  // 1: ID
  return s.parse (TOK_ID);
}
bool CCSyntax::template_name () { return TemplateName::parse (*this); }

bool CCSyntax::ClassTemplateName::parse (CCSyntax &s) {
  // 1: ID
  return s.parse (TOK_ID);
}
bool CCSyntax::class_template_name () { return ClassTemplateName::parse (*this); }

bool CCSyntax::NamespaceName::parse (CCSyntax &s) {
  // 1: original_ns_name
  // 1: namespace_alias
  return (OriginalNsName::check (s) || NamespaceAlias::check (s));
}
bool CCSyntax::namespace_name () { return NamespaceName::parse (*this); }

bool CCSyntax::OriginalNsName::parse (CCSyntax &s) {
  // 1: ID
  return s.parse (TOK_ID);
}
bool CCSyntax::original_ns_name () { return OriginalNsName::parse (*this); }

bool CCSyntax::NamespaceAlias::parse (CCSyntax &s) {
  // 1: ID
  return s.parse (TOK_ID);
}
bool CCSyntax::namespace_alias () { return NamespaceAlias::parse (*this); }

/*****************************************************************************/
/*                                                                           */
/*               A . 2   L e x i c a l  c o n v e n t i o n s                */
/*                                                                           */
/*****************************************************************************/

bool CCSyntax::Literal::parse (CCSyntax &s) {
  // 1: cmpd_str
  // 1: TOK_INT_VAL  (e.g. 4711)
  // 1: TOK_ZERO_VAL (0)
  // 1: TOK_CHAR_VAL (e.g. 'x')
  // 1: TOK_FLT_VAL  (e.g. 3.14)
  // 1: TOK_BOOL_VAL (e.g. true)
  return CSyntax::Literal::parse (s) || s.parse (TOK_BOOL_VAL);
}
bool CCSyntax::literal () { return Literal::parse (*this); }

/*****************************************************************************/
/*                                                                           */
/*                      A . 4   E x p r e s s i o n s                        */
/*                                                                           */
/*****************************************************************************/

bool CCSyntax::PrimExpr::parse (CCSyntax &s) {
  // 1: literal
  // 1: THIS
  // 1: id_expr
  // 3: (  expr  )
  return (s.parse (TOK_THIS) ||
		   CSyntax::PrimExpr::parse (s));
}
bool CCSyntax::prim_expr () { return PrimExpr::parse (*this); }

bool CCSyntax::IdExpr::parse (CCSyntax &s) {
  // 1: qual_id
  // 1: unqual_id
  return (QualId::check (s) ||
          UnqualId::check (s));
}
bool CCSyntax::id_expr () { return IdExpr::parse (*this); }

bool CCSyntax::QualId::parse (CCSyntax &s) {
  // 2: colon_colon  identifier
  // 2: colon_colon  oper_fct_id
  // 2: colon_colon  template_id
  // 2: nested_name_spec  unqual_id
  // 3: nested_name_spec  template_key  unqual_id
  // 3: colon_colon  nested_name_spec  unqual_id
  // 4: colon_colon  nested_name_spec  template_key  unqual_id
  bool root = ColonColon::check (s);
  bool nested = NestedNameSpec::check (s);
  bool ok = (nested ?
             (s.template_key (),
              UnqualId::check (s)) :
             (root &&
              (s.template_id () ||
               Identifier::check (s) ||
               s.oper_fct_id ())));
  if (root || nested)  // TODO: move this into an aspect
    s.semantic ().reset_search_scope ();
  return ok;
}
bool CCSyntax::qual_id () { return QualId::parse (*this); }

bool CCSyntax::UnqualId::parse (CCSyntax &s) {
  // 1: identifier
  // 1: oper_fct_id
  // 1: conv_fct_id
  // 1: template_id
  // 2: ~  class_name
  return (s.template_id () ||
          Identifier::check (s) ||
          s.oper_fct_id () ||
          ConvFctId::check (s) ||
          (s.parse (TOK_TILDE) && ClassName::check (s)));
}
bool CCSyntax::unqual_id () { return UnqualId::parse (*this); }

bool CCSyntax::ColonColon::parse (CCSyntax &s) {
  // 1: ::
  return s.parse (TOK_COLON_COLON);
}
bool CCSyntax::colon_colon () { return ColonColon::parse (*this); }

bool CCSyntax::NestedNameSpec::parse (CCSyntax &s) {
  // 1+: nested_name_spec1..
  return seq<CCSyntax, NestedNameSpec1> (s);
}
bool CCSyntax::nested_name_spec () { return NestedNameSpec::parse (*this); }

bool CCSyntax::NestedNameSpec1::parse (CCSyntax &s) {
  // 2: class_or_ns_name  ::
  // 3: template_key  class_or_ns_name  ::
  return ((s.template_key (), ClassOrNsName::check (s)) && s.parse (TOK_COLON_COLON));
}
bool CCSyntax::nested_name_spec1 () { return NestedNameSpec1::parse (*this); }

bool CCSyntax::ClassOrNsName::parse (CCSyntax &s) {
  // 1: class_name
  // 1: namespace_name
  return (ClassName::check (s) || NamespaceName::check (s));
}
bool CCSyntax::class_or_ns_name () { return ClassOrNsName::parse (*this); }

bool CCSyntax::PostfixExpr::parse (CCSyntax &s) {
  // 1 : postfix_expr2
  // 1+: postfix_expr2  postfix_expr1..
  if (! PostfixExpr2::check (s))
    return false;

  // TODO: be careful! don't use seq here. There is call advice.
  while (PostfixExpr1::check (s));
  return true;
}
bool CCSyntax::postfix_expr () { return PostfixExpr::parse (*this); }

bool CCSyntax::PostfixExpr2::parse (CCSyntax &s) {
  // 1 : cmpd_literal
  // 1 : prim_expr
  // 1 : construct_expr
  // 4 : TYPEID  (  expr  )
  // 4 : TYPEID  (  type_id  )
  // 7 : CONST_CAST  <  type_id  >  (  expr  )
  // 7 : REINT_CAST  <  type_id  >  (  expr  )
  // 7 : STATIC_CAST  <  type_id  >  (  expr  )
  // 7 : DYNAMIC_CAST  <  type_id  >  (  expr  )
  static int any_new_cast[] = { TOK_DYN_CAST, TOK_REINT_CAST, 
                                TOK_STAT_CAST, TOK_CONST_CAST, 0 };  
  // new-style casts
  if (s.parse (any_new_cast)) {
    return (s.parse (TOK_LESS) && s.type_id () &&
            s.parse (TOK_GREATER) &&
            s.parse (TOK_OPEN_ROUND) && Expr::check (s) &&
            s.parse (TOK_CLOSE_ROUND));
  }
  // construct expression, compound literal, member access expression
  else if (ConstructExpr::check (s) || CmpdLiteral::check (s) || PrimExpr::check (s)) {
    return true;
  }
  // typeid (RTTI)
  else if (s.parse (TOK_TYPEID)) {
    return (s.parse (TOK_OPEN_ROUND) &&
            (Expr::check (s) || s.type_id ()) &&
                s.parse (TOK_CLOSE_ROUND));
  }
  return false;
}
bool CCSyntax::postfix_expr2() { return PostfixExpr2::parse (*this); }

bool CCSyntax::ConstructExpr::parse (CCSyntax &s) {
  // 3 : simple_type_spec  (  )
  // 4 : simple_type_spec  (  expr_list  )
  // 5 : TYPENAME  nested_name_spec  identifier  (  )
  // 5 : TYPENAME  nested_name_spec  template_id  (  )
  // 6 : TYPENAME  nested_name_spec  identifier  (  expr_list  )
  // 6 : TYPENAME  nested_name_spec  template_id  (  expr_list  )
  // 6 : TYPENAME  nested_name_spec  template_key  template_id  (  )
  // 6 : TYPENAME  ::  nested_name_spec  identifier  (  )
  // 6 : TYPENAME  ::  nested_name_spec  template_id  (  )
  // 7 : TYPENAME  nested_name_spec  template_key  template_id  (  expr_list  )
  // 7 : TYPENAME  ::  nested_name_spec  identifier  (  expr_list  )
  // 7 : TYPENAME  ::  nested_name_spec  template_id  (  expr_list  )
  // 7 : TYPENAME  ::  nested_name_spec  template_key  template_id  (  )
  // 8 : TYPENAME  ::  nested_name_spec  template_key  template_id  (  expr_list  )
  if (s.parse (TOK_TYPENAME)) {
    return (s.parse (TOK_COLON_COLON), NestedNameSpec::check (s) &&
            (s.template_key () ? s.template_id () :
             s.template_id () || Identifier::check (s)) &&
            s.semantic ().reset_search_scope () && // TODO: remove sem stuff here
            s.parse (TOK_OPEN_ROUND) &&
            (ExprList::check (s), s.parse (TOK_CLOSE_ROUND)));
  }
  return (s.simple_type_spec () && s.parse (TOK_OPEN_ROUND) &&
          (ExprList::check (s), s.parse (TOK_CLOSE_ROUND)));
}
bool CCSyntax::construct_expr() { return ConstructExpr::parse (*this); }

bool CCSyntax::PostfixExpr1::parse (CCSyntax &s) {
  // 1: --
  // 1: ++
  // 2: .  id_expr
  // 2: ->  id_expr
  // 2: .  pseudo_dtor_name
  // 2: ->  pseudo_dtor_name
  // 2: (  )
  // 3: .  template_key  id_expr
  // 3: ->  template_key  id_expr
  // 3: (  expr_list  )
  // 3: [  expr  ]
  return
    (s.parse (TOK_OPEN_ROUND) ?
      (ExprList::check (s), s.parse (TOK_CLOSE_ROUND)) :
       s.parse (TOK_OPEN_SQUARE) ?
      (Expr::check (s) && s.parse (TOK_CLOSE_SQUARE)) :
     (s.parse (TOK_DECR) || s.parse (TOK_INCR) ||
     ((s.parse (TOK_DOT) || s.parse (TOK_PTS)) &&
      (PseudoDtorName::check (s) ||
       (s.template_key (), IdExpr::check (s))))));
}
bool CCSyntax::postfix_expr1() { return PostfixExpr1::parse (*this); }

bool CCSyntax::PseudoDtorName::parse (CCSyntax &s) {
  // 2: ~  type_name
  // 3: ::  ~  type_name
  // 3: nested_name_spec  ~  type_name
  // 4: ::  nested_name_spec  ~  type_name
  // 4: type_name  ::  ~  type_name
  // 5: ::  type_name  ::  ~  type_name
  // 5: nested_name_spec  type_name  ::  ~  type_name
  // 6: ::  nested_name_spec  type_name  ::  ~  type_name
  // 6: nested_name_spec  template_key  template_id  ::  ~  type_name
  // 7: ::  nested_name_spec  template_key  template_id  ::  ~  type_name
  bool ok = ((ColonColon::check (s), NestedNameSpec::check (s),
             (s.template_key () ? s.template_id () : TypeName::check (s))) &&
             ! s.parse (TOK_COLON_COLON));
  s.semantic ().reset_search_scope (); // TODO: remove sem stuff here
  if (! ok) 
    return false;
  return (s.parse (TOK_TILDE) && TypeName::check (s));
}
bool CCSyntax::pseudo_dtor_name() { return PseudoDtorName::parse (*this); }

bool CCSyntax::UnaryExpr::parse (CCSyntax &s) {
  // 1: new_expr
  // 1: delete_expr
  // 1: postfix_expr
  // 1: offsetof_expr
  // 1: typetrait_expr
  // 2: any_unary_op  cast_expr
  // 2: SIZEOF  unary_expr
  // 2: ALIGNOF  unary_expr
  // 4: SIZEOF  unary_expr1
  // 4: ALIGNOF  unary_expr1
  static int any_unary_op[] = { TOK_AND, TOK_MUL, TOK_PLUS, TOK_MINUS, 
                                TOK_TILDE, TOK_NOT, TOK_DECR, TOK_INCR, 0 };
  return (s.parse (any_unary_op) ?
           CastExpr::check (s) :
          (s.parse (TOK_SIZEOF) || s.parse (TOK_ALIGNOF)) ?
           (UnaryExpr1::check (s) || UnaryExpr::check (s)) :
          (NewExpr::check (s) ||
           DeleteExpr::check (s) ||
           OffsetofExpr::check (s) ||
           TypeTraitExpr::check (s) ||
           PostfixExpr::check (s)));
}
bool CCSyntax::unary_expr () { return UnaryExpr::parse (*this); }

bool CCSyntax::TypeTraitExpr::parse (CCSyntax &s) {
  // 4: any_unary_type_trait_op  (  type_id  )
  // 6: any_binary_type_trait_op  (  type_id  ,  type_id  )
  static int any_unary_type_trait_op[] = {
    TOK_HAS_NOTHROW_ASSIGN, TOK_HAS_NOTHROW_COPY, TOK_HAS_NOTHROW_CTOR,
    TOK_HAS_TRIVIAL_ASSIGN, TOK_HAS_TRIVIAL_COPY, TOK_HAS_TRIVIAL_CTOR,
    TOK_HAS_TRIVIAL_DTOR, TOK_HAS_VIRTUAL_DTOR, TOK_IS_ABSTRACT,
    TOK_IS_CLASS, TOK_IS_EMPTY, TOK_IS_ENUM, TOK_IS_POD, TOK_IS_TRIVIAL,
    TOK_IS_POLYMORPHIC, TOK_IS_UNION, 0 };
  static int any_binary_type_trait_op[] = {
    TOK_IS_BASE_OF, 0 };
  return (s.parse (any_unary_type_trait_op) ?
           s.parse (TOK_OPEN_ROUND) &&
           s.type_id () &&
           s.parse (TOK_CLOSE_ROUND) :
          s.parse (any_binary_type_trait_op) &&
           s.parse (TOK_OPEN_ROUND) &&
           s.type_id () &&
           s.parse (TOK_COMMA) &&
           s.type_id () &&
           s.parse (TOK_CLOSE_ROUND));
}
bool CCSyntax::type_trait_expr () { return TypeTraitExpr::parse (*this); }

bool CCSyntax::NewExpr::parse (CCSyntax &s) {
  // 2: NEW  new_type_id
  // 3: NEW  new_type_id  new_init
  // 3: NEW  new_placement  new_type_id
  // 3: ::  NEW  new_type_id
  // 4: ::  NEW  new_type_id  new_init
  // 4: ::  NEW  new_placement  new_type_id
  // 4: NEW  new_placement  new_type_id  new_init
  // 4: NEW  (  type_id  )
  // 5: NEW  (  type_id  )  new_init
  // 5: NEW  new_placement  (  type_id  )
  // 5: ::  NEW  new_placement  new_type_id  new_init
  // 5: ::  NEW  (  type_id  )
  // 6: ::  NEW  (  type_id  )  new_init
  // 6: NEW  new_placement  (  type_id  )  new_init
  // 6: ::  NEW  new_placement  (  type_id  )
  // 7: ::  NEW  new_placement  (  type_id  )  new_init
  return ((s.parse (TOK_COLON_COLON), s.parse (TOK_NEW)) &&
          s.opt (NewPlacement::check (s)) &&
          (NewTypeId::check (s) ||
           (s.parse (TOK_OPEN_ROUND) &&
            s.type_id () &&
            s.parse (TOK_CLOSE_ROUND))) &&
          s.opt (NewInit::check (s)));
}
bool CCSyntax::new_expr () { return NewExpr::parse (*this); }

bool CCSyntax::NewPlacement::parse (CCSyntax &s) {
  // 3: (  expr_list  )
  return (s.parse (TOK_OPEN_ROUND) && 
          ExprList::check (s) &&
          s.parse (TOK_CLOSE_ROUND));
}
bool CCSyntax::new_placement () { return NewPlacement::parse (*this); }

bool CCSyntax::NewTypeId::parse (CCSyntax &s) {
  // 2: type_spec_seq  new_declarator
  // 2: type_spec_seq  private_name
  return s.type_spec_seq () && 
    (NewDeclarator::check (s) || PrivateName::check (s));
}
bool CCSyntax::new_type_id () { return NewTypeId::parse (*this); }

bool CCSyntax::NewDeclarator::parse (CCSyntax &s) {
  // 1 : direct_new_declarator
  // 1+: ptr_operator..
  // 2+: ptr_operator..  direct_new_declarator
  return (DirectNewDeclarator::check (s) ||
          (seq<CCSyntax, PtrOperator> (s) &&
           s.opt (DirectNewDeclarator::check (s))));
}
bool CCSyntax::new_declarator () { return NewDeclarator::parse (*this); }

bool CCSyntax::DirectNewDeclarator::parse (CCSyntax &s) {
  // 3 : [  expr  ]
  // 4+: [  expr  ]  direct_new_declarator1...
  return ((s.parse (TOK_OPEN_SQUARE) && Expr::check (s) &&
           s.parse (TOK_CLOSE_SQUARE)) &&  
          s.opt (seq<CCSyntax, DirectNewDeclarator1>(s)));
}
bool CCSyntax::direct_new_declarator () { return DirectNewDeclarator::parse (*this); }

bool CCSyntax::DirectNewDeclarator1::parse (CCSyntax &s) {
  // 3: [  const_expr  ]
  return (s.parse (TOK_OPEN_SQUARE) && ConstExpr::check (s) &&
          s.parse (TOK_CLOSE_SQUARE));
}
bool CCSyntax::direct_new_declarator1 () { return DirectNewDeclarator1::parse (*this); }

bool CCSyntax::NewInit::parse (CCSyntax &s) {
  // 2: (  )
  // 3: (  expr_list  )
  return (s.parse (TOK_OPEN_ROUND) && (ExprList::check (s),
          s.parse (TOK_CLOSE_ROUND)));
}
bool CCSyntax::new_init () { return NewInit::parse (*this); }

bool CCSyntax::DeleteExpr::parse (CCSyntax &s) {
  // 2: DELETE  cast_expr
  // 3: ::  DELETE  cast_expr
  // 4: DELETE  [  ]  cast_expr
  // 5: ::  DELETE  [  ]  cast_expr
  return ((s.parse (TOK_COLON_COLON), s.parse (TOK_DELETE)) &&
          (s.parse (TOK_OPEN_SQUARE) ? s.parse (TOK_CLOSE_SQUARE) : true) && 
          CastExpr::check (s));
}
bool CCSyntax::delete_expr () { return DeleteExpr::parse (*this); }

bool CCSyntax::PmExpr::parse (CCSyntax &s) {
  // 1 : cast_expr
  // 3+: cast_expr  any_pm_op  cast_expr ..
  static int any_pm_op[] = { TOK_DOT_STAR, TOK_PTS_STAR, 0 };
  return list<CCSyntax, CastExpr> (s, any_pm_op);
}
bool CCSyntax::pm_expr () { return PmExpr::parse (*this); }

bool CCSyntax::MulExpr::parse (CCSyntax &s) {
  // 1 : pm_expr
  // 3+: pm_expr  any_mul_op  pm_expr ..
  static int any_mul_op[] = { TOK_MUL, TOK_DIV, TOK_MODULO, 0 };
  return list<CCSyntax, PmExpr> (s, any_mul_op);
}
bool CCSyntax::mul_expr () { return MulExpr::parse (*this); }

bool CCSyntax::RelExpr::parse (CCSyntax &s) {
  // 1 : shift_expr
  // 3+: shift_expr  any_rel_op  shift_expr ...
  static int any_rel_op[] = { TOK_LESS, TOK_GREATER, TOK_LEQ, TOK_GEQ, 0 };
  if (! ShiftExpr::check (s))
    return false;
  while (true) {
    // ���14.2.3 check if this is the end of a template argument list
    if (s.look_ahead (TOK_GREATER)) {
      if ((s.semantic ().in_template_arg_list () &&
           ! s.is_nested (s.semantic ().in_template_arg_list ())) ||
          (s.semantic ().in_template_param_list () &&
           ! s.is_nested (s.semantic ().in_template_param_list ())))
      break;
    }
    if (! s.parse (any_rel_op))
      break;
    if (! ShiftExpr::check (s))
      return false;
  }
  return true;
}
bool CCSyntax::rel_expr () { return RelExpr::parse (*this); }

bool CCSyntax::CondExpr::parse (CCSyntax &s) {
  // 1: log_or_expr
  // 5: log_or_expr  ?  expr  :  ass_expr
  return (LogOrExpr::check (s) &&
          (s.parse (TOK_QUESTION) ? (Expr::check (s) &&
           s.parse (TOK_COLON) && AssExpr::check (s)) : true));
}
bool CCSyntax::cond_expr () { return CondExpr::parse (*this); }

bool CCSyntax::AssExpr::parse (CCSyntax &s) {
  // 1 : cond_expr
  // 1 : throw_expr
  // 2+: ass_expr1..  cond_expr
  // 2+: ass_expr1..  throw_expr
  return (seq<CCSyntax, AssExpr1> (s),
           (s.throw_expr () || CondExpr::check (s)));
}
bool CCSyntax::ass_expr () { return AssExpr::parse (*this); }

bool CCSyntax::AssExpr1::parse (CCSyntax &s) {
  // 2: log_or_expr  any_ass_op
  static int any_ass_op[] = { TOK_ASSIGN, TOK_MUL_EQ, TOK_DIV_EQ, TOK_MOD_EQ, 
                              TOK_ADD_EQ, TOK_SUB_EQ, TOK_RSH_EQ, TOK_LSH_EQ,
                              TOK_AND_EQ, TOK_XOR_EQ, TOK_IOR_EQ, 0 };
  return (s.is_ass_expr () && LogOrExpr::check (s) && s.parse (any_ass_op));
}
bool CCSyntax::ass_expr1 () { return AssExpr1::parse (*this); }

bool CCSyntax::ConstExpr::parse (CCSyntax &s) {
  // 1: cond_expr
  return CondExpr::check (s);
}
bool CCSyntax::const_expr () { return ConstExpr::parse (*this); }

/*****************************************************************************/
/*                                                                           */
/*                      A . 5   S t a t e m e n t s                          */
/*                                                                           */
/*****************************************************************************/

bool CCSyntax::Stmt::parse (CCSyntax &s) {
  // 1: label_stmt
  // 1: decl_stmt
  // 1: expr_stmt
  // 1: cmpd_stmt
  // 1: select_stmt
  // 1: iter_stmt
  // 1: jump_stmt
  // 1: try_block
  return (LabelStmt::check (s) ||
          ambiguous<DeclStmt, ExprStmt> (s) ||
          CmpdStmt::check (s) ||
          SelectStmt::check (s) ||
          IterStmt::check (s) ||
          JumpStmt::check (s) ||
          s.try_block ()); 
}
bool CCSyntax::stmt () { return Stmt::parse (*this); }

bool CCSyntax::StmtSeq::parse (CCSyntax &s) {
  // 1+: stmt..
  static int skip[] = { TOK_SEMI_COLON, TOK_CLOSE_CURLY, 0 };
  static int finish[] = { TOK_CLOSE_CURLY, 0 };
  if (! catch_error<CCSyntax, Stmt> (s, "invalid statement", finish, skip))
    return false;
  while (catch_error<CCSyntax, Stmt> (s, "invalid statement", finish, skip));
  return true;
}
bool CCSyntax::stmt_seq () { return StmtSeq::parse (*this); }

bool CCSyntax::SubStmt::parse (CCSyntax &s) {
  // 1: stmt
  // a substatement that is not a compound-statement is as if it 
  // would be a compound-statement containing the original statement;
  // in this case it implicitly defines a local scope
  if (! s.look_ahead (TOK_OPEN_CURLY))
    s.semantic ().enter_local_scope (); // enter local (block) scope
  return Stmt::check (s); 
}
bool CCSyntax::sub_stmt () { return SubStmt::parse (*this); }

bool CCSyntax::Condition::parse (CCSyntax &s) {
  // 1: expr
  // 1: condition2
  return (Expr::check (s) || Condition2::check (s));
}
bool CCSyntax::condition () { return Condition::parse (*this); }

bool CCSyntax::Condition1::parse (CCSyntax &s) {
  // 2: type_spec_seq  declarator
  return (s.type_spec_seq () && Declarator::check (s));
}
bool CCSyntax::condition1 () { return Condition1::parse (*this); }

bool CCSyntax::Condition2::parse (CCSyntax &s) {
  // 3: condition1  =  ass_expr
  return (Condition1::check (s) && s.parse (TOK_ASSIGN) && AssExpr::check (s));
}
bool CCSyntax::condition2 () { return Condition2::parse (*this); }

bool CCSyntax::DeclStmt::parse (CCSyntax &s) {
  // 1: block_decl
  return BlockDecl::check (s);
}
bool CCSyntax::decl_stmt () { return DeclStmt::parse (*this); }

/*****************************************************************************/
/*                                                                           */
/*                     A . 6   D e c l a r a t i o n s                       */
/*                                                                           */
/*****************************************************************************/

bool CCSyntax::Decl::parse (CCSyntax &s) {
  // 1: block_decl
  // 1: fct_def
  // 1: explicit_specialization
  // 1: template_decl
  // 1: explicit_instantiation
  // 1: linkage_spec
  // 1: namespace_def
  bool parsed = (s.explicit_specialization () ||
                 s.template_decl () ||
                 s.explicit_instantiation () ||
                 NamespaceDef::check (s) ||
                 LinkageSpec::check (s) ||
                 (s.is_fct_def () ?
                   (s.fct_def () || BlockDecl::check (s)) :
                   (BlockDecl::check (s) || s.fct_def ())));
  return parsed;
}
bool CCSyntax::decl () { return Decl::parse (*this); }
bool CCSyntax::decl_check () { return Decl::check (*this); }

bool CCSyntax::BlockDecl::parse (CCSyntax &s) {
  // 1: simple_decl
  // 1: asm_def
  // 1: ns_alias_def
  // 1: using_decl
  // 1: using_directive
  return (SimpleDecl::check (s) ||
           AsmDef::check (s) ||
           NsAliasDef::check (s) ||
           UsingDirective::check (s) ||
           UsingDecl::check (s));
}
bool CCSyntax::block_decl () { return BlockDecl::parse (*this); }

bool CCSyntax::SimpleDecl::parse (CCSyntax &s) {
  // 1: ;
  // 2: decl_spec_seq  ;
  // 3: decl_spec_seq  init_declarator_list  ;
  return CSyntax::SimpleDecl::parse (s);
}
bool CCSyntax::simple_decl () { return SimpleDecl::parse (*this); }

bool CCSyntax::MiscSpec::parse (CCSyntax &s) {
  // 1: TYPEDEF
  // 1: FRIEND
  return (CSyntax::MiscSpec::parse (s) || s.parse (TOK_FRIEND));
}
bool CCSyntax::misc_spec () { return MiscSpec::parse (*this); }

bool CCSyntax::StorageClassSpec::parse (CCSyntax &s) {
  // 1: MUTABLE
  // 1: ...
  return (CSyntax::StorageClassSpec::parse (s) || s.parse (TOK_MUTABLE));
}
bool CCSyntax::storage_class_spec () { return StorageClassSpec::parse (*this); }

bool CCSyntax::FctSpec::parse (CCSyntax &s) {
  // 1: VIRTUAL
  // 1: EXPLICIT
  // 1: ...
  return (CSyntax::FctSpec::parse (s) ||
           s.parse (TOK_VIRTUAL) ||
           s.parse (TOK_EXPLICIT));
}
bool CCSyntax::fct_spec () { return FctSpec::parse (*this); }

bool CCSyntax::SimpleTypeSpec::parse (CCSyntax &s) {
  // 1: prim_types
  // 1: type_name
  // 2: nested_name_spec  type_name
  // 2: ::  type_name
  // 3: ::  nested_name_spec  type_name
  // 3: nested_name_spec  template_key  template_id
  // 4: ::  nested_name_spec  template_key  template_id
  return ((s.is_prim_type () && s.consume ()) ||
     (ColonColon::check (s),
      ((NestedNameSpec::check (s),
        (s.template_key () ? s.class_template_id () : TypeName::check (s))))));
}
bool CCSyntax::simple_type_spec () { return SimpleTypeSpec::parse (*this); }

bool CCSyntax::TypeName::parse (CCSyntax &s) {
  // 1: class_name
  // 1: enum_name
  // 1: typedef_name
  return (ClassName::check (s) ||
          EnumName::check (s) ||
          TypedefName::check (s));
}   
bool CCSyntax::type_name () { return TypeName::parse (*this); }

bool CCSyntax::ElaboratedTypeSpec::parse (CCSyntax &s) {
  // 2: class_key  identifier
  // 2: enum_key  identifier
  // 2: TYPENAME  template_id
  // 3: class_key  ::  identifier
  // 3: enum_key  ::  identifier
  // 3: class_key  nested_name_spec  identifier
  // 3: enum_key  nested_name_spec  identifier
  // 3: TYPENAME  nested_name_spec  identifier
  // 3: TYPENAME  nested_name_spec  template_id
  // 4: class_key  ::  nested_name_spec  identifier
  // 4: enum_key  ::  nested_name_spec  identifier
  // 4: TYPENAME  nested_name_spec  template_key  template_id
  // 4: TYPENAME  ::  nested_name_spec  identifier
  // 4: TYPENAME  ::  nested_name_spec  template_id
  // 5: TYPENAME  ::  nested_name_spec  template_key  template_id
  return
    ((ClassKey::check (s) || EnumKey::check (s)) ?
      (ColonColon::check (s), NestedNameSpec::check (s),
       s.template_id () || Identifier::check (s)) :
     (s.parse (TOK_TYPENAME) &&
      (ColonColon::check (s), NestedNameSpec::check (s),
      (s.template_key () ? s.template_id () :
       s.template_id () || Identifier::check (s)))));
}
bool CCSyntax::elaborated_type_spec () { return ElaboratedTypeSpec::parse (*this); }

bool CCSyntax::DeclSpecSeq1::parse (CCSyntax &s) {
  // 1: decl_spec
  // if the current template id is part of a declarator id, then it is no
  // declaration specifier; declarator ids are not instantiated whereas 
  // declaration specifiers are instantiated
  return s.is_tpl_declarator_id () ? false : CSyntax::DeclSpecSeq1::parse (s);
}
bool CCSyntax::decl_spec_seq1 () { return DeclSpecSeq1::parse (*this); }

bool CCSyntax::EnumeratorList::parse (CCSyntax &s) {
  // 1 : enumerator_def
  // 3+: enumerator_def  ,  enumerator_def ..
  return s.opt (list<CCSyntax, EnumeratorDef> (s, TOK_COMMA, true));
}
bool CCSyntax::enumerator_list () { return EnumeratorList::parse (*this); }

bool CCSyntax::EnumeratorDef::parse (CCSyntax &s) {
  // 1: enumerator
  // 3: enumerator  =  const_expr
  return CSyntax::EnumeratorDef::parse (s); // the same as for C
}
bool CCSyntax::enumerator_def () { return EnumeratorDef::parse (*this); }

bool CCSyntax::LinkageSpec::parse (CCSyntax &s) {
  // 4+: (EXTERN  str_literal)+  {  }
  // 5+: (EXTERN  str_literal)+  {  decl_seq  }
  // 3+: (EXTERN  str_literal)+  decl
  bool parsed = false;
  // parse common prefix: sequence of more or more 'EXTERN "C"'
  while (s.parse (TOK_EXTERN) && (parsed = StrLiteral::check (s)));
  if (! parsed)
    return false;

  return
    ((s.parse (TOK_OPEN_CURLY) && s.opt (DeclSeq::check (s)) && s.parse (TOK_CLOSE_CURLY)) ||
    Decl::check (s));
}
bool CCSyntax::linkage_spec () { return LinkageSpec::parse (*this); }

/*****************************************************************************/
/*                                                                           */
/*                    A . 6 . 1   N a m e s p a c e s                        */
/*                                                                           */
/*****************************************************************************/

bool CCSyntax::NamespaceDef::parse (CCSyntax &s) {
  // 1: named_ns_def
  // 1: unnamed_ns_def
  return (NamedNsDef::check (s) || UnnamedNsDef::check (s));
}
bool CCSyntax::namespace_def () { return NamespaceDef::parse (*this); }

bool CCSyntax::NamedNsDef::parse (CCSyntax &s) {
  // 1: original_ns_def
  // 1: extension_ns_def
  return (ExtensionNsDef::check (s) || OriginalNsDef::check (s));
}
bool CCSyntax::named_ns_def () { return NamedNsDef::parse (*this); }

bool CCSyntax::OriginalNsDef::parse (CCSyntax &s) {
  // 4: original_ns_def1  {  namespace_body  }
  return (OriginalNsDef1::check (s) && s.parse (TOK_OPEN_CURLY) &&
      NamespaceBody::check (s) && s.parse (TOK_CLOSE_CURLY));
}
bool CCSyntax::original_ns_def () { return OriginalNsDef::parse (*this); }

bool CCSyntax::OriginalNsDef1::parse (CCSyntax &s) {
  // 2: NAMESPACE  identifier
  // 3: INLINE  NAMESPACE  identifier
  return ((s.parse (TOK_INLINE), s.parse (TOK_NAMESPACE)) && Identifier::check (s) &&
      s.look_ahead (TOK_OPEN_CURLY));
}
bool CCSyntax::original_ns_def1 () { return OriginalNsDef1::parse (*this); }

bool CCSyntax::ExtensionNsDef::parse (CCSyntax &s) {
  // 4: extension_ns_def1  {  namespace_body  }
  return (ExtensionNsDef1::check (s) && s.parse (TOK_OPEN_CURLY) &&
           NamespaceBody::check (s) && s.parse (TOK_CLOSE_CURLY));
}
bool CCSyntax::extension_ns_def () { return ExtensionNsDef::parse (*this); }

bool CCSyntax::ExtensionNsDef1::parse (CCSyntax &s) {
  // 2: NAMESPACE  original_ns_name
  // 3: INLINE  NAMESPACE  original_ns_name
  return ((s.parse (TOK_INLINE), s.parse (TOK_NAMESPACE)) && OriginalNsName::check (s) &&
      s.look_ahead (TOK_OPEN_CURLY));
}
bool CCSyntax::extension_ns_def1 () { return ExtensionNsDef1::parse (*this); }

bool CCSyntax::UnnamedNsDef::parse (CCSyntax &s) {
  // 4: unnamed_ns_def1  {  namespace_body  }
  return (UnnamedNsDef1::check (s) && s.parse (TOK_OPEN_CURLY) &&
      NamespaceBody::check (s) && s.parse (TOK_CLOSE_CURLY));
}
bool CCSyntax::unnamed_ns_def () { return UnnamedNsDef::parse (*this); }

bool CCSyntax::UnnamedNsDef1::parse (CCSyntax &s) {
  // 2: NAMESPACE  private_name
  // 3: INLINE  NAMESPACE  private_name
  return ((s.parse (TOK_INLINE), s.parse (TOK_NAMESPACE)) && PrivateName::check (s) &&
         s.look_ahead (TOK_OPEN_CURLY));
}
bool CCSyntax::unnamed_ns_def1 () { return UnnamedNsDef1::parse (*this); }

bool CCSyntax::NamespaceBody::parse (CCSyntax &s) {
  // 1: decl_seq
  return s.opt (DeclSeq::check (s));
}
bool CCSyntax::namespace_body () { return NamespaceBody::parse (*this); }

bool CCSyntax::NsAliasDef::parse (CCSyntax &s) {
  // 5: NAMESPACE  identifier  =  qual_ns_spec  ;
  return (s.parse (TOK_NAMESPACE) && Identifier::check (s) &&
          s.parse (TOK_ASSIGN) && QualNsSpec::check (s) &&
          s.parse (TOK_SEMI_COLON));
}
bool CCSyntax::ns_alias_def () { return NsAliasDef::parse (*this); }

bool CCSyntax::QualNsSpec::parse (CCSyntax &s) {
  // 1: namespace_name
  // 2: nested_name_spec  namespace_name
  // 2: colon_colon  namespace_name
  // 3: colon_colon  nested_name_spec  namespace_name
  return (ColonColon::check (s), NestedNameSpec::check (s), NamespaceName::check (s));
}
bool CCSyntax::qual_ns_spec () { return QualNsSpec::parse (*this); }

bool CCSyntax::UsingDecl::parse (CCSyntax &s) {
  // 4: USING  ::  unqual_id  ;
  // 4: USING  nested_name_spec  unqual_id  ;
  // 5: USING  ::  nested_name_spec  unqual_id  ;
  // 5: USING  TYPENAME  nested_name_spec  unqual_id  ;
  // 6: USING  TYPENAME  ::  nested_name_spec  unqual_id  ;
  if (! s.parse (TOK_USING))
    return false;
  bool is_type = s.parse (TOK_TYPENAME);
  bool is_root = ColonColon::check (s);
  return ((NestedNameSpec::check (s) || (! is_type && is_root)) &&
           UnqualId::check (s) && s.parse (TOK_SEMI_COLON));
}
bool CCSyntax::using_decl () { return UsingDecl::parse (*this); }

bool CCSyntax::UsingDirective::parse (CCSyntax &s) {
  // 4: USING  NAMESPACE  namespace_name  ;
  // 5: USING  NAMESPACE  ::  namespace_name  ;
  // 5: USING  NAMESPACE  nested_name_spec  namespace_name  ;
  // 6: USING  NAMESPACE  ::  nested_name_spec  namespace_name  ;
  return
    (s.parse (TOK_USING) && s.parse (TOK_NAMESPACE) &&
     (ColonColon::check (s), NestedNameSpec::check (s),
      (NamespaceName::check (s) && s.parse (TOK_SEMI_COLON))));
}
bool CCSyntax::using_directive () { return UsingDirective::parse (*this); }

/*****************************************************************************/
/*                                                                           */
/*                     A . 7   D e c l a r a t o r s                         */
/*                                                                           */
/*****************************************************************************/

bool CCSyntax::InitDeclarator::parse (CCSyntax &s) {
  return CSyntax::InitDeclarator::parse (s);
}
bool CCSyntax::init_declarator () { return InitDeclarator::parse (*this); }

bool CCSyntax::DirectDeclarator1::parse (CCSyntax &s) {
  // 3: [  array_delim  ]
  // 3: (  param_decl_clause  )
  // 4: (  param_decl_clause  )  cv_qual_seq
  // 4: (  param_decl_clause  )  exception_spec
  // 5: (  param_decl_clause  )  cv_qual_seq  exception_spec
  return (s.parse (TOK_OPEN_ROUND) ?  
           (ParamDeclClause::check (s) && 
            s.parse (TOK_CLOSE_ROUND) &&
            s.opt (CvQualSeq::check (s)) &&
            s.opt (s.exception_spec ())) : 
          (s.parse (TOK_OPEN_SQUARE) && 
           ArrayDelim::check (s) &&
           s.parse (TOK_CLOSE_SQUARE)));
}
bool CCSyntax::direct_declarator1 () { return DirectDeclarator1::parse (*this); }

bool CCSyntax::ArrayDelim::parse (CCSyntax &s) {
  // 0:
  // 1: const_expr
  return s.opt (ConstExpr::check (s));
}
bool CCSyntax::array_delim () { return ArrayDelim::parse (*this); }

bool CCSyntax::PtrOperator::parse (CCSyntax &s) {
  // 1: &
  // 1: *
  // 2: *  cv_qual_seq
  // 2: nested_name_spec  *
  // 3: nested_name_spec  *  cv_qual_seq
  // 3: ::  nested_name_spec  *
  // 4: ::  nested_name_spec  *  cv_qual_seq
  bool ok =              // Win __cdecl hack :(
    ((s.parse (TOK_AND) && s.opt (CvQualSeq::check (s))) ||
     ((ColonColon::check (s) ? 
       NestedNameSpec::check (s) : 
      s.opt (NestedNameSpec::check (s))) &&
       s.parse (TOK_MUL) && s.opt (CvQualSeq::check (s))));
  return ok;
//  semantic ().reset_search_scope ();
//  return ok ? builder ().ptr_operator () : (CTree*)0;
}
bool CCSyntax::ptr_operator () { return PtrOperator::parse (*this); }

bool CCSyntax::DeclaratorId::parse (CCSyntax &s) {
  // 1: id_expr
  // 1: type_name
  // 2: nested_name_spec  type_name
  // 2: ::  type_name
  // 3: ::  nested_name_spec  type_name
  return ((ColonColon::check (s), NestedNameSpec::check (s), TypeName::check (s)) || IdExpr::check (s));
}
bool CCSyntax::declarator_id () { return DeclaratorId::parse (*this); }

bool CCSyntax::DirectAbstDeclarator::parse (CCSyntax &s) {
  // 1+: direct_declarator1...
  // 3 : ( abst_declarator )
  // 3+: ( abst_declarator )  direct_declarator1...
  return ((DirectDeclarator1::check (s) ||
           (s.parse (TOK_OPEN_ROUND) && 
            AbstDeclarator::check (s) &&
            s.parse (TOK_CLOSE_ROUND))) &&  
          s.opt (seq<CCSyntax, DirectDeclarator1> (s)));
}
bool CCSyntax::direct_abst_declarator () { return DirectAbstDeclarator::parse (*this); }

bool CCSyntax::ParamDeclClause::parse (CCSyntax &s) {
  // 0:
  // 1: ...
  // 1: param_decl_list
  // 3: param_decl_list  ... 
  return s.opt ((s.param_decl_list (), s.parse (TOK_ELLIPSIS)));
}
bool CCSyntax::param_decl_clause () { return ParamDeclClause::parse (*this); }

CTree *CCSyntax::rule_param_decl () { 
  // 1: param_decl1 
  // 3: param_decl1  =  ass_expr
  if (! ParamDecl1::check (*this))
    return (CTree*)0;
  semantic ().finish_decl ();
  // parse param_init later due to potential use of names of members
  // that could not be resolved till the end of the class definition 
  if (semantic ().in_class_def () && look_ahead (TOK_ASSIGN)) { 
    CTree *pd = builder ().param_decl ();
    semantic ().parse_delayed (&CCSyntax::param_init, pd);
    skip_param_init ();
    return pd;
  // not in class definition, so parse as usual
  } else {
    param_init ();
    return builder ().param_decl ();
  }
}

bool CCSyntax::param_decl () { 
  return parse (&CCSyntax::rule_param_decl);
}

CTree *CCSyntax::rule_param_init () { 
  // 2: =  ass_expr
  return (parse (TOK_ASSIGN) && AssExpr::check (*this)) ?
    builder ().param_init () : (CTree*)0;
}

bool CCSyntax::param_init () {
  return parse (&CCSyntax::rule_param_init);
}

CTree *CCSyntax::rule_fct_def () { 
  // 2: declarator  fct_body
  // 2: declarator  fct_try_block
  // 3: declarator  ctor_init  fct_body
  // 3: decl_spec_seq  declarator  fct_body
  // 3: decl_spec_seq  declarator  fct_try_block
  // 4: decl_spec_seq  declarator  ctor_init  fct_body
  if (! ((DeclSpecSeq::check (*this) ||
          semantic ().empty_decl_spec_seq ()), 
         semantic ().reset_search_scope (true),
         Declarator::check (*this))) {
    semantic ().finish_declarator ();
    return (CTree*)0;
  }

  // introduce name and check scope; functions shall be 
  // defined only in namespace or class scope
  if (! semantic ().introduce_function ())
    return (CTree*)0;

  // skip function bodies on demand
  bool skip_fct_bodies = false;
  if ((_skip_bodies & SKIP_BODIES_ALL) ||
      ((_skip_bodies & SKIP_BODIES_TPL) && semantic ().skip_tpl_fct_bodies ()) ||
      ((_skip_bodies & SKIP_BODIES_NON_PRJ) && semantic ().non_project_loc ()) ||
      ((_skip_bodies & SKIP_BODIES_NON_PRIM) && semantic ().non_primary_loc ()))
    skip_fct_bodies = true;

  // parse fct_body and ctor_init later due to potential use of names 
  // of members that could not be resolved till the end of the class 
  // definition
  if (semantic ().in_class_def ()) { 
    CTree *fd = semantic ().finish_fct_def ();
    if (look_ahead (TOK_TRY)) {
      semantic ().parse_delayed (&CCSyntax::fct_try_block, fd);
      skip_fct_try_block ();
    } else {
      if (look_ahead (TOK_COLON)) {
        semantic ().parse_delayed (&CCSyntax::ctor_init, fd);
        skip_ctor_init ();
      } else if (look_ahead (TOK_ASSIGN)) {
        semantic ().parse_delayed (&CCSyntax::pure_spec, fd);
        skip_ctor_init ();
      }
      if (look_ahead (TOK_OPEN_CURLY)) {
        semantic ().parse_delayed (skip_fct_bodies ? &CCSyntax::skipped_fct_body : &CCSyntax::fct_body, fd);
        skip_fct_body ();
      } else {
        delete fd;
        fd = (CTree*)0;
      }
    }
    return fd;
  // not in class definition, so parse as usual
  } else
    return (fct_try_block () ||
            ((ctor_init () || pure_spec ()), 
             skip_fct_bodies ? skipped_fct_body() : fct_body ())) ? 
      semantic ().finish_fct_def () : (CTree*)0;
}

bool CCSyntax::fct_def () {
  return parse (&CCSyntax::rule_fct_def);
}

CTree *CCSyntax::rule_skipped_fct_body () {
  static int stop_tokens[] = { TOK_CLOSE_CURLY, 0 };

  if (!parse (TOK_OPEN_CURLY)) return (CTree*)0;
  semantic ().enter_local_scope (); // enter local (block) scope
  skip (stop_tokens, false);
  if (!parse (TOK_CLOSE_CURLY)) return (CTree*)0;
  return semantic ().cmpd_stmt ();
}

bool CCSyntax::skipped_fct_body () {
  return parse (&CCSyntax::rule_skipped_fct_body);
}

CTree *CCSyntax::rule_fct_body () { 
  // 1: cmpd_stmt
  return cmpd_stmt () ? 
    builder ().fct_body () : (CTree*)0;
}

bool CCSyntax::fct_body () {
  return parse (&CCSyntax::rule_fct_body);
}

CTree *CCSyntax::rule_init () { 
  // 2: =  init_clause 
  // 3: (  expr_list  )
  return (parse (TOK_ASSIGN) ? init_clause () : 
          (parse (TOK_OPEN_ROUND) && expr_list () &&
           parse (TOK_CLOSE_ROUND))) ?
    builder ().init () : (CTree*)0;
}

bool CCSyntax::init () {
  return parse (&CCSyntax::rule_init);
}

CTree *CCSyntax::rule_init_clause () { 
  // 1: ass_expr
  // 2: {  }
  // 3: {  init_list  }
  return (AssExpr::check (*this) ||
          (parse (TOK_OPEN_CURLY) && 
           (init_list (), parse (TOK_CLOSE_CURLY)))) ?
    builder ().init_clause () : (CTree*)0;
}

bool CCSyntax::init_clause () {
  return parse (&CCSyntax::rule_init_clause);
}

/*****************************************************************************/
/*                                                                           */
/*                          A . 8   C l a s s e s                            */
/*                                                                           */
/*****************************************************************************/

CTree *CCSyntax::rule_class_spec () { 
  // 3: class_head  {  }
  // 4: class_head  {  member_spec  }
  if (! (is_class_def () && ClassHead::check (*this)))
    return (CTree*)0;
  semantic ().reenter_class_scope ();
  return (parse (TOK_OPEN_CURLY) &&
          (member_spec (), parse (TOK_CLOSE_CURLY))) ? 
    semantic ().class_spec () : semantic ().class_spec_err ();
}

bool CCSyntax::class_spec () {
  return predict_1 (_class_spec_1) && parse (&CCSyntax::rule_class_spec);
}

bool CCSyntax::ClassHead::parse (CCSyntax &s) {
  // 1: class_head1
  // 2: class_head1 base_clause
  return (ClassHead1::check (s) && s.opt (s.base_clause ()));
}
bool CCSyntax::class_head () { return ClassHead::parse (*this); }

bool CCSyntax::ClassHead1::parse (CCSyntax &s) {
  // 1: class_key
  // 2: class_key  identifier
  // 2: class_key  template_id
  // 3: class_key  nested_name_spec  identifier
  // 3: class_key  nested_name_spec  template_id
  s.semantic ().enter_entity_name ();
  bool parsed = 
    (ClassKey::check (s) &&
     (NestedNameSpec::check (s) ? 
      (s.template_id () || Identifier::check (s)) :
      (s.template_id () || Identifier::check (s) || PrivateName::check (s))));
  s.semantic ().reset_search_scope ();
  s.semantic ().leave_entity_name ();
  return (parsed && (s.look_ahead (TOK_OPEN_CURLY) || s.look_ahead (TOK_COLON)));
}
bool CCSyntax::class_head1 () { return ClassHead1::parse (*this); }

CTree *CCSyntax::rule_member_decl () { 
  // 1: member_decl1
  // 1: fct_def
  // 1: using_decl
  // 1: member_template_decl
  // 1: access_decl
  // 2: access_spec  :
  if (access_spec ()) {
    if (parse (TOK_COLON)) 
      return semantic ().access_spec ();
    else 
      return (CTree*)0;
  }
  if (! (UsingDecl::check (*this) ||
         access_decl () ||
         member_template_decl () ||
         (is_fct_def () ? fct_def () :
          member_decl1 () /*||
          parse (&CCSyntax::fct_def)*/))) 
    return (CTree*)0;
  semantic ().finish_decl ();
  return builder ().member_decl (); 
}

bool CCSyntax::member_decl () {
  return parse (&CCSyntax::rule_member_decl);
}

CTree *CCSyntax::rule_member_decl1 () { 
  // 1: ;
  // 2: decl_spec_seq  ;
  // 2: member_declarator_list  ;
  // 3: decl_spec_seq  member_declarator_list  ;
  return ((DeclSpecSeq::check (*this) ||
           semantic ().empty_decl_spec_seq ()), 
          member_declarator_list (), 
          parse (TOK_SEMI_COLON)) ?
    semantic ().member_decl1 () : (CTree*)0;
}

bool CCSyntax::member_decl1 () {
  return parse (&CCSyntax::rule_member_decl1);
}

CTree *CCSyntax::rule_access_decl () { 
  // 3: nested_name_spec  unqual_id  ;
  // 4: nested_name_spec  template_key  unqual_id  ;
  // 4: ::  nested_name_spec  unqual_id  ;
  // 5: ::  nested_name_spec  template_key  unqual_id  ;
  bool ok =  
    ((ColonColon::check (*this), NestedNameSpec::check (*this)) && 
     (template_key (), 
      unqual_id ()) && 
     parse (TOK_SEMI_COLON));
  semantic ().reset_search_scope ();
  return ok ? semantic ().access_decl () : (CTree*)0;
}

bool CCSyntax::access_decl () {
  return parse (&CCSyntax::rule_access_decl);
}

CTree *CCSyntax::rule_member_declarator () { 
  // 1: declarator
  // 2: declarator  pure_spec
  // 2: declarator  const_init
  // 3: identifier  :  const_expr
  // 3: private_name  :  const_expr
  CT_InitDeclarator *md;

  // lookahead: check if this is a bitfield declarator
  State s = save_state ();
  bool bitfield = (Identifier::check (*this), parse (TOK_COLON));
  restore_state (s);

  // parse const_expr and const_init later due to potential use of names 
  // of members that could not be resolved till the end of the class 
  // definition
  if (semantic ().in_class_def () && ! bitfield) { 
    if (Declarator::check (*this)) {
      // optional extensions
      // InitDeclaratorExt::check (*this);
      // constant initializer
      if (! pure_spec () && look_ahead (TOK_ASSIGN)) {
        md = (CT_InitDeclarator*)semantic ().introduce_member ();
        // initializers of const members are evaluated immediately
        if (md && md->Object () && md->Object ()->TypeInfo ()->isConst ()) {
          if (const_init ())
            md->Initializer (builder ().get_node (builder ().nodes ()-1));
        // initializers of non-const members are evaluated later
        } else if (md) {
          semantic ().parse_delayed (&CCSyntax::const_init, md);
          skip_const_init ();
        }
      // pure specifier; always is "0"
      } else
        md = (CT_InitDeclarator*)semantic ().introduce_member ();
      return md;
    }
  // not in class definition scope, so parse as usual
  } else {
    if (bitfield) {
      if ((Identifier::check (*this) ||
           PrivateName::check (*this)) && parse (TOK_COLON) && 
          ConstExpr::check (*this))
        return semantic ().introduce_member ();
    } else {
      if (Declarator::check (*this) && /*opt (InitDeclaratorExt::check (*this)) &&*/
          opt (pure_spec () || const_init ()))
        return semantic ().introduce_member ();
    }
  }
  return semantic ().member_declarator_err ();
}

bool CCSyntax::member_declarator () {
  return parse (&CCSyntax::rule_member_declarator);
}

CTree *CCSyntax::rule_pure_spec () { 
  // 2: =  0
  return (parse (TOK_ASSIGN) && parse (TOK_ZERO_VAL)) ?
    semantic ().pure_spec () : (CTree*)0;
}

bool CCSyntax::pure_spec () {
  return parse (&CCSyntax::rule_pure_spec);
}

CTree *CCSyntax::rule_const_init () { 
  // 2: =  const_expr
  return (parse (TOK_ASSIGN) && ConstExpr::check (*this)) ?
    semantic ().const_init () : (CTree*)0;
}

bool CCSyntax::const_init () {
  return parse (&CCSyntax::rule_const_init);
}

/*****************************************************************************/
/*                                                                           */
/*                   A . 9   D e r i v e d  c l a s s e s                    */
/*                                                                           */
/*****************************************************************************/

CTree *CCSyntax::rule_base_clause () { 
  // 2: :  base_spec_list
  return (parse (TOK_COLON) && base_spec_list ()) ?
    builder ().base_clause () : (CTree*)0;
}

bool CCSyntax::base_clause () {
  return parse (&CCSyntax::rule_base_clause);
}

CTree *CCSyntax::rule_base_spec_list () { 
  // 1 : base_spec
  // 3+: base_spec  ,  base_spec ..
  return list (&CCSyntax::base_spec, TOK_COMMA) ?
    builder ().base_spec_list () : (CTree*)0;
}

bool CCSyntax::base_spec_list () {
  return parse (&CCSyntax::rule_base_spec_list);
}

CTree *CCSyntax::rule_base_spec () { 
  // 1: class_name
  // 2: ::  class_name
  // 2: VIRTUAL  class_name
  // 2: access_spec  class_name
  // 2: nested_name_spec  class_name
  // 3: nested_name_spec  template_key  class_name
  // 3: ::  nested_name_spec  class_name
  // 3: VIRTUAL  ::  class_name
  // 3: access_spec  ::  class_name
  // 3: VIRTUAL  nested_name_spec  class_name
  // 3: access_spec  nested_name_spec  class_name
  // 3: VIRTUAL  access_spec  class_name
  // 3: access_spec  VIRTUAL  class_name
  // 4: ::  nested_name_spec  template_key  class_name
  // 4: VIRTUAL  nested_name_spec  template_key  class_name
  // 4: access_spec  nested_name_spec  template_key  class_name
  // 4: VIRTUAL  ::  nested_name_spec  class_name
  // 4: access_spec  ::  nested_name_spec  class_name
  // 4: VIRTUAL  access_spec  nested_name_spec  class_name
  // 4: access_spec  VIRTUAL  nested_name_spec  class_name
  // 4: VIRTUAL  access_spec  ::  class_name
  // 4: access_spec  VIRTUAL  ::  class_name
  // 5: VIRTUAL  ::  nested_name_spec  template_key  class_name
  // 5: access_spec  ::  nested_name_spec  template_key  class_name
  // 5: VIRTUAL  access_spec  nested_name_spec  template_key  class_name
  // 5: access_spec  VIRTUAL  nested_name_spec  template_key  class_name
  // 5: VIRTUAL  access_spec  ::  nested_name_spec  class_name
  // 5: access_spec  VIRTUAL  ::  nested_name_spec  class_name
  // 6: VIRTUAL  access_spec  ::  nested_name_spec  template_key  class_name
  // 6: access_spec  VIRTUAL  ::  nested_name_spec  template_key  class_name
  bool ok = false;
  if (parse (TOK_VIRTUAL) ? opt (access_spec ()) :
      (access_spec () ? opt (parse (TOK_VIRTUAL)) : true)) {
    ColonColon::check (*this);
    NestedNameSpec::check (*this);
    template_key ();
    semantic ().enter_base_spec ();
    ok = class_name ();
    semantic ().leave_base_spec ();
  }
  semantic ().reset_search_scope ();
  return ok ? builder ().base_spec () : (CTree*)0;
}

bool CCSyntax::base_spec () {
  return parse (&CCSyntax::rule_base_spec);
}

void CCSyntax::init_access_spec () {
  _access_spec_1.set (TOK_PRIVATE);
  _access_spec_1.set (TOK_PROTECTED);
  _access_spec_1.set (TOK_PUBLIC);
}

CTree *CCSyntax::rule_access_spec () { 
  // 1: PUBLIC
  // 1: PRIVATE
  // 1: PROTECTED
  consume (); // already checked by look ahead
  return builder ().access_spec ();
}

bool CCSyntax::access_spec () {
  return predict_1 (_access_spec_1) && parse (&CCSyntax::rule_access_spec);
}

/*****************************************************************************/
/*                                                                           */
/*        A . 1 0   S p e c i a l  m e m b e r  f u n c t i o n s            */
/*                                                                           */
/*****************************************************************************/

bool CCSyntax::ConvFctId::parse (CCSyntax &s) {
  // 2: OPERATOR  conv_type_id
  return s.consume () &&  // TOK_OPERATOR checked during prediction
         ConvTypeId::check (s);
}
bool CCSyntax::conv_fct_id () { return ConvFctId::parse (*this); }

bool CCSyntax::ConvTypeId::parse (CCSyntax &s) {
  // 2: type_spec_seq  conv_declarator
  // 2: type_spec_seq  private_name
  return s.type_spec_seq () &&
         (s.conv_declarator () || PrivateName::check (s));
}
bool CCSyntax::conv_type_id () { return ConvTypeId::parse (*this); }

CTree *CCSyntax::rule_conv_declarator () { 
  // 1+: ptr_operator..  
  return seq (&CCSyntax::ptr_operator) ?
    semantic ().abst_declarator () : (CTree*)0;
}

bool CCSyntax::conv_declarator () {
  return parse (&CCSyntax::rule_conv_declarator);
}

CTree *CCSyntax::rule_ctor_init () { 
  // 2: :  mem_init_list
  return (parse (TOK_COLON) && mem_init_list ()) ?
    semantic ().ctor_init () : (CTree*)0;
}

bool CCSyntax::ctor_init () {
  return parse (&CCSyntax::rule_ctor_init);
}

CTree *CCSyntax::rule_mem_init_list () { 
  // 1 : mem_init
  // 3+: mem_init  ,  mem_init ..
  return list (&CCSyntax::mem_init, TOK_COMMA) ?
    builder ().mem_init_list () : (CTree*)0;
}

bool CCSyntax::mem_init_list () {
  return parse (&CCSyntax::rule_mem_init_list);
}

CTree *CCSyntax::rule_mem_init () { 
  // 3: mem_init_id  (  )
  // 4: mem_init_id  (  expr_list  )
  return (mem_init_id () && parse (TOK_OPEN_ROUND) && 
          (expr_list (), parse (TOK_CLOSE_ROUND))) ?
    builder ().mem_init () : (CTree*)0;
}

bool CCSyntax::mem_init () {
  return parse (&CCSyntax::rule_mem_init);
}

CTree *CCSyntax::rule_mem_init_id () { 
  // 1: identifier
  // 1: class_name
  // 2: nested_name_spec  class_name
  // 2: ::  class_name
  // 3: ::  nested_name_spec  class_name
  bool ok = 
    ((ColonColon::check (*this), NestedNameSpec::check (*this), 
      class_name ()) ||
     Identifier::check (*this));
  semantic ().reset_search_scope ();
  return ok ? builder ().mem_init_id () : (CTree*)0;
}

bool CCSyntax::mem_init_id () {
  return parse (&CCSyntax::rule_mem_init_id);
}

/*****************************************************************************/
/*                                                                           */
/*                     A . 1 1   O v e r l o a d i n g                       */
/*                                                                           */
/*****************************************************************************/

void CCSyntax::init_oper_fct_id () { 
  _oper_fct_id_1.set (TOK_OPERATOR);
}

CTree *CCSyntax::rule_oper_fct_id () { 
  // 2: OPERATOR  any_operator
  // 2: OPERATOR  NEW
  // 2: OPERATOR  DELETE
  // 3: OPERATOR  (  )
  // 3: OPERATOR  [  ]
  // 4: OPERATOR  NEW  [  ]
  // 4: OPERATOR  DELETE  [  ]
  static int any_operator[] = { 
    TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV, TOK_MODULO, TOK_ROOF, 
    TOK_AND, TOK_OR, TOK_TILDE, TOK_NOT, TOK_LESS, TOK_GREATER, 
    TOK_EQL, TOK_GEQ, TOK_LEQ, TOK_NEQ, TOK_AND_AND, TOK_OR_OR, 
    TOK_LSH, TOK_RSH, TOK_DECR, TOK_INCR, TOK_ASSIGN, TOK_COMMA, 
    TOK_ADD_EQ, TOK_SUB_EQ, TOK_MUL_EQ, TOK_DIV_EQ, TOK_MOD_EQ, 
    TOK_AND_EQ, TOK_IOR_EQ, TOK_LSH_EQ, TOK_RSH_EQ, TOK_XOR_EQ,
    TOK_PTS_STAR, TOK_PTS, 0 };
  return (parse (TOK_OPERATOR) && 
          ((parse (TOK_NEW) || parse (TOK_DELETE)) ? 
            (parse (TOK_OPEN_SQUARE) ? parse (TOK_CLOSE_SQUARE) : true) :
           parse (TOK_OPEN_ROUND) ? parse (TOK_CLOSE_ROUND) : 
           parse (TOK_OPEN_SQUARE) ? parse (TOK_CLOSE_SQUARE) : 
           parse (any_operator))) ?
    builder ().oper_fct_id () : (CTree*)0;
}

bool CCSyntax::oper_fct_id () {
  return predict_1 (_oper_fct_id_1) && parse (&CCSyntax::rule_oper_fct_id);
}

/*****************************************************************************/
/*                                                                           */
/*                       A . 1 2   T e m p l a t e s                         */
/*                                                                           */
/*****************************************************************************/

void CCSyntax::init_template_key () {
  _template_key_1.set (TOK_TEMPLATE);
}

CTree *CCSyntax::rule_template_key () {
  // 1: TEMPLATE
  consume ();
  return semantic ().template_key ();
}

bool CCSyntax::template_key () {
  return look_ahead () == TOK_TEMPLATE && parse (&CCSyntax::rule_template_key);
}


CTree *CCSyntax::rule_template_decl () { 
  // 5: TEMPLATE  <  template_param_list  >  decl
  // 6: EXPORT  TEMPLATE  <  template_param_list  >  decl
  if (! ((parse (TOK_EXPORT), parse (TOK_TEMPLATE)) && parse (TOK_LESS) && 
          template_param_list () && parse (TOK_GREATER)))
    return (CTree*)0;
  semantic ().enter_template_decl ();
  decl ();
  return semantic ().template_decl ();
}

bool CCSyntax::template_decl () {
  return parse (&CCSyntax::rule_template_decl);
}

CTree *CCSyntax::rule_member_template_decl () { 
  // 5: TEMPLATE  <  template_param_list  >  member_decl
  // 6: EXPORT  TEMPLATE  <  template_param_list  >  member_decl
  if (! ((parse (TOK_EXPORT), parse (TOK_TEMPLATE)) && parse (TOK_LESS) && 
          template_param_list () && parse (TOK_GREATER)))
    return (CTree*)0;
  semantic ().enter_template_decl ();
  member_decl ();
  return semantic ().template_decl ();
}

bool CCSyntax::member_template_decl () {
  return parse (&CCSyntax::rule_member_template_decl);
}

CTree *CCSyntax::rule_template_param_list () { 
  // 1 : template_param
  // 3+: template_param  ,  template_param ..
  semantic ().enter_template_param_list (token_provider->get_state ());
  bool success = list (&CCSyntax::template_param, TOK_COMMA);
  semantic ().leave_template_param_list ();
  return success ?
    semantic ().template_param_list () : (CTree*)0;
}

bool CCSyntax::template_param_list () {
  return parse (&CCSyntax::rule_template_param_list);
}

CTree *CCSyntax::rule_template_param () { 
  // 1: type_param
  // 1: non_type_param
  return (type_param () || non_type_param ()) ?
    semantic ().template_param () : (CTree*)0;
}

bool CCSyntax::template_param () {
  return parse (&CCSyntax::rule_template_param);
}

CTree *CCSyntax::rule_type_param () { 
  // 1: CLASS
  // 1: TYPENAME
  // 2: CLASS  identifier
  // 2: TYPENAME  identifier
  // 3: CLASS  =  type_id
  // 3: TYPENAME  =  type_id
  // 4: CLASS  identifier  =  type_id
  // 4: TYPENAME  identifier  =  type_id
  // 5: TEMPLATE  <  template_param_list  >  CLASS 
  // 6: TEMPLATE  <  template_param_list  >  CLASS  identifier
  // 7: TEMPLATE  <  template_param_list  >  CLASS  =  id_expr
  // 8: TEMPLATE  <  template_param_list  >  CLASS  identifier  =  id_expr
  return (((parse (TOK_CLASS) || parse (TOK_TYPENAME)) ?
            (Identifier::check (*this) || PrivateName::check (*this), 
             parse (TOK_ASSIGN) ? type_id () : true) :
           parse (TOK_TEMPLATE) && 
            parse (TOK_LESS) && template_param_list () && 
            parse (TOK_GREATER) && parse (TOK_CLASS) && 
            (Identifier::check (*this) || PrivateName::check (*this), 
             parse (TOK_ASSIGN) ? 
              (semantic ().enter_expr (), 
               (id_expr () ? semantic ().leave_expr () :
                (semantic ().leave_expr (), false))) : true)) &&
          (look_ahead (TOK_COMMA) || look_ahead (TOK_GREATER))) ?
    semantic ().introduce_type_param () : (CTree*)0;
}

bool CCSyntax::type_param () {
  return parse (&CCSyntax::rule_type_param);
}

CTree *CCSyntax::rule_non_type_param () { 
  // 1: decl_spec_seq 
  // 2: decl_spec_seq  abst_declarator
  // 2: decl_spec_seq  declarator
  // 2: decl_spec_seq  param_init
  // 3: decl_spec_seq  abst_declarator  param_init
  // 3: decl_spec_seq  declarator  param_init
  if (! (DeclSpecSeq::check (*this) &&
         (Declarator::check (*this) ||
          abst_declarator () ||
          PrivateName::check (*this)))) {
    semantic ().finish_declarator ();
    return (CTree*)0;
  }
  CTree *result = semantic ().introduce_non_type_param ();
  if (! result)
    return (CTree*)0;
  param_init ();
  return builder ().non_type_param (result);
}

bool CCSyntax::non_type_param () {
  return  parse (&CCSyntax::rule_non_type_param);
}

void CCSyntax::init_template_id () {
  init_oper_fct_id ();
  init_conv_fct_id ();
  _template_id_1.set (TOK_ID);
  _template_id_1 |= _oper_fct_id_1;
  _template_id_1 |= _conv_fct_id_1;
}

CTree *CCSyntax::rule_template_id () { 
  // 3: template_name  <  >
  // 4: template_name  <  template_arg_list  >
  if (! ((template_name () ||
          oper_fct_id () ||
          ConvFctId::check (*this)) && parse (TOK_LESS)))
    return (CTree*)0;
  SearchScope sc;
  get_search_scope (sc);
  semantic ().reset_search_scope ();
  semantic ().enter_entity_name (false);
  template_arg_list ();
  semantic ().leave_entity_name ();
  set_search_scope (sc);
  return parse (TOK_GREATER) ?
    semantic ().introduce_template_instance () : (CTree*)0;
}

bool CCSyntax::template_id () {
  return predict_1 (_template_id_1) && parse (&CCSyntax::rule_template_id);
}

void CCSyntax::init_class_template_id () {
  _template_id_1.set (TOK_ID);
}

CTree *CCSyntax::rule_class_template_id () { 
  // 3: template_name  <  >
  // 4: template_name  <  template_arg_list  >
  if (! (class_template_name () && parse (TOK_LESS)))
    return (CTree*)0;
  SearchScope sc;
  get_search_scope (sc);
  semantic ().reset_search_scope ();
  semantic ().enter_entity_name (false);
  template_arg_list ();
  semantic ().leave_entity_name ();
  set_search_scope (sc);
  return parse (TOK_GREATER) ?
    semantic ().introduce_template_instance () : (CTree*)0;
}

bool CCSyntax::class_template_id () {
  return predict_1 (_template_id_1) && parse (&CCSyntax::rule_class_template_id);
}

CTree *CCSyntax::rule_template_arg_list () { 
  // 1 : template_arg
  // 3+: template_arg  ,  template_arg ..
  semantic ().enter_template_arg_list (token_provider->get_state ());
  bool success = list (&CCSyntax::template_arg, TOK_COMMA);
  semantic ().leave_template_arg_list ();
  return success ?
    builder ().template_arg_list () : (CTree*)0;
}

bool CCSyntax::template_arg_list () {
  return parse (&CCSyntax::rule_template_arg_list);
}

CTree *CCSyntax::rule_template_arg () { 
  // 1: template_type_arg
  // 1: template_non_type_arg
  // 1: template_template_arg
  return (template_type_arg () ||
          template_non_type_arg () ||
          template_template_arg ()) ?
    builder ().template_arg () : (CTree*)0;
}

bool CCSyntax::template_arg () {
  return parse (&CCSyntax::rule_template_arg);
}

CTree *CCSyntax::rule_template_type_arg () { 
  // 1: type_id
  static int argsep[] = { TOK_COMMA, TOK_GREATER, 0 };
  return (type_id () && look_ahead (argsep)) ?
    builder ().template_type_arg () : (CTree*)0;
}

bool CCSyntax::template_type_arg () {
  return parse (&CCSyntax::rule_template_type_arg);
}

CTree *CCSyntax::rule_template_non_type_arg () { 
  // 1: ass_expr
  static int argsep[] = { TOK_COMMA, TOK_GREATER, 0 };
  return (AssExpr::check (*this) && look_ahead (argsep)) ?
    builder ().template_non_type_arg () : (CTree*)0;
}

bool CCSyntax::template_non_type_arg () {
  return parse (&CCSyntax::rule_template_non_type_arg);
}

CTree *CCSyntax::rule_template_template_arg () { 
  // 1: id_expr
  static int argsep[] = { TOK_COMMA, TOK_GREATER, 0 };
  semantic ().enter_expr ();
  bool parsed = id_expr () && look_ahead (argsep);
  semantic ().leave_expr ();
  return parsed ? builder ().template_template_arg () : (CTree*)0;
}

bool CCSyntax::template_template_arg () {
  return parse (&CCSyntax::rule_template_template_arg);
}

void CCSyntax::init_explicit_instantiation () { 
  _explicit_instantiation_1.set (TOK_TEMPLATE);
  _explicit_instantiation_1.set (TOK_EXTERN);
}

CTree *CCSyntax::rule_explicit_instantiation () { 
  // 2: TEMPLATE  decl
  // 3: EXTERN  TEMPLATE  decl  => GCC EXTENSION!!!
  if (! (parse (TOK_EXTERN), parse (TOK_TEMPLATE)))
    return (CTree*)0;
  semantic ().enter_template_param_list (0);
  return decl () ? semantic ().explicit_instantiation () : (CTree*)0;
}

bool CCSyntax::explicit_instantiation () {
  return predict_1 (_explicit_instantiation_1) &&
         parse (&CCSyntax::rule_explicit_instantiation);
}

void CCSyntax::init_explicit_specialization () {
  _explicit_specialization_1.set (TOK_TEMPLATE);
}

CTree *CCSyntax::rule_explicit_specialization () { 
  // 4: TEMPLATE  <  >  decl
  if (! (consume () && parse (TOK_LESS) && parse (TOK_GREATER)))
    return (CTree*)0;
  semantic ().enter_template_param_list (0);
  return decl () ?
    semantic ().explicit_specialization () : (CTree*)0;
}

bool CCSyntax::explicit_specialization () {
  return predict_1 (_explicit_specialization_1) &&
         parse (&CCSyntax::rule_explicit_specialization);
}

/*****************************************************************************/
/*                                                                           */
/*                A . 1 3  E x c e p t i o n  h a n d l i n g                */
/*                                                                           */
/*****************************************************************************/

CTree *CCSyntax::rule_try_block () { 
  // 3+: TRY  cmpd_stmt  handler_seq
  return (parse (TOK_TRY) && cmpd_stmt () && 
          handler_seq ()) ?
    builder ().try_block () : (CTree*)0;
}

bool CCSyntax::try_block () {
  return parse (&CCSyntax::rule_try_block);
}

CTree *CCSyntax::rule_fct_try_block () { 
  // 3+: TRY  fct_body  handler_seq
  // 4+: TRY  ctor_init  fct_body  handler_seq
  bool skip_fct_bodies = false;
  if ((_skip_bodies & SKIP_BODIES_ALL) ||
      ((_skip_bodies & SKIP_BODIES_TPL) && semantic ().skip_tpl_fct_bodies ()) ||
      ((_skip_bodies & SKIP_BODIES_NON_PRJ) && semantic ().non_project_loc ()) ||
      ((_skip_bodies & SKIP_BODIES_NON_PRIM) && semantic ().non_primary_loc ()))
    skip_fct_bodies = true;
  return (parse (TOK_TRY) && opt (ctor_init ()) && 
          (skip_fct_bodies ? skipped_fct_body() : fct_body()) && 
          handler_seq ()) ?
    builder ().fct_try_block () : (CTree*)0;
}

bool CCSyntax::fct_try_block () {
  return parse (&CCSyntax::rule_fct_try_block);
}

CTree *CCSyntax::rule_handler_seq () { 
  // 1+: handler..
  return seq (&CCSyntax::handler) ?
    builder ().handler_seq () : (CTree*)0;
}

bool CCSyntax::handler_seq () {
  return parse (&CCSyntax::rule_handler_seq);
}

CTree *CCSyntax::rule_handler () { 
  // 5: CATCH  (  exception_decl  )  cmpd_stmt
  if (! parse (TOK_CATCH))
    return (CTree*)0;
  semantic ().enter_local_scope (); // enter local (block) scope
  return (parse (TOK_OPEN_ROUND) && exception_decl () && 
          parse (TOK_CLOSE_ROUND) && cmpd_stmt ()) ?
    semantic ().handler () : (CTree*)0;
}

bool CCSyntax::handler () {
  return parse (&CCSyntax::rule_handler);
}

CTree *CCSyntax::rule_exception_decl () { 
  // 1: ...
  // 1: type_spec_seq 
  // 2: type_spec_seq  abst_declarator
  // 2: type_spec_seq  declarator
  if (parse (TOK_ELLIPSIS))
    return builder ().exception_decl ();
  bool parsed = (type_spec_seq () && 
                 (Declarator::check (*this) ||
                  abst_declarator () ||
                  PrivateName::check (*this)));
  semantic ().finish_declarator ();
  return parsed ?
    semantic ().introduce_exception () : (CTree*)0;
}

bool CCSyntax::exception_decl () {
  return parse (&CCSyntax::rule_exception_decl);
}

CTree *CCSyntax::rule_throw_expr () { 
  // 1: THROW
  // 2: THROW  ass_expr
  return (parse (TOK_THROW) && opt (AssExpr::check (*this))) ?
    builder ().throw_expr () : (CTree*)0;
}

bool CCSyntax::throw_expr () {
  return parse (&CCSyntax::rule_throw_expr);
}

CTree *CCSyntax::rule_exception_spec () { 
  // 3: THROW  (  )
  // 4: THROW  (  ...  )              // MS VC++ extension :(
  // 4: THROW  (  type_id_list  )
  if (! (parse (TOK_THROW) && parse (TOK_OPEN_ROUND)))
    return (CTree*)0;
  semantic ().enter_exception_spec ();
  if ((parse (TOK_ELLIPSIS) || type_id_list (), 
       parse (TOK_CLOSE_ROUND)))
    return semantic ().exception_spec ();
  semantic ().leave_exception_spec ();
  return (CTree*)0;
}

bool CCSyntax::exception_spec () {
  return  parse (&CCSyntax::rule_exception_spec);
}

CTree *CCSyntax::rule_type_id_list () { 
  // 1 : type_id
  // 3+: type_id  ,  type_id ..
  return list (&CCSyntax::type_id, TOK_COMMA) ? 
    builder ().type_id_list () : (CTree*)0;
}

bool CCSyntax::type_id_list () {
  return parse (&CCSyntax::rule_type_id_list);
}

} // namespace Puma
