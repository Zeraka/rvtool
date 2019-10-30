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

#include "Puma/CCBuilder.h"
#include "Puma/CCSyntax.h"
#include "Puma/CTokens.h"
#include "Puma/CTemplateInstance.h"
#include "Puma/DelayedParseTree.h"

namespace Puma {


void CCBuilder::cleanup(CTree* tree) {
  if (! tree)
    return;

  for (int i = tree->Sons()-1; i >= 0; i--)
    cleanup(tree->Son(i));

  if (tree->NodeName() == CT_TemplateName::NodeId()) {
    // remove the template instance from the template
    CObjectInfo* obj = tree->SemObject()->Object();
    if (obj) {
      CTemplateInstance* instance = obj->TemplateInstance();
      if (instance && instance->PointOfInstantiation() == tree && instance->Template()) {
        instance->Template()->removeInstance(instance->Object());
      }
    }
  } else if (tree->IsDelayedParse()) {
    // remove the delayed parse node from the corresponding class
    CT_DelayedParse* dp = (CT_DelayedParse*)tree;
    if (dp->Class())
      dp->Class()->removeDelayed(dp);
  }
}


void CCBuilder::Delete() {
  cleanup(Top());
  CBuilder::Delete();
}



/*****************************************************************************/
/*                                                                           */
/*                           A . 1   K e y w o r d s                         */
/*                                                                           */
/*****************************************************************************/

CTree *CCBuilder::namespace_name () {
  // 1: original_ns_name
  // 1: namespace_alias
  return get_node ();
}

/*****************************************************************************/
/*                                                                           */
/*                      A . 4   E x p r e s s i o n s                        */
/*                                                                           */
/*****************************************************************************/

CTree *CCBuilder::prim_expr () { 
  // 1: literal      => node
  // 1: THIS         => token
  // 1: id_expr      => node
  // 3: (  expr  )   => token  node  token
  if (nodes () == 3)
    return new CT_BracedExpr (get_node (0), get_node (1), get_node (2));
  else if (get_node ()->token ()->type () == TOK_THIS)
    return simple_name ();
  else
    return get_node ();
}

CTree *CCBuilder::id_expr () { 
  return get_node (); 
}

CTree *CCBuilder::unqual_id () { 
  // 1: template_id
  // 1: identifier
  // 1: oper_fct_id
  // 1: conv_fct_id
  // 2: ~  class_name
  if (nodes () == 2)
    return new CT_DestructorName (get_node (0), get_node (1));
  else
    return get_node (); 
}

CTree *CCBuilder::qual_id () { 
  // 2: ::  identifier
  // 2: ::  oper_fct_id
  // 2: ::  template_id
  // 2:     nested_name_spec                unqual_id
  // 3: ::  nested_name_spec                unqual_id
  // 3:     nested_name_spec  template_key  unqual_id
  // 4: ::  nested_name_spec  template_key  unqual_id
  CT_QualName *result;
  int root = 0, num = nodes ();
  if (get_node (0)->NodeName () != Container::NodeId ()) {
    root = 1;
    if (num == 2) {
      result = new CT_RootQualName (2);
      result->AddSon (get_node (0));
      result->AddSon (get_node (1));
      return result;
    }
  }

  Container *nns = (Container*)get_node (root);
  CT_SimpleName *sn = (CT_SimpleName*)get_node (num-1);
  // name prefixed by `template'
  if (get_node (num-2)->NodeName () != Container::NodeId ())
    sn->PrefixSon (get_node (num-2));
  if (root == 1) {
    result = new CT_RootQualName (nns->Sons ()+2);
    result->AddSon (get_node (0));
  } else 
    result = new CT_QualName (nns->Sons ()+1);
  copy_list (result, nns);
  result->AddSon (sn);
  delete nns;
  return result;
}

CTree *CCBuilder::nested_name_spec () { 
  Container *c, *nns = (Container*)get_node ();
  CT_SimpleName *sn;
  int sons;
  
  for (int i = 1; i < nodes (); i++) {
    c = (Container*)get_node (i);
    sons = c->Sons ();
    sn = (CT_SimpleName*)c->Son (sons-2);
    if (sons == 3) 
      sn->PrefixSon (c->Son (0));
    nns->AddSon (sn);
    nns->AddSon (c->Son (sons-1));
    delete c;
  }
  return nns; 
}

CTree *CCBuilder::nested_name_spec1 () { 
  return container (); 
}

CTree *CCBuilder::class_or_ns_name () { 
  // 1: class_name
  // 1: namespace_name
  return get_node ();
}

CTree *CCBuilder::postfix_expr (CTree *postfix) { 
  // 1 : postfix_expr2
  // 1+: postfix_expr2  postfix_expr1..
  CTree *result = postfix; 
  if (nodes () == 1) {
    result = get_node (0);
  } else {  
    Container *c = (Container*)get_node (nodes ()-1);
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
    int token = c->token ()->type ();
    if (token == TOK_DECR || token == TOK_INCR)
      result = new CT_PostfixExpr (result, c->Son (0));
    else if (token == TOK_OPEN_SQUARE)
      result = new CT_IndexExpr (result, c->Son (0), c->Son (1), c->Son (2));
    else if (token == TOK_DOT || token == TOK_PTS) {
      CT_SimpleName *sn = (CT_SimpleName*)c->Son (c->Sons ()-1);
      // name prefixed by `template'
      if (c->Sons () == 3)
        sn->PrefixSon (c->Son (1));
      if (token == TOK_DOT)
        result = new CT_MembRefExpr (result, c->Son (0), sn);
      else
        result = new CT_MembPtrExpr (result, c->Son (0), sn);
    } else if (token == TOK_OPEN_ROUND) {
      bool args = (c->Sons () == 3);
      CT_ExprList *el = args ? (CT_ExprList*)c->Son (1) : new CT_ExprList;
      el->AddProperties (CT_List::OPEN_CLOSE);
      el->PrefixSon (c->Son (0));
      el->AddSon (c->Son (args ? 2 : 1));
      result = new CT_CallExpr (result, el);
    } 
    delete c;
  }
  return result;
}

CTree *CCBuilder::postfix_expr2 () {
  // 1 : cmpd_literal
  // 1 : prim_expr
  // 1 : construct_expr
  // 4 : TYPEID  (  expr  )
  // 4 : TYPEID  (  type_id  )
  // 7 : CONST_CAST  <  type_id  >  (  expr  )
  // 7 : REINT_CAST  <  type_id  >  (  expr  )
  // 7 : STATIC_CAST  <  type_id  >  (  expr  )
  // 7 : DYNAMIC_CAST  <  type_id  >  (  expr  )
  int num = nodes ();
  // typeid ( ... )
  if (num == 4) {
    return new CT_TypeidExpr (get_node (0), get_node (1),
                              get_node (2), get_node (3));
  }
  // new style casts
  else if (num == 7) {
    int type = get_node (0)->token ()->type ();
    CTree* expr = new CT_BracedExpr (get_node (4), get_node (5), get_node (6));
    if (type == TOK_REINT_CAST)
      return new CT_ReintCast (get_node (0), get_node (1), get_node (2),
                               get_node (3), expr);
    else if (type == TOK_CONST_CAST)
      return new CT_ConstCast (get_node (0), get_node (1), get_node (2),
                               get_node (3), expr);
    else if (type == TOK_STAT_CAST)
      return new CT_StaticCast (get_node (0), get_node (1), get_node (2),
                                get_node (3), expr);
    else if (type == TOK_DYN_CAST)
      return new CT_DynamicCast (get_node (0), get_node (1), get_node (2),
                                 get_node (3), expr);
  }
  return get_node (0);
}

CTree *CCBuilder::construct_expr () {
  // 3 : simple_type_spec  (  )
  // 4 : simple_type_spec  (  expr_list  )
  // 5 : TYPENAME  nested_name_spec  identifier  (  )
  // 5 : TYPENAME  nested_name_spec  template_id  (  )
  // 6 : TYPENAME  nested_name_spec  identifier  (  expr_list  )
  // 6 : TYPENAME  nested_name_spec  template_id  (  expr_list  )
  // 6 : TYPENAME  nested_name_spec  template_key  template_id  (  )
  // 6 : TYPENAME  ::  nested_name_spec  identifier  (  )
  // 6 : TYPENAME  ::  nested_name_spec  template_id  (  )
  // 7 : TYPENAME  ::  nested_name_spec  identifier  (  expr_list  )
  // 7 : TYPENAME  ::  nested_name_spec  template_id  (  expr_list  )
  // 7 : TYPENAME  ::  nested_name_spec  template_key  template_id  (  )
  // 7 : TYPENAME  nested_name_spec  template_key  template_id  (  expr_list  )
  // 8 : TYPENAME  ::  nested_name_spec  template_key  template_id  (  expr_list  )
  CT_ExprList *el;
  int num = nodes ();
  // TYPENAME ...
  if (num > 4) {
    int offset = 0;
    if (get_node (num-2)->NodeName () == CT_ExprList::NodeId ()) {
      el = (CT_ExprList*)get_node (num-2);
      el->PrefixSon (get_node (num-3));
      offset = 1;
    } else {
      el = new CT_ExprList;
      el->PrefixSon (get_node (num-2));
    }
    el->AddSon (get_node (num-1));
    el->AddProperties (CT_List::OPEN_CLOSE);

    CT_SimpleName *name = (CT_SimpleName*)get_node (num-3-offset);
    // name prefixed by `template'
    if (get_node (num-4-offset)->token ()->type () == TOK_TEMPLATE)
      name->PrefixSon (get_node (num-4-offset));
    CT_QualName *qn;
    int root = (get_node (1)->NodeName () == Container::NodeId ()) ? 0 : 1;
    Container *nns = (Container*)get_node (root+1);
    if (root) {
      qn = new CT_RootQualName (nns->Sons ()+2);
      qn->AddSon (get_node (1));
    } else
      qn = new CT_QualName (nns->Sons ()+1);
    copy_list (qn, nns);
    qn->AddSon (name);
    return new CT_ConstructExpr (new CT_ClassSpec (get_node (0), qn), el);
  }
  // simple_type_spec ...
  else {
    CT_ExprList *el = (num == 4) ? (CT_ExprList*)get_node (2) : new CT_ExprList;
    el->PrefixSon (get_node (1));
    el->AddSon (get_node (num - 1));
    el->AddProperties (CT_List::OPEN_CLOSE);
    return new CT_ConstructExpr (get_node (0), el);
  }
}

CTree *CCBuilder::pseudo_dtor_name () { 
  // 2:                                      ~  type_name
  // 3: ::                                   ~  type_name
  // 3:     nested_name_spec                 ~  type_name
  // 4: ::  nested_name_spec                 ~  type_name
  // 4:                       type_name  ::  ~  type_name
  // 5: ::                    type_name  ::  ~  type_name
  // 5:     nested_name_spec  type_name  ::  ~  type_name
  // 6: ::  nested_name_spec  type_name  ::  ~  type_name
  // 6:     nested_name_spec  template_key  template_id  ::  ~  type_name
  // 7: ::  nested_name_spec  template_key  template_id  ::  ~  type_name
  int num = nodes ();
  CTree *dtor = new CT_DestructorName (get_node (num-2), get_node (num-1));
  if (num == 2) 
    return dtor;
    
  CT_QualName *result;
  int root = (get_node (0)->token ()->type () == TOK_COLON_COLON) ? 1 : 0;
  if (get_node (root)->NodeName () == Container::NodeId ()) {
    Container *nns = (Container*)get_node (root);
    if (root == 1) {
      result = new CT_RootQualName (nns->Sons () + (num > 4 ? 4 : 2));
      result->AddSon (get_node (0));
    } else 
      result = new CT_QualName (nns->Sons () + (num > 4 ? 3 : 1));
    copy_list (result, nns);
    delete nns;
  } else {
    if (root == 1) {
      result = new CT_RootQualName (num > 3 ? 4 : 2);
      result->AddSon (get_node (0));
    } else 
      result = new CT_QualName (3);
  }
  if (num > (get_node (root)->NodeName () == Container::NodeId () ? 4 : 3)) {
    CT_SimpleName *sn = (CT_SimpleName*)get_node (num-4);
    // name prefixed by `template'
    if ((num == 6 && ! root) || num == 7)
      sn->PrefixSon (get_node (num-5));
    result->AddSon (sn);
    result->AddSon (get_node (num-3));
  }
  result->AddSon (dtor);
  return result;
}

CTree *CCBuilder::type_trait_expr () {
  // 4: any_unary_type_trait_op  (  type_id  )
  // 6: any_binary_type_trait_op  (  type_id  ,  type_id  )
  int num = nodes ();
  if (num == 4) {
    return new CT_TypeTraitExpr (get_node (0),
      get_node (1), get_node (2), get_node (3));
  }
  else if (num == 6) {
    return new CT_TypeTraitExpr (get_node (0), get_node (1),
      get_node (2), get_node (3), get_node (4), get_node (5));
  }
  return 0;
}

CTree *CCBuilder::new_expr () {
  // 2:     NEW                 new_type_id
  // 3:     NEW                 new_type_id    new_init
  // 3:     NEW  new_placement  new_type_id
  // 3: ::  NEW                 new_type_id
  // 4: ::  NEW                 new_type_id    new_init
  // 4: ::  NEW  new_placement  new_type_id
  // 4:     NEW  new_placement  new_type_id    new_init
  // 4:     NEW                 (  type_id  )
  // 5:     NEW                 (  type_id  )  new_init
  // 5:     NEW  new_placement  (  type_id  )
  // 5: ::  NEW  new_placement  new_type_id    new_init
  // 5: ::  NEW                 (  type_id  )
  // 6: ::  NEW                 (  type_id  )  new_init
  // 6:     NEW  new_placement  (  type_id  )  new_init
  // 6: ::  NEW  new_placement  (  type_id  )
  // 7: ::  NEW  new_placement  (  type_id  )  new_init
  int root = (get_node ()->token ()->type () == TOK_COLON_COLON) ? 2 : 1;
  CTree *opn = new CT_OperatorName (get_node (root-1));
  if (root == 2) {
    CT_RootQualName *rqn = new CT_RootQualName (2);
    rqn->AddSon (get_node (0));
    rqn->AddSon (opn);
    opn = rqn;
  }
  int np = (get_node (root)->NodeName () == CT_ExprList::NodeId ()) ? 1 : 0;
  int el = (get_node (root+np)->token ()->type () == TOK_OPEN_ROUND) ? 2 : 0;
  int num = nodes ();
  return new CT_NewExpr (opn, 
    np ? get_node (root) : 0, 
    el ? get_node (root+np) : 0,
    el ? get_node (root+np+1) : get_node (root+np),
    el ? get_node (root+np+2) : 0, 
    (num-root-np-el) == 2 ? get_node (num-1) : 0);
}

CTree *CCBuilder::new_placement () { 
  // 3: (  expr_list  )
  CT_ExprList *result = (CT_ExprList*)get_node (1);
  result->PrefixSon (get_node (0));
  result->AddSon (get_node (2));
  result->AddProperties (CT_List::OPEN_CLOSE);
  return result;
}

CTree *CCBuilder::direct_new_declarator () { 
  // 3 : [  expr  ]
  // 4+: [  expr  ]  direct_new_declarator1...
  int num = nodes ()-1;
  CTree *result = get_node (num);  // PrivateName, pushed by semantic
  result = new CT_ArrayDeclarator (result, get_node (0), 
             new CT_ArrayDelimiter (0, 0, 0, get_node (1)), get_node (2));

  for (int i = 3; i < num; i++) {
    Container *d1 = (Container*)get_node (i);
    result = new CT_ArrayDeclarator (result, d1->Son (0), 
               new CT_ArrayDelimiter (0, 0, 0, d1->Son (1)), d1->Son (2));
    delete d1;
  }
  return result;
}

CTree *CCBuilder::direct_new_declarator1 () { 
  // 3: [  const_expr  ]
  return container ();
}

CTree *CCBuilder::new_init () { 
  // 2: (  )
  // 3: (  expr_list  )
  CT_ExprList *result = (nodes () == 3) ? 
    (CT_ExprList*)get_node (1) : new CT_ExprList;
  result->PrefixSon (get_node (0));
  result->AddSon (get_node (nodes () - 1));
  result->AddProperties (CT_List::OPEN_CLOSE);
  return result;
}

CTree *CCBuilder::delete_expr () { 
  // 2:     DELETE        cast_expr
  // 3: ::  DELETE        cast_expr
  // 4:     DELETE  [  ]  cast_expr
  // 5: ::  DELETE  [  ]  cast_expr
  int num = nodes (), root = (num == 3 || num == 5) ? 1 : 0;
  CTree *result;
  if (num > 3)
    result = new CT_OperatorName (0, get_node (root), get_node (root+1),
                                  get_node (root+2));
  else 
    result = new CT_OperatorName (get_node (root));
  if (root == 1) {
    CT_RootQualName *rqn = new CT_RootQualName;
    rqn->AddSon (get_node ());
    rqn->AddSon (result);
    result = rqn;
  }
  return new CT_DeleteExpr (result, get_node (num-1));
}

CTree *CCBuilder::pm_expr () { 
  return lr_bin_expr (); 
}

/*****************************************************************************/
/*                                                                           */
/*                      A . 5   S t a t e m e n t s                          */
/*                                                                           */
/*****************************************************************************/

CTree *CCBuilder::condition () {
  // 1: expr
  // 1: condition2
  // 2: type_spec_seq  declarator
  // 3: condition1 = ass_expr
  if (nodes () == 1)
    return get_node ();
  else if (nodes () == 2)
      return new CT_Condition (get_node (0), get_node (1));
  else if (nodes () == 3) {
    CT_ExprList *el = new CT_ExprList;
    el->AddSon (get_node (1));
    el->AddSon (get_node (2));
    el->AddProperties (CT_List::INTRO);
    CTree *cond = get_node ();
    ((CT_Condition*)cond)->Initializer (el);
    return cond;
  }
  else
    return (CTree*)0;
}

CTree *CCBuilder::decl_stmt () {
  // 1: block_decl
  return new CT_DeclStmt (get_node ());
}

/*****************************************************************************/
/*                                                                           */
/*                     A . 6   D e c l a r a t i o n s                       */
/*                                                                           */
/*****************************************************************************/

CTree *CCBuilder::decl () { 
  // 1: block_decl
  // 1: fct_def
  // 1: explicit_specialization
  // 1: template_decl
  // 1: explicit_instantiation
  // 1: linkage_spec
  // 1: namespace_def
  return get_node ();
}

CTree *CCBuilder::simple_type_spec () { 
  // 1: prim_types
  // 1: type_name
  // 2: nested_name_spec  type_name
  // 2: ::  type_name
  // 3: ::  nested_name_spec  type_name
  // 3: nested_name_spec  template_key  template_id
  // 4: ::  nested_name_spec  template_key  template_id
  int num = nodes ();
  if (num == 1) {
    if (get_node ()->NodeName () == CT_Token::NodeId ())
      return prim_ds ();
    else
      return get_node ();
  }

  CT_SimpleName *sn = (CT_SimpleName*)get_node (num-1);
  // name prefixed by `template'
  if (get_node (num-2)->token ()->type () == TOK_TEMPLATE)
    sn->PrefixSon (get_node (num-2));

  int root = (get_node ()->NodeName () == Container::NodeId ()) ? 0 : 1;
  CT_QualName *result;
  if (num == 2 && root) {
    result = new CT_RootQualName (2);
    result->AddSon (get_node (0));
  } else {
    Container *nns = (Container*)get_node (root);
    if (root) {
      result = new CT_RootQualName (nns->Sons ()+2);
      result->AddSon (get_node (0));
    } else
      result = new CT_QualName (nns->Sons ()+1);
    copy_list (result, nns);
    delete nns;
  }
  result->AddSon (sn);
  return result;
}

CTree *CCBuilder::elaborated_type_spec () { 
  // 2: TYPENAME                                       template_id
  // 2: class_key                                      identifier
  // 2: ENUM                                           identifier
  // 3: class_key  ::                                  identifier
  // 3: ENUM       ::                                  identifier
  // 3: class_key      nested_name_spec                identifier
  // 3: ENUM           nested_name_spec                identifier
  // 3: TYPENAME       nested_name_spec                identifier
  // 3: TYPENAME       nested_name_spec                template_id
  // 4: class_key  ::  nested_name_spec                identifier
  // 4: ENUM       ::  nested_name_spec                identifier
  // 4: TYPENAME       nested_name_spec  template_key  template_id
  // 4: TYPENAME   ::  nested_name_spec                identifier
  // 4: TYPENAME   ::  nested_name_spec                template_id
  // 5: TYPENAME   ::  nested_name_spec  template_key  template_id
  int num = nodes ();
  CT_SimpleName *name = (CT_SimpleName*)get_node (num-1);
  if (num > 2) {
    // name prefixed by `template'
    if (get_node (num-2)->token ()->type () == TOK_TEMPLATE)
      name->PrefixSon (get_node (num-2));
    CT_QualName *qn;
    int root = (get_node (1)->NodeName () == Container::NodeId ()) ? 0 : 1;
    if (root && num == 3) {
      qn = new CT_RootQualName (2);
      qn->AddSon (get_node (1));
    } else {
      Container *nns = (Container*)get_node (root+1);
      if (root) {
        qn = new CT_RootQualName (nns->Sons ()+2);
        qn->AddSon (get_node (1));
      } else
        qn = new CT_QualName (nns->Sons ()+1);
      copy_list (qn, nns);
    }
    qn->AddSon (name);
    name = qn;
  }
  int key = get_node ()->token ()->type ();
  if (key == TOK_CLASS || key == TOK_STRUCT || key == TOK_TYPENAME)
    return new CT_ClassSpec (get_node (0), name);
  else if (key == TOK_UNION)
    return new CT_UnionSpec (get_node (0), name);
  else // TOK_ENUM
    return new CT_EnumSpec (get_node (0), name);
}

CTree *CCBuilder::linkage_spec () { 
  // 4+: (EXTERN  str_literal)+  {  }
  // 5+: (EXTERN  str_literal)+  {  decl_seq  }
  // 3+: (EXTERN  str_literal)+  decl
  CT_DeclSpecSeq *dss = new CT_DeclSpecSeq ();
  int i = 0, key;
  do {
    dss->AddSon (get_node (i++));
    dss->AddSon (get_node (i++));
    key = get_node (i)->token ()->type ();
  } while (key == TOK_EXTERN);
  // now the three cases from above ...
  int n = nodes ();
  if (i == n - 2)
    return new CT_LinkageSpec (dss, get_node (n-2), new CT_DeclList (1), get_node (n-1));
  else if (i == n - 3) {
    Container *decls = (Container*)get_node (n-2);
    CT_DeclList *dl = new CT_DeclList (decls->Sons ());
    copy_list (dl, decls);
    delete decls;
    return new CT_LinkageSpec (dss, get_node (n-3), dl, get_node (n-1));
  }
  else // i == n - 1
    return new CT_LinkageSpec (dss, 0, get_node (nodes () - 1), 0);
}

/*****************************************************************************/
/*                                                                           */
/*                    A . 6 . 1   N a m e s p a c e s                        */
/*                                                                           */
/*****************************************************************************/

CTree *CCBuilder::namespace_def () {
  // 1: named_ns_def
  // 1: unamed_ns_def
  return get_node ();
}

CTree *CCBuilder::named_ns_def () {
  // 1: original_ns_def
  // 1: extension_ns_def
  return get_node ();
}

CTree *CCBuilder::orig_namespace_def () {
  // 4: orig_ns_def1  {  namespace_body  }
  CT_MembList *ml;
  ml = (nodes () == 4) ? (CT_MembList*)get_node (2) : new CT_MembList (1);
  ml->AddProperties (CT_List::OPEN_CLOSE);
  ml->PrefixSon (get_node (1));
  ml->AddSon (get_node (nodes ()-1));
  CT_NamespaceDef *nsd = (CT_NamespaceDef*)get_node (0);
  nsd->Members (ml);
  return nsd;
}

CTree *CCBuilder::orig_namespace_def1 () {
  // 2: NAMESPACE  identifier
  // 3: INLINE  NAMESPACE  identifier
  if (nodes() == 3)
    return new CT_NamespaceDef (get_node (0), get_node (1), get_node (2));
  else
    return new CT_NamespaceDef (0, get_node (0), get_node (1));
}

CTree *CCBuilder::namespace_body () {
  // 0: 
  // 1: decl_seq
  CT_MembList *ml;
  if (nodes ()) {
    Container *decls = (Container*)get_node ();
    ml = new CT_MembList (decls->Sons ());
    copy_list (ml, decls);
    delete decls;
  } else
    ml = new CT_MembList (1);
  return ml;
}

CTree *CCBuilder::ns_alias_def () {
  // 5: NAMESPACE  identifier  =  qual_ns_spec  ;
  return new CT_NamespaceAliasDef (get_node (0), get_node (1), get_node (2),
                                   get_node (3), get_node (4));
}

CTree *CCBuilder::qual_ns_spec () {
  // 1: namespace_name
  // 2: nested_name_spec  namespace_name
  // 2: colon_colon  namespace_name
  // 3: colon_colon  nested_name_spec  namespace_name
  int num = nodes ();
  if (num == 1)
    return get_node ();

  CT_QualName *result;
  int root = 0;
  if (get_node (0)->NodeName () != Container::NodeId ()) {
    root = 1;
    if (nodes () == 2) {
      result = new CT_RootQualName (2);
      result->AddSon (get_node (0));
      result->AddSon (get_node (1));
      return result;
    }
  }

  Container *nns = (Container*)get_node (root);
  if (root == 1) {
    result = new CT_RootQualName (nns->Sons ()+2);
    result->AddSon (get_node (0));
  } else 
    result = new CT_QualName (nns->Sons ()+1);
  copy_list (result, nns);
  result->AddSon (get_node (1+root));
  delete nns;
  return result;
}

CTree *CCBuilder::using_decl () {
  // 4: USING            ::                    unqual_id  ;
  // 4: USING                nested_name_spec  unqual_id  ;
  // 5: USING            ::  nested_name_spec  unqual_id  ;
  // 5: USING  TYPENAME      nested_name_spec  unqual_id  ;
  // 6: USING  TYPENAME  ::  nested_name_spec  unqual_id  ;
  CT_QualName *qn;
  int root = 0, num = nodes ();
  if (get_node (1)->token ()->type () == TOK_TYPENAME)
    root++;
  if (get_node (root+1)->NodeName () != Container::NodeId ())
    root = 2;
  if (root == 2 && num == 4) {
    qn = new CT_RootQualName (2);
    qn->AddSon (get_node (1));
    qn->AddSon (get_node (num-2));
  } else {
    Container *nns = (Container*)get_node (num-3);
    if (root == 2) {
      qn = new CT_RootQualName (nns->Sons ()+2);
      qn->AddSon (get_node (num-4));
    } else 
      qn = new CT_QualName (nns->Sons ()+1);
    copy_list (qn, nns);
    qn->AddSon (get_node (num-2));
  }
  if (root && get_node (1)->token ()->type () == TOK_TYPENAME)
    return new CT_UsingDecl (get_node (0), get_node (1), qn, get_node (num-1));
  return new CT_UsingDecl (get_node (0), qn, get_node (num-1));
}

CTree *CCBuilder::using_directive () {
  // 4: USING  NAMESPACE                        namespace_name  ;
  // 5: USING  NAMESPACE  ::                    namespace_name  ;
  // 5: USING  NAMESPACE      nested_name_spec  namespace_name  ;
  // 6: USING  NAMESPACE  ::  nested_name_spec  namespace_name  ;
  CTree *n;
  int root = 0, num = nodes ();
  if (get_node (2)->NodeName () != Container::NodeId ())
    root = 1;
  if (num == 4) {
    n = get_node (2);
  } else if (num == 5 && root) {
    n = new CT_RootQualName (2);
    ((CT_QualName*)n)->AddSon (get_node (2));
    ((CT_QualName*)n)->AddSon (get_node (3));
  } else {
    Container *nns = (Container*)get_node (num-3);
    if (root) {
      n = new CT_RootQualName (nns->Sons ()+2);
      ((CT_QualName*)n)->AddSon (get_node (2));
    } else 
      n = new CT_QualName (nns->Sons ()+1);
    copy_list ((CT_QualName*)n, nns);
    ((CT_QualName*)n)->AddSon (get_node (num-2));
  }
  return new CT_UsingDirective (get_node (0), get_node (1), n, get_node (num-1));
}

/*****************************************************************************/
/*                                                                           */
/*                     A . 7   D e c l a r a t o r s                         */
/*                                                                           */
/*****************************************************************************/

CTree *CCBuilder::declarator () {
  // 1 : direct_declarator
  // 2+: ptr_operator..  direct_declarator
  int num = nodes ();
  CTree *result = get_node (num-1);
  for (int i = num-1; i > 0; i--) {
    // 1: &
    // 1: *
    // 2: *  cv_qual_seq
    // 2: nested_name_spec  *
    // 3: nested_name_spec  *  cv_qual_seq
    // 3: ::  nested_name_spec  *
    // 4: ::  nested_name_spec  *  cv_qual_seq
    Container *p = (Container*)get_node (i-1); // ptr-operator
    int type = p->Son (0)->token ()->type ();
    if (type == TOK_AND)
      result = new CT_RefDeclarator (p->Son (0), result);
    else if (type == TOK_MUL && p->Sons () == 1)
      result = new CT_PtrDeclarator (p->Son (0), 0, result);
    else if (type == TOK_MUL && p->Sons () == 2)
      result = new CT_PtrDeclarator (p->Son (0), p->Son (1), result);
    else {
      int root = (type == TOK_COLON_COLON) ? 1 : 0;
      Container *nns = (Container*)p->Son (root);
      CT_QualName *qn = 0;
      if (root) {
        qn = new CT_RootQualName (nns->Sons ());
        qn->AddSon (p->Son (0));
      } else if (nns->Sons () > 2)
        qn = new CT_QualName (nns->Sons ()-1);

      if (nns->Sons () == 2)
        qn = (CT_QualName*)nns->Son (0);
      else {
        for (int i = 0; i < nns->Sons ()-1; i++)
          qn->AddSon (nns->Son (i));
      }

      result = new CT_MembPtrDeclarator (qn, 
        nns->Son (nns->Sons ()-1), p->Son (root+1), 
        (p->Sons () == (root+3)) ? p->Son (root+2) : 0, result);
      delete nns;
    }
    delete p;
  }
  return result;
}

CTree *CCBuilder::declarator_id () { 
  // 1: id_expr                 => node
  // 1: type_name               => node
  // 2: nest_name type_name     => cont node
  // 2: :: type_name            => node node
  // 3: :: nest_name type_name  => node cont node
  int num = nodes ();
  if (num == 1) 
    return get_node ();
    
  CT_QualName *result;
  int root = (get_node (0)->NodeName () == CT_Token::NodeId ()) ? 1 : 0;
  if ((num-root) == 2) {
    Container *nns = (Container*)get_node (root);
    if (root) {
      result = new CT_RootQualName (nns->Sons ()+2);
      result->AddSon (get_node (0));
    } else
      result = new CT_QualName (nns->Sons ()+1);
    copy_list (result, nns);
    delete nns;
  } else {
    result = new CT_RootQualName (2);
    result->AddSon (get_node (0));
  }
  result->AddSon (get_node (num-1));
  return result;
}

CTree *CCBuilder::param_init () {
  // 2: =  ass_expr
  CT_ExprList *el = new CT_ExprList;
  el->AddSon (get_node (0));
  el->AddSon (get_node (1));
  el->AddProperties (CT_List::INTRO);
  return el;
}

/*****************************************************************************/
/*                                                                           */
/*                          A . 8   C l a s s e s                            */
/*                                                                           */
/*****************************************************************************/

CTree *CCBuilder::class_head1 () {
  // 1: class_key  private_name
  // 2: class_key  identifier
  // 2: class_key  template_id
  // 3: class_key  nested_name_spec  identifier
  // 3: class_key  nested_name_spec  template_id
  // ... base_clause not yet parsed
  CTree *result = get_node (1);
  // nested (qualified) name
  if (result->NodeName () == Container::NodeId ()) { 
    Container *nns = (Container*)result;
    CT_QualName *qn = new CT_QualName (nns->Sons ()+1);
    copy_list (qn, nns);
    qn->AddSon (get_node (2));
    result = qn;
  } 
  if (get_node (0)->token ()->type () == TOK_UNION)
    result = new CT_UnionDef (get_node (0), result, 0);
  else 
    result = new CT_ClassDef (get_node (0), result, 0);
  return result;
}

CTree *CCBuilder::class_head () {
  // 1: class_head1
  // 2: class_head1 base_clause
  if (nodes () == 2)
    ((CT_ClassDef*)get_node ())->BaseClasses (get_node (1));
  return get_node ();
}

CTree *CCBuilder::class_spec () {
  // 3: class_head  {  }
  // 4: class_head  {  member_spec  }
  int num = nodes ();
  CT_MembList *ml = (num == 4) ? 
    (CT_MembList*)get_node (2) : new CT_MembList;
  ml->PrefixSon (get_node (1));
  ml->AddSon (get_node (num-1));
  ml->AddProperties (CT_List::OPEN_CLOSE);
  CT_ClassDef *result = (CT_ClassDef*)get_node (0);
  result->Members (ml);
  return result;
}

CTree *CCBuilder::member_decl () { 
  // 1: member_decl1
  // 1: fct_def
  // 1: access_decl
  // 2: access_spec  :
  if (nodes () == 1) 
    return get_node ();
  else
    return new CT_AccessSpec (get_node (0), get_node (1));
}

CTree *CCBuilder::member_decl1 () { 
  // 1: ;
  // 2: decl_spec_seq                          ;
  // 2:                member_declarator_list  ;
  // 3: decl_spec_seq  member_declarator_list  ;
  int num = nodes ();
  if (num == 1)
    return new CT_ObjDecl (new CT_DeclSpecSeq, new CT_DeclaratorList, get_node (0));
  else if (num == 2) {
    if (get_node (0)->NodeName () == CT_DeclSpecSeq::NodeId ())
      return new CT_ObjDecl (get_node (0), new CT_DeclaratorList, get_node (1));
    else
      return new CT_ObjDecl (new CT_DeclSpecSeq, get_node (0), get_node (1));
  } else // if (num == 3)
    return new CT_ObjDecl (get_node (0), get_node (1), get_node (2));
}

CTree *CCBuilder::access_decl () { 
  // 3:     nested_name_spec                unqual_id  ;
  // 4: ::  nested_name_spec                unqual_id  ;
  // 4:     nested_name_spec  template_key  unqual_id  ;
  // 5: ::  nested_name_spec  template_key  unqual_id  ;
  int num = nodes ();
  int root = (get_node ()->NodeName () != Container::NodeId ()) ? 1 : 0;
  Container *nns = (Container*)get_node (root);
  CT_QualName *qn;
  if (root) {
    qn = new CT_RootQualName (nns->Sons ()+2);
    qn->AddSon (get_node (0));
  } else
    qn = new CT_QualName (nns->Sons ()+1);
  copy_list (qn, nns);
  CT_SimpleName *sn = (CT_SimpleName*)get_node (num-2);
  // name prefixed by `template'
  if (get_node (num-3)->token ()->type () == TOK_TEMPLATE)
    sn->PrefixSon (get_node (num-3));
  qn->AddSon (sn);
  delete nns;
  return new CT_AccessDecl (qn, get_node (num-1));
}

CTree *CCBuilder::member_declarator () { 
  // 3: identifier  :  const_expr
  // 3: private_name  :  const_expr
  // 1: declarator
  // 2: declarator  pure_spec
  // 2: declarator  const_init
  int num = nodes ();
  bool bitfield = (num >= 2 && get_node (1)->token ()->type () == TOK_COLON);
  if (bitfield) {
    if (num == 3)
      return new CT_BitFieldDeclarator (get_node (0), get_node (1), get_node (2));
    else if (num == 2)
      return new CT_BitFieldDeclarator (get_node (0), get_node (1));
  }
  else {
    if (num == 1)
      return new CT_InitDeclarator (get_node (0));
    else
      return new CT_InitDeclarator (get_node (0), get_node (1));
  }
  return 0;
}

CTree *CCBuilder::pure_spec () { 
  CT_ExprList *el = new CT_ExprList;
  el->AddSon (get_node (0));
  el->AddSon (new CT_Integer (get_node (1)));
  el->AddProperties (CT_List::INTRO);
  return el;
}

CTree *CCBuilder::const_init () { 
  CT_ExprList *el = new CT_ExprList;
  el->AddSon (get_node (0));
  el->AddSon (get_node (1));
  el->AddProperties (CT_List::INTRO);
  return el;
}

/*****************************************************************************/
/*                                                                           */
/*                   A . 9   D e r i v e d  c l a s s e s                    */
/*                                                                           */
/*****************************************************************************/

CTree *CCBuilder::base_clause () { 
  // 2: :  base_spec_list
  CT_BaseSpecList *bsl = (CT_BaseSpecList*)get_node (1);
  bsl->PrefixSon (get_node (0));
  return bsl;
}

CTree *CCBuilder::base_spec_list () { 
  return list (new CT_BaseSpecList);
}

CTree *CCBuilder::base_spec () { 
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
  int num = nodes (), i, offset = 0;
  CTree *t, *v, *a = t = v = (CTree*)0;
  CTree *n = get_node (num-1);
  if (num > 1) {
    if ((i = 0, get_node (i)->token ()->type () == TOK_VIRTUAL) ||
        (i = 1, get_node (i)->token ()->type () == TOK_VIRTUAL))
      v = get_node (i);
    if ((i = 0, get_node (i)->token ()->type () == TOK_PUBLIC ||
                get_node (i)->token ()->type () == TOK_PRIVATE ||
                get_node (i)->token ()->type () == TOK_PROTECTED) ||
        (i = 1, get_node (i)->token ()->type () == TOK_PUBLIC ||
                get_node (i)->token ()->type () == TOK_PRIVATE ||
                get_node (i)->token ()->type () == TOK_PROTECTED))
      a = get_node (i);
    if (get_node (num-2)->token ()->type () == TOK_TEMPLATE) {
      // name prefixed by `template'
      offset = 1;
      ((CT_SimpleName*)n)->PrefixSon (get_node (num-2));
    }
    
    CT_QualName *qn;
    if (get_node (num-2-offset)->NodeName () == Container::NodeId ()) {
      Container *nns = (Container*)get_node (num-2-offset);
      if (num > (2+offset) && get_node (num-3-offset)->token ()->type () == TOK_COLON_COLON) {
        qn = new CT_RootQualName (nns->Sons ()+2);
        qn->AddSon (get_node (num-3-offset));
      } else
        qn = new CT_QualName (nns->Sons ()+1);
      copy_list (qn, nns);
      qn->AddSon (n);
      delete nns;
      n = qn;
    } else if (get_node (num-2-offset)->token ()->type () == TOK_COLON_COLON) {
      qn = new CT_RootQualName (2);
      qn->AddSon (get_node (num-2-offset));
      qn->AddSon (n);
      n = qn;
    }
  }
  return new CT_BaseSpec (v, a, n);
}

CTree *CCBuilder::access_spec () { 
  return get_node ();
}

/*****************************************************************************/
/*                                                                           */
/*        A . 1 0   S p e c i a l  m e m b e r  f u n c t i o n s            */
/*                                                                           */
/*****************************************************************************/

CTree *CCBuilder::conv_fct_id () { 
  // 2: OPERATOR  conv_type_id
  return new CT_ConversionName (get_node (0), get_node (1));
}

CTree *CCBuilder::ctor_init () { 
  // 2: :  mem_init_list
  CT_MembInitList *mil = (CT_MembInitList*)get_node (1);
  mil->PrefixSon (get_node (0));
  return mil;
}

CTree *CCBuilder::mem_init_list () { 
  return list (new CT_MembInitList);
}

CTree *CCBuilder::mem_init () { 
  // 3: mem_init_id  (  )
  // 4: mem_init_id  (  expr_list  )
  int num = nodes ();
  CT_ExprList *el = (num == 4) ? (CT_ExprList*)get_node (2) : new CT_ExprList;
//  if (num != 4)
//    Push (el);
  el->PrefixSon (get_node (1));
  el->AddSon (get_node (num-1));
  el->AddProperties (CT_List::OPEN_CLOSE);
  return new CT_MembInit (get_node (0), el);
}

CTree *CCBuilder::mem_init_id () { 
  // 1: identifier
  // 1: class_name
  // 2: nested_name_spec  class_name
  // 2: ::  class_name
  // 3: ::  nested_name_spec  class_name
  int num = nodes ();
  if (num == 1)
    return get_node ();
  CT_QualName *result;
  if (get_node (num-2)->NodeName () == Container::NodeId ()) {
    Container *nns = (Container*)get_node (num-2);
    if (num == 3) {
      result = new CT_RootQualName (nns->Sons ()+2);
      result->AddSon (get_node (0));
    } else
      result = new CT_QualName (nns->Sons ()+1);
    copy_list (result, nns);
    delete nns;
  } else {
    result = new CT_RootQualName (2);
    result->AddSon (get_node (0));
  }
  result->AddSon (get_node (num-1));
  return result;
}

/*****************************************************************************/
/*                                                                           */
/*                     A . 1 1   O v e r l o a d i n g                       */
/*                                                                           */
/*****************************************************************************/

CTree *CCBuilder::oper_fct_id () { 
  // 2: OPERATOR  any_operator
  // 2: OPERATOR  NEW
  // 2: OPERATOR  DELETE
  // 3: OPERATOR  (  )
  // 3: OPERATOR  [  ]
  // 4: OPERATOR  NEW  [  ]
  // 4: OPERATOR  DELETE  [  ]
  int num = nodes ();
  if (num == 2)
    return new CT_OperatorName (get_node (0), get_node (1), 0, 0);
  else if (num == 3)
    return new CT_OperatorName (get_node (0), 0, get_node (1), get_node (2));
  else // if (num == 4)
    return new CT_OperatorName (get_node (0), get_node (1), get_node (2), 
                                get_node (3));
}

/*****************************************************************************/
/*                                                                           */
/*                       A . 1 2   T e m p l a t e s                         */
/*                                                                           */
/*****************************************************************************/

CTree *CCBuilder::template_decl () {
  // 5: TEMPLATE  <  template_param_list  >  decl
  // 6: EXPORT  TEMPLATE  <  template_param_list  >  decl
  int offset = (nodes () == 6) ? 1 : 0;
  CT_TemplateParamList *pl = (CT_TemplateParamList*) get_node (2 + offset);
  pl->AddSon (get_node (3 + offset));
  pl->PrefixSon (get_node (1 + offset));
  pl->PrefixSon (get_node (0 + offset));
  return new CT_TemplateDecl (offset ? get_node (0) : (CTree*)0,
                              pl, get_node (4 + offset));
}

CTree *CCBuilder::template_param_list () {
  // 1 : template_param
  // 3+: template_param  ,  template_param ..
  return list (new CT_TemplateParamList);
}

CTree *CCBuilder::template_param () {
  // 1: type_param
  // 1: non_type_param
  return get_node ();
}

CTree *CCBuilder::type_param () {
  // 2: CLASS  identifier
  // 2: TYPENAME  identifier
  // 4: CLASS  identifier  =  type_id
  // 4: TYPENAME  identifier  =  type_id
  // 6: TEMPLATE  <  template_param_list  >  CLASS  identifier
  // 8: TEMPLATE  <  template_param_list  >  CLASS  identifier  =  id_expr
  CT_TemplateParamList *pl = (CT_TemplateParamList*)0;
  if (nodes () >= 6) {
    pl = (CT_TemplateParamList*)get_node (2);
    pl->AddSon (get_node (3));
    pl->PrefixSon (get_node (1));
    pl->PrefixSon (get_node (0));
  }
  int num = nodes ();
  if (num == 2 || num == 6)
    return new CT_TypeParamDecl (pl, get_node (num-2), get_node (num-1)); 
  CT_ExprList *el = new CT_ExprList;
  el->AddSon (get_node (num-2));
  el->AddSon (get_node (num-1));
  el->AddProperties (CT_List::INTRO);
  return new CT_TypeParamDecl (pl, get_node (num-4), get_node (num-3), el); 
}

CTree *CCBuilder::non_type_param (CTree *result) {
  // 2: decl_spec_seq  private_name
  // 2: decl_spec_seq  abst_declarator
  // 2: decl_spec_seq  declarator
  // 3: decl_spec_seq  private_name  param_init
  // 3: decl_spec_seq  abst_declarator  param_init
  // 3: decl_spec_seq  declarator  param_init
  if (! result)
    result = new CT_NonTypeParamDecl (get_node (0), get_node (1));
  else if (nodes () == 3)
    ((CT_NonTypeParamDecl*)result)->Initializer (get_node (2));
  return result;
}

CTree *CCBuilder::template_id () {
  // 3: template_name  <  >
  // 4: template_name  <  template_arg_list  >
  CT_TemplateArgList *tal;
  if (nodes () == 3) 
    tal = new CT_TemplateArgList;
  else
    tal = (CT_TemplateArgList*)get_node (2);
  tal->PrefixSon (get_node (1));
  tal->AddSon (get_node (nodes ()-1));
  return new CT_TemplateName (get_node (0), tal);
}

CTree *CCBuilder::template_arg_list () {
  // 1 : template_arg
  // 3+: template_arg  ,  template_arg ..
  return list (new CT_TemplateArgList);
}

CTree *CCBuilder::template_arg () {
  // 1: template_type_arg
  // 1: template_non_type_arg
  // 1: template_template_arg
  return get_node ();
}

CTree *CCBuilder::template_type_arg () {
  // 1: type_id
  return get_node ();
}

CTree *CCBuilder::template_non_type_arg () {
  // 1: ass_expr
  return get_node ();
}

CTree *CCBuilder::template_template_arg () {
  // 1: id_expr
  return get_node ();
}

CTree *CCBuilder::explicit_instantiation () {
  // 2: TEMPLATE  decl
  // 3: EXTERN  TEMPLATE  decl  => GCC EXTENSION!!!
  CT_TemplateParamList *tpl = new CT_TemplateParamList;
  tpl->AddSon (get_node (nodes ()-2));
  if (nodes () == 3) {
    CT_DeclSpecSeq *dss = new CT_DeclSpecSeq ();
    dss->AddSon (get_node (0));
    return new CT_LinkageSpec (dss, 0,
      new CT_TemplateDecl (0, tpl, get_node (nodes ()-1)), 0);
  }
  return new CT_TemplateDecl (0, tpl, get_node (nodes ()-1));
}

CTree *CCBuilder::explicit_specialization () {
  // 4: TEMPLATE  <  >  decl
  CT_TemplateParamList *tpl = new CT_TemplateParamList;
  tpl->AddSon (get_node (0));
  tpl->AddSon (get_node (1));
  tpl->AddSon (get_node (2));
  return new CT_TemplateDecl (0, tpl, get_node (3));
}

/*****************************************************************************/
/*                                                                           */
/*                A . 1 3  E x c e p t i o n  h a n d l i n g                */
/*                                                                           */
/*****************************************************************************/

CTree *CCBuilder::try_block () { 
  // 3: TRY  cmpd_stmt  handler_seq
  return new CT_TryStmt (get_node (0), get_node (1), get_node (2));
}

CTree *CCBuilder::fct_try_block () { 
  // 3+: TRY  fct_body  handler_seq
  // 4+: TRY  ctor_init  fct_body  handler_seq
  return container ();
}

CTree *CCBuilder::handler_seq () { 
  // 1+: handler..
  return list (new CT_HandlerSeq);
}

CTree *CCBuilder::handler () { 
  // 5: CATCH  (  exception_decl  )  cmpd_stmt
  CT_ArgDeclList *adl = new CT_ArgDeclList (3);
  adl->AddSon (get_node (1));
  adl->AddSon (get_node (2));
  adl->AddSon (get_node (3));
  return new CT_Handler (get_node (0), adl, get_node (4));
}

CTree *CCBuilder::exception_decl () { 
  // 1: ...
  // 1: type_spec_seq 
  // 2: type_spec_seq  abst_declarator
  // 2: type_spec_seq  declarator
  if (nodes () == 2)
    return new CT_ArgDecl (get_node (0), get_node (1));
  return new CT_ArgDecl (get_node (0));
}

CTree *CCBuilder::throw_expr () { 
  // 1: THROW
  // 2: THROW  ass_expr
  if (nodes () == 1)
    return new CT_ThrowExpr (get_node (0));
  return new CT_ThrowExpr (get_node (0), get_node (1));
}

CTree *CCBuilder::exception_spec () { 
  // 3: THROW  (  )
  // 4: THROW  (  ...  )
  // 4: THROW  (  type_id_list  )
  CT_ArgDeclList *adl;
  if (nodes () == 3 || get_node (2)->NodeName () != CT_ArgDeclList::NodeId ()) {
    adl = new CT_ArgDeclList;
    if (nodes () == 4)
      adl->AddSon (get_node (2));
  } else
    adl = (CT_ArgDeclList*)get_node (2);  
  adl->PrefixSon (get_node (1));
  adl->AddSon (get_node (nodes ()-1));
  return new CT_ExceptionSpec (get_node (0), adl);
}

CTree *CCBuilder::type_id_list () { 
  // 1 : type_id
  // 3+: type_id  ,  type_id ..
  return list (new CT_ArgDeclList (3));
}


} // namespace Puma
