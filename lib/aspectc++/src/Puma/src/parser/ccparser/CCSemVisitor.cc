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

#include "Puma/CCSemVisitor.h"
#include "Puma/CCSemExpr.h"
#include "Puma/CStructure.h"
#include "Puma/CLocalScope.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CAttributeInfo.h"
#include "Puma/CRecord.h"

namespace Puma {


// print semantic error messages

#define SEM_MSG(node__,mesg__) \
  err << node__->token ()->location () << mesg__ << endMessage
  
#define SEM_ERROR(node__,mesg__) \
  SEM_MSG (node__, sev_error << mesg__)

#define SEM_WARNING(node__,mesg__) \
  SEM_MSG (node__, sev_warning << mesg__)


void CCSemVisitor::run (CTree *node, CStructure *scope) {
  current_scope = scope;
  if (! current_scope && node->NodeName () == CT_Program::NodeId ())
    current_scope = ((CT_Program*)node)->Scope ();
  if (current_scope)
    visit (node);
}


CTypeInfo *CCSemVisitor::resolveExpr (CTree *expr, CTree *node) const {
  CCSemExpr cse (err, current_scope);
  return cse.resolveExpr (expr, node);
}


CTypeInfo *CCSemVisitor::resolveInit (CTree *expr, CTree *node) const {
  CCSemExpr cse (err, current_scope);
  return cse.resolveInit (expr, node);
}


void CCSemVisitor::pre_visit (CTree *node) {
  if (! node) 
    return;

  const char *id = node->NodeName ();
  if (id == CT_Program::NodeId ())
    pre_action ((CT_Program*)node);
  else if (id == CT_ArgDeclList::NodeId ())
    pre_action ((CT_ArgDeclList*)node);
  else if (id == CT_ArgNameList::NodeId ())
    pre_action ((CT_ArgNameList*)node);
  else if (id == CT_ArgDeclSeq::NodeId ())
    pre_action ((CT_ArgDeclSeq*)node);
  else if (id == CT_MembList::NodeId ())
    pre_action ((CT_MembList*)node);
  else if (id == CT_MembInitList::NodeId ())
    pre_action ((CT_MembInitList*)node);
  else if (id == CT_MembInit::NodeId ())
    pre_action ((CT_MembInit*)node);
  else if (id == CT_CmpdStmt::NodeId ())
    pre_action ((CT_CmpdStmt*)node);
  else if (id == CT_IfStmt::NodeId ())
    pre_action ((CT_IfStmt*)node);
  else if (id == CT_IfElseStmt::NodeId ())
    pre_action ((CT_IfElseStmt*)node);
  else if (id == CT_SwitchStmt::NodeId ())
    pre_action ((CT_SwitchStmt*)node);
  else if (id == CT_ForStmt::NodeId ())
    pre_action ((CT_ForStmt*)node);
  else if (id == CT_WhileStmt::NodeId ())
    pre_action ((CT_WhileStmt*)node);
  else if (id == CT_DoStmt::NodeId ())
    pre_action ((CT_DoStmt*)node);
  else if (id == CT_Handler::NodeId ())
    pre_action ((CT_Handler*)node);
  else if (id == CT_TemplateDecl::NodeId ())
    pre_action ((CT_TemplateDecl*)node);
  else if (id == CT_ExprStmt::NodeId ())
    pre_action ((CT_ExprStmt*)node);
  else if (id == CT_CaseStmt::NodeId ())
    pre_action ((CT_CaseStmt*)node);
  else if (id == CT_GotoStmt::NodeId ())
    pre_action ((CT_GotoStmt*)node);
  else if (id == CT_ReturnStmt::NodeId ())
    pre_action ((CT_ReturnStmt*)node);
  else if (id == CT_InitDeclarator::NodeId ())
    pre_action ((CT_InitDeclarator*)node);
  else if (id == CT_ArgDecl::NodeId ())
    pre_action ((CT_ArgDecl*)node);
  else if (id == CT_Condition::NodeId ())
    pre_action ((CT_Condition*)node);
  else if (id == CT_ArrayDeclarator::NodeId ())
    pre_action ((CT_ArrayDeclarator*)node);
  else if (id == CT_BitFieldDeclarator::NodeId ())
    pre_action ((CT_BitFieldDeclarator*)node);
  else if (id == CT_CmpdLiteral::NodeId ())
    pre_action ((CT_CmpdLiteral*)node);
  else if (id == CT_ExprList::NodeId ())
    pre_action ((CT_ExprList*)node);
}


void CCSemVisitor::post_visit (CTree *node) {
  if (! node) 
    return;

  const char *id = node->NodeName ();
  if (id == CT_Program::NodeId ())
    post_action ((CT_Program*)node);
  else if (id == CT_ArgDeclList::NodeId ())
    post_action ((CT_ArgDeclList*)node);
  else if (id == CT_ArgNameList::NodeId ())
    post_action ((CT_ArgNameList*)node);
  else if (id == CT_ArgDeclSeq::NodeId ())
    post_action ((CT_ArgDeclSeq*)node);
  else if (id == CT_MembList::NodeId ())
    post_action ((CT_MembList*)node);
  else if (id == CT_MembInitList::NodeId ())
    post_action ((CT_MembInitList*)node);
  else if (id == CT_CmpdStmt::NodeId ())
    post_action ((CT_CmpdStmt*)node);
  else if (id == CT_IfStmt::NodeId ())
    post_action ((CT_IfStmt*)node);
  else if (id == CT_IfElseStmt::NodeId ())
    post_action ((CT_IfElseStmt*)node);
  else if (id == CT_SwitchStmt::NodeId ())
    post_action ((CT_SwitchStmt*)node);
  else if (id == CT_ForStmt::NodeId ())
    post_action ((CT_ForStmt*)node);
  else if (id == CT_WhileStmt::NodeId ())
    post_action ((CT_WhileStmt*)node);
  else if (id == CT_Handler::NodeId ())
    post_action ((CT_Handler*)node);
  else if (id == CT_TemplateDecl::NodeId ())
    post_action ((CT_TemplateDecl*)node);
}


///////////////////////////////////////////////////////
// pre_action functions ///////////////////////////////
///////////////////////////////////////////////////////


void CCSemVisitor::pre_action (CT_Program *node) {
  if (node->Scope ())
    current_scope = node->Scope ();
}


void CCSemVisitor::pre_action (CT_ArgDeclList *node) {
  if (node->Scope ())
    current_scope = node->Scope ();
}


void CCSemVisitor::pre_action (CT_ArgNameList *node) {
  if (node->Scope ())
    current_scope = node->Scope ();
}


void CCSemVisitor::pre_action (CT_ArgDeclSeq *node) {
  if (node->Scope ())
    current_scope = node->Scope ();
}


void CCSemVisitor::pre_action (CT_MembList *node) {
  if (node->Scope ())
    current_scope = node->Scope ();
}


void CCSemVisitor::pre_action (CT_MembInitList *node) {
  if (node->Scope ())
    current_scope = node->Scope ();
}


void CCSemVisitor::pre_action (CT_MembInit *node) {
  CT_ExprList *init = node->Initializer ();
  if (init) {
    unsigned entries = init->Entries ();
    // resolve initializer arguments
    for (unsigned i = 0; i < entries; i++) 
      resolveExpr (init->Entry (i), init);
  }
  prune ();
}


void CCSemVisitor::pre_action (CT_CmpdStmt *node) {
  if (node->Scope ())
    current_scope = node->Scope ();
}


void CCSemVisitor::pre_action (CT_IfStmt *node) {
  CTypeInfo *type;
  CTree *expr;
  
  if (node->Scope ())
    current_scope = node->Scope ();

  // evaluation not yet complete!  

  expr = node->Condition ();
  if (expr) {
    if (expr->NodeName () != CT_Condition::NodeId ()) {
      type = resolveExpr (expr, node);
//      if (! type->isScalar ())
//        SEM_ERROR (node, "controlling expression of `if' statement not scalar");
    }
  }
}


void CCSemVisitor::pre_action (CT_IfElseStmt *node) {
  CTypeInfo *type;
  CTree *expr;
  
  if (node->Scope ())
    current_scope = node->Scope ();
  
  // evaluation not yet complete!  

  expr = node->Condition ();
  if (expr) {
    if (expr->NodeName () != CT_Condition::NodeId ()) {
      type = resolveExpr (expr, node);
//      if (! type->isScalar ())
//        SEM_ERROR (node, "controlling expression of `if' statement not scalar");
    }
  }
}


void CCSemVisitor::pre_action (CT_SwitchStmt *node) {
  CTypeInfo *type;
  CTree *expr;
  
  if (node->Scope ())
    current_scope = node->Scope ();
  
  // evaluation not yet complete!  

  expr = node->Condition ();
  if (expr) {
    if (expr->NodeName () != CT_Condition::NodeId ()) {
      type = resolveExpr (expr, node);
//      type = promote (node->Condition (), node);
//      if (! type->isInteger ())
//        SEM_ERROR (node, 
//          "controlling expression of `switch' statement not an integer");
    }
  }
}


void CCSemVisitor::pre_action (CT_ForStmt *node) {
  CTypeInfo *type;
  CTree *expr;
  
  if (node->Scope ())
    current_scope = node->Scope ();
  
  // evaluation not yet complete!  

  expr = node->Condition ();
  if (expr) {
    if (expr->NodeName () != CT_Condition::NodeId ()) {
      type = resolveExpr (expr, node);
//      if (! type->isScalar ())
//        SEM_ERROR (node, "controlling expression of `for' statement not scalar");
    }
  }
  expr = node->Expr ();
  if (expr)
    // there are no further constraints on this expression
    resolveExpr (expr, node);
}


void CCSemVisitor::pre_action (CT_WhileStmt *node) {
  CTypeInfo *type;
  CTree *expr;
  
  if (node->Scope ())
    current_scope = node->Scope ();
  
  // evaluation not yet complete!  

  expr = node->Condition ();
  if (expr) {
    if (expr->NodeName () != CT_Condition::NodeId ()) {
      type = resolveExpr (expr, node);
//      if (! type->isScalar ())
//        SEM_ERROR (node, "controlling expression of `while' statement not scalar");
    }
  }
}


void CCSemVisitor::pre_action (CT_Handler *node) {
  if (node->Scope ())
    current_scope = node->Scope ();
}


void CCSemVisitor::pre_action (CT_TemplateDecl *node) {
  if (node->Scope ())
    current_scope = node->Scope ();
}


void CCSemVisitor::pre_action (CT_DoStmt *node) {
  CTypeInfo *type;
  CTree *expr;
  
  // evaluation not yet complete!  

  expr = node->Expr ();
  if (expr) {
    type = resolveExpr (expr, node);
//    if (! type->isScalar ())
//      SEM_ERROR (node, "controlling expression of `do' statement not scalar");
  }
}


void CCSemVisitor::pre_action (CT_ExprStmt *node) {
  if (node->Expr ())
    resolveExpr (node->Expr (), node);
  prune ();
}


void CCSemVisitor::pre_action (CT_CaseStmt *node) {
  if (node->Expr ()) {
//    if (! isConstIntExpr (node->Expr ()))
//      SEM_ERROR (node, "label of `case' statement not a valid integer constant");
  }
}


void CCSemVisitor::pre_action (CT_GotoStmt *node) {
  if (! current_scope->LocalScope ())
    SEM_ERROR (node, "`goto' outside of function");
  else
    check_goto_label (node->Label ());
}

void CCSemVisitor::check_goto_label (CTree *tree) {
  CFunctionInfo *finfo;
  const char *name;
  
  finfo = current_scope->LocalScope ()->Function ();
  if (finfo) {
    CT_SimpleName *label = (CT_SimpleName *)tree;
    name = label->Text ();
    if (! finfo->Label (name))
      SEM_ERROR (label, "label `" << name << "' used but not defined");
  }
}

void CCSemVisitor::pre_action (CT_ReturnStmt *node) {
  CFunctionInfo *finfo = 0;
  CTypeInfo *t2 = 0, *t1 = 0;
//  CConstant *v = 0;
//  CTree *expr = 0;

  // evaluation not yet complete!

  if (! current_scope->LocalScope ())
    SEM_ERROR (node, "`return' outside of function");
  else {
    finfo = current_scope->LocalScope ()->Function ();
    if (finfo)
      t1 = finfo->TypeInfo ()->ReturnType ();
    else
      t1 = &CTYPE_UNDEFINED;
  }

  if (node->Expr ()) {
    t2 = resolveExpr (node->Expr (), node);
    if (t1->isVoid ()) {
      if (t2 && ! (t2->isVoid () || t2->is_undefined ()))
        SEM_WARNING (node, "`return' with a value, in function returning void");
//   } else {
//      expr = node->Expr ();
//      v = expr->Value () ? expr->Value ()->Constant () : 0;
//      if (t1->isScalar ()) {
//        if (t1->isArithmetic () && t2->isArithmetic ()) {
//          if (! compatible (t1, t2))
//            cast_to (t1, expr, node);
//        } else if (! (t1->isPointer () && t2->isPointer () && 
//                      (compatibleBase (t1, t2) ||
//                        t1->VirtualType ()->BaseType ()->isVoid () || 
//                       t2->VirtualType ()->BaseType ()->isVoid ()) &&
//                      (t2->isConst () ? t1->isConst () : true) &&
//                      (t2->isVolatile () ? t1->isVolatile () : true) &&
//                      (t2->isRestrict () ? t1->isRestrict () : true)) &&
//                   ! (t1->isPointer () && t2->isInteger () && v && v->isNull ()) &&
//                   ! (t1->VirtualType ()->is_bool () && t2->isPointer ())) {
//          SEM_ERROR (node, "incompatible types in `return' statement");
//        } else if (! compatible (t1, t2))
//          cast_to (t1, expr, node);
//      } else if (t1->isRecord ()) {
//        if (! compatible (t1, t2))
//          SEM_ERROR (node, "incompatible types in `return' statement");
//      } else
//        SEM_ERROR (node, "invalid return type");
//    }
    }
  } else if (t1 && ! (t1->isVoid () || t1->is_undefined ())) 
    SEM_WARNING (node, "`return' with no value, in function returning non-void");

  prune ();
}


void CCSemVisitor::pre_action (CT_InitDeclarator *node) {
  CObjectInfo *info;
  CStructure *scope;
  CTree *init;

  scope = current_scope;
  info = node->Object ();

  // �8.5.10 the initializer of a static member
  // is in the scope of the member's class
  if (info && info->AttributeInfo () && info->isClassMember () &&
      ! info->Scope ()->isRecord ())
    current_scope = info->AttributeInfo ()->Record ();
  
  init = node->Initializer ();
  if (init && (! init->Type () || init->Type ()->is_undefined ()))
    resolveInit (init, node);
    
  current_scope = scope;
  prune ();
}


void CCSemVisitor::pre_action (CT_ArgDecl *node) {
  CTree *init = node->Initializer ();
  if (init)
    resolveInit (init, node);
  prune ();
}


void CCSemVisitor::pre_action (CT_Condition *node) {
  // TODO: evaluation not complete! => type conversions etc.
  CTree *init = node->Initializer ();
  if (init)
    resolveInit (init, node);
  prune ();
}


void CCSemVisitor::pre_action (CT_ArrayDeclarator *node) {
  // error diagnostics not yet implemented!

//  CT_SimpleName *name;
//  const char *array;
//  CTypeInfo *type;
//  CConstant *v;
//  CTree *expr;
//  
//  expr = node->Delimiter ()->Expr ();
//  if (expr) {
//    name = findName (node);
//    array = name ? name->Text () : "";
//    type = resolveExpr (expr, node->Delimiter ());
//    expr = node->Delimiter ()->Expr ();
//    
//    if (! type->isInteger ())
//      SEM_ERROR (node, "size of array `" << array << "' has non-integer type");
//    else if (isConstIntExpr (expr)) {
//      v = expr->Value ()->Constant ();
//      if (/*v->isNull () ||*/ v->isNegative ())
//        SEM_ERROR (node, "array `" << array << "' has invalid size");
//      else if (node->Type ()->TypeArray ())
//        node->Type ()->TypeArray ()->Dimension (v->convert_to_uint ());
//    }
//  }
}


void CCSemVisitor::pre_action (CT_BitFieldDeclarator *node) {
  // error diagnostics not yet implemented!

//  CObjectInfo *info;
//  const char *name;
//  CConstant *v;
//  CTree *expr;
//  
//  expr = node->Expr ();
//  if (expr) {
//    resolveExpr (expr, node);
//    expr = node->Expr ();
//    info = node->Object ();
//    name = info ? info->Name () : "";
//    
//    if (! isConstIntExpr (expr)) 
//      SEM_ERROR (node, "width of bit-field `" << name
//        << "' isn't integer constant expression");
//    else {
//      v = expr->Value ()->Constant ();
//      if (v->isNegative ())
//        SEM_ERROR (node, "negative width in bit-field `" << name << "'");
//      else if (info && info->TypeInfo () && info->TypeInfo ()->TypeBitField ())
//        info->TypeInfo ()->TypeBitField ()->Dimension (v->convert_to_uint ());
//    }
//  }
}


void CCSemVisitor::pre_action (CT_ExprList *node) {
  // do not visit the expression list 
  // it's evaluated when the init declarator is visited
  prune ();
}


void CCSemVisitor::pre_action (CT_CmpdLiteral *node) {
  // do not visit the compound literal
  // it's evaluated when the init declarator is visited
  prune ();
}


///////////////////////////////////////////////////////
// post_action functions //////////////////////////////
///////////////////////////////////////////////////////


void CCSemVisitor::post_action (CT_Program *node) {
  if (node->Scope ())
    current_scope = node->Scope ();
}


void CCSemVisitor::post_action (CT_ArgDeclList *node) {
  if (node->Scope ())
    current_scope = node->Scope ()->Parent ()->Structure ();
}


void CCSemVisitor::post_action (CT_ArgNameList *node) {
  if (node->Scope ())
    current_scope = node->Scope ()->Parent ()->Structure ();
}


void CCSemVisitor::post_action (CT_ArgDeclSeq *node) {
  if (node->Scope ())
    current_scope = node->Scope ()->Parent ()->Structure ();
}


void CCSemVisitor::post_action (CT_MembList *node) {
  if (node->Scope ())
    current_scope = node->Scope ()->Parent ()->Structure ();
}


void CCSemVisitor::post_action (CT_MembInitList *node) {
  if (node->Scope ())
    current_scope = node->Scope ()->Parent ()->Structure ();
}


void CCSemVisitor::post_action (CT_CmpdStmt *node) {
  if (node->Scope ())
    current_scope = node->Scope ()->Parent ()->Structure ();
}


void CCSemVisitor::post_action (CT_IfStmt *node) {
  if (node->Scope ())
    current_scope = node->Scope ()->Parent ()->Structure ();
}


void CCSemVisitor::post_action (CT_IfElseStmt *node) {
  if (node->Scope ())
    current_scope = node->Scope ()->Parent ()->Structure ();
}


void CCSemVisitor::post_action (CT_SwitchStmt *node) {
  if (node->Scope ())
    current_scope = node->Scope ()->Parent ()->Structure ();
}


void CCSemVisitor::post_action (CT_ForStmt *node) {
  if (node->Scope ())
    current_scope = node->Scope ()->Parent ()->Structure ();
}


void CCSemVisitor::post_action (CT_WhileStmt *node) {
  if (node->Scope ())
    current_scope = node->Scope ()->Parent ()->Structure ();
}


void CCSemVisitor::post_action (CT_Handler *node) {
  if (node->Scope ())
    current_scope = node->Scope ()->Parent ()->Structure ();
}


void CCSemVisitor::post_action (CT_TemplateDecl *node) {
  // visit template instance code
  if (node->Scope ()) {
    current_scope = node->Scope ();
    
    /*CTemplateInfo *tinfo = current_scope->TemplateInfo ();
    if (tinfo) {
      for (unsigned i = 0; i < tinfo->Instances (); i++) {
        CObjectInfo *instance = tinfo->Instance (i);
        CTree *tree = instance->Tree ();
        if (tree) {
          visit (tree);
        }
      }
    }*/
  }

  if (node->Scope ())
    current_scope = node->Scope ()->Parent ()->Structure ();
}


} // namespace Puma
