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

#include "Puma/CBuilder.h"
#include "Puma/CSyntax.h"
#include "Puma/CTokens.h"

namespace Puma {


/*****************************************************************************/
/*                                                                           */
/*                           A . 1   K e y w o r d s                         */
/*                                                                           */
/*****************************************************************************/

CTree *CBuilder::simple_name () {
  return new CT_SimpleName (get_node ());
}
   
/*****************************************************************************/
/*                                                                           */
/*               A . 2   L e x i c a l  c o n v e n t i o n s                */
/*                                                                           */
/*****************************************************************************/

CTree *CBuilder::literal () {
  if (get_node (0)->NodeName () == CT_String::NodeId () ||
      get_node (0)->NodeName () == CT_WideString::NodeId ())
    return get_node ();
  
  CTree *result;
  const char *txt = get_node (0)->token ()->text ();
  switch (get_node (0)->token ()->type ()) {
    case TOK_INT_VAL:
    case TOK_ZERO_VAL: 
      result = new CT_Integer ((CT_Token*)get_node (0)); break;
    case TOK_CHAR_VAL: 
      result = (txt[0] == 'L') ? 
               (CTree*)new CT_WideCharacter ((CT_Token*)get_node (0)) :
	       (CTree*)new CT_Character ((CT_Token*)get_node (0)); 
      break;
    case TOK_FLT_VAL: 
      result = new CT_Float ((CT_Token*)get_node (0)); break;
    case TOK_BOOL_VAL: 
      result = new CT_Bool ((CT_Token*)get_node (0)); break;
    default: 
      result = (CTree*)0;
  };
  return result;
}

CTree *CBuilder::cmpd_str () {
  bool wide = false;
  for (int i = 0; (i < nodes ()) && ! wide; i++) 
    if (*(get_node (i)->token ()->text ()) == 'L')
      wide = true;
      
  return (wide) ? list (new CT_WideString (nodes ())) :
                  list (new CT_String (nodes ()));
}

CTree *CBuilder::str_literal () {
  return get_node ();
}

/*****************************************************************************/
/*                                                                           */
/*                   A . 3   B a s i c  c o n c e p t s                      */
/*                                                                           */
/*****************************************************************************/

CTree *CBuilder::trans_unit () {
  Container *declarations = (Container*)get_node ();
  CT_Program *result = new CT_Program (declarations->Sons ());
  copy_list (result, declarations);
  delete declarations;
  return result;
}

/*****************************************************************************/
/*                                                                           */
/*                      A . 4   E x p r e s s i o n s                        */
/*                                                                           */
/*****************************************************************************/

CTree *CBuilder::prim_expr () { 
  // 1: literal      => node
  // 1: id_expr      => node
  // 3: (  expr  )   => token  node  token
  if (nodes () == 3)
    return new CT_BracedExpr (get_node (0), get_node (1), get_node (2));
  else
    return get_node ();
}

CTree *CBuilder::cmpd_literal () { 
  // 5 : (  type_id  )  {  init_list  }
  // 6 : (  type_id  )  {  init_list  }
  CT_ExprList *el;
  int num = nodes ();
  if (num == 5) {
    el = new CT_ExprList ();
  } else {
    el = (CT_ExprList*)get_node (4);
    if ((el->Sons () % 2) == 0)
      el->AddProperties (CT_List::END_SEP);
  }
  el->PrefixSon (get_node (3));
  el->AddSon (get_node (num-1));
  el->AddProperties (CT_List::OPEN_CLOSE);
  return new CT_CmpdLiteral (get_node (0), get_node (1), get_node (2), el);
}

CTree *CBuilder::postfix_expr () { 
  // 1 : prim_expr
  // 1 : cmpd_literal
  // 1+: prim_expr  postfix_expr1...
  // 1+: cmpd_literal  postfix_expr1...
  CTree *result = get_node (0); 
  int num = nodes ();
  for (int n = 1; n < num; n++) {
    Container *c = (Container*)get_node (n);
    // 1: --
    // 1: ++
    // 2: .  identifier
    // 2: ->  identifier
    // 2: (  )
    // 3: (  expr_list  )
    // 3: [  expr  ]
    int token = c->token ()->type ();
    if (token == TOK_DECR || token == TOK_INCR)
      result = new CT_PostfixExpr (result, c->Son (0));
    else if (token == TOK_DOT)
      result = new CT_MembRefExpr (result, c->Son (0), c->Son (1));
    else if (token == TOK_PTS)
      result = new CT_MembPtrExpr (result, c->Son (0), c->Son (1));
    else if (token == TOK_OPEN_SQUARE)
      result = new CT_IndexExpr (result, c->Son (0), c->Son (1), c->Son (2));
    else if (token == TOK_OPEN_ROUND) {
      bool args = (c->Sons () == 3);
      CT_ExprList *el = args ? (CT_ExprList*)c->Son (1) : new CT_ExprList;
      if (! args)
        Push (el);
      el->AddProperties (CT_List::OPEN_CLOSE);
      el->PrefixSon (c->Son (0));
      el->AddSon (c->Son (args ? 2 : 1));
      result = new CT_CallExpr (result, el);
    } 
    delete c;
  }
  return result;
}

CTree *CBuilder::postfix_expr1 () { 
  return container ();
}

CTree *CBuilder::expr_list () { 
  return list (new CT_ExprList); 
}

CTree *CBuilder::unary_expr () { 
  // 1: postfix_expr
  // 1: offsetof_expr
  // 2: --  unary_expr
  // 2: ++  unary_expr
  // 2: any_unary_op  cast_expr
  // 2: sizeof  unary_expr
  // 2: sizeof  unary_expr1
  // 2: alignof  unary_expr
  // 2: alignof  unary_expr1
  if (nodes () == 1)
    return get_node ();
  int token = get_node ()->token ()->type ();
  if (token == TOK_AND)
    return new CT_AddrExpr (get_node (0), get_node (1));
  if (token == TOK_MUL)
    return new CT_DerefExpr (get_node (0), get_node (1));
  if (token == TOK_SIZEOF) {
    if (get_node (1)->NodeName () == Container::NodeId ()) {
      Container *c = (Container*) get_node (1);
      CTree *result = new CT_SizeofExpr (get_node (0), 
        c->Son (0), c->Son (1), c->Son (2));
      delete c;
      return result;
    } else
      return new CT_SizeofExpr (get_node (0), get_node (1));
  }
  if (token == TOK_ALIGNOF) {
    if (get_node (1)->NodeName () == Container::NodeId ()) {
      Container *c = (Container*) get_node (1);
      CTree *result = new CT_AlignofExpr (get_node (0), 
        c->Son (0), c->Son (1), c->Son (2));
      delete c;
      return result;
    } else
      return new CT_AlignofExpr (get_node (0), get_node (1));
  }
  return new CT_UnaryExpr (get_node (0), get_node (1));
}

CTree *CBuilder::unary_expr1 () { 
  // 3: (  type_id  )
  return container ();
}

CTree *CBuilder::cast_expr () { 
  // 1: cast_expr1
  // 1: unary_expr
  return get_node ();
}

CTree *CBuilder::cast_expr1 () { 
  // 2+: cast_expr2... unary_expr
  int num = nodes ();
  CTree *result = get_node (num - 1);
  for (int i = num - 2; i >= 0; i--) {
    Container *c = (Container*) get_node (i);
    result = new CT_CastExpr (c->Son (0), c->Son (1), c->Son (2), result);
    delete c;
  }
  return result;  
}

CTree *CBuilder::cast_expr2 () { 
  // 3: (  type_id  )
  return container ();
}

CTree *CBuilder::offsetof_expr () {
  // 6: OFFSETOF  (  type_spec  ,  memb_designator  )
  return new CT_OffsetofExpr (get_node (0), get_node (1),
    get_node (2), get_node (3), get_node (4), get_node (5));
}

CTree *CBuilder::memb_designator () {
  // 1 : identifier
  // 2+: identifier  designator...
  CT_DesignatorSeq *result = new CT_DesignatorSeq (nodes ());
  for (int i = 0; i < nodes (); i++)
    result->AddSon (get_node (i));
  return result;
}

// helper function for binary expression
CTree *CBuilder::lr_bin_expr () {
  CTree *result = get_node (0);
  for (int n = 1; n < nodes (); n += 2)
    result = new CT_BinaryExpr (result, get_node (n), get_node (n + 1));
  return result;
}

CTree *CBuilder::mul_expr ()     { return lr_bin_expr (); }
CTree *CBuilder::add_expr ()     { return lr_bin_expr (); }
CTree *CBuilder::shift_expr ()   { return lr_bin_expr (); }
CTree *CBuilder::rel_expr ()     { return lr_bin_expr (); }
CTree *CBuilder::equ_expr ()     { return lr_bin_expr (); }
CTree *CBuilder::and_expr ()     { return lr_bin_expr (); }
CTree *CBuilder::excl_or_expr () { return lr_bin_expr (); }
CTree *CBuilder::incl_or_expr () { return lr_bin_expr (); }
CTree *CBuilder::log_and_expr () { return lr_bin_expr (); }
CTree *CBuilder::log_or_expr ()  { return lr_bin_expr (); }
CTree *CBuilder::expr ()         { return lr_bin_expr (); }

CTree *CBuilder::cond_expr () {
  CTree *result = get_node (0);
  if (nodes () == 5)
    result = new CT_IfThenExpr (result, get_node (1), get_node (2),
			        get_node (3), get_node (4));
  return result;
}

CTree *CBuilder::ass_expr () { 
  // 1 : cond_expr
  // 2+: ass_expr1...  cond_expr
  int num = nodes ();
  CTree *result = get_node (num - 1);
  for (int i = num - 2; i >= 0; i--) {
    Container *c = (Container*) get_node (i);
    result = new CT_BinaryExpr (c->Son (0), c->Son (1), result);
    delete c;
  }
  return result;  
}

CTree *CBuilder::ass_expr1 () { 
  return container (); 
}

CTree *CBuilder::const_expr () { 
  return get_node (); 
}

/*****************************************************************************/
/*                                                                           */
/*                      A . 5   S t a t e m e n t s                          */
/*                                                                           */
/*****************************************************************************/

CTree *CBuilder::stmt () {
  return get_node (); 
}

CTree *CBuilder::label_stmt () {
  // 3: identifier  :  stmt         => node token node
  // 3: DEFAULT  :  stmt            => token token node
  // 4: CASE  const_expr  :  stmt   => token node token node
  if (nodes () == 3) {
    if (get_node (0)->NodeName () == CT_SimpleName::NodeId ()) {
      return new CT_LabelStmt (get_node (0), get_node (1), get_node (2));
    } else
      return new CT_DefaultStmt (get_node (0), get_node (1), get_node (2));
  } else 
    return new CT_CaseStmt (get_node (0), get_node (1), get_node (2), 
			    get_node (3));
}

CTree *CBuilder::expr_stmt () {
  // 1: ;
  // 2: expr  ;
  if (nodes () == 1)
    return new CT_ExprStmt ((CTree*)0, get_node (0));
  else
    return new CT_ExprStmt (get_node (0), get_node (1));
}

CTree *CBuilder::cmpd_stmt () {
  // 2: {  }              => token token
  // 3: {  stmt_seq  }    => token cont token
  CT_CmpdStmt *result = new CT_CmpdStmt;
  result->AddSon (get_node (0));
  if (nodes () == 3) {
    Container *container = (Container*)get_node (1);
    for (int i = 0; i < container->Sons (); i++)
      result->AddSon (container->Son (i));
    delete container;
    result->AddSon (get_node (2));
  } else
    result->AddSon (get_node (1));
  return result;
}

CTree *CBuilder::stmt_seq () {
  return container (); 
}

CTree *CBuilder::select_stmt () {
  // 5: SWITCH  (  condition  )  sub_stmt
  // 5: IF  (  condition  )  sub_stmt
  // 7: IF  (  condition  )  sub_stmt  ELSE  sub_stmt
  if (get_node (0)->token ()->type () == TOK_SWITCH)
    return new CT_SwitchStmt (get_node (0), get_node (1), get_node (2),
                              get_node (3), get_node (4));
  else if (nodes () == 5)
    return new CT_IfStmt (get_node (0), get_node (1), get_node (2),
                          get_node (3), get_node (4));
  else
    return new CT_IfElseStmt (get_node (0), get_node (1), get_node (2),
                get_node (3), get_node (4), get_node (5), get_node (6));
}

CTree *CBuilder::sub_stmt () {
  return get_node ();
}

CTree *CBuilder::condition (CTree *) {
  return get_node ();
}

CTree *CBuilder::iter_stmt () {
  // 5: WHILE  (  condition  )  sub_stmt
  // 6: FOR  (  for_init_stmt  ;  )  sub_stmt
  // 7: DO  sub_stmt  WHILE  (  expr  )  ;
  // 7: FOR  (  for_init_stmt  ;  expr  )  sub_stmt
  // 7: FOR  (  for_init_stmt  condition  ;  )  sub_stmt
  // 8: FOR  (  for_init_stmt  condition  ;  expr  )  sub_stmt
  if (nodes () == 5) {
    return new CT_WhileStmt (get_node (0), get_node (1), get_node (2), 
			     get_node (3), get_node (4));
  } else if (nodes () == 6) {
    return new CT_ForStmt (get_node (0), get_node (1), get_node (2), 
                           (CTree*)0, get_node (3), (CTree*)0, 
			   get_node (4), get_node (5));
  } else if (nodes () == 7) {
    if (get_node (0)->token ()->type () == TOK_DO) {
      return new CT_DoStmt (get_node (0), get_node (1), get_node (2), 
                            get_node (3), get_node (4), get_node (5), 
			    get_node (6));
    } else if (get_node (3)->token ()->type () == TOK_SEMI_COLON) {
      return new CT_ForStmt (get_node (0), get_node (1), get_node (2), 
                            (CTree*)0, get_node (3), get_node (4), 
			    get_node (5), get_node (6));
    } else {
      return new CT_ForStmt (get_node (0), get_node (1), get_node (2), 
			     get_node (3), get_node (4), (CTree*)0, 
			     get_node (5), get_node (6));
    }
  } else { // nodes () == 8
    return new CT_ForStmt (get_node (0), get_node (1), get_node (2), 
			   get_node (3), get_node (4), get_node (5), 
			   get_node (6), get_node (7));
  }
}

CTree *CBuilder::for_init_stmt () {
  // 1: expr_stmt
  // 1: simple_decl
  return get_node ();
}

CTree *CBuilder::jump_stmt () {
  // 2: BREAK  ;
  // 2: CONTINUE  ;
  // 2: RETURN  ;
  // 3: RETURN  expression  ;
  // 3: GOTO  identifier  ;
  CT_Token *token = (CT_Token*)get_node (0);
  if (token->token ()->type () == TOK_BREAK)
    return new CT_BreakStmt (token, get_node (1));
  else if (token->token ()->type () == TOK_CONTINUE)
    return new CT_ContinueStmt (token, get_node (1));
  else if (token->token ()->type () == TOK_GOTO)
    return new CT_GotoStmt (token, get_node (1), get_node (2));
  else { // RETURN
    if (nodes () == 2) {
      return new CT_ReturnStmt (token, (CTree*)0, get_node (1));
    } else {
      return new CT_ReturnStmt (token, get_node (1), get_node (2));
    }
  } 
}

/*****************************************************************************/
/*                                                                           */
/*                     A . 6   D e c l a r a t i o n s                       */
/*                                                                           */
/*****************************************************************************/

CTree *CBuilder::decl_seq () { 
  return container (); 
}

CTree *CBuilder::decl () { 
  return get_node (); 
}

CTree *CBuilder::block_decl () { 
  return get_node (); 
}

CTree *CBuilder::simple_decl () { 
  int offset = 0;
  CT_DeclSpecSeq *dss;
  if (get_node ()->NodeName () == CT_DeclSpecSeq::NodeId ())
    dss = (CT_DeclSpecSeq*)get_node (offset++);
  else //{
    dss = new CT_DeclSpecSeq;
//    Push (dss);
//  }
  CT_DeclaratorList *dl;
  if (offset + 1 < nodes ())
    dl = (CT_DeclaratorList*)get_node (offset++);
  else //{
    dl = new CT_DeclaratorList;
//    Push (dl);
//  }
  return new CT_ObjDecl (dss, dl, get_node (offset));
}

CTree *CBuilder::decl_spec () { 
  return get_node (); 
}

CTree *CBuilder::decl_spec_seq () { 
  return list (new CT_DeclSpecSeq); 
}

CTree *CBuilder::decl_spec_seq1 () { 
  return get_node (); 
}

// helper function for primitive declaration specifiers
CTree *CBuilder::prim_ds () { 
  return new CT_PrimDeclSpec ((CT_Token*)get_node ()); 
}

CTree *CBuilder::misc_spec ()          { return prim_ds (); }
CTree *CBuilder::storage_class_spec () { return prim_ds (); }
CTree *CBuilder::fct_spec ()           { return prim_ds (); }

CTree *CBuilder::type_spec () { 
  return get_node (); 
}

CTree *CBuilder::simple_type_spec () { 
  return (get_node ()->NodeName () == CT_Token::NodeId ()) ?
    prim_ds () : get_node ();
}

CTree *CBuilder::type_name () { 
  return get_node (); 
}

CTree *CBuilder::elaborated_type_spec () { 
  // 2: class_key  identifier
  // 2: ENUM  identifier
  int token = get_node ()->token ()->type ();
  if (token == TOK_CLASS || token == TOK_STRUCT)
    return new CT_ClassSpec (get_node (0), get_node (1));
  if (token == TOK_UNION)
    return new CT_UnionSpec (get_node (0), get_node (1));
  else // TOK_ENUM
    return new CT_EnumSpec (get_node (0), get_node (1));
}

CTree *CBuilder::enum_key () {
  return get_node ();
}

CTree *CBuilder::enum_spec () {
  // 4: enum_spec1 {  enumerator_list  }
  CT_EnumeratorList *el = (CT_EnumeratorList*)get_node (2);
  if (el->Sons () > 0 && (el->Sons () % 2) == 0)
    el->AddProperties (CT_List::END_SEP);
  el->PrefixSon (get_node (1));
  el->AddSon (get_node (3));
  CT_EnumDef *result = (CT_EnumDef*)get_node (0);
  result->Enumerators (el);
  return result;
}

CTree *CBuilder::enum_spec1 () {
  // 2: ENUM  identifier
  // 2: ENUM  private_name
  return new CT_EnumDef (get_node (0), get_node (1));
}

CTree *CBuilder::enumerator_list () { 
  return list (new CT_EnumeratorList);
}

CTree *CBuilder::enumerator_def () { 
  // 1: enumerator
  // 3: enumerator  =  const_expr
  CT_Enumerator *e = (CT_Enumerator*)get_node (0);
  if (nodes () == 3) {
    CT_ExprList *el = new CT_ExprList;
//    Push (el);
    el->AddSon (get_node (1));
    el->AddSon (get_node (2));
    el->AddProperties (CT_List::INTRO);
    e->Initializer (el);
  }
  return e;
}

CTree *CBuilder::enumerator () { 
  return new CT_Enumerator (get_node ());
}

CTree *CBuilder::asm_def () { 
  // 1: ASM  (  str_literal  )  ;
  return new CT_AsmDef (get_node (0), get_node (1), get_node (2),
                        get_node (3), get_node (4));
}


/*****************************************************************************/
/*                                                                           */
/*                     A . 7   D e c l a r a t o r s                         */
/*                                                                           */
/*****************************************************************************/

CTree *CBuilder::init_declarator_list () { 
  return list (new CT_DeclaratorList); 
}

CTree *CBuilder::init_declarator () {
  CTree *id = get_node ();
  if (nodes () == 2 && get_node (1)->NodeName () == CT_ExprList::NodeId ())
    ((CT_InitDeclarator*)id)->Initializer (get_node (1));
  return id;
}

CTree *CBuilder::init_declarator1 () {
  return new CT_InitDeclarator (get_node (0));
}

CTree *CBuilder::declarator () {
  // 1 : direct_declarator
  // 2+: ptr_operator..  direct_declarator
  int num = nodes ();
  CTree *result = get_node (num-1);
  for (int i = num-1; i > 0; i--) {
    Container *p = (Container*)get_node (i-1); // ptr-operator
    if (p->Sons () == 1)
      result = new CT_PtrDeclarator (p->Son (0), (CTree*)0, result);
    else if (p->Sons () == 2)
      result = new CT_PtrDeclarator (p->Son (0), p->Son (1), result);
    delete p;
  }
  return result;
}

CTree *CBuilder::direct_declarator () { 
  // 1 : declarator_id                            => node
  // 2+: declarator_id  direct_declarator1...     => node cont...
  // 3 : (  declarator  )                         => token node token
  // 3+: (  declarator  )  direct_declarator1...  => token node token cont...
  if (nodes () == 1)
    return get_node (0); 

  CTree *result;
  int braced = (get_node (0)->token ()->type () == TOK_OPEN_ROUND) ? 2 : 0;
  if (! braced)
    result = get_node (0);
  else {
    // win specific declaration specifier
    if (get_node (2)->token ()->type () != TOK_CLOSE_ROUND) {
      braced++;
      result = new CT_BracedDeclarator (get_node (0), get_node (1), get_node (2), get_node (3));
    } else
      result = new CT_BracedDeclarator (get_node (0), get_node (1), get_node (2));
    // braced declarator only
    if (nodes () == braced+1)
      return result;
  }
  
  for (int i = 1 + braced; i < nodes (); i++) {
    Container *d1 = (Container*)get_node (i);
    if (d1->Son (0)->token ()->type () == TOK_OPEN_SQUARE) 
      result = new CT_ArrayDeclarator (result, d1->Son (0), d1->Son (1), 
                                       d1->Son (2));
    else {
      CT_ArgDeclList *args = (CT_ArgDeclList*)d1->Son (1);
      args->PrefixSon (d1->Son (0));
      args->AddSon (d1->Son (2));
      if (d1->Sons () == 4) {
        if (d1->Son (3)->NodeName () == CT_DeclSpecSeq::NodeId ())
          result = new CT_FctDeclarator (result, args, d1->Son (3), 0);
        else
          result = new CT_FctDeclarator (result, args, 0, d1->Son (3));
      } else if (d1->Sons () == 5) {
        result = new CT_FctDeclarator (result, args, d1->Son (3), d1->Son (4));
      } else
        result = new CT_FctDeclarator (result, args, 0, 0);
    }
    delete d1;
  }
  return result;
} 

CTree *CBuilder::direct_declarator1 () { 
  // 3: [  array_delim  ]
  // 3: (  param_decl_clause  )
  // 3: (  identifier_list  )
  return container ();
} 

CTree *CBuilder::identifier_list () { 
  return list (new CT_ArgNameList);
} 

CTree *CBuilder::array_delim () { 
  // 0:
  // 1: *
  // 1: ass_expr
  // 1: cv_qual_seq
  // 2: cv_qual_seq  *
  // 2: cv_qual_seq  ass_expr
  // 2: STATIC  ass_expr
  // 3: STATIC  cv_qual_seq  ass_expr
  // 3: cv_qual_seq  STATIC  ass_expr
  bool pos0 = false;
  CTree *star, *stat, *quals, *expr = star = stat = quals = (CTree*)0;
  if (nodes () == 1) {
    if (get_node (0)->NodeName () == CT_DeclSpecSeq::NodeId ())
      quals = get_node (0);
    else if (get_node (0)->NodeName () == CT_Token::NodeId () && 
             get_node (0)->token ()->type () == TOK_MUL) 
      star = get_node (0);
    else
      expr = get_node (0);
  } else if (nodes () == 2) {
    if (get_node (0)->NodeName () == CT_Token::NodeId ()) {
      stat = get_node (0);
      expr = get_node (1);
    } else {
      quals = get_node (0);
      if (get_node (1)->NodeName () == CT_Token::NodeId () && 
          get_node (1)->token ()->type () == TOK_MUL)
        star = get_node (1);
      else
        expr = get_node (1);
    }
  } else if (nodes () == 3) {
    if (get_node (0)->NodeName () == CT_Token::NodeId ()) {
      stat = get_node (0); 
      quals = get_node (1);
    } else {
      quals = get_node (0); pos0 = true;
      stat = get_node (1);
    }
    expr = get_node (2);
  }
  if (pos0)
    return new CT_ArrayDelimiter (star, quals, stat, expr, pos0);
  return new CT_ArrayDelimiter (star, stat, quals, expr, pos0);
}

CTree *CBuilder::ptr_operator () { 
  return container (); 
}

CTree *CBuilder::cv_qual_seq () { 
  return list (new CT_DeclSpecSeq); 
}

CTree *CBuilder::cv_qual () { 
  return prim_ds (); 
}

CTree *CBuilder::declarator_id () { 
  return get_node (); 
}

CTree *CBuilder::type_id () { 
  return new CT_NamedType (get_node (0), get_node (1));
}

CTree *CBuilder::abst_declarator () {
  // 1 : direct_abst_declarator
  // 1+: ptr_operator..
  // 2+: ptr_operator..  direct_abst_declarator
  return declarator ();
}

CTree *CBuilder::direct_abst_declarator () { 
  // 1 : direct_abst_declarator1                              => cont
  // 2+: direct_abst_declarator1  direct_abst_declarator1...  => cont cont...
  // 3 : ( abst_declarator )                                  => token node token
  // 3+: ( abst_declarator )  direct_abst_declarator1...      => token node token cont...
  CTree *result;
  int braced = (get_node (0)->NodeName () != Container::NodeId ()) ? 3 : 0;
  if (! braced)
    result = get_node (nodes () - 1); // PrivateName, pushed by semantic
  else {
    // win specific declaration specifier
    if (get_node (2)->token ()->type () != TOK_CLOSE_ROUND) {
      braced++;
      result = new CT_BracedDeclarator (get_node (0), get_node (1), get_node (2), get_node (3));
    } else
      result = new CT_BracedDeclarator (get_node (0), get_node (1), get_node (2));
    // braced declarator only
    if (nodes () == braced)
      return result;
  }
  
  int num = nodes () - (braced ? 0 : 1);
  for (int i = 0 + braced; i < num; i++) {
    Container *d1 = (Container*)get_node (i);
    if (d1->Son (0)->token ()->type () == TOK_OPEN_SQUARE) {
      if (d1->Son (1)->NodeName () == CT_ArrayDelimiter::NodeId ()) {
        result = new CT_ArrayDeclarator (result, d1->Son (0), d1->Son (1), d1->Son (2));
      } else if (d1->Son (1)->token ()->type () == TOK_CLOSE_SQUARE) {
        result = new CT_ArrayDeclarator (result, d1->Son (0),
          new CT_ArrayDelimiter (0, 0, 0, 0), d1->Son (1));
      } else if (d1->Son (1)->NodeName () == CT_Token::NodeId () &&
                 d1->Son (1)->token ()->type () == TOK_MUL) {
        result = new CT_ArrayDeclarator (result, d1->Son (0),
          new CT_ArrayDelimiter (d1->Son (1), 0, 0, 0), d1->Son (2));
      } else {
        result = new CT_ArrayDeclarator (result, d1->Son (0),
          new CT_ArrayDelimiter (0, 0, 0, d1->Son (1)), d1->Son (2));
      }
    } else {
      CT_ArgDeclList *args = (CT_ArgDeclList*)d1->Son (1);
      args->PrefixSon (d1->Son (0));
      args->AddSon (d1->Son (2));
      if (d1->Sons () == 4) {
        if (d1->Son (3)->NodeName () == CT_DeclSpecSeq::NodeId ())
          result = new CT_FctDeclarator (result, args, d1->Son (3), 0);
        else
          result = new CT_FctDeclarator (result, args, 0, d1->Son (3));
      } else if (d1->Sons () == 5) {
        result = new CT_FctDeclarator (result, args, d1->Son (3), d1->Son (4));
      } else
        result = new CT_FctDeclarator (result, args, 0, 0);
    }
    delete d1;
  }
  return result;
} 

CTree *CBuilder::direct_abst_declarator1 () { 
  // 2: [  ]
  // 3: [  *  ]
  // 3: [  ass_expr  ]
  // 3: (  param_decl_clause  )
  return container ();
} 

CTree *CBuilder::param_decl_clause () {
  // 0: 			    => 
  // 1: ...			    => token
  // 1: param_decl_list		    => cont
  // 2: param_decl_list  ...	    => cont token
  if (nodes () == 0)
    return new CT_ArgDeclList ();
    
  CT_ArgDeclList *result;
  if (get_node (0)->NodeName () != Container::NodeId ()) {
    result = new CT_ArgDeclList ();
    result->AddSon (new CT_ArgDecl (get_node (0)));
    return result;
  }
  
  Container *al = (Container*)get_node (0);
  int add_args = (nodes () == 2) ? 1 : 0;
  result = new CT_ArgDeclList (al->Sons () + add_args);  
  copy_list (result, al);
  if (add_args) { 
    if ((al->Sons () % 2) == 1)
      result->AddProperties (CT_List::NO_LAST_SEP);
    result->AddSon (new CT_ArgDecl (get_node (nodes ()-1))); // ...
  }
  
  delete al;
  return result;
}

CTree *CBuilder::param_decl_list () {
  return container ();  
}

CTree *CBuilder::param_decl () {
  // 1: param_decl1 
  // 3: param_decl1  param_init
  CT_ArgDecl *ad = (CT_ArgDecl*)get_node (0);
  if (nodes () == 2)
    ad->Initializer (get_node (1));
  return ad;
}

CTree *CBuilder::param_decl1 () {
  // 2: decl_spec_seq  abst_declarator
  // 2: decl_spec_seq  declarator
  // 2: decl_spec_seq  private_name
  return new CT_ArgDecl (get_node (0), get_node (1));
}

CTree *CBuilder::fct_def () {
  // 2: declarator  fct_body
  // 2: declarator  fct_try_block
  // 3: declarator  ctor_init  fct_body
  // 3: declarator  arg_decl_seq  fct_body
  // 3: decl_spec_seq  declarator  fct_body
  // 3: decl_spec_seq  declarator  fct_try_block
  // 4: decl_spec_seq  declarator  ctor_init  fct_body
  // 4: decl_spec_seq  declarator  arg_decl_seq  fct_body
  CTree *dss, *hs, *ci, *as, *t = dss = hs = ci = as = (CTree*)0;
  CTree *d, *b = 0;
  if (get_node (0)->NodeName () == CT_DeclSpecSeq::NodeId ())
    dss = get_node (0);
  int offset = dss ? 1 : 0;
  d = get_node (offset);
  if (nodes () == 1 || (nodes () == 2 && 
       get_node (1)->NodeName () != CT_CmpdStmt::NodeId () &&
       get_node (1)->NodeName () != Container::NodeId ())) {
    b = 0; // parse delayed
  } else if (get_node (1+offset)->NodeName () == CT_CmpdStmt::NodeId ()) {
    b = get_node (1+offset);
  } else if (get_node (1+offset)->NodeName () == CT_MembInitList::NodeId () ||
             get_node (1+offset)->NodeName () == CT_ExprList::NodeId ()) {
    ci = get_node (1+offset);
    b = get_node (2+offset);
  } else if (get_node (1+offset)->NodeName () == CT_ArgDeclSeq::NodeId ()) {
    as = get_node (1+offset);
    b = get_node (2+offset);
  } else {
    Container *cont = (Container*)get_node (1+offset); 
    t = cont->Son (0); 
    if (cont->Son (1)->NodeName () == CT_MembInitList::NodeId ())
      ci = cont->Son (1);
    offset = ci ? 1 : 0;
    b = cont->Son (1+offset);
    hs = cont->Son (2+offset);
    delete cont;
  }
  return new CT_FctDef (dss, d, t, ci, as, b, hs); 
}

CTree *CBuilder::arg_decl_seq () {
  return list (new CT_ArgDeclSeq (nodes ()));
}

CTree *CBuilder::fct_body () {
  // 1: cmpd_stmt
  return get_node (0);
}

CTree *CBuilder::init () {
  // 2: =  init_clause 
  // 3: (  expr_list  )
  CT_ExprList *result;
  if (get_node (1)->NodeName () != CT_ExprList::NodeId ()) {
    result = new CT_ExprList;
    result->AddSon (get_node (1));
  } else
    result = (CT_ExprList*)get_node (1);

  result->PrefixSon (get_node (0));
  if (nodes () == 2)  // =  init_clause
    result->AddProperties (CT_List::INTRO);
  else {              // (  expr_list  )
    result->AddSon (get_node (2));
    result->AddProperties (CT_List::OPEN_CLOSE);
  }
  return result;
}

CTree *CBuilder::init_clause () {
  // 1: ass_expr
  // 2: {  }
  // 3: {  init_list  }
  if (nodes () == 1) 
    return get_node (0);

  CT_ExprList *result = 0;
  if (nodes () == 2) {        // "{ }"
    result = new CT_ExprList;
    result->AddSon (get_node (0));
    result->AddSon (get_node (1));
  } else if (nodes () == 3) { // "{ init_list }"
    result = (CT_ExprList*)get_node (1);
    if ((result->Sons () % 2) == 0)
      result->AddProperties (CT_List::END_SEP);
    result->PrefixSon (get_node (0));
    result->AddSon (get_node (2));
  } 
  result->AddProperties (CT_List::OPEN_CLOSE);
  return result;
}

CTree *CBuilder::init_list () { 
  return list (new CT_ExprList); 
}

CTree *CBuilder::init_list_item () { 
  // 1: init_clause
  // 2: designation  init_clause
  if (nodes () == 1)
    return get_node (); 
  Container *c = (Container*)get_node (0);
  CTree *result = new CT_BinaryExpr (c->Son (0), c->Son (1), get_node (1)); 
  delete c;
  return result;
}

CTree *CBuilder::designation () { 
  // 2+: designator...  =
  CT_DesignatorSeq *ds = new CT_DesignatorSeq (nodes () - 1);
  for (int i = 0; i < nodes () - 1; i++)
    ds->AddSon (get_node (i));
  Container *result = new Container;
  result->AddSon (ds);
  result->AddSon (get_node (nodes () - 1));
  return result;
}

CTree *CBuilder::designator () { 
  // 2: .  identifier
  // 3: [  const_expr  ]
  if (nodes () == 2)
    return new CT_MembDesignator (get_node (0), get_node (1));
  else
    return new CT_IndexDesignator (get_node (0), get_node (1), get_node (2));
}

/*****************************************************************************/
/*                                                                           */
/*                          A . 8   C l a s s e s                            */
/*                                                                           */
/*****************************************************************************/

CTree *CBuilder::class_spec () {
  CT_ClassDef *result = (CT_ClassDef*)get_node (0);
  CT_MembList *ml = nodes () == 4 ?
    (CT_MembList*)get_node (2) : new CT_MembList;
  ml->PrefixSon (get_node (1));
  ml->AddSon (get_node (nodes () - 1));
  ml->AddProperties (CT_List::OPEN_CLOSE);
  result->Members (ml);
  return result;
}

CTree *CBuilder::class_head () { 
  CT_ClassDef *result;
  if (get_node (0)->token ()->type () == TOK_UNION)
    result = new CT_UnionDef (get_node (0), get_node (1));
  else
    result = new CT_ClassDef (get_node (0), get_node (1));
  return result;
}

CTree *CBuilder::class_key () { 
  return get_node (); 
}

CTree *CBuilder::member_spec () { 
  return list (new CT_MembList); 
}

CTree *CBuilder::member_decl () { 
  // 1: ;
  // 2: type_spec_seq                          ;
  // 3: type_spec_seq  member_declarator_list  ;
  if (nodes () == 1) {
    return new CT_ObjDecl (new CT_DeclSpecSeq, new CT_DeclaratorList, get_node (0));
  }
  else if (nodes () == 2)
    return new CT_ObjDecl (get_node (0), new CT_DeclaratorList, get_node (1));
  else
    return new CT_ObjDecl (get_node (0), get_node (1), get_node (2));
}

CTree *CBuilder::member_declarator_list () { 
  return list (new CT_DeclaratorList); 
}

CTree *CBuilder::member_declarator () { 
  // 1: declarator
  // 3: declarator  :  const_expr
  // 3: private_name  :  const_expr 
  if (nodes () == 1)
    return new CT_InitDeclarator (get_node (0));
  else 
    return new CT_BitFieldDeclarator (get_node (0), get_node (1), get_node (2));
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
/*                A . 1 3   E x c e p t i o n  h a n d l i n g               */
/*                                                                           */
/*****************************************************************************/


} // namespace Puma
