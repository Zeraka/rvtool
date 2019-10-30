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

#include "Puma/CAttributeInfo.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CArgumentInfo.h"
#include "Puma/CTypedefInfo.h"
#include "Puma/CWStrLiteral.h"
#include "Puma/CStrLiteral.h"
#include "Puma/CObjectInfo.h"
#include "Puma/CSemVisitor.h"
#include "Puma/CLocalScope.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CStructure.h"
#include "Puma/CClassInfo.h"
#include "Puma/CScopeInfo.h"
#include "Puma/CEnumInfo.h"
#include "Puma/CFileInfo.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CConstant.h"
#include "Puma/CSemExpr.h"
#include "Puma/Limits.h"
#include "Puma/CTree.h"

namespace Puma {


// print semantic error messages

#define SEM_MSG(node__,mesg__) \
  err << node__->token ()->location () << mesg__ << endMessage
  
#define SEM_ERROR(node__,mesg__) \
  SEM_MSG (node__, sev_error << mesg__)

#define SEM_WARNING(node__,mesg__) \
  SEM_MSG (node__, sev_warning << mesg__)


CSemVisitor::CSemVisitor (ErrorSink &e, CStructure *scope) : err (e) {
  current_scope = scope;
}

CTypeInfo *CSemVisitor::resolveExpr (CTree *expr, CTree *node) const {
  CSemExpr cse (err, current_scope);
  return cse.resolveExpr (expr, node);
}

void CSemVisitor::resolveInit (CObjectInfo *info, CTree *expr,
                                     CTree *node) const {
  CSemExpr cse (err, current_scope);
  cse.resolveInit (info, expr, node);
}

void CSemVisitor::run (CTree *node, CStructure *scope) {
  current_scope = scope;
  if (! current_scope && node->NodeName () == CT_Program::NodeId ())
    current_scope = ((CT_Program*)node)->Scope ();
  if (current_scope)
    visit (node);
}

void CSemVisitor::pre_visit (CTree *node) {
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
  else if (id == CT_ArrayDeclarator::NodeId ())
    pre_action ((CT_ArrayDeclarator*)node);
  else if (id == CT_BitFieldDeclarator::NodeId ())
    pre_action ((CT_BitFieldDeclarator*)node);
  else if (id == CT_CmpdLiteral::NodeId ())
    pre_action ((CT_CmpdLiteral*)node);
  else if (id == CT_ExprList::NodeId ())
    pre_action ((CT_ExprList*)node);
}

void CSemVisitor::post_visit (CTree *node) {
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
}



///////////////////////////////////////////////////////
// pre_action functions ///////////////////////////////
///////////////////////////////////////////////////////



void CSemVisitor::pre_action (CT_Program *node) {
  current_scope = node->Scope ();
}

void CSemVisitor::pre_action (CT_ArgDeclList *node) {
  current_scope = node->Scope ();
}

void CSemVisitor::pre_action (CT_ArgNameList *node) {
  current_scope = node->Scope ();
}

void CSemVisitor::pre_action (CT_ArgDeclSeq *node) {
  current_scope = node->Scope ();
}

void CSemVisitor::pre_action (CT_MembList *node) {
  current_scope = node->Scope ();
}

void CSemVisitor::pre_action (CT_CmpdStmt *node) {
  current_scope = node->Scope ();
}

void CSemVisitor::pre_action (CT_IfStmt *node) {
  CTypeInfo *type;
  CTree *expr;
  
  current_scope = node->Scope ();
  
  expr = node->Condition ();
  if (expr) {
    type = resolveExpr (expr, node);
    if (! type->isScalar ())
      SEM_ERROR (node, "controlling expression of `if' statement not scalar");
  }
}

void CSemVisitor::pre_action (CT_IfElseStmt *node) {
  CTypeInfo *type;
  CTree *expr;
  
  current_scope = node->Scope ();
  
  expr = node->Condition ();
  if (expr) {
    type = resolveExpr (expr, node);
    if (! type->isScalar ())
      SEM_ERROR (node, "controlling expression of `if' statement not scalar");
  }
}

void CSemVisitor::pre_action (CT_SwitchStmt *node) {
  CTypeInfo *type;
  CTree *expr;
  
  current_scope = node->Scope ();
  
  expr = node->Condition ();
  if (expr) {
    type = resolveExpr (expr, node);
    type = CSemExpr::promote (node->Condition (), node);
    if (! type->isInteger ())
      SEM_ERROR (node, 
        "controlling expression of `switch' statement not an integer");
  }
}

void CSemVisitor::pre_action (CT_ForStmt *node) {
  CTypeInfo *type;
  CTree *expr;
  
  current_scope = node->Scope ();
  
  expr = node->Condition ();
  if (expr) {
    type = resolveExpr (expr, node);
    if (! type->isScalar ())
      SEM_ERROR (node, "controlling expression of `for' statement not scalar");
  }
  expr = node->Expr ();
  if (expr)
    // there are no further constraints on this expression
    resolveExpr (expr, node); 
}

void CSemVisitor::pre_action (CT_WhileStmt *node) {
  CTypeInfo *type;
  CTree *expr;
  
  current_scope = node->Scope ();
  
  expr = node->Condition ();
  if (expr) {
    type = resolveExpr (expr, node);
    if (! type->isScalar ())
      SEM_ERROR (node, "controlling expression of `while' statement not scalar");
  }
}

void CSemVisitor::pre_action (CT_DoStmt *node) {
  CTypeInfo *type;
  CTree *expr;
  
  expr = node->Expr ();
  if (expr) {
    type = resolveExpr (expr, node);
    if (! type->isScalar ())
      SEM_ERROR (node, "controlling expression of `do' statement not scalar");
  }
}

void CSemVisitor::pre_action (CT_ExprStmt *node) {
  if (node->Expr ())
    resolveExpr (node->Expr (), node);
  prune ();
}

void CSemVisitor::pre_action (CT_CaseStmt *node) {
  if (node->Expr ()) {
    resolveExpr (node->Expr (), node);
    if (! CSemExpr::isConstIntExpr (node->Expr ()))
      SEM_ERROR (node, "label of `case' statement not a valid integer constant");
  }
}

void CSemVisitor::pre_action (CT_GotoStmt *node) {
  if (! current_scope->LocalScope ())
    SEM_ERROR (node, "`goto' outside of function");
  else
    check_goto_label (node->Label ());
}

void CSemVisitor::check_goto_label (CTree *tree) {
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

void CSemVisitor::pre_action (CT_ReturnStmt *node) {
  CFunctionInfo *finfo = 0;
  CTypeInfo *t2 = 0, *t1 = 0;
  CConstant *v = 0;
  CTree *expr = 0;
  
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
    expr = node->Expr ();
    if (t1->isVoid ()) 
      SEM_WARNING (node, "`return' with a value, in function returning void");
    else {
      v = expr->Value () ? expr->Value ()->Constant () : 0;
      if (t1->isScalar ()) {
        if (t1->isArithmetic () && t2->isArithmetic ()) {
          if (! CSemExpr::compatible (t1, t2))
            CSemExpr::cast_to (t1, expr, node);
        } else if (! (t1->isPointer () && t2->isPointer () && 
                      (CSemExpr::compatibleBase (t1, t2) ||
                       t1->VirtualType ()->BaseType ()->isVoid () || 
                       t2->VirtualType ()->BaseType ()->isVoid ()) &&
                      (t2->isConst () ? t1->isConst () : true) &&
                      (t2->isVolatile () ? t1->isVolatile () : true) &&
                      (t2->isRestrict () ? t1->isRestrict () : true)) &&
                   ! (t1->isPointer () && t2->isInteger () && v && v->isNull ()) &&
                   ! (t1->VirtualType ()->is_bool () && t2->isPointer ())) {
          SEM_WARNING (node, "incompatible types in `return' statement");
        } else if (! CSemExpr::compatible (t1, t2))
          CSemExpr::cast_to (t1, expr, node);
      } else if (t1->isRecord ()) {
        if (! CSemExpr::compatible (t1, t2))
          SEM_WARNING (node, "incompatible types in `return' statement");
      } else
        SEM_ERROR (node, "invalid return type");
    }
  } else if (! t1->isVoid ()) 
    SEM_WARNING (node, "`return' with no value, in function returning non-void");

  prune ();
}

void CSemVisitor::pre_action (CT_InitDeclarator *node) {
  CT_ExprList *init = node->Initializer ();
  if (init) {
    CObjectInfo* obj = node->Object ();
    resolveInit (obj, init, node);
    if (obj && obj->isStatic ())
      for (int i = init->Entries () - 1; i >= 0; i--)
        if (! CSemExpr::isConstExpr (init->Entry (i)))
          SEM_ERROR (node, "initializer is not constant");
  }
}

void CSemVisitor::pre_action (CT_ArrayDeclarator *node) {
  CT_SimpleName *name;
  CObjectInfo *info;
  const char *array;
  CTypeArray *atype;
  CTypeInfo *type;
  CConstant *v;
  CTree *expr;
  
  name = findName (node);
  if (name) {
    array = name->Text ();
    info = name->Object ();
  } else {
    array = "";
    info = 0;
  }
  expr = node->Delimiter ()->Expr ();

  if (expr) {
    type = expr->Type ();
    if (! type->isInteger ())
      SEM_ERROR (node, "size of array `" << array << "' has non-integer type");
    else if (CSemExpr::isConstIntExpr (expr)) {
      v = expr->Value ()->Constant ();
      if (/*v->isNull () ||*/ v->isNegative ())
        SEM_ERROR (node, "array `" << array << "' has invalid size");
    }
  } 

  // check the array size on the top level array declarator only
  // to avoid redundant error messages, extern arrays can have
  // unspecified dimension
  if (info && ! info->isExtern () && 
      node->Parent ()->NodeName () != CT_ArrayDeclarator::NodeId ()) {
    atype = name->Object ()->TypeInfo ()->VirtualType ()->TypeArray ();
    if (atype && ! atype->isVarArray ()) {
      if (! atype->BaseType ()->isArray () && ! atype->hasDimension ()) {
        // assume dimension 1
        SEM_WARNING (node, "array `" << array << "' assumed to have one element");
        atype->Dimension (1);
      } else do {
        if (! atype->hasDimension ()) {
          SEM_ERROR (node, "array size missing in `" << array << "'");
          break;
        }
        atype = atype->BaseType ()->VirtualType ()->TypeArray ();
      } while (atype && ! atype->isVarArray ());
    } 
  }
}

void CSemVisitor::pre_action (CT_BitFieldDeclarator *node) {
  CObjectInfo *info;
  const char *name;
  CConstant *v;
  CTree *expr;
  
  expr = node->Expr ();
  if (expr) {
    resolveExpr (expr, node);
    expr = node->Expr ();
    info = node->Object ();
    name = info ? info->Name ().c_str () : "";
    
    if (! CSemExpr::isConstIntExpr (expr)) 
      SEM_ERROR (node, "width of bit-field `" << name
        << "' isn't integer constant expression");
    else {
      v = expr->Value ()->Constant ();
      if (v->isNegative ())
        SEM_ERROR (node, "negative width in bit-field `" << name << "'");
      else if (info && info->TypeInfo () && info->TypeInfo ()->TypeBitField ())
        info->TypeInfo ()->TypeBitField ()->Dimension (v->convert_to_uint ());
    }
  }
}

void CSemVisitor::pre_action (CT_ExprList *node) {
  // do not visit the expression list 
  // it's evaluated when the init declarator is visited
  prune ();
}

void CSemVisitor::pre_action (CT_CmpdLiteral *node) {
  // do not visit the compound literal
  // it's evaluated when the init declarator is visited
  prune ();
}



///////////////////////////////////////////////////////
// post_action functions //////////////////////////////
///////////////////////////////////////////////////////



void CSemVisitor::post_action (CT_Program *node) {
  current_scope = node->Scope ();
}

void CSemVisitor::post_action (CT_ArgDeclList *node) {
  current_scope = node->Scope ()->Parent ()->Structure ();
}

void CSemVisitor::post_action (CT_ArgNameList *node) {
  current_scope = node->Scope ()->Parent ()->Structure ();
}

void CSemVisitor::post_action (CT_ArgDeclSeq *node) {
  current_scope = node->Scope ()->Parent ()->Structure ();
}

void CSemVisitor::post_action (CT_MembList *node) {
  current_scope = node->Scope ()->Parent ()->Structure ();
}

void CSemVisitor::post_action (CT_CmpdStmt *node) {
  current_scope = node->Scope ()->Parent ()->Structure ();
}

void CSemVisitor::post_action (CT_IfStmt *node) {
  current_scope = node->Scope ()->Parent ()->Structure ();
}

void CSemVisitor::post_action (CT_IfElseStmt *node) {
  current_scope = node->Scope ()->Parent ()->Structure ();
}

void CSemVisitor::post_action (CT_SwitchStmt *node) {
  current_scope = node->Scope ()->Parent ()->Structure ();
}

void CSemVisitor::post_action (CT_ForStmt *node) {
  current_scope = node->Scope ()->Parent ()->Structure ();
}

void CSemVisitor::post_action (CT_WhileStmt *node) {
  current_scope = node->Scope ()->Parent ()->Structure ();
}



///////////////////////////////////////////////////////
// common helper //////////////////////////////////////
///////////////////////////////////////////////////////



CT_SimpleName *CSemVisitor::findName (CTree *node) const {
  const char *id = node->NodeName ();
  if (id == CT_InitDeclarator::NodeId ())
    return findName (((CT_InitDeclarator*)node)->Declarator ());
  else if (id == CT_FctDeclarator::NodeId ())
    return findName (((CT_FctDeclarator*)node)->Declarator ());
  else if (id == CT_ArrayDeclarator::NodeId ())
    return findName (((CT_ArrayDeclarator*)node)->Declarator ());
  else if (id == CT_BracedDeclarator::NodeId ())
    return findName (((CT_BracedDeclarator*)node)->Declarator ());
  else if (id == CT_BitFieldDeclarator::NodeId ())
    return findName (((CT_BitFieldDeclarator*)node)->Declarator ());
  else if (id == CT_RefDeclarator::NodeId ())
    return findName (((CT_RefDeclarator*)node)->Declarator ());
  else if (id == CT_PtrDeclarator::NodeId ())
    return findName (((CT_PtrDeclarator*)node)->Declarator ());
  else if (id == CT_MembPtrDeclarator::NodeId ())
    return findName (((CT_MembPtrDeclarator*)node)->Declarator ());
  else if (id == CT_SimpleName::NodeId () || id == CT_PrivateName::NodeId ())
    return (CT_SimpleName*)node;
  return (CT_SimpleName*)0;
}


} // namespace Puma
