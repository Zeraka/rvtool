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

#include "Puma/CTree.h"
#include "Puma/CSemDatabase.h"
#include "Puma/CFileInfo.h"
#include "Puma/CCSemExpr.h"
#include "Puma/CStructure.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CCNameLookup.h"
#include "Puma/CCOverloading.h"
#include "Puma/CCAssocScopes.h"
#include "Puma/CConstant.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CLocalScope.h"
#include "Puma/CArgumentInfo.h"
#include "Puma/CEnumeratorInfo.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CNamespaceInfo.h"
#include "Puma/CTemplateParamInfo.h"
#include "Puma/CTemplateInstance.h"
#include "Puma/CWStrLiteral.h"
#include "Puma/CStrLiteral.h"
#include "Puma/CCConversions.h"
#include "Puma/Utf8.h"

// this include shall be removed as soon as ac++ can better deal with
// the include cycle problem
#include "Puma/CCSemVisitor.h"

#include <climits>
#include <string>
#include "Puma/WChar.h"    /* wcslen() */
#include <ctype.h>         /* isdigit(), isxdigit() */
#include <stdio.h>         /* sprintf() */
#include <string.h>
#include <stdlib.h>        /* strtod() */
#include <set>


namespace Puma {


// print semantic error messages

#define SEM_MSG(node__,mesg__) \
  {if (node__->token ()) err << node__->token ()->location (); err << mesg__ << endMessage;}
  
#define SEM_ERROR(node__, mesg__) \
  SEM_MSG (node__, sev_error << mesg__)

#define SEM_WARNING(node__, mesg__) \
  SEM_MSG (node__, sev_warning << mesg__)

#define SEM_WARNING__out_of_range(node__,what__) \
  SEM_WARNING (node__, what__ << " out of range")
  
#define SEM_WARNING__is_unsigned(node__) \
  SEM_WARNING (node__, \
    "decimal constant is so large that it is unsigned")

#define SEM_ERROR__invalid_op(node__,type__,op__) \
  SEM_ERROR (node__, "invalid operand to " << type__ << " `" << op__ << "'")

#define SEM_ERROR__left_op(node__,op__,what__) \
  SEM_ERROR (node__, "left operand of `" << op__ << "' " << what__)

#define SEM_ERROR__right_op(node__,op__,what__) \
  SEM_ERROR (node__, "right operand of `" << op__ << "' " << what__)

#define SEM_ERROR__comp_between(node__,what__) \
  SEM_ERROR (node__, "comparison between " << what__)

#define SEM_ERROR__undeclared(name__) \
  SEM_ERROR (name__, "`" << *name__ << "' undeclared here")

#define SEM_ERROR__must_include(node__,file__,what__) \
  SEM_ERROR (node__, "must #include " << file__ << " before using " << what__)


///////////////////////////////////////////////////////
// basic operations on expressions ////////////////////
///////////////////////////////////////////////////////


bool CCSemExpr::isRvalue (CTree *expr) {
  // §3.10 any expression is either an lvalue or an rvalue,
  // so everything that is not an lvalue is an rvalue
  return (! isLvalue (expr));
}


bool CCSemExpr::isLvalue (CTree *expr) {
  CObjectInfo *info;
  CTypeInfo *type;
  const char *id;
  int oper;
  
  if (! expr || ! expr->Type ())
    return false;

  type = expr->Type ();
  id = expr->NodeName ();

  if (type->isVoid ())
    return false;
  if (type->isAddress () || type->isTemplateParam ())
    return true;
  if (id == CT_String::NodeId () || 
      id == CT_WideString::NodeId () ||
      id == CT_CmpdLiteral::NodeId () ||
      id == CT_IndexExpr::NodeId () ||
      id == CT_TypeidExpr::NodeId () ||
      id == CT_DerefExpr::NodeId ()) 
    return true;
  if (id == CT_Integer::NodeId () ||
      id == CT_Character::NodeId () ||
      id == CT_WideCharacter::NodeId () ||
      id == CT_Float::NodeId () ||
      id == CT_Bool::NodeId ())
    return false;
  if (id == CT_CallExpr::NodeId () && type->isAddress ())
    return true;
  if (id == CT_MembRefExpr::NodeId () || id == CT_MembPtrExpr::NodeId ()) {
    info = ((CT_SimpleName*)expr->Son (2))->Name ()->Object ();
    if (info) {
      if (info->EnumeratorInfo ())
        return false;
      if (info->AttributeInfo ()) { 
        //if (info->isStatic ())
          return true;
        //return isLvalue (expr->Son (0));
      }
      if (info->FunctionInfo () && info->isStatic ())
        return true;
    }
    return false;
  }
  if (id == CT_UnaryExpr::NodeId ()) {
    oper = expr->Son (0)->token ()->type ();
    if (oper == TOK_INCR || oper == TOK_DECR)
      return true;
    return false;
  }
  if (id == CT_BinaryExpr::NodeId ()) {
    oper = expr->Son (1)->token ()->type ();
    if (oper == TOK_DOT_STAR || oper == TOK_PTS_STAR) {
      type = expr->Son (2)->Type ();
      if (! type->isFunction ()) {
        if (oper == TOK_DOT_STAR && ! isLvalue (expr->Son (0)))
          return false;
        return true;
      }
    }
    if (oper == TOK_ASSIGN || oper == TOK_MUL_EQ || oper == TOK_DIV_EQ ||
        oper == TOK_MOD_EQ || oper == TOK_ADD_EQ || oper == TOK_SUB_EQ ||
        oper == TOK_RSH_EQ || oper == TOK_LSH_EQ || oper == TOK_AND_EQ ||
        oper == TOK_XOR_EQ || oper == TOK_IOR_EQ)
      return true;
    if (oper == TOK_COMMA)
      return isLvalue (expr->Son (2));
    return false;
  }
  if (id == CT_SimpleName::NodeId ()) {
    info = ((CT_SimpleName*)expr)->Name ()->Object ();
    if (info)
      return (type->isFunction () || info->ArgumentInfo () ||
              (info->AttributeInfo () && ! info->EnumeratorInfo ()));
    return false;
  } 
  if (id == CT_BracedExpr::NodeId ())
    return isLvalue (expr->Son (1));
  if (id == CT_IfThenExpr::NodeId ())
    return (isLvalue (expr->Son (2)) && isLvalue (expr->Son (4)));
  if (id == CT_QualName::NodeId () || id == CT_RootQualName::NodeId ())
    return isLvalue (((CT_QualName*)expr)->Name ());
  if (id == CT_CastExpr::NodeId ())
    return isLvalue (expr->Son (3));
  if (id == CT_ImplicitCast::NodeId ())
    return isLvalue (expr->Son (0));
  return false;
}


CTypeInfo *CCSemExpr::castToType (CTypeInfo *type, CTree *node, bool ref) {
  CSemValue *cast = 0;
  CExprValue *value;

  // cast implicitly if types differ
  if (*node->Type () != *type) {
    if (node->Parent ()) {
      CTree* base = node->Parent ();
      CT_ImplicitCast *cast_node = new CT_ImplicitCast (node);
      base->ReplaceSon (node, cast_node);
      cast = cast_node;
    } else if (node->SemValue ()) {
      cast = node->SemValue ();
    }

    if (cast) {
      if (ref)
        cast->setTypeRef (type);    // refer to a shared type
      else
        cast->setType (type);       // type is new (to be deleted by this node)

      // cast constant value if any
      value = node->Value ();
      if (value) {
        if (value->Constant ()) {
          value = value->Constant ()->cast_to (type);
          cast->setValue (value);
        } else
          cast->setValueRef (value);
      }
    }
  }
  return type;
}


bool CCSemExpr::isModifiable (CTypeInfo *type, unsigned long pos) { 
  // error checks not yet implemented!
  // too simple yet!
  if (type->isFunction ())
    return false;
  return true;
}


void CCSemExpr::passOnConstant (CTree *node, CTypeInfo *t1) const {
  CTypeInfo *t2;
  CConstant *value;
  CTree* base;

  t2 = node->Type ();
  base = node->Parent ();
  value = node->Value () ? node->Value ()->Constant () : 0;
  if (value && base && base->SemValue ()) {
    if (t1 && t2 && *t1 != *t2) {
      value = value->cast_to (t1);
      base->SemValue ()->setValue (value);
    } else
      base->SemValue ()->setValueRef (value);
  }
}


CTree *CCSemExpr::baseTree (CTree *node) const {
  while (true) {
    if (node->NodeName () == CT_BracedExpr::NodeId ())
      node = node->Son (1);
    else if (node->NodeName () == CT_ImplicitCast::NodeId ())
      node = node->Son (0);
    else
      break;
  }
  return node;
}


bool CCSemExpr::isAddrExpr (CTree *node) {
  if (node->NodeName () == CT_AddrExpr::NodeId ())
    return true;
  else if (node->NodeName () == CT_BracedExpr::NodeId ())
    return isAddrExpr (((CT_BracedExpr*)node)->Expr ());
  else if (node->NodeName () == CT_ImplicitCast::NodeId ())
    return isAddrExpr (((CT_ImplicitCast*)node)->Expr ());
  return false;
}


CObjectInfo *CCSemExpr::findObject (CTree *node) {
  const char *id = node->NodeName ();
  if (id == CT_BracedExpr::NodeId () ||
      id == CT_DerefExpr::NodeId () ||
      id == CT_AddrExpr::NodeId () ||
      id == CT_UnaryExpr::NodeId ()) {
    return findObject (node->Son (1));
  } else if (id == CT_ImplicitCast::NodeId () ||
             id == CT_PostfixExpr::NodeId () ||
             id == CT_IndexExpr::NodeId ()) {
    return findObject (node->Son (0));
  } else if (id == CT_CastExpr::NodeId ()) {
    return findObject (node->Son (3));
  } else if (id == CT_BinaryExpr::NodeId ()) {
    int oper = node->Son (1)->token ()->type ();
    if (oper == TOK_COMMA)
      return findObject (node->Son (2));
    if (oper == TOK_ASSIGN || oper == TOK_MUL_EQ || oper == TOK_DIV_EQ ||
        oper == TOK_MOD_EQ || oper == TOK_ADD_EQ || oper == TOK_SUB_EQ ||
        oper == TOK_RSH_EQ || oper == TOK_LSH_EQ || oper == TOK_AND_EQ ||
        oper == TOK_XOR_EQ || oper == TOK_IOR_EQ)
      return findObject (node->Son (0));
  } else if (id == CT_QualName::NodeId () ||
             id == CT_RootQualName::NodeId ()) {
    return findObject (((CT_QualName*)node)->Name ());
  } else if (id == CT_SimpleName::NodeId () ||
             id == CT_OperatorName::NodeId () ||
             id == CT_DestructorName::NodeId () ||
             id == CT_ConversionName::NodeId () ||
             id == CT_TemplateName::NodeId () ||
             id == CT_PrivateName::NodeId ()) {
    return ((CT_SimpleName*)node)->Name ()->Object ();
  } else if (id == CT_MembPtrExpr::NodeId () ||
             id == CT_MembRefExpr::NodeId ()) {
    return ((CT_MembPtrExpr*)node)->Object ();
  } else if (id == CT_CallExpr::NodeId ()) {
    return ((CT_CallExpr*)node)->Object ();
  } else if (id == CT_NewExpr::NodeId ()) {
    return ((CT_NewExpr*)node)->Object ();
  } else if (id == CT_ConstructExpr::NodeId ()) {
    return ((CT_ConstructExpr*)node)->Object ();
  } else if (id == CT_StaticCast::NodeId () ||
             id == CT_ConstCast::NodeId () ||
             id == CT_DynamicCast::NodeId () ||
             id == CT_ReintCast::NodeId ()) {
    return findObject (((CT_StaticCast*)node)->Expr ());
  }
  return (CObjectInfo*)0;
}


///////////////////////////////////////////////////////
// implicit conversions ///////////////////////////////
///////////////////////////////////////////////////////


// ���5.9 usual arithmetic conversions
CTypeInfo *CCSemExpr::usualArithmeticConv (CTree *op1, CTree *op2) {
  CTypeInfo *t1, *t2;

  t1 = op1->Type ();
  t2 = op2->Type ();
  if (! t1 || ! t2)
    return t1;

  CTree* oldp1 = op1->Parent ();
  CTree* oldp2 = op2->Parent ();

  if (t1->is_long_double () && ! t2->is_long_double ())
    t2 = castToType (&CTYPE_LONG_DOUBLE, op2, true);
  else if (t2->is_long_double () && ! t1->is_long_double ())
    t1 = castToType (&CTYPE_LONG_DOUBLE, op1, true);
  else if (t1->is_double () && ! t2->is_double ())
    t2 = castToType (&CTYPE_DOUBLE, op2, true);
  else if (t2->is_double () && ! t1->is_double ())
    t1 = castToType (&CTYPE_DOUBLE, op1, true);
  else if (t1->is_float () && ! t2->is_float ())
    t2 = castToType (&CTYPE_FLOAT, op2, true);
  else if (t2->is_float () && ! t1->is_float ())
    t1 = castToType (&CTYPE_FLOAT, op1, true);
  else { // integral promotion of both operands
    t1 = intPromotion (op1);
    t2 = intPromotion (op2);
  }

  if (oldp1 != op1->Parent ())
    op1 = op1->Parent ();
  if (oldp2 != op2->Parent ())
    op2 = op2->Parent ();

  if (t1->is_unsigned_int128 () && ! t2->is_unsigned_int128 ())
    t2 = castToType (&CTYPE_UNSIGNED_INT128, op2, true);
  else if (t2->is_unsigned_int128 () && ! t1->is_unsigned_int128 ())
    t1 = castToType (&CTYPE_UNSIGNED_INT128, op1, true);
  else if (t1->is_int128 () && ! t2->is_int128 ())
    t2 = castToType (&CTYPE_INT128, op2, true);
  else if (t2->is_int128 () && ! t1->is_int128 ())
    t1 = castToType (&CTYPE_INT128, op1, true);
  else if (t1->is_unsigned_long_long () && ! t2->is_unsigned_long_long ())
    t2 = castToType (&CTYPE_UNSIGNED_LONG_LONG, op2, true);
  else if (t2->is_unsigned_long_long () && ! t1->is_unsigned_long_long ())
    t1 = castToType (&CTYPE_UNSIGNED_LONG_LONG, op1, true);
  else if (t1->is_long_long () && ! t2->is_long_long ())
    t2 = castToType (&CTYPE_LONG_LONG, op2, true);
  else if (t2->is_long_long () && ! t1->is_long_long ())
    t1 = castToType (&CTYPE_LONG_LONG, op1, true);
  else if (t1->is_unsigned_long () && ! t2->is_unsigned_long ())
    t2 = castToType (&CTYPE_UNSIGNED_LONG, op2, true);
  else if (t2->is_unsigned_long () && ! t1->is_unsigned_long ())
    t1 = castToType (&CTYPE_UNSIGNED_LONG, op1, true);
  else if ((t1->is_long () && t2->is_unsigned_int ()) ||
           (t2->is_long () && t1->is_unsigned_int ())) {
    t1 = castToType (&CTYPE_UNSIGNED_LONG, op1, true);
    t2 = castToType (&CTYPE_UNSIGNED_LONG, op2, true);
  }
  else if (t1->is_long () && ! t2->is_long ())
    t2 = castToType (&CTYPE_LONG, op2, true);
  else if (t2->is_long () && ! t1->is_long ())
    t1 = castToType (&CTYPE_LONG, op1, true);
  else if (t1->is_unsigned_int () && ! t2->is_unsigned_int ())
    t2 = castToType (&CTYPE_UNSIGNED_INT, op2, true);
  else if (t2->is_unsigned_int () && ! t1->is_unsigned_int ())
    t1 = castToType (&CTYPE_UNSIGNED_INT, op1, true);

  return t1;
}


// ���4.5 integral promotion
CTypeInfo *CCSemExpr::intPromotion (CTree *expr) {
  CTypeInfo *result = expr->Type ();
  if (! result)
    return result;

  while (result->TypeAddress ())
    result = result->BaseType ();

  if (result->is_char () || result->is_short () ||
      result->is_signed_char () || result->is_unsigned_char () ||
      result->is_unsigned_short () || result->is_wchar_t () ||
      result->is_bool ()) {
    result = castToType (&CTYPE_INT, expr, true);
  } else if (result->isEnum ()) {
    result = castToType (result->VirtualType ()->TypeEnum ()->UnderlyingType (), expr, true);
  } else if (result->TypeBitField ()) {
    if (result->BaseType ()->isEnum () ||
        (result->BaseType ()->isInteger () &&
         (*result->BaseType () < CTYPE_INT)))
      result = castToType (&CTYPE_INT, expr, true);
  }

  return result;
}


// ���4.1, ���4.2, ���4.3 lvalue to rvalue conversions
CTypeInfo *CCSemExpr::convLvalueToRvalue (CTree *expr, CTree *dest) {
  CTypeInfo *result/*, *target*/;
  bool is_lvalue;

  result = expr->Type ();
  is_lvalue = isLvalue (expr);

  // ���4.1 lvalue-to-rvalue conversion
  if (is_lvalue && ! result->isFunction () && ! result->isArray ()) {
    CTypeInfo *qt = result;
    while (qt->isAddress ())
      qt = qt->BaseType ();
    if (! qt->isRecord () && qt->isQualified ())
      result = castToType (qt->UnqualType (), expr, true);
  // ���4.2 array-to-pointer conversion
  } else if (result->isArray ()) {
    result = result->VirtualType ()->BaseType ();
    result = new CTypePointer (result->Duplicate ());
    castToType (result, expr, false);
  // ���4.3 function-to-pointer conversion
  } else if (is_lvalue && result->isFunction ()) {
    CFunctionInfo* fct = result->VirtualType ()->TypeFunction ()->FunctionInfo ();
    if (fct && fct->isMethod () && ! fct->isStaticMethod ()) {
      result = new CTypeMemberPointer (result->Duplicate (), fct->Record ());
    } else {
      result = new CTypePointer (result->Duplicate ());
    }
    castToType (result, expr, false);
  }

  return result;
}


///////////////////////////////////////////////////////
// expression evaluation //////////////////////////////
///////////////////////////////////////////////////////


CTypeInfo *CCSemExpr::resolveExpr (CTree *node, CTree *base) {
  CTypeInfo *result;

  // resolve expression
  result = resolve (node, base);

  return result;
}


CTypeInfo *CCSemExpr::resolve (CTree *node, CTree *base) {
  const char *id = node->NodeName ();
  CTypeInfo *result = 0, *type;

  // do not analyse an expression twice
  type = node->Type ();
  if (type && ! type->is_undefined ()) {
    if (type->isDependent ())
      dependent = true;
    if (dependent || ! (id == CT_ConstructExpr::NodeId ()))
      return type;
  }

  if (! (id == CT_BracedExpr::NodeId () ||
         id == CT_AddrExpr::NodeId () ||
         id == CT_MembPtrExpr::NodeId () || 
         id == CT_MembRefExpr::NodeId () ||
         id == CT_SimpleName::NodeId () ||
         id == CT_OperatorName::NodeId () ||
         id == CT_DestructorName::NodeId () ||
         id == CT_ConversionName::NodeId () ||
         id == CT_TemplateName::NodeId () ||
         id == CT_QualName::NodeId () ||
         id == CT_RootQualName::NodeId ())) {
    fct_call = 0;
    dependent_args = false;
  }

  // call the node type specific resolve function
  if (!(result = node->resolve (*this, base))) {
    SEM_ERROR ((node->token () ? node : base), "expression expected");
    result = &CTYPE_UNDEFINED;
    if (node->SemValue ())
      node->SemValue ()->setTypeRef (result);
  }

  // recognize dependent result types
  type = node->Type ();
  if (type) {
    // ignore return type of function types
    if (type->TypeFunction ()) {
      if (type->TypeFunction ()->ArgTypes ()->isDependent ())
        dependent = true;
    // type depends on template parameter
    } else if (type->isDependent ()) {
      dependent = true;
    // if result type is a class object type, ensure
    // that the class is really instantiated
    } else if (type->UnqualType()->TypeClass()) {
      type->UnqualType()->TypeClass()->instantiate(current_scope);
    }
  }
  
  return result;
}


// fallback: unhandled kind of expression
CTypeInfo *CCSemExpr::resolve (CT_Expression *node, CTree *base) {
  SEM_ERROR (node, "illegal expression");
  CTypeInfo *result = &CTYPE_UNDEFINED;
  if (node->SemValue ())
    node->SemValue ()->setTypeRef (result);
  return result;
}


// ���5.2.2 function call
CTypeInfo *CCSemExpr::resolve (CT_CallExpr *node, CTree *base) {
  bool dep_base, dep_args;
  CTypeInfo *t1, *result;
  CCandidateInfo *cand;
  CT_ExprList *args;
  CObjectInfo *info;
  unsigned entries;
  CRecord *sinfo;
  
  info = 0;
  dep_args = false;
  dep_base = resetDependent ();
  result = &CTYPE_UNDEFINED;
  args = node->Arguments ();
  entries = args->Entries ();

  // resolve arguments first
  for (unsigned i = 0; i < entries; i++) {
    resolve (args->Entry (i), args);
    
    // handle dependent arguments
    if (dependent) {
      dep_args = true;
      dependent = false;
    }
  }

  if (dep_args)
    dependent = true;
    
  // resolve postfix expression
  fct_call = node;
  dependent_args = dep_args;
  t1 = resolve (node->Expr (), node); 
  dependent_args = false;
  fct_call = 0;  

  // handle dependent expressions
  if (isDependent (node, dep_base))
    return result;

  // function call: op1 ( op2 , .. )
  if (t1->isFunction ()) {
    info = t1->VirtualType ()->TypeFunction ()->FunctionInfo ();
    if (! info)
      result = t1->VirtualType ()->TypeFunction ()->ReturnType ();
  } else if (t1->isPointer () && t1->PtrBaseType ()->isFunction ()) {
    info = t1->PtrBaseType ()->VirtualType ()->TypeFunction ()->FunctionInfo ();
    if (! info)
      result = t1->PtrBaseType ()->VirtualType ()->TypeFunction ()->ReturnType ();
  // ���13.3.1.1.2 call to object of class type
  // ���13.5.4 function call operator overloading
  } else if (t1->isRecord ()) {
    sinfo = t1->VirtualType ()->TypeRecord ()->Record ();
    if (sinfo) {
      // ensure that class is instantiated
      if (sinfo->ClassInfo ())
        sinfo->TypeInfo ()->TypeClass ()->instantiate (current_scope);
      
      // candidates are at least the function call operators 
      // (operator ()) of the object's class, and all conversion
      // functions of this class and its base classes that yield
      // "pointer to function", "reference to pointer to function", 
      // or "reference to function"
      CCNameLookup nl (err, current_scope);
      nl.lookup ("operator ()", sinfo, false, true);
      
      CCOverloading ovl (err);
      ovl.ObjectType (t1);
      ovl.collectConvFunctions (sinfo);
      
      // filter the conversion functions
      for (unsigned i = ovl.Candidates (); i > 0; i--) {
        t1 = ovl.Candidate (i-1)->Function ()->ConversionType ();
        if (! (t1->isFunction () || (t1->isPointer () && 
                t1->PtrBaseType ()->isFunction ())))
          ovl.removeCandidate (i-1);
      }          

      ovl.addCandidates (nl);
      ovl.resolve (args);

      // check result of function overload resolution
      if (ovl.Function ()) { 
        if (validOvlFctSet (node, "overloaded operator", "operator ()", ovl)) {
          info = ovl.Function ();
          cand = ovl.Candidate (0);

          // apply the function argument conversions determined by
          // overload resolution
          for (unsigned i = 0; i < cand->ConvSequences (); i++)
            cand->ConvSequence (i)->Apply ();
        }
      } else
        SEM_ERROR (node, "non-function in function call");
    }  
  } else if (! t1->isUndefined ())
    SEM_ERROR (node, "non-function in function call");
  
  if (info && info->FunctionInfo ()) {
    node->Object (info);
    result = info->FunctionInfo ()->ReturnType ();
  }
  
  node->setTypeRef (result);
  return result;
}


// ���5.2.5 class member access
CTypeInfo *CCSemExpr::resolve (CT_MembPtrExpr *node, CTree *base) {
  CTypeInfo *t1, *t2, *result, *obj_type;
  CT_CallExpr *fct_call_args;
  bool dep_base, dep_args, new_type;
  CObjectInfo *info, *obj;
  CT_SimpleName *member;
  unsigned long pos;
  const char *op;
  CRecord *sinfo;
  LONG_LONG v;
  CTree *init;
  int oper;
  
  dep_base = resetDependent ();
  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();
  member = (CT_SimpleName*)node->Son (2);
  result = &CTYPE_UNDEFINED;
  fct_call_args = fct_call;
  dep_args = dependent_args;
  dependent_args = false;
  new_type = false;
  fct_call = 0;
  sinfo = 0;
  info = 0;
  obj_type = 0;

  // resolve only the first operand
  t1 = resolve (node->Son (0), node); 

  // handle dependent expressions
  if (isDependent (node, dep_base)) {
    member->setTypeRef (result);
    if (member != member->Name ())
      member->Name ()->setTypeRef (result);
    return result;
  }

  // apply lvalue-to-rvalue conversions
  t1 = convLvalueToRvalue (node->Son (0));
  t2 = 0;
  
  // error diagnostics not yet complete!

  // op1 -> op2
  if (oper == TOK_PTS) {
    // operator may be overloaded
    dep_base = resetDependent ();
    CTypeInfo *ovl_type = 0;
    do {
      // repeat resolving operator->()
      ovl_type = overloadedOperator (node->Son (0), node, op, oper, node->Son (0));
    } while (ovl_type);
    if (isDependent (node, dep_base)) {
      member->setTypeRef (result);
      if (member != member->Name ())
        member->Name ()->setTypeRef (result);
      return result;
    }

    t1 = node->Son (0)->Type ();
    if (! (t1->isPointer () && t1->VirtualType ()->BaseType ()->isRecord ())) { 
      SEM_ERROR__left_op (node, op, "not pointer to class object");
    } else {
      obj_type = t1->VirtualType ()->BaseType ();
      t2 = obj_type->VirtualType ();
    }
  // op1 . op2
  } else {
    if (! t1->isRecord ()) {
      SEM_ERROR__left_op (node, op, "not a class object");
    } else {
      obj_type = t1;
      t2 = t1->VirtualType ();
    }
  }

  // get the class info
  if (t2) {
    sinfo = t2->TypeRecord ()->Record ();
    pos = node->Son (1)->token_node ()->Number ();
    // ensure that class is instantiated
    if (sinfo && sinfo->ClassInfo ())
      sinfo->TypeInfo ()->TypeClass ()->instantiate (current_scope);
    if (! t2->isComplete (pos)) {
      sinfo = 0;
      SEM_ERROR__left_op (node, op, "not a complete object type");
    }
  }

  // lookup the class member
  if (sinfo) {
    sinfo = sinfo->DefObject ()->Record ();
    // ensure that class is instantiated
    if (sinfo->ClassInfo ())
      sinfo->TypeInfo ()->TypeClass ()->instantiate (current_scope);

    // explicit destructor call
    if (member->Name ()->NodeName () == CT_DestructorName::NodeId ()) {
      // get the type name after the tilde
      obj = ((CT_SimpleName*)member->Name ()->Son (1))->Object ();
      // destructor name must match type of expression
      if (! obj || ! obj->TypeInfo () || ! obj->TypeInfo ()->TypeRecord () || 
          ! obj->TypeInfo ()->TypeRecord ()->Record () || ! sinfo->TypeInfo ()) {
        SEM_ERROR (node, "destructor name `" << *member 
          << "' does not match type `" << sinfo->Name () << "' of expression");
      } else {
        CRecord *dinfo = obj->TypeInfo ()->TypeRecord ()->Record ();
        if (! (*sinfo->TypeInfo () == *obj->TypeInfo () ||
               (dinfo->ClassInfo () && sinfo->ClassInfo () &&
                sinfo->ClassInfo ()->isBaseClass (dinfo->ClassInfo (), true)))) {
          SEM_ERROR (node, "destructor name `" << *member 
            << "' does not match type `" << sinfo->Name () << "' of expression");
        } else {
          // valid call to destructor
          std::string dtor_name = "~";
          dtor_name = dtor_name + dinfo->Name ().c_str ();
          info = dinfo->Function (dtor_name.c_str ());
        }
      }
    // call to ordinary member
    } else {
      // do a class member name lookup 
      CCNameLookup nl (err, current_scope);  
      nl.lookup (member, sinfo, false, true); 

      // no member found
      if (! nl.Objects ()) {
        SEM_ERROR (node, "`" << sinfo->Name () << "' has no member named `" 
          << *member << "'");
      // validate name lookup result
      } else if (checkDeclsFound (node, nl)) {
        // possibly overloaded function; resolve it
        if (nl.Object (0)->FunctionInfo ()) {
          if (! fct_call_args) {
            // 13.4 address of overloaded function
            // TODO: Not yet implemented!
            if (nl.Objects () > 1) {
              SEM_WARNING (node, "cannot resolve overloaded function");
            } else
              info = nl.Object (0);
          } else if (dep_args) {
            dependent = true;
          } else {
            CCOverloading ovl (err);
            ovl.ObjectType (obj_type);
            ovl.addCandidates (nl);
            ovl.resolveMember (fct_call_args, member);

            // check result of function overload resolution
            if (validOvlFctSet (node, "function", member->Text (), ovl))
              info = ovl.Function ();
          }
        // non-function member
        } else {
          info = nl.Object (0);
          if (info->Record () || info->TypedefInfo () || info->EnumInfo ()) {
            SEM_ERROR (node, "`" << *member << 
              "' does not refer to object or function");
            info = 0;
          }
        }
      }
    }
  }

  // member found
  if (info) {
    // determine result type
    result = info->TypeInfo ();
    if (info->AttributeInfo () && ! info->isStatic () && 
        ! info->isMutable () && ! result->isAddress ()) {
      obj_type = obj_type->NonReferenceType ();
      if (obj_type->isConst () || obj_type->isVolatile ()) {
        // class data member called on a const/volatile class object
        // the resulting type of the data member is changed to be const/volatile
        new_type = true;

        // 3.9.3.2 CVqualifiers: Any cvqualifiers applied to an array type affect
        //         the array element type, not the array type (8.3.4).
        std::stack<CTypeInfo*> array_nodes;
        while (result->isArray ()) { // skip array nodes
          array_nodes.push (result);
          result = result->BaseType ();
        }

        // add qualifiers if necessary
        CTypeQualified *qt = result->TypeQualified ();
        if (qt) {
          qt = new CTypeQualified (qt->BaseType ()->Duplicate (), 
            (obj_type->isConst () || qt->isConst ()), (obj_type->isVolatile () || qt->isVolatile ()), false);
        } else {
          qt = new CTypeQualified (result->Duplicate (), 
            obj_type->isConst (), obj_type->isVolatile (), false);
        }

        // duplicate array nodes if neccessary
        result = qt;
        while (!array_nodes.empty ()) {
          CTypeArray *at = (CTypeArray*)array_nodes.top ();
          bool has_dim = at->hasDimension ();
          long int dim = at->Dimension ();
          bool is_fixed = at->isFixed ();
          at = new CTypeArray (result, (CTypeQualified*)(at->Qualifiers () ? at->Qualifiers ()->Duplicate () : 0));
          at->hasDimension (has_dim);
          at->Dimension (dim);
          at->isFixed (is_fixed);
          result = at;
          array_nodes.pop ();
        }
      }
    }

    node->Object (info);
    member->Object (info);
    if (member != member->Name ())
      member->Name ()->Object (info);

    // name of an enumeration constant
    if (info->EnumeratorInfo ()) {
      if (info->EnumeratorInfo ()->hasValue ()) {
        v = info->EnumeratorInfo ()->Value ();
        member->setValue (new CConstant (v, result));
        if (member != member->Name ())
          member->Name ()->setValue (new CConstant (v, result));
        node->setValueRef (member->Value ()->Constant ());
      }
      else if (isDependent (info->Init ())) {
        dependent = true;
      }
    // name of a constant variable or class data member
    } else if (info->AttributeInfo () && info->TypeInfo ()->isConst ()) {
      init = info->AttributeInfo ()->Init ();
      if (init && init->Value () && init->Value ()->Constant ()) {
        member->setValueRef (init->Value ()->Constant ());
        if (member != member->Name ())
          member->Name ()->setValueRef (init->Value ()->Constant ());
        node->setValueRef (member->Value ()->Constant ());
      }
      else if (isDependent (init)) {
        dependent = true;
      }
    }
  }

  if (new_type) {
    member->setType (result);
    if (member != member->Name ())
      member->Name ()->setType (result->Duplicate ());
    node->setType (result->Duplicate ());
  } else {
    member->setTypeRef (result);
    if (member != member->Name ())
      member->Name ()->setTypeRef (result);
    node->setTypeRef (result);
  }
  return result;
}


// ���5.1 unqualified and qualified names
CTypeInfo *CCSemExpr::resolve (CT_SimpleName *node, CTree *base) {
  bool arg_dep_lookup, new_type, is_type_name;
  CTypeInfo *result, *type;
  CT_TemplateArgList *tal;
  CTree *init, *arg;
  CObjectInfo *info;
  unsigned args;
  LONG_LONG v;
  
  result = &CTYPE_UNDEFINED;
  arg_dep_lookup = false;
  new_type = false;
  info = 0;
  
  // error diagnostics not yet complete!

  // handle dependent names
  if (node->NodeName () == CT_QualName::NodeId () ||
      (node->NodeName () == CT_RootQualName::NodeId () &&
       node->Entries () > 1)) {
    info = ((CT_SimpleName*)node->Entry (node->Entries ()-2))->Object ();
    if (! info || (info->TypeInfo () && info->TypeInfo ()->isDependent ()))
      dependent = true;
  } 
  if (node->Name ()->NodeName () == CT_TemplateName::NodeId ()) {
    info = node->Name ()->Object ();
    if (info && info->FunctionInfo () && info->FunctionInfo ()->isTemplate ()) {
      tal = ((CT_TemplateName*)node->Name ())->Arguments ();
      args = tal->Entries ();
      for (unsigned i = 0; i < args; i++) {
        arg = tal->Entry (i);
        is_type_name = arg->NodeName () == CT_NamedType::NodeId ()  &&
                       arg->SemObject ()->Object ();
        if (is_type_name)
          type = arg->SemObject ()->Object ()->TypeInfo ();
        else
          type = arg->Type ();

        if (type && type->isDependent (true, is_type_name)) {
          dependent = true;
          break;
        }
      }
    }
  }
  
  // check function call arguments (if any)
  if (fct_call && dependent_args)
    dependent = true;

  if (dependent) {
    node->setTypeRef (result);
    if (node != node->Name ())
      node->Name ()->setTypeRef (result);
    return result;
  }

  // do an ordinary name lookup in current scope 
  // (unqualified name) or scope denoted by the 
  // scope resolution operator (qualified name)
  CCNameLookup nl (err, current_scope);         
  nl.lookup (node, current_scope, true, true); 

  // name not found, could be a built-in function
  if (! nl.Objects () && ! lookupBuiltin (node, nl)) {
    // is a built-in but no corresponding declaration found,
    // handle like a dependent name
    dependent = true;
    node->setTypeRef (result);
    if (node != node->Name ())
      node->Name ()->setTypeRef (result);
    return result;
  }

  // name not found
  if (! nl.Objects ()) {
    // handle dependent class scope, i.e. classes derived
    // from template parameters  
    if (depBaseClass ()) {
      dependent = true;
      node->setTypeRef (result);
      if (node != node->Name ())
        node->Name ()->setTypeRef (result);
      return result;
    }

    // may be the name of a function that can only be found 
    // using argument-dependent name lookup
    if (fct_call &&
        node->NodeName () != CT_QualName::NodeId () &&
        node->NodeName () != CT_RootQualName::NodeId ()) {
      // collect scopes associated with the function call arguments
      CCAssocScopes scopes (fct_call->Arguments ());

      if (! scopes.Namespaces ()) {
        SEM_ERROR__undeclared (node);
      } else {
        arg_dep_lookup = true;

        // lookup name in associated namespaces as if 
        // the name was qualified by the namespace's name
        for (unsigned i = 0; i < scopes.Namespaces (); i++)
          nl.lookupInAssNs (node, scopes.Namespace (i));

        // name is definitely not known here
        if (! nl.Objects ()) {
          SEM_ERROR__undeclared (node);
        } else {
          if (nl.Objects () == 1 && ! nl.Object (0)->FunctionInfo ())
            nl.reset (current_scope);
          else
            checkDeclsFound (node, nl);
        }
      }
    } else
      SEM_ERROR__undeclared (node);
  }

  // validate name lookup result
  checkDeclsFound (node, nl);

  // name found
  if (nl.Objects ()) {
    info = nl.Object (0);

    // not yet complete! (see ���13.4 etc)

    // possibly overloaded function; resolve it
    if (fct_call && info->FunctionInfo ()) {
      // do an argument-dependent name lookup 
      // if function is not a class member
      if (! arg_dep_lookup && ! info->isClassMember ()) {
        CCAssocScopes scopes (fct_call->Arguments ());
        if (scopes.Namespaces ()) {
          for (unsigned i = 0; i < scopes.Namespaces (); i++)
            nl.lookupInAssNs (node, scopes.Namespace (i));

          if (! checkDeclsFound (node, nl))
            info = 0; // to show the algorithm that there's something wrong
        }    
      }

      // do overload resolution
      if (info) {
        CCOverloading ovl (err);
        ovl.addCandidates (nl);
        ovl.resolve (fct_call);

        // check result of function overload resolution
        if (validOvlFctSet (node, "function", node->Text (), ovl))
          info = ovl.Function ();
        else
          info = 0;
      } 
    }
  }
  
  // name found
  if (info) {
    // determine result type: if the name occured in the scope of a
    // const/volatile method and it's the name of a class member 
    // that is not a reference nor static or mutable, its type also 
    // becomes const/volatile
    result = info->TypeInfo ();
    if (current_scope->LocalScope () && info->AttributeInfo () && 
        ! info->isStatic () && ! info->isMutable () && ! result->isAddress ()) {
      CFunctionInfo *fct = current_scope->LocalScope ()->Function ();
      if (fct) {
        CTypeFunction *ft = fct->TypeInfo ();
        if (fct->isMethod () && (ft->isConst () || ft->isVolatile ())) {
          CRecord *ac = info->AttributeInfo ()->Record ();
          CClassInfo *fc = fct->Record ()->ClassInfo ();
          if (ac && fc && (*ac == *fc || fc->isBaseClass (ac->ClassInfo (), true))) {
            // class data member used in a const/volatile method of the same or derived class
            // the type of the data member is changed to be const/volatile
            new_type = true;
            CTypeQualified *qt = result->TypeQualified ();
            if (qt) {
              qt = new CTypeQualified (qt->BaseType ()->Duplicate (), 
                (ft->isConst () || qt->isConst ()), (ft->isVolatile () || qt->isVolatile ()), false);
            } else {
              qt = new CTypeQualified (result->Duplicate (), 
                ft->isConst (), ft->isVolatile (), false);
            }
            result = qt;
          }
        }
      }
    }

    node->Object (info);
    if (node != node->Name ())
      node->Name ()->Object (info);

    // name of an enumeration constant
    if (info->EnumeratorInfo ()) {
      if (info->EnumeratorInfo ()->hasValue ()) {
        v = info->EnumeratorInfo ()->Value ();
        node->setValue (new CConstant (v, result));
        if (node != node->Name ())
          node->Name ()->setValue (new CConstant (v, result));
      }
      else if (isDependent (info->Init ())) {
        dependent = true;
      }
    // name of a constant variable or class data member
    } else if (info->AttributeInfo () && info->TypeInfo ()->isConst ()) {
      init = info->AttributeInfo ()->Init ();
      if (init && init->Value () && init->Value ()->Constant ()) {
        node->setValueRef (init->Value ()->Constant ());
        if (node != node->Name ())
          node->Name ()->setValueRef (init->Value ()->Constant ());
      }
      else if (isDependent (init)) {
        dependent = true;
      }
    }

    // name of a template parameter or type depends on a
    // template parameter => expression is dependent
    if (info->TemplateParamInfo () ||
        (info->TypeInfo () && info->TypeInfo ()->isDependent ()))
      dependent = true;
  }

  if (new_type) {
    node->setType (result);
    if (node != node->Name ())
      node->Name ()->setType (result->Duplicate ());
  } else {
    node->setTypeRef (result);
    if (node != node->Name ())
      node->Name ()->setTypeRef (result);
  }
  return result;
}


// binary expressions
CTypeInfo *CCSemExpr::resolve (CT_BinaryExpr *node, CTree *base) {
  CTypeInfo *result, *oop;
  const char *op;
  bool dep_base;
  int oper;

  dep_base = resetDependent ();
  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();
  result = &CTYPE_UNDEFINED;

  // resolve operands
  resolve (node->Son (0), node);
  resolve (node->Son (2), node);

  // handle dependent expressions
  if (isDependent (node, dep_base))
    return result;

  // apply lvalue-to-rvalue conversions
  if (oper != TOK_COMMA) // see ���5.18 comma operator
    convLvalueToRvalue (node->Son (0));
  convLvalueToRvalue (node->Son (2));

  // operator may be overloaded (���13.5 pointer-to-member 
  // operator ".*" cannot be overloaded)
  if (oper != TOK_DOT_STAR) { 
    dep_base = resetDependent ();
    oop = overloadedOperator (node, base, op, oper, node->Son (0), node->Son (2));
    if (isDependent (node, dep_base)) {
      return result;
    } else if (oop)
      return oop;
  }
  
  // ���5.5 pointer-to-member operators: op1 .* op2; op1 ->* op2
  if (oper == TOK_DOT_STAR || oper == TOK_PTS_STAR)
    result = binary_ptm (node, base);
  // ���5.6 multiplicative operators: op1 * op2; op1 / op2; op1 % op2
  else if (oper == TOK_MUL || oper == TOK_DIV || oper == TOK_MODULO)
    result = binary_mul (node, base);
  // ���5.7 additive operators: op1 + op2; op1 - op2
  else if (oper == TOK_PLUS || oper == TOK_MINUS)
    result = binary_add (node, base);
  // ���5.8 shift operators: op1 << op2; op1 >> op2
  else if (oper == TOK_LSH || oper == TOK_RSH) 
    result = binary_shift (node, base);
  // ���5.9 relational operators: op1 < op2; op1 > op2; op1 <= op2; op1 >= op2
  else if (oper == TOK_LESS || oper == TOK_GREATER || 
           oper == TOK_LEQ || oper == TOK_GEQ) 
    result = binary_rel (node, base);
  // ���5.10 equality operators: op1 == op2; op1 != op2
  else if (oper == TOK_EQL || oper == TOK_NEQ) 
    result = binary_equ (node, base);
  // ���5.11 bitwise AND operator: op1 & op2
  // ���5.12 bitwise exclusive OR operator: op1 ^ op2
  // ���5.13 bitwise inclusive OR operator: op1 | op2
  else if (oper == TOK_AND || oper == TOK_ROOF || oper == TOK_OR)
    result = binary_bit (node, base);
  // ���5.14 logical AND operator: op1 && op2
  // ���5.15 logical OR operator: op1 || op2
  else if (oper == TOK_AND_AND || oper == TOK_OR_OR)
    result = binary_log (node, base);
  // ���5.18 comma operator: op1 , op2
  else if (oper == TOK_COMMA)
    result = binary_comma (node, base);
  // ���5.17 assignment operators: op1 = op2; op1 *= op2; op1 /= op2;
  //   op1 %= op2; op1 += op2; op1 -= op2; op1 >>= op2; 
  //   op1 <<= op2; op1 &= op2; op1 ^= op2; op1 |= op2
  else if (oper == TOK_ASSIGN || oper == TOK_MUL_EQ || oper == TOK_DIV_EQ ||
           oper == TOK_MOD_EQ || oper == TOK_ADD_EQ || oper == TOK_SUB_EQ ||
           oper == TOK_RSH_EQ || oper == TOK_LSH_EQ || oper == TOK_AND_EQ ||
           oper == TOK_XOR_EQ || oper == TOK_IOR_EQ) 
    result = binary_ass (node, base);

  node->setTypeRef (result);
  
  // transform the expression into a call expression with a built-in operator
  // as the target function
  CTypeInfo *t1 = node->Son (0)->Type (); 
  CTypeInfo *t2 = node->Son (2)->Type ();
  builtinOpExpr (node, base, op, oper, result, t1, t2);
  
  return result;
}


// ���5.17 assignment operators
CTypeInfo *CCSemExpr::binary_ass (CT_BinaryExpr *node, CTree *base) {
  CTypeInfo *t1, *result;
  unsigned long pos;
  const char *op;
  CConstant *v2;
  int oper;

  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();
  pos = node->Son (1)->token_node ()->Number ();
  result = t1 = node->Son (0)->Type (); 

  // error diagnostics not yet complete!

  if (! isLvalue (node->Son (0))) {
    SEM_ERROR (node, "non-lvalue in assignment");
  } else {
    if (! isModifiable (t1, pos))
      SEM_WARNING (node, "assignment of unmodifiable location");
    if (! t1->isRecord ())
      result = t1->UnqualType ();
    else if (! t1->VirtualType ()->isComplete (pos))
      SEM_ERROR (node, "assignment to incomplete type");
      
    if (oper == TOK_ADD_EQ || oper == TOK_SUB_EQ) {
      if (! t1->isScalar ())
        SEM_ERROR__left_op (node, op, "not a scalar type");
    } else if (oper != TOK_ASSIGN) {
      if (! t1->isArithmetic ())
        SEM_ERROR__left_op (node, op, "not an arithmetic type");
    }
  
    v2 = node->Son (2)->Value () ? node->Son (2)->Value ()->Constant () : 0;
    if (v2) {
      if ((oper == TOK_MOD_EQ || oper == TOK_DIV_EQ) && v2->isNull ()) {
        SEM_WARNING (node, "division by zero");
      } else if ((oper == TOK_RSH_EQ || oper == TOK_LSH_EQ) && v2->isNegative ()) {
        SEM_WARNING (node, "negative shift count");
      }
    }
  }
  
  return result;
}


// ���5.18 comma operator
CTypeInfo *CCSemExpr::binary_comma (CT_BinaryExpr *node, CTree *base) {
  return node->Son (2)->Type ();
}


// ���5.14 logical AND operator
// ���5.15 logical OR operator
CTypeInfo *CCSemExpr::binary_log (CT_BinaryExpr *node, CTree *base) {
  CTypeInfo *t1, *t2;
  const char *op;
  int oper;

  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();
  t1 = node->Son (0)->Type (); 
  t2 = node->Son (2)->Type ();

  // error diagnostics not yet complete!

  if (! (t1->isScalar () && t2->isScalar ())) {
    SEM_ERROR__invalid_op (node, "binary", op);
  } else {
    if (! t1->VirtualType ()->is_bool ())
      castToType (&CTYPE_BOOL, node->Son (0), true);
    if (! t2->VirtualType ()->is_bool ())
      castToType (&CTYPE_BOOL, node->Son (2), true);

    apply_binary_op (node, oper);
  }

  return &CTYPE_BOOL;
}


// ���5.11 bitwise AND operator
// ���5.12 bitwise exclusive OR operator
// ���5.13 bitwise inclusive OR operator
CTypeInfo *CCSemExpr::binary_bit (CT_BinaryExpr *node, CTree *base) {
  CTypeInfo *t1, *t2, *result;
  const char *op;
  int oper;

  result = &CTYPE_UNDEFINED;
  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();
  t1 = node->Son (0)->Type (); 
  t2 = node->Son (2)->Type ();

  // error diagnostics not yet complete!

  if (! (t1->isInteger () && t2->isInteger ())) {
    SEM_ERROR__invalid_op (node, "binary", op);
  } else {
    // apply usual arithmetic conversions
    result = usualArithmeticConv (node->Son (0), node->Son (2));
    result = result->NonReferenceType (); // yields an r-value
    apply_binary_op (node, oper);
  }
  return result;
}


// ���5.10 equality operators
CTypeInfo *CCSemExpr::binary_equ (CT_BinaryExpr *node, CTree *base) {
  CTypeInfo *t1, *t2;
  const char *op;
  CConstant *v1;
  int oper;
  bool l;

  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();
  t1 = node->Son (0)->Type (); 
  t2 = node->Son (2)->Type ();

  // error diagnostics not yet complete!

  if (t1->isArithmetic () && t2->isArithmetic ()) {
    // apply usual arithmetic conversions
    usualArithmeticConv (node->Son (0), node->Son (2));
  } else if ((t1->isInteger () && t2->isPointer ()) ||
             (t2->isInteger () && t1->isPointer ())) {
    l = (t1->isInteger ());
    v1 = node->Son (l?0:2)->Value () ? node->Son (l?0:2)->Value ()->Constant () : 0;
    if (! (v1 && v1->isNull ())) { 
      // not a null pointer constant
      SEM_ERROR__comp_between (node, "pointer and integer");
    } else { 
      // implicitly cast integer constant to pointer type
      castToType (l?t2:t1, node->Son (l?0:2), true);
    }
  } else if (! (t1->isScalar () && t2->isScalar ()))
    SEM_ERROR__invalid_op (node, "binary", op);

  apply_binary_op (node, oper);
  return &CTYPE_BOOL;
}


// ���5.9 relational operators
CTypeInfo *CCSemExpr::binary_rel (CT_BinaryExpr *node, CTree *base) {
  CTypeInfo *t1, *t2;
  const char *op;
  int oper;

  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();
  t1 = node->Son (0)->Type (); 
  t2 = node->Son (2)->Type ();

  // error diagnostics not yet complete!

  if (t1->isArithmetic () && t2->isArithmetic ())
    // apply usual arithmetic conversions
    usualArithmeticConv (node->Son (0), node->Son (2));
  else if (! (t1->isScalar () && t2->isScalar ()))
    SEM_ERROR__invalid_op (node, "binary", op);
    
  apply_binary_op (node, oper);
  return &CTYPE_BOOL;
}


// ���5.8 shift operators
CTypeInfo *CCSemExpr::binary_shift (CT_BinaryExpr *node, CTree *base) {
  CTypeInfo *t1, *t2, *result;
  const char *op;
  CConstant *v2;
  int oper;

  result = &CTYPE_UNDEFINED;
  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();
  t1 = node->Son (0)->Type (); 
  t2 = node->Son (2)->Type ();

  // error diagnostics not yet complete!

  if (! (t1->isInteger () && t2->isInteger ())) {
    SEM_ERROR__invalid_op (node, "binary", op);
  } else {
    // apply integral promotions on both operands
    t1 = intPromotion (node->Son (0));
    t2 = intPromotion (node->Son (2));
    result = t1->NonReferenceType (); // yields an r-value

    v2 = node->Son (2)->Value () ? node->Son (2)->Value ()->Constant () : 0;
    if (v2 && v2->isNegative ())
      SEM_WARNING (node, "negative shift count");

    apply_binary_op (node, oper);
  }
  
  return result;
}


// ���5.7 additive operators
CTypeInfo *CCSemExpr::binary_add (CT_BinaryExpr *node, CTree *base) {
  CTypeInfo *t1, *t2, *result;
  int oper;

  result = &CTYPE_UNDEFINED;
  oper = node->Son (1)->token ()->type ();
  t1 = node->Son (0)->Type (); 
  t2 = node->Son (2)->Type ();

  // error diagnostics not yet complete!

  if (t1->isArithmetic () && t2->isArithmetic ()) {
    // apply usual arithmetic conversions
    result = usualArithmeticConv (node->Son (0), node->Son (2));
  } else if (oper == TOK_PLUS) {
    if (! (t1->isArithmetic () && t2->isPointer ()) &&
        ! (t2->isArithmetic () && t1->isPointer ())) {
      SEM_ERROR__invalid_op (node, "binary", "+");
    } else {
      result = t1->isPointer () ? t1 : t2;
    }
  } else { // oper == TOK_MINUS
    if (! (t1->isPointer () && t2->isInteger ()) &&
        ! (t1->isPointer () && t2->isPointer () && sameUnqualBaseType (t1, t2))) {
      SEM_ERROR__invalid_op (node, "binary", "-");
    } else {                       /* ptrdiff_t */
      result = t2->isPointer () ? CTypeInfo::CTYPE_PTRDIFF_T : t1;
    }
  }
  
  result = result->NonReferenceType (); // yields a non-reference type
  
  apply_binary_op (node, oper);
  return result;
}


// ���5.6 multiplicative operators
CTypeInfo *CCSemExpr::binary_mul (CT_BinaryExpr *node, CTree *base) {
  CTypeInfo *t1, *t2, *result;
  const char *op;
  CConstant *v2;
  int oper;

  result = &CTYPE_UNDEFINED;
  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();
  t1 = node->Son (0)->Type (); 
  t2 = node->Son (2)->Type ();

  // error diagnostics not yet complete!

  if (oper == TOK_MODULO ? ! (t1->isInteger () && t2->isInteger ()) :
                           ! (t1->isArithmetic () && t2->isArithmetic ())) {
    SEM_ERROR__invalid_op (node, "binary", op);
  } else {
    // apply usual arithmetic conversions
    result = usualArithmeticConv (node->Son (0), node->Son (2));
    result = result->NonReferenceType (); // yields an r-value

    v2 = node->Son (2)->Value () ? node->Son (2)->Value ()->Constant () : 0;
    if ((oper == TOK_MODULO || oper == TOK_DIV) && v2 && v2->isNull ()) 
      SEM_WARNING (node, "division by zero");
    apply_binary_op (node, oper);
  }
  
  return result;
}


// ���5.5 pointer-to-member operators
CTypeInfo *CCSemExpr::binary_ptm (CT_BinaryExpr *node, CTree *base) {
  CTypeInfo *t1, *t2, *result;
  CRecord *pclass, *oclass;
  const char *op;
  int oper;

  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();
  t1 = node->Son (0)->Type (); 
  t2 = node->Son (2)->Type ();

  result = &CTYPE_UNDEFINED;
  if (t2->TypeMemberPointer ())
    result = t2->BaseType ();

  // error diagnostics not yet complete!

  if (! t2->TypeMemberPointer ()) {
    SEM_ERROR__right_op (node, op, "not pointer to member");
  } else if (oper == TOK_DOT_STAR && ! t1->isRecord ()) {
    SEM_ERROR__left_op (node, op, "not of class type");
  } else if (oper == TOK_PTS_STAR && ! (t1->isPointer () && 
            t1->VirtualType ()->BaseType ()->isRecord ())) {
    SEM_ERROR__left_op (node, op, "not pointer to class type");
  } else {
    pclass = t2->TypeMemberPointer ()->Record ();
    if (oper == TOK_DOT_STAR)
      oclass = t1->VirtualType ()->TypeRecord ()->Record ();
    else
      oclass = t1->VirtualType ()->BaseType ()->VirtualType ()->
               TypeRecord ()->Record ();
    if (! sameOrBaseClass (pclass, oclass))
      SEM_ERROR (node, "member type `" 
        << (pclass ? pclass->Name ().c_str () : "<UNKNOWN>") << "' " 
        << "incompatible with object type `" << oclass->Name () << "'");
  }
  
  return result;
}


// ���5.3 unary expression
CTypeInfo *CCSemExpr::resolve (CT_UnaryExpr *node, CTree *base) {
  CTypeInfo *result, *t1, *oop;
  unsigned long pos;
  const char *op;
  bool dep_base;
  CConstant *v1;
  int oper;

  dep_base = resetDependent ();
  result = &CTYPE_UNDEFINED;
  op = node->Son (0)->token ()->text ();
  oper = node->Son (0)->token ()->type ();
  pos = node->Son (1)->token_node ()->Number ();

  // resolve operands
  t1 = resolve (node->Son (1), node);

  // handle dependent expressions
  if (isDependent (node, dep_base))
    return result;

  // apply lvalue-to-rvalue conversions
  t1 = convLvalueToRvalue (node->Son (1));

  // operator may be overloaded
  dep_base = resetDependent ();
  oop = overloadedOperator (node, base, op, oper, node->Son (1));
  if (isDependent (node, dep_base)) {
    if (oper == TOK_NOT) {
      result = &CTYPE_BOOL;
      node->setTypeRef (result);
    }
    return result;
  } else if (oop) {
    return oop;
  } else
    t1 = node->Son (1)->Type ();

  // prefix in-/decrement: ++ op1; -- op1
  if (oper == TOK_DECR || oper == TOK_INCR) {
    if (! isLvalue (node->Son (1))) { 
      SEM_ERROR (node, "non-lvalue in prefix increment/decrement");
    } else {
      if (! isModifiable (t1, pos)) {
        if (oper == TOK_DECR) {
          SEM_WARNING (node, "decrement of unmodifiable location");
        } else {
          SEM_WARNING (node, "increment of unmodifiable location");
        }
      }
      if (! t1->isScalar () ||
          (oper == TOK_DECR && t1->VirtualType ()->is_bool ())) {
        SEM_ERROR__invalid_op (node, "unary", op);
      }
    }
    result = t1->UnqualType ();
  // + op1; - op1; ~ op1
  } else if (oper == TOK_PLUS || oper == TOK_MINUS || oper == TOK_TILDE) {
    if ((oper == TOK_PLUS && t1->isScalar ()) ||
        (oper == TOK_MINUS && t1->isArithmetic ()) ||
        (oper == TOK_TILDE && t1->isInteger ())) {
      if (t1->isInteger ())
        t1 = intPromotion (node->Son (1));
      v1 = node->Son (1)->Value () ? node->Son (1)->Value ()->Constant () : 0;
      if (v1 && ! t1->isPointer ())
        node->setValue (v1->compute (oper));
    } else {
      SEM_ERROR__invalid_op (node, "unary", op);
    }
    result = t1;
  // ! op1
  } else if (oper == TOK_NOT) {
    if (! t1->isScalar ()) {
      SEM_ERROR__invalid_op (node, "unary", op);
    } else {
      if (! t1->VirtualType ()->is_bool ())
        t1 = castToType (&CTYPE_BOOL, node->Son (1), true);
      v1 = node->Son (1)->Value () ? node->Son (1)->Value ()->Constant () : 0;
      if (v1)
        node->setValue (v1->compute (oper));
    }
    result = &CTYPE_BOOL;
  }

  node->setTypeRef (result);

  // transform the expression into a call expression with a built-in operator
  // as the target function
  t1 = node->Son (1)->Type ();
  builtinOpExpr (node, base, op, oper, result, t1);

  return result;
}


// ���5.2.6 postfix increment and decrement
CTypeInfo *CCSemExpr::resolve (CT_PostfixExpr *node, CTree *base) {
  CTypeInfo *result, *t1, *oop;
  CT_Integer *second;
  unsigned long pos;
  const char *op;
  bool dep_base;
  int oper;
  
  dep_base = resetDependent ();
  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();
  pos = node->Son (1)->token_node ()->Number ();
  
  // resolve operand
  t1 = resolve (node->Son (0), node); 
  
  // handle dependent expressions
  if (isDependent (node, dep_base))
    return &CTYPE_UNDEFINED;

  // apply lvalue-to-rvalue conversions
  t1 = convLvalueToRvalue (node->Son (0));

  // operator may be overloaded
  if (isClassOrEnum (t1)) {
    // postfix increment and decrement operators have an
    // imlicit second operand of type int and value 0,
    // create this operand (a little bit tricky :( )
    second = new CT_Integer (node->Son (1));
    second->setTypeRef (&CTYPE_INT);
    second->setValue (new CConstant ((U_LONG_LONG)0, &CTYPE_INT));
    dep_base = resetDependent ();
    oop = overloadedOperator (node, base, op, oper, node->Son (0), second);
    delete second;
    // restore parent pointer
    node->ReplaceSon (node->Son (1), node->Son (1));
    if (isDependent (node, dep_base))
      return &CTYPE_UNDEFINED;
    else if (oop)
      return oop;
    else
      t1 = node->Son (0)->Type ();
  }
  
  // postfix in-/decrement: op1 ++; op1 --
  if (! isLvalue (node->Son (0))) { 
    SEM_ERROR (node, "non-lvalue in postfix increment/decrement");
  } else {
    if (! isModifiable (t1, pos)) {
      if (oper == TOK_DECR) {
        SEM_WARNING (node, "decrement of unmodifiable location");
      } else {
        SEM_WARNING (node, "increment of unmodifiable location");
      }
    }
    if (! t1->isScalar () ||
        (oper == TOK_DECR && t1->VirtualType ()->is_bool ())) {
      SEM_ERROR__invalid_op (node, "unary", op);
    }
  }

  result = t1->UnqualType ();
  node->setTypeRef (result);

  // transform the expression into a call expression with a built-in operator
  // as the target function
  builtinOpExpr (node, base, op, oper, result, t1);

  return result;
}


// ���5.2.3 explicit type conversion (functional notation)
CTypeInfo *CCSemExpr::resolve (CT_ConstructExpr *node, CTree *base) {
  CTypeInfo *result;
  CT_ExprList *init;
  unsigned entries;
  bool dep_base;
  
  dep_base = resetDependent ();

  // resolve initializer
  init = node->Initializer ();
  entries = init->Entries ();
  for (unsigned i = 0; i < entries; i++)
    resolve (init->Entry (i), init);

  // handle dependent expressions
  //if (isDependent (node, dep_base))
  //  return &CTYPE_UNDEFINED;

  // error diagnostics not yet implemented!
  
  if (node->TypeName ()->Object ())
    result = node->TypeName ()->Object ()->TypeInfo ();
  else
    result = &CTYPE_UNDEFINED;

  if (result) {
    // constructor call
    if (result->isRecord ()) {
      // to be implemented!
    // cast expression (functional notation)
    } else if (entries == 1) {
      if (! dependent) {
        // apply lvalue-to-rvalue conversions
        convLvalueToRvalue (init->Entry (0));

        // handle constants
        passOnConstant (init->Entry (0), result);
        passOnConstant (init, result);
      }
    }
  }

  if (isDependent (node->TypeName ())) {
    // if the type of the object to be constructed is dependent on a template
    // parameter, the construct expression is dependent as well
    result = &CTYPE_UNDEFINED;
    dependent = true;
  }

  // reset dependent state
  if (dep_base)
    dependent = true;

  node->setTypeRef (result);
  return result;
}


// GNU C/C++ extension
CTypeInfo *CCSemExpr::resolve (CT_CmpdLiteral *node, CTree *base) {
  CTypeInfo *result;
//  bool dep_base;

//  dep_base = resetDependent ();

  // handle dependent expressions
//  if (isDependent (node, dep_base))
//    return &CTYPE_UNDEFINED;

  // evaluation not yet complete!

  if (node->TypeName ()->Object ())
    result = node->TypeName ()->Object ()->TypeInfo ();
  else
    result = &CTYPE_UNDEFINED;
  
  node->setTypeRef (result);
  return result;
}


// ���5.3.4 new expression
CTypeInfo *CCSemExpr::resolve (CT_NewExpr *node, CTree *base) {
  CTypeInfo *result;
  unsigned entries;
  bool dep_base;
  
  dep_base = resetDependent ();
  result = &CTYPE_UNDEFINED;

  // resolve placement
  if (node->Placement ()) {
    entries = node->Placement ()->Entries ();
    for (unsigned i = 0; i < entries; i++)
      resolve (node->Placement ()->Entry (i), node->Placement ());
  }

  // resolve initializer
  if (node->Initializer ()) {
    entries = node->Initializer ()->Entries ();
    for (unsigned i = 0; i < entries; i++)
      resolve (node->Initializer ()->Entry (i), node->Initializer ());
  }

  // handle dependent expressions
  if (isDependent (node, dep_base))
    return result;

  // error diagnostics not yet implemented!

  if (node->TypeName ()->Object ()) {
    result = node->TypeName ()->Object ()->TypeInfo ();
    if (result->isArray ())
      result = result->VirtualType ()->BaseType ();
    else if (result->UnqualType()->TypeClass())
      result->UnqualType()->TypeClass()->instantiate(current_scope);
    result = new CTypePointer (result->Duplicate ());
    node->setType (result);
  } else
    node->setTypeRef (result);
  
  return result;
}


// ���5.3.5 delete expression
CTypeInfo *CCSemExpr::resolve (CT_DeleteExpr *node, CTree *base) {
  CTypeInfo *result, *t1;
  bool dep_base;
  
  dep_base = resetDependent ();
  result = &CTYPE_VOID;

  // resolve operands
  t1 = resolve (node->Expr (), node); 

  // handle dependent expressions
  if (isDependent (node, dep_base)) {
    node->setTypeRef (result);
    return result;
  }

  // error diagnostics not yet implemented!
  
  node->setTypeRef (result);
  return result;
}


// ���5.16 conditional operator
CTypeInfo *CCSemExpr::resolve (CT_IfThenExpr *node, CTree *base) {
  CTypeInfo *result, *t1, *t2, *t3;
  CTree *op1, *op2, *op3;
  CConstant *v1, *v2;
  bool dep_base;

  dep_base = resetDependent ();
  result = &CTYPE_UNDEFINED;

  // resolve operands
  t1 = resolve (node->Son (0), node);
  t2 = resolve (node->Son (2), node);
  t3 = resolve (node->Son (4), node);
  op1 = node->Son (0);
  op2 = node->Son (2);
  op3 = node->Son (4);

  // handle dependent expressions
  if (isDependent (node, dep_base))
    return result;

  // error diagnostics not yet complete!

  if (! t1->is_bool ()) {
    t1 = castToType (&CTYPE_BOOL, op1, true);
    op1 = node->Son (0);
  }

  // either of the operands has type void
  if (t2->UnqualType ()->is_void () || t3->UnqualType ()->is_void ()) {
    // apply lvalue-to-rvalue conversions
    t2 = convLvalueToRvalue (op2);
    t3 = convLvalueToRvalue (op3);
    op2 = node->Son (2);
    op3 = node->Son (4);

    if (baseTree (op2)->NodeName () == CT_ThrowExpr::NodeId ())
      result = t3;
    else if (baseTree (op3)->NodeName () == CT_ThrowExpr::NodeId ())
      result = t2;
    else if (t2->UnqualType ()->is_void () && t3->UnqualType ()->is_void ())
      result = &CTYPE_VOID;
    else
      SEM_ERROR__invalid_op (node, "conditional", "?:");
    node->setTypeRef (result);
  // equal types and operands are lvalues
  } else if (*t2->VirtualType () == *t3->VirtualType () &&
             isLvalue (op2) && isLvalue (op3)) {
    result = t2;
    node->setTypeRef (result);
  // types are different and one is a class type
  } else if (*t2 != *t3 && (t2->isRecord () || t3->isRecord ())) {
    // ���5.16.3,5 convert the second operand to the third and vice versa
    CCConversions cvs(err);
    CCConvSeq *seq1 = cvs.implicitConversions (t2, t3, op3, node, true);
    CCConvSeq *seq2 = cvs.implicitConversions (t3, t2, op2, node, true);
    if (seq1 && !seq2) {
      seq1->Apply ();
      result = t2;
      node->setTypeRef (result);
    } else if (!seq1 && seq2) {
      seq2->Apply ();
      result = t3;
      node->setTypeRef (result);
    }
    if (seq1) delete seq1;
    if (seq2) delete seq2;
  }

  // ���5.16.6
  if (result == &CTYPE_UNDEFINED) {
    // apply lvalue-to-rvalue conversions
    t2 = convLvalueToRvalue (op2);
    t3 = convLvalueToRvalue (op3);
    op2 = node->Son (2);
    op3 = node->Son (4);

    // types are equal; result is t2
    if (*t2->VirtualType () == *t3->VirtualType ()) {
      result = t2;
      node->setTypeRef (result);
    // arithmetic operands
    } else if (t2->isArithmetic () && t3->isArithmetic ()) {
      result = usualArithmeticConv (op2, op3);
      op2 = node->Son (2);
      op3 = node->Son (4);
      node->setTypeRef (result);
    // pointer types
    } else {
      v1 = op2->Value () ? op2->Value ()->Constant () : 0;
      v2 = op3->Value () ? op3->Value ()->Constant () : 0;

      // pointer to pointer conversion
      if (t2->isPointer () && t3->isPointer ()) {
        // not yet implemented!
        result = t2;
      // pointer to null pointer conversion
      } else if (t2->isPointer () && t3->isArithmetic () && v2 && v2->isNull ()) {
        result = castToType (t2, op3, true);
        op3 = node->Son (4);
      } else if (t3->isPointer () && t2->isArithmetic () && v1 && v1->isNull ()) {
        result = castToType (t3, op2, true);
        op2 = node->Son (2);
      } else
        SEM_ERROR__invalid_op (node, "conditional", "?:");
      node->setTypeRef (result);
    }
  }

  // handle constants
  v1 = op1->Value () ? op1->Value ()->Constant () : 0;
  if (v1) {
    if (v1->isNull ())
      passOnConstant (op3, result);
    else
      passOnConstant (op2, result);
  }

  return result;
}


// ���5.3.1.2 indirection operator
CTypeInfo *CCSemExpr::resolve (CT_DerefExpr *node, CTree *base) {
  CTypeInfo *result, *t1, *oop;
  bool dep_base;

  dep_base = resetDependent ();

  // resolve operands
  t1 = resolve (node->Son (1), node);

  // handle dependent expressions
  if (isDependent (node, dep_base))
    return &CTYPE_UNDEFINED;

  // apply lvalue-to-rvalue conversions
  t1 = convLvalueToRvalue (node->Son (1));

  // operator may be overloaded
  dep_base = resetDependent ();
  oop = overloadedOperator (node, base, "*", node->Son (0)->token ()->type (), node->Son (1));
  if (isDependent (node, dep_base))
    return &CTYPE_UNDEFINED;
  else if (oop)
    return oop;
  else
    t1 = node->Son (1)->Type ();

  // error diagnostics not yet complete!

  bool error = true;
  if (! t1->isPointer ()) {
    SEM_ERROR__invalid_op (node, "unary", "*");
    result = &CTYPE_UNDEFINED;
  } else {
    if (t1->VirtualType ()->BaseType ()->isVoid ()) {
      SEM_ERROR (node, "dereferencing pointer to void");
      result = &CTYPE_UNDEFINED;
    }
    else
      error = false;
  }

  if (error)
    node->setTypeRef (result);
  else {
    // if type is a class object type, ensure
    // that the class is really instantiated
    t1 = t1->VirtualType ()->BaseType ()->Duplicate ();
    if (t1->UnqualType()->TypeClass()) {
      t1->UnqualType()->TypeClass()->instantiate(current_scope);
    }
    result = new CTypeAddress (t1);
    node->setType (result);
    // transform the expression into a call expression with a built-in operator
    // as the target function
    t1 = node->Son (1)->Type (); 
    builtinOpExpr (node, base, "*", node->Son (0)->token ()->type (), result, t1);
  }
  return result;
}


// ���5.3.1.2 address operator
CTypeInfo *CCSemExpr::resolve (CT_AddrExpr *node, CTree *base) {
  CTypeInfo *result, *t1, *oop;
  CObjectInfo *info;
  bool dep_base;
  CTree *op1;
  
  dep_base = resetDependent ();
  result = &CTYPE_UNDEFINED;

  // resolve operand
  t1 = resolve (node->Son (1), node); 
  op1 = node->Son (1);

  // handle dependent expressions
  if (isDependent (node, dep_base))
    return result;

  // operator may be overloaded
  dep_base = resetDependent ();
  oop = overloadedOperator (node, base, "&", node->Son (0)->token ()->type (), node->Son (1));
  if (isDependent (node, dep_base))
    return result;
  else if (oop)
    return oop;
  else {
    op1 = node->Son (1);
    t1 = op1->Type ();
  }

  // error diagnostics not yet complete!

  // address of class member
  bool error = false;
  if ((op1->NodeName () == CT_QualName::NodeId () ||
       op1->NodeName () == CT_RootQualName::NodeId ()) &&
      (info = ((CT_QualName*)op1)->Name ()->Object ()) &&
      info->isClassMember ()) {
    if (info->isStatic ())
      result = new CTypePointer (t1->Duplicate ());
    else if (info->FunctionInfo ())
      result = new CTypeMemberPointer (t1->Duplicate (), 
                                       info->FunctionInfo ()->Record ());
    else if (info->AttributeInfo ())
      result = new CTypeMemberPointer (t1->Duplicate (), 
                                       info->AttributeInfo ()->Record ());
    node->setType (result);
  // address of lvalue
  } else if (isLvalue (op1)) {
    if (t1->isAddress ())
      t1 = t1->BaseType ();
    result = new CTypePointer (t1->Duplicate ());
    node->setType (result);
  // neither class member nor lvalue
  } else {
    SEM_ERROR__invalid_op (node, "unary", "&");
    result = &CTYPE_UNDEFINED;
    node->setTypeRef (result);
    error = true;
  }

  if (!error) {
    // transform the expression into a call expression with a built-in operator
    // as the target function
    builtinOpExpr (node, base, "&", node->Son (0)->token ()->type (), result, t1);
  }

  return result;
}


// ���5.4 explicit type conversion (cast notation)
CTypeInfo *CCSemExpr::resolve (CT_CastExpr *node, CTree *base) {
  CTypeInfo *result, *t1;
  bool dep_base;

  dep_base = resetDependent ();

  // resolve operands
  t1 = resolve (node->Expr (), node); 

  // let casts yield a type independently of the
  // dependence of its operand (_not_ standard 
  // conform but safe to do)

  // handle dependent expressions
  //if (isDependent (node, dep_base))
  //  return &CTYPE_UNDEFINED;

  // apply lvalue-to-rvalue conversions
  if (! dependent)
    t1 = convLvalueToRvalue (node->Expr ());

  // error diagnostics not yet implemented!

  if (node->TypeName ()->Object ())
    result = node->TypeName ()->Object ()->TypeInfo ();
  else
    result = &CTYPE_UNDEFINED;

  // handle constants
  if (! dependent)
    passOnConstant (node->Expr (), result);

  // reset dependent state
  if (dep_base)
    dependent = true;

  node->setTypeRef (result);
  return result;
}


// ���5.2.7 dynamic cast
CTypeInfo *CCSemExpr::resolve (CT_DynamicCast *node, CTree *base) {
  CTypeInfo *result, *t1;
  bool dep_base;

  dep_base = resetDependent ();

  // resolve operands
  t1 = resolve (node->Expr (), node); 

  // let casts yield a type independently of the
  // dependence of its operand (_not_ standard 
  // conform but safe to do)

  // handle dependent expressions
  //if (isDependent (node, dep_base))
  //  return &CTYPE_UNDEFINED;

  // apply lvalue-to-rvalue conversions
  
  if (! dependent)
    t1 = convLvalueToRvalue (node->Expr ());

  // error diagnostics not yet implemented!

  if (node->TypeName ()->Object ())
    result = node->TypeName ()->Object ()->TypeInfo ();
  else
    result = &CTYPE_UNDEFINED;

  // reset dependent state
  if (dep_base)
    dependent = true;

  node->setTypeRef (result);
  return result;
}


// ���5.2.9 static cast
CTypeInfo *CCSemExpr::resolve (CT_StaticCast *node, CTree *base) {
  CTypeInfo *result, *t1;
  bool dep_base;
  
  dep_base = resetDependent ();

  // resolve operands
  t1 = resolve (node->Expr (), node); 

  // let casts yield a type independently of the
  // dependence of its operand (_not_ standard 
  // conform but safe to do)

  // handle dependent expressions
  //if (isDependent (node, dep_base))
  //  return &CTYPE_UNDEFINED;

  // apply lvalue-to-rvalue conversions
  if (! dependent)
    t1 = convLvalueToRvalue (node->Expr ());

  // error diagnostics not yet implemented!

  if (node->TypeName ()->Object ())
    result = node->TypeName ()->Object ()->TypeInfo ();
  else
    result = &CTYPE_UNDEFINED;

  // handle constants
  if (! dependent)
    passOnConstant (node->Expr (), result);

  // reset dependent state
  if (dep_base)
    dependent = true;

  node->setTypeRef (result);
  return result;
}


// ���5.2.10 reinterpret cast
CTypeInfo *CCSemExpr::resolve (CT_ReintCast *node, CTree *base) {
  CTypeInfo *result, *t1;
  bool dep_base;

  dep_base = resetDependent ();

  // resolve operands
  t1 = resolve (node->Expr (), node); 

  // let casts yield a type independently of the
  // dependence of its operand (_not_ standard 
  // conform but safe to do)

  // handle dependent expressions
  //if (isDependent (node, dep_base))
  //  return &CTYPE_UNDEFINED;

  // apply lvalue-to-rvalue conversions
  if (! dependent)
    t1 = convLvalueToRvalue (node->Expr ());

  // error diagnostics not yet implemented!

  if (node->TypeName ()->Object ())
    result = node->TypeName ()->Object ()->TypeInfo ();
  else
    result = &CTYPE_UNDEFINED;

  // handle constants
  if (! dependent)
    passOnConstant (node->Expr (), result);

  // reset dependent state
  if (dep_base)
    dependent = true;

  node->setTypeRef (result);
  return result;
}


// ���5.2.11 const cast
CTypeInfo *CCSemExpr::resolve (CT_ConstCast *node, CTree *base) {
  CTypeInfo *result, *t1;
  bool dep_base;

  dep_base = resetDependent ();

  // resolve operands
  t1 = resolve (node->Expr (), node); 

  // let casts yield a type independently of the
  // dependence of its operand (_not_ standard 
  // conform but safe to do)

  // handle dependent expressions
  //if (isDependent (node, dep_base))
  //  return &CTYPE_UNDEFINED;

  // apply lvalue-to-rvalue conversions
  if (! dependent)
    t1 = convLvalueToRvalue (node->Expr ());

  // error diagnostics not yet implemented!

  if (node->TypeName ()->Object ())
    result = node->TypeName ()->Object ()->TypeInfo ();
  else
    result = &CTYPE_UNDEFINED;

  // handle constants
  if (! dependent)
    passOnConstant (node->Expr (), result);

  // reset dependent state
  if (dep_base)
    dependent = true;

  node->setTypeRef (result);
  return result;
}


// ���15.1 throw expression
CTypeInfo *CCSemExpr::resolve (CT_ThrowExpr *node, CTree *base) {
  CTypeInfo *result, *t1;
  //unsigned long pos;
  bool dep_base;

  result = &CTYPE_VOID; // a throw expression is of type void

  if (node->Expr ()) {
    dep_base = resetDependent ();

    // resolve operands
    t1 = resolve (node->Expr (), node);

    // handle dependent expressions
    if (isDependent (node, dep_base)) {
      node->setTypeRef (result);
      return result; 
    }

    // apply lvalue-to-rvalue conversions
    t1 = convLvalueToRvalue (node->Expr ());

    // error diagnostics not yet complete!

    // if the exception type is a class object type, ensure
    // that the class is really instantiated
    if (t1->UnqualType()->TypeClass()) {
      t1->UnqualType()->TypeClass()->instantiate(current_scope);
    }

    //pos = node->token_node ()->Number ();
    //if (! t1->isComplete (pos))
    //  SEM_ERROR (node, "incomplete type in throw expression");
  }

  node->setTypeRef (result);
  return result;
}


// ��5.2.8 type identification
CTypeInfo *CCSemExpr::resolve (CT_TypeidExpr *node, CTree *base) {
  CTypeInfo *result, *t1;

  bool found_type_info = false;
  CCNameLookup nl (err, current_scope);
  // search namespace std
  nl.lookupNamespace ("std", current_scope->SemDB ()->FileInfo (0));
  if (nl.Objects () == 1) {
    CNamespaceInfo *std = (CNamespaceInfo*)nl.Object (0);
    nl.reset (current_scope);
    // search class type_info in namespace std
    nl.lookupType ("type_info", std);
    if (nl.Objects () == 1)
      found_type_info = true;
  }
  
  if (found_type_info) {
    // get std::type_info's type
    CTypeInfo *type_info = CTypeInfo::Duplicate (nl.Object ()->TypeInfo ());
    // qualify the type with 'const'
    result = new CTypeQualified (type_info, true, false, false);
  } else {
    SEM_ERROR__must_include (node, "<typeinfo>", "typeid");
    result = &CTYPE_UNDEFINED;
  }  
  
  // remember and reset the current dependent expression state
  bool dep_base = resetDependent ();

  // resolve the sub-expression
  if (node->Arg ()->NodeName () != CT_NamedType::NodeId ()) {
    resolve (node->Arg (), node);
  }

  // ensure that the type gets instantiated
  t1 = node->Arg ()->Type ();
  if (t1 && t1->UnqualType()->TypeClass()) {
    t1->UnqualType()->TypeClass()->instantiate(current_scope);
  }

  // reset dependent state
  if (dep_base)
    dependent = true;

  node->setTypeRef (result);
  return result;
}


// ���5.3.3 sizeof expression
CTypeInfo *CCSemExpr::resolve (CT_SizeofExpr *node, CTree *base) {
  U_LONG_LONG size;
  CTypeInfo *result, *t1, *unqual;
  CObjectInfo *info;
  bool dep_base;

  dep_base = resetDependent ();
  result = CTypeInfo::CTYPE_SIZE_T; // size_t
  size = 0;

  // resolve operand; can be either expression or named type
  if (node->TypeName ()) { // named type
    info = node->TypeName ()->Object ();
    t1 = info ? info->TypeInfo () : 0;
    if (t1 && t1->isDependent ())
      dependent = true;
  } else {
    t1 = resolve (node->Expr (), node);
  }

  // handle dependent expressions
  if (isDependent (node, dep_base)) {
    node->setTypeRef (result);
    return result;
  }

  // evaluation not yet complete!

  // sizeof op1; sizeof ( op1 )
  if (! t1 || t1->isFunction ()/* || ! t1->isComplete (pos)*/) {
    SEM_ERROR__invalid_op (node, "unary", "sizeof");
  } else {
    unqual = t1->UnqualType();
    if (unqual->is_char() || unqual->is_signed_char() || unqual->is_unsigned_char()) {
      size = 1; // always 1 byte
    } else {
      if (unqual->TypeClass())
        unqual->TypeClass()->instantiate(current_scope);
      size = t1->Size () / 8; // size in bytes
    }
  }

  node->setValue (new CConstant (size, result));
  node->setTypeRef (result);
  return result;
}


// alignof expression
CTypeInfo *CCSemExpr::resolve (CT_AlignofExpr *node, CTree *base) {
  unsigned long align;
  CTypeInfo *result, *t1;
  CObjectInfo *info;
  bool dep_base;

  dep_base = resetDependent ();
  result = CTypeInfo::CTYPE_SIZE_T; // size_t
  align = 0;

  // resolve operand; can be either expression or named type
  if (node->TypeName ()) { // named type
    info = node->TypeName ()->Object ();
    t1 = info ? info->TypeInfo () : 0;
  } else {
    t1 = resolve (node->Expr (), node);
    //info = findObject (node->Expr ());
  }

  // handle dependent expressions
  if (isDependent (node, dep_base)) {
    node->setTypeRef (result);
    return result;
  }

  // evaluation not yet complete!

  // sizeof op1; sizeof ( op1 )
  if (! t1 || t1->isFunction ()/* || ! t1->isComplete (pos)*/) {
    SEM_ERROR__invalid_op (node, "unary", "alignof");
  } else {
    if (t1->UnqualType()->TypeClass())
      t1->UnqualType()->TypeClass()->instantiate(current_scope);
    align = t1->Align (); // alignment in bytes
  }

  // NOT CORRECT! TO BE FIXED!
  if (! align)
    align = 1;

  node->setValue (new CConstant ((LONG_LONG)align, result));
  node->setTypeRef (result);
  return result;
}


// type trait expression
CTypeInfo *CCSemExpr::resolve (CT_TypeTraitExpr *node, CTree *base) {
  CTypeInfo *result, *t1, *t2;
  CObjectInfo *info;
  bool value = false;

  result = &CTYPE_BOOL;
  value = false;

  // get the type(s)
  info = node->FirstType ()->Object ();
  t1 = info ? info->TypeInfo () : 0;
  t2 = 0;
  if (node->SecondType ()) {
    info = node->SecondType ()->Object ();
    t2 = info ? info->TypeInfo () : 0;
  }

  // evaluation not yet complete!

  // type_trait ( op1 ); type_trait ( op1 , op2 )
  if (! t1 /* || ! t1->isComplete (pos)*/) {
    SEM_ERROR__invalid_op (node, "unary", Token::get_static_text (node->Operator ()));
  } else {
    CRecord* c1 = t1->isClassOrUnion () ? t1->VirtualType ()->TypeRecord ()->Record () : 0;
    CRecord* c2 = t2 && t2->isClassOrUnion () ? t2->VirtualType ()->TypeRecord ()->Record () : 0;
    switch (node->Operator ()) {
      case TOK_HAS_NOTHROW_ASSIGN:
        // TODO: No-throw not checked! Too expensive analysis! Also functions
        //       called in the operator shall not throw exceptions!
        value = ! t1->isConst () && ! t1->isAddress () &&
                (t1->isPOD () || (c1 && c1->hasTrivialAssign ()));
        break;
      case TOK_HAS_NOTHROW_COPY:
        // TODO: No-throw not checked! Too expensive analysis! Also functions
        //       called in the constructor shall not throw exceptions!
        value = t1->isPOD () || t1->isAddress () || (c1 && c1->hasTrivialCopy ());
        break;
      case TOK_HAS_NOTHROW_CTOR:
        // TODO: No-throw not checked! Too expensive analysis! Also functions
        //       called in the constructor shall not throw exceptions!
        value = t1->isPOD () || (c1 && c1->hasTrivialCtor ());
        break;
      case TOK_HAS_TRIVIAL_ASSIGN:
        value = ! t1->isConst () && ! t1->isAddress () &&
                (t1->isPOD () || (c1 && c1->hasTrivialAssign ()));
        break;
      case TOK_HAS_TRIVIAL_COPY:
        value = t1->isPOD () || t1->isAddress () || (c1 && c1->hasTrivialCopy ());
        break;
      case TOK_HAS_TRIVIAL_CTOR:
        value = t1->isPOD () || (c1 && c1->hasTrivialCtor ());
        break;
      case TOK_HAS_TRIVIAL_DTOR:
        value = t1->isPOD () || t1->isAddress () || (c1 && c1->hasTrivialDtor ());
        break;
      case TOK_HAS_VIRTUAL_DTOR:
        value = c1 && c1->ClassInfo () && c1->ClassInfo ()->hasVirtualDtor ();
        break;
      case TOK_IS_ABSTRACT:
        value = c1 && c1->ClassInfo () && c1->ClassInfo ()->isAbstract ();
        break;
      case TOK_IS_BASE_OF:
        value = c1 && c2 && c1->ClassInfo () && c2->ClassInfo () &&
                ((*c1 == *c2) || c2->ClassInfo ()->isBaseClass (c1->ClassInfo (), true));
        break;
      case TOK_IS_CLASS:
        value = t1->isClass ();
        break;
      case TOK_IS_EMPTY:
        value = c1 && c1->ClassInfo () && c1->ClassInfo ()->isEmpty ();
        break;
      case TOK_IS_ENUM:
        value = t1->isEnum ();
        break;
      case TOK_IS_POD:
        value = t1->isPOD ();
        break;
      case TOK_IS_TRIVIAL:
        value = t1->isTrivial ();
        break;
      case TOK_IS_POLYMORPHIC:
        value = c1 && c1->ClassInfo () && c1->ClassInfo ()->isPolymorphic ();
        break;
      case TOK_IS_UNION:
        value = t1->isUnion ();
        break;
      default:
        break;
    }
  }

  node->setValue (new CConstant ((LONG_LONG)value, result));
  node->setTypeRef (result);
  return result;
}


// offsetof expression
CTypeInfo *CCSemExpr::resolve (CT_OffsetofExpr *node, CTree *base) {
  unsigned long offset;
  CTypeInfo *result;

  // TODO: calculate the offset in bytes of the given member
  offset = 0;
  result = CTypeInfo::CTYPE_SIZE_T;

  node->setValue (new CConstant ((LONG_LONG)offset, result));
  node->setTypeRef (result);
  return result;
}


// ���5.2.1 subscripting
CTypeInfo *CCSemExpr::resolve (CT_IndexExpr *node, CTree *base) {
  CTypeInfo *result, *t1, *t2, *oop;
  bool dep_base;
  int oper;
  
  oper = node->Son (1)->token ()->type ();
  dep_base = resetDependent ();

  // resolve operands
  t1 = resolve (node->Son (0), node); 
  t2 = resolve (node->Son (2), node); 
  
  // handle dependent expressions
  if (isDependent (node, dep_base))
    return &CTYPE_UNDEFINED;

  // apply lvalue-to-rvalue conversions
  t1 = convLvalueToRvalue (node->Son (0));

  // operator may be overloaded
  dep_base = resetDependent ();
  oop = overloadedOperator (node, base, "[]", oper, node->Son (0), node->Son (2));
  if (isDependent (node, dep_base))
    return &CTYPE_UNDEFINED;
  else if (oop)
    return oop;
  else {
    t1 = node->Son (0)->Type ();
    t2 = node->Son (2)->Type ();
  }

  // subscripting: op1 [ op2 ]
  bool error = true;
  if (! t1->isPointer () || ! t2->isInteger ()) {
    SEM_ERROR__invalid_op (node, "array subscript", "[]");
    result = &CTYPE_UNDEFINED;
  } else
    error = false;

  if (error)
    node->setTypeRef (result);
  else {
    result = new CTypeAddress (t1->VirtualType ()->BaseType ()->Duplicate ());
    node->setType (result);
    // transform the expression into a call expression with a built-in operator
    // as the target function
    t1 = node->Son (0)->Type (); 
    builtinOpExpr (node, base, "[]", oper, result, t1, t2);
  }
  return result;
}


// ���5.1.5 parenthesized expression
CTypeInfo *CCSemExpr::resolve (CT_BracedExpr *node, CTree *base) {
  CTypeInfo *result;
  bool dep_base;
  
  dep_base = resetDependent ();
  
  // resolve operands
  result = resolve (node->Expr (), node);

  // handle dependent expressions
  if (isDependent (node, dep_base))
    return &CTYPE_UNDEFINED;

  // handle constants
  passOnConstant (node->Expr (), result);

  node->setTypeRef (result);
  return result;
}


// ���2.13.5 boolean literal
CTypeInfo *CCSemExpr::resolve (CT_Bool *node, CTree *base) {
  CTypeInfo *result;
  U_LONG_LONG v;

  v = (strcmp (node->Son (0)->token ()->text (), "true") == 0) ? 1 : 0;
  result = &CTYPE_BOOL;

  node->setValue (new CConstant (v, result));
  node->setTypeRef (result);
  return result;
}


// ���2.13.2 character literal
CTypeInfo *CCSemExpr::resolve (CT_Character *node, CTree *base) {
  CTypeInfo *result;
  LONG_LONG v;
  int chars;
  
  chars = scanCharacter (node, v);
  if (chars == 1) {                 // character constant
    result = &CTYPE_CHAR;
    v = (char)v;
  } else {                         // integer character constant
    if (chars > 4) {
      SEM_ERROR (node, "integer character constant too long");
    } else {
      SEM_WARNING (node, "multi-character character constant");
    }
    result = &CTYPE_INT; 
    v = (int)v;
  }

  node->setValue (new CConstant (v, result));
  node->setTypeRef (result);
  return result;
}


// ���2.13.2 wide character literal
CTypeInfo *CCSemExpr::resolve (CT_WideCharacter *node, CTree *base) {
  CTypeInfo *result;
  LONG_LONG v;
  int chars;
  
  chars = scanCharacter (node, v);
  if (chars == 1) {
    result = &CTYPE_WCHAR_T;
    v = (wchar_t)v;
  } else {
    if (chars > 4) {
      SEM_ERROR (node, "integer character constant too long");
    } else {
      SEM_WARNING (node, "multi-character character constant");
    }
    result = &CTYPE_INT;
    v = (int)v;
  }

  node->setValue (new CConstant (v, result));
  node->setTypeRef (result);
  return result;
}


// ���2.13.4 string literal
CTypeInfo *CCSemExpr::resolve (CT_String *node, CTree *base) {
  CTypeInfo *result;
  const char *s, *e;
  char *p, *buf;
  long int ct;
  LONG_LONG i;
  bool ucs;

  // create string buffer
  i = 0;
  for (int t = 0; t < node->Sons (); t++) {
    s = node->Son (t)->token ()->text ();
    i += strlen (s) - 2;
  }
  buf = new char[i+10];

  // collect the string characters from all tokens
  buf[0] = '\0';
  ct = 0;
  for (int t = 0; t < node->Sons (); t++) {
    s = node->Son (t)->token ()->text ();

    i = strlen (s);
    if (s[0] == '"') {
      s++;
      i--;
      if (s[i-1] == '"') 
        i--;
    }
    e = &s[i];

    p = buf + strlen (buf);
    while (s && *s && s != e) {
      i = nextCharacter (s, ucs);
      if (ucs && i == (LONG_LONG)-1) {
        SEM_ERROR (node, "incomplete universal character");
        break;
      }
      if (ucs) {
        // turn the char value into a utf-8 octet stream
        utf8_encode (i, p, ct);
      }
      else {
        if (i > UCHAR_MAX)
          SEM_WARNING__out_of_range (node, "escape sequence");
        *p++ = (char)(i&0xff);
        ct++;
      }
    }

    *p++ = 0;
    *p = 0;
  }

  // type `array of const char'
  result = new CTypeQualified (&CTYPE_CHAR, true, false, false);
  result = new CTypeArray (result);
  result->TypeArray ()->Dimension (ct+1);
  node->setValue (new CStrLiteral (buf, ct, result));
  node->setType (result);

  return result;
}


// ���2.13.4 wide string literal
CTypeInfo *CCSemExpr::resolve (CT_WideString *node, CTree *base) {
  CTypeInfo *result;
  const char *s, *e;
  wchar_t *p, *buf;
  long int ct;
  LONG_LONG i;
  bool ucs;

  // create string buffer
  i = 0;
  for (int t = 0; t < node->Sons (); t++) {
    s = node->Son (t)->token ()->text ();
    if (*s == 'L') 
      s++;
    i += strlen (s) - 2;
  }
  buf = new wchar_t[i+2];

  // collect the string characters from all tokens
  buf[0] = L'\0';
  ct = 0;
  for (int t = 0; t < node->Sons (); t++) {
    s = node->Son (t)->token ()->text ();
    if (*s == 'L') 
      s++;

    i = strlen (s);
    if (s[0] == '"') {
      s++;
      i--;
      if (s[i-1] == '"') 
        i--;
    }
    e = &s[i];

    p = buf + wcslen (buf);
    while (s && *s && s != e) {
      i = nextCharacter (s, ucs);
      if (ucs && i == (LONG_LONG)-1) {
        SEM_ERROR (node, "incomplete universal character");
        break;
      }
      if (i > ULONG_MAX)
        SEM_WARNING__out_of_range (node, "escape sequence");
      *p++ = (wchar_t)(i&0xffffffff);
      ct++;
    }

    *p++ = 0;
    *p = 0;
  }

  // type `array of const wchar_t'
  result = new CTypeQualified (&CTYPE_WCHAR_T, true, false, false);
  result = new CTypeArray (result);
  result->TypeArray ()->Dimension (ct+1);
  node->setValue (new CWStrLiteral (buf, ct, result));
  node->setType (result);

  return result;
}


// ���2.13.1 integer literal
CTypeInfo *CCSemExpr::resolve (CT_Integer *node, CTree *base_node) {
  bool uns, lng, lnglng, trunc;
  U_LONG_LONG val, max, i;
  CTypeInfo *result;
  const char *s;
  int base;
  
  s = node->Son (0)->token ()->text ();
  if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
    base = 16;
    s += 2;
  } else if (s[0] == '0' && isdigit (s[1])) {
    base = 8;
    s++;
  } else 
    base = 10;

  val = 0;
  max = (ULLONG_MAX / base) + (ULLONG_MAX % base);
  uns = lng = lnglng = trunc = false;

  while (isxdigit (*s)) {
    if (isdigit (*s)) 
      i = *s - '0';
    else if (*s >= 'A' && *s <= 'F') 
      i = *s - 'A' + 10;
    else 
      i = *s - 'a' + 10;
    s++;
    if (! trunc && ((val + i) > max || ((val + i) == max && isxdigit (*s)))) {
      trunc = true;
      SEM_WARNING (node, "integer constant is too large" << 
        " - truncated to " << CTYPE_UNSIGNED_LONG_LONG.Size() << " bits");
    }
    val = (val * base) + i;
  }

  while (*s != 0) {
    if (*s == 'u' || *s == 'U') 
      uns = true;
    else if (*s == 'l' || *s == 'L') { 
      if (lng) {
        lng = false;
        lnglng = true;
      } else
        lng = true;
    }
    s++;
  }

  // determine type and handle overflows
  long int size = CTypePrimitive::SizeOfNumber (val);
  if (lnglng) {
    if (! uns && size > CTYPE_LONG_LONG.Size())
      overflow (node, base, uns);
  } else if (uns && lng) {
    if (size > CTYPE_UNSIGNED_LONG.Size()) {
      lng = false;
      lnglng = true;
    }
  } else if (lng) {
    if (size > CTYPE_LONG_LONG.Size())
      overflow (node, base, uns);
    else if ((base != 10 && size > CTYPE_UNSIGNED_LONG.Size()) ||
             (base == 10 && size > CTYPE_LONG.Size())) {
      lng = false;
      lnglng = true;
    } else if (base != 10 && size > CTYPE_LONG.Size())
      overflow (node, base, uns);
  } else if (uns) {
    if (size > CTYPE_UNSIGNED_LONG.Size())
      lnglng = true;
    else if (size > CTYPE_UNSIGNED_INT.Size())
      lng = true;
  } else {
    if (size > CTYPE_LONG_LONG.Size()) {
      overflow (node, base, uns);
      if (base != 10)
        lnglng = true;
    } else if ((base != 10 && size > CTYPE_UNSIGNED_LONG.Size()) ||
               (base == 10 && size > CTYPE_LONG.Size())) {
      lnglng = true;
    } else if (base != 10 && size > CTYPE_LONG.Size()) {
      overflow (node, base, uns);
      lng = true;
    } else if ((base != 10 && size > CTYPE_UNSIGNED_INT.Size()) ||
               (base == 10 && size > CTYPE_INT.Size())) {
      lng = true;
    } else if (size > CTYPE_INT.Size())
      overflow (node, base, uns);
  }

  // set type
  if (uns && lnglng)
    result = &CTYPE_UNSIGNED_LONG_LONG;
  else if (lnglng)
    result = &CTYPE_LONG_LONG;
  else if (uns && lng)
    result = &CTYPE_UNSIGNED_LONG;
  else if (lng)
    result = &CTYPE_LONG;
  else if (uns)
    result = &CTYPE_UNSIGNED_INT;
  else
    result = &CTYPE_INT;

  node->setValue (uns ? new CConstant ((U_LONG_LONG)val, result) :
                        new CConstant ((LONG_LONG)val, result));
  node->setTypeRef (result);
  return result;
}


void CCSemExpr::overflow (CTree *node, int base, bool &uns) const {
  if (base == 10) 
    SEM_WARNING__is_unsigned (node);
  uns = true;
}


// ���2.13.3 floating literal
CTypeInfo *CCSemExpr::resolve (CT_Float *node, CTree *base) {
  CTypeInfo *result;
  long double v;
  bool lng, flt;
  const char *s;
  int ln;

  s = node->Son (0)->token ()->text ();
  ln = strlen (s);

  lng = (s[ln-1] == 'l' || s[ln-1] == 'L');
  flt = (s[ln-1] == 'f' || s[ln-1] == 'F');

  v = strtod(s, NULL);

  if (lng) 
    result = &CTYPE_LONG_DOUBLE;
  else if (flt) 
    result = &CTYPE_FLOAT;
  else 
    result = &CTYPE_DOUBLE;

  node->setValue (new CConstant (v, result));
  node->setTypeRef (result);
  return result;
}


///////////////////////////////////////////////////////
// ���8.5 initializers //////////////////////////////////
///////////////////////////////////////////////////////


CTypeInfo *CCSemExpr::resolveInit (CTree *node, CTree *base) {
  CTypeInfo *result;
  bool dep_base;

  // evaluation not yet complete!
  // error diagnostics not yet implemented!

  // get object type
  CObjectInfo* obj = base->SemObject () ? base->SemObject ()->Object () : 0;
  CTypeInfo* otype = &CTYPE_UNDEFINED;
  if (obj && obj->TypeInfo ()) {
    otype = obj->TypeInfo ();
  }

  if (node->NodeName () == CT_ExprList::NodeId ()) {
    CT_ExprList *list = (CT_ExprList*)node;
    unsigned entries = list->Entries ();

    // = { init_list }
    if (node->end_token ()->type () == TOK_CLOSE_CURLY) {
      dep_base = resetDependent ();
      // handle arrays of unknown size; count the top-level elements
      // (all but the first array dimension must be given)
      CTypeArray* array = otype->VirtualType ()->TypeArray ();
      if (array && ! array->hasDimension ()) {
        // set the first dimension of the array
        // also update type of linked objects
        CObjectInfo* curr = obj;
        do {
          array = curr->TypeInfo ()->VirtualType ()->TypeArray ();
          if (array)
            array->Dimension (entries);
          curr = curr->NextObject ();
        } while (curr != obj);
      }
      // has target type
      result = otype;
      if (node->SemValue ())
        node->SemValue ()->setTypeRef (result);
      // reset dependent state
      if (dep_base)
        dependent = true;
    // ( expr_list )
    } else if (node->token ()->type () == TOK_OPEN_ROUND) {
      dep_base = resetDependent ();
      // resolve initializer arguments
      for (unsigned i = 0; i < entries; i++)
        resolveExpr (list->Entry (i), list);
      // has target type
      result = otype;
      if (node->SemValue ())
        node->SemValue ()->setTypeRef (result);
      // reset dependent state
      if (dep_base)
        dependent = true;
    // = ass_expr
    } else {
      dep_base = resetDependent ();
      // resolve initializer arguments
      for (unsigned i = 0; i < entries; i++)
        resolveExpr (list->Entry (i), list);
      // handle dependent expressions
      if (isDependent (node, dep_base))
        return &CTYPE_UNDEFINED;

      // set type and value
      CTree *last = list->Entry (entries-1);
      passOnConstant (last, 0);
      if (last->Type ())
        result = last->Type ();
      else
        result = &CTYPE_UNDEFINED;
      if (node->SemValue ())
        node->SemValue ()->setTypeRef (result);
    }
  // ass_expr
  } else {
    dep_base = resetDependent ();
    result = resolveExpr (node, base);
    // handle dependent expressions
    if (isDependent (node, dep_base))
      return &CTYPE_UNDEFINED;
  }

  return result;
}


///////////////////////////////////////////////////////
// ���13.3.1.2 operator overloading /////////////////////
///////////////////////////////////////////////////////


// ���13.3.1.2 operator overloading
CTypeInfo *CCSemExpr::overloadedOperator (CTree *node, CTree *base, 
 const char *op, int oper, CTree *arg0, CTree *arg1) {
  CTypeInfo *t0, *t1 = 0, *type = 0;
  CCandidateInfo *cand;
  CFunctionInfo *finfo;
  CRecord *cinfo;
  char opname[1000]; // enough space for every possible operator name

  t0 = arg0->Type ();
  if (arg1)
    t1 = arg1->Type ();
    
  // ���13.3.1.2.2 at least one operand must have class or enumeration type
  if (! isClassOrEnum (t0) && ! isClassOrEnum (t1))
    return 0;
  
  // operator call depends on dependent operators
  if (t0->isDependent () || (t1 && t1->isDependent ())) {
    dependent = true;
    return 0;
  }

  // operator may be overloaded; collect the candidates
  // for overload resolution 
  CCOverloading ovl (err, true);

  // create the full operator name for name lookup purposes
  sprintf (opname, "operator %s", op);

  // ���13.3.1.2.3 if first operand has class type, overloaded 
  // operator may be a class member (member candidates)
  if (t0->isRecord () && oper != TOK_QUESTION) {
    cinfo = t0->VirtualType ()->TypeRecord ()->Record ();
    if (cinfo) {
      // ensure that class is instantiated
      if (cinfo->ClassInfo ())
        cinfo->TypeInfo ()->TypeClass ()->instantiate (current_scope);
      // lookup operator name in object's class
      CCNameLookup nl (err, current_scope);
      nl.lookup (opname, cinfo, false, true);
      // add candidates looked up
      ovl.addCandidates (nl);
    }
  }

  // collect non-member candidates doing a special unqualified
  // name lookup ignoring all member functions
  unsigned curr_pos = node->token_node ()->Number ();
  CCNameLookup nl (err, current_scope);
  nl.lookupOpName (opname, current_scope, curr_pos);

  // additionally do an argument-dependent name lookup 
  CCAssocScopes scopes;
  scopes.determineAssocScopes (t0);
  if (t1)
    scopes.determineAssocScopes (t1);
  if (scopes.Namespaces ()) 
    for (unsigned i = 0; i < scopes.Namespaces (); i++)
      nl.lookupInAssNs (opname, scopes.Namespace (i), curr_pos);

  if (nl.Objects () && oper != TOK_QUESTION) {
    // if no operand has a class type, only those functions that 
    // have a first parameter of type "t0" or "reference to cv t0"
    // (when enumeration type), or a second parameter of type "t1"
    // or type "reference to cv t1"
    if (! t0->isRecord () && (! t1 || ! t1->isRecord ())) {
      for (unsigned i = 0; i < nl.Objects (); i++) {
        finfo = nl.Object (i)->FunctionInfo ();
        if (finfo) {
          if (! finfo->Arguments ())
            continue;
          if (t0->isEnum ()) {
            type = finfo->Argument ((unsigned)0)->TypeInfo ();
            if (*type->VirtualType () == *t0->VirtualType ())
              ovl.addCandidate (finfo);
          } else if (t1 && t1->isEnum () && finfo->Arguments () > 1) {
            type = finfo->Argument ((unsigned)1)->TypeInfo ();
            if (*type->VirtualType () == *t1->VirtualType ())
              ovl.addCandidate (finfo);
          }
        }
      }
    } else
      ovl.addCandidates (nl);
  }

  // create and add built-in candidates
  CSemDatabase *db = current_scope->SemDB ();
  ovl.createBuiltinOperators (db, opname, oper, arg0, arg1);

  // let overload resolution know the qualification
  // of the object the operator is called on
  CTypeInfo *qt0 = t0;
  while (qt0->isAddress ())
    qt0 = qt0->BaseType ();
  if (qt0->isRecord () && qt0->isQualified ())
    ovl.ObjectType (t0);

  // do overload resolution
  if (arg1)      // base node of arguments
    ovl.resolve ((node == arg0 && base ? base : node), arg0, arg1);
  else
    ovl.resolve ((node == arg0 && base ? base : node), arg0);

  // check result of overload resolution
  if (ovl.Function ()) {
    // operator call is overloaded
    if (validOvlFctSet (node, "overloaded operator", opname, ovl)) {
      finfo = ovl.Function ();
      type = finfo->ReturnType ();
      if (!type || *type == CTYPE_EMPTY) // no return type???
        type = &CTYPE_UNDEFINED;

      // apply the operand conversions determined by
      // overload resolution
      cand = ovl.Candidate (0);
      for (unsigned i = 0; i < cand->ConvSequences (); i++)
        cand->ConvSequence (i)->Apply ();

      // if not resolved to a built-in operator
      if (! finfo->isBuiltin ()) {
        if (base && base->NodeName () == CT_MembPtrExpr::NodeId ()) {
          // A member pointer expr can trigger a sequence of operator->() calls.
          // In order to avoid loss of information ImplicitCall nodes are created.
          CT_ImplicitCall *call = new CT_ImplicitCall (node);
          call->Object (finfo);
          call->setTypeRef (type);
          base->ReplaceSon (node, call);
        }
        else {
          if (node->SemValue ()) {
            node->SemValue ()->setTypeRef (type);
          }
          if (node->SemObject ()) {
            node->SemObject ()->Object (finfo);
          }
        }
        return type;
      }
    }
  // overloaded operator depends on a template parameter
  } else if (ovl.isDependent ())
    dependent = true;

  // operator call is not overloaded
  return 0;
}


///////////////////////////////////////////////////////
// constant expression ////////////////////////////////
///////////////////////////////////////////////////////


// ���5.19 integer constant expression
bool CCSemExpr::isConstIntExpr (CTree *node) { 
  if (isConstIntegralExpr (node) &&
      node->Type () && node->Type ()->isInteger () && 
      node->Value () && node->Value ()->Constant ())
    return true;
  return false;
}


// ���5.19 integer constant expression
bool CCSemExpr::isConstIntegralExpr (CTree *node) { 
  const char *id = node->NodeName ();
  CObjectInfo *info;
  int oper;

  // - literals
  if (id == CT_Bool::NodeId () ||
      id == CT_Character::NodeId () ||
      id == CT_WideCharacter::NodeId () ||
      id == CT_Integer::NodeId () ||
      // not correct: only allowed if casted to integral 
      // or enumeration type; to be corrected!!!
      id == CT_Float::NodeId ()) {  
    return true;
  // - enumerators
  // - const variables
  // - static data members of intergral or enumeration type 
  //   initialized with const expression
  // - non-type template parameters of integral or enumeration type
  // - no functions, class objects, pointers, and references
  } else if (id == CT_SimpleName::NodeId () ||
             id == CT_OperatorName::NodeId () ||
             id == CT_DestructorName::NodeId () ||
             id == CT_ConversionName::NodeId () ||
             id == CT_TemplateName::NodeId ()) {
    info = ((CT_SimpleName*)node)->Name ()->Object ();
    if (info) {
      if (info->EnumeratorInfo ())
        return true;
      if (info->TemplateParamInfo () && 
          ! info->TemplateParamInfo ()->isTypeParam () &&
          info->TypeInfo ()->isInteger ())
        return true;
      if (info->AttributeInfo () && 
          ! info->TypeInfo ()->isPointer () &&
          ! info->TypeInfo ()->isAddress ()) {
        if (! info->AttributeInfo ()->Record () && 
            info->TypeInfo ()->isConst ())
          return true;
        if (info->AttributeInfo ()->Record () && 
            info->isStatic () && info->TypeInfo ()->isInteger ()) {
          CTree* init = info->AttributeInfo ()->DefObject ()->Init ();
          if (init && isConstIntegralExpr (init))
            return true;
        }
      }
    }
  } else if (id == CT_QualName::NodeId () || 
             id == CT_RootQualName::NodeId ()) {
    return isConstIntegralExpr (((CT_QualName*)node)->Name ());
  } else if (id == CT_SizeofExpr::NodeId () ||
             id == CT_AlignofExpr::NodeId () ||
             id == CT_TypeTraitExpr::NodeId () ||
             id == CT_OffsetofExpr::NodeId ()) {
    return true;
  // - no assignment and comma operators
  } else if (id == CT_BinaryExpr::NodeId ()) {
    oper = node->Son (1)->token ()->type ();
    if (oper == TOK_ASSIGN || oper == TOK_MUL_EQ || oper == TOK_DIV_EQ ||
        oper == TOK_MOD_EQ || oper == TOK_ADD_EQ || oper == TOK_SUB_EQ ||
        oper == TOK_RSH_EQ || oper == TOK_LSH_EQ || oper == TOK_AND_EQ ||
        oper == TOK_XOR_EQ || oper == TOK_IOR_EQ || oper == TOK_COMMA)
      return false; 
    return isConstIntegralExpr (node->Son (0)) && 
           isConstIntegralExpr (node->Son (2));
  // no increment and decrement operators
  } else if (id == CT_PostfixExpr::NodeId ()) {
    oper = node->Son (1)->token ()->type ();
    if (oper == TOK_DECR || oper == TOK_INCR)
      return false;
    return isConstIntegralExpr (node->Son (0));
  // no increment and decrement operators
  } else if (id == CT_UnaryExpr::NodeId ()) {
    oper = node->Son (0)->token ()->type ();
    if (oper == TOK_DECR || oper == TOK_INCR)
      return false;
    return isConstIntegralExpr (node->Son (1));
  } else if (id == CT_IfThenExpr::NodeId ()) {
    CT_IfThenExpr *ite = (CT_IfThenExpr*)node;
    return isConstIntegralExpr (ite->Condition ()) && 
           (ite->LeftOperand () ? 
            isConstIntegralExpr (ite->LeftOperand ()) : true) &&
              isConstIntegralExpr (ite->RightOperand ());
  } else if (id == CT_CastExpr::NodeId ()) {
    return isConstIntegralExpr (node->Son (3));
  } else if (id == CT_ConstructExpr::NodeId ()) {
    CT_ConstructExpr *ce = (CT_ConstructExpr*)node;
    CObjectInfo *info = ce->TypeName ()->Object ();
    if (info && info->TypeInfo () && ! info->TypeInfo ()->isRecord () &&
        ce->Initializer ()->Entries () == 1)
      return isConstIntegralExpr (ce->Initializer ()->Entry (0));
  } else if (id == CT_BracedExpr::NodeId ()) {
    return isConstIntegralExpr (node->Son (1));
  } else if (id == CT_ImplicitCast::NodeId ()) {
    return isConstIntegralExpr (node->Son (0));
  } else if (id == CT_StaticCast::NodeId () ||
             id == CT_ReintCast::NodeId ()) 
    return isConstIntegralExpr (node->Son (4));
  return false;
}


// ���5.19 constant expressions 
bool CCSemExpr::isConstExpr (CTree *node) {
  return isConstIntegralExpr (node);
}


///////////////////////////////////////////////////////
// helper /////////////////////////////////////////////
///////////////////////////////////////////////////////


void CCSemExpr::apply_binary_op (CTree *node, int oper) {
  CConstant *v1, *v2;
  CTree *op1, *op2;

  op1 = node->Son (0); 
  op2 = node->Son (2);

  // apply operator on constant values if any
  v1 = op1->Value () ? op1->Value ()->Constant () : (CConstant*)0; 
  v2 = op2->Value () ? op2->Value ()->Constant () : (CConstant*)0; 
  if (v1 && v2 && node->SemValue ())
    node->SemValue ()->setValue (v1->compute (oper, v2));
}


int CCSemExpr::scanCharacter (CTree *node, LONG_LONG &v) const {
  const char *s;
  const char *t;
  LONG_LONG ch;
  int i, j;
  int wid = 8;
  bool ucs;

  s = node->Son (0)->token ()->text ();
  if (*s == 'L') {
    wid = MB_LEN_MAX;
    s++;
  }

  i = strlen (s);
  if (s[0] == '\'') {
    s++;
    i--;
    if (s[i-1] == '\'') 
      i--;
  }
  t = &s[i];

  v = 0;        
  for (j = 0; j < 4; j++) {
    ch = nextCharacter (s, ucs);
    if (ucs && i == (LONG_LONG)-1) {
      SEM_ERROR (node, "incomplete universal character");
      break;
    }
    if ((wid == 8 && ch > UCHAR_MAX) ||
        (wid == MB_LEN_MAX && ch > ULONG_MAX))
      SEM_WARNING__out_of_range (node, "escape sequence");
    v = (v << wid) | ch;
    if (s == NULL || *s == 0 || s == t) 
      break;
  }
  
  return j + 1;
}


LONG_LONG CCSemExpr::nextCharacter (const char *&s, bool &ucs) const {
  LONG_LONG ch, v;
  ucs = false;

  ch = *s++;
  if (ch == '\\') {
    ch = *s++;
    if (ch >= '0' && ch <= '7') {
      v = ch - '0';
      if (*s >= '0' && *s <= '7') {
        v = (v<<3) + *s++ - '0';
        if (*s >= '0' && *s <= '7') 
          v = (v<<3) + *s++ - '0';
      }
      ch = v;
    } else if (ch == 'x') {
      v = 0;
      while (true) {
        if (isdigit (*s)) 
          v = (v<<4) + *s++ - '0';
        else if (*s >= 'a' && *s <= 'f') 
          v = (v<<4) + *s++ - 'a' + 10;
        else if (*s >= 'A' && *s <= 'F') 
          v = (v<<4) + *s++ - 'A' + 10;
        else 
          break;
      }
      ch = v;
    } else if (ch == 'u' || ch == 'U') {
      ucs = true;
      ch = utf8_scan (s, ch == 'U');
    } else {
      switch (ch) {
        case 'n': ch = '\n'; break;
        case 'b': ch = '\b'; break;
        case 'r': ch = '\r'; break;
        case 't': ch = '\t'; break;
        case 'f': ch = '\f'; break;
        case 'v': ch = '\v'; break;
        case 'a': ch = '\a'; break;
        default : break;
      }
    }
  }
  return ch;
}


bool CCSemExpr::sameOrBaseClass (CRecord *c1, CRecord *c2) {
  if (! (c1 && c2))
    return false;
  if (*c1 == *c2)
    return true;
  return baseClass (c1->ClassInfo (), c2->ClassInfo ());
}


bool CCSemExpr::baseClass (CClassInfo *c1, CClassInfo *c2) {
  CClassInfo *baseclass;
  if (! (c1 && c2))
    return false;
  
  c2 = c2->DefObject ()->ClassInfo ();
  for (unsigned i = 0; i < c2->BaseClasses (); i++) {
    baseclass = c2->BaseClass (i)->Class ();
    if (*c1 == *baseclass || baseClass (c1, baseclass))
      return true;
  }
  return false;
}


bool CCSemExpr::sameUnqualBaseType (CTypeInfo *t1, CTypeInfo *t2) const { 
  return *t1->VirtualType ()->BaseType ()->UnqualType () == 
         *t2->VirtualType ()->BaseType ()->UnqualType (); 
}


// check result of function overload resolution
bool CCSemExpr::validOvlFctSet (CTree *node, const char *what, 
 const char *name, CCOverloading &ovl) {
  CTemplateInstance *inst;
  CObjectInfo *info;
  CTree *arg = 0;

  // more than one candidate left
  if (ovl.isAmbiguous () && ! ovl.isDependent ()) {
    std::ostringstream fname;
    fname << name << " (";
    for (unsigned i = 0; i < ovl.Arguments (); i++) {
      arg = ovl.Argument (i);
      if (arg && arg->Type ())
        arg->Type ()->TypeText (fname);
      else
        fname << "<unknown>";
      if (i+1 < ovl.Arguments ())
        fname << ",";
    }
    fname << ")";
    
    SEM_ERROR (node, "call to " << what << " `" << fname.str ().c_str () << "' is ambiguous");

    for (unsigned i = 0; i < ovl.Candidates (); i++) {
      info = ovl.Candidate (i)->Function ();
      inst = info->TemplateInstance ();
      if (inst) {
        info = inst->Template ()->ObjectInfo ();
      }
      if (info->Tree () && info->Tree ()->token ())
        err << info->Tree ()->token ()->location ();
      else if (node->token ())
        err << node->token ()->location ();
      if (i == 0)
        err << "candidates are: ";
      else
        err << "                ";
      std::ostringstream sig;
      info->TypeInfo ()->TypeText (sig, info->Name ());
      if (inst) {
        sig << " [with ";
        for (unsigned j = 0; j < inst->DeducedArgs (); j++) {
          DeducedArgument *darg = inst->DeducedArg (j);
          if (j != 0) 
            sig << ", ";
          if (darg->Type ()) 
            darg->TemplateParam ()->TypeInfo ()->TypeText (sig);
          else 
            darg->TemplateParam ()->ValueType ()->TypeText (sig, darg->TemplateParam ()->Name ());
          sig << " = ";
          if (darg->Type ())
            darg->Type ()->TypeText (sig);
          else if (darg->Value ())
            darg->Value ()->print (sig);
          else
            sig << "<null>";
        }
        sig << "]";
      } 
      err << sig.str ().c_str ();
      if (info->isBuiltin ())
        err << " <built-in>";
      err << endMessage;
    }
    return false;
  // no matching function 
  } else if (! ovl.Function () && ! ovl.isDependent ()) {
    SEM_ERROR (node, "no matching function for call to `" << name << "'");
    return false;
  // dependent call
  } else if (ovl.isDependent ()) {
    dependent = true;
    return false;
  // delayed parse problem
  } else if (ovl.Function () && ovl.Candidate(0)->hasDelayedParseProblem ()) {
    SEM_ERROR (node, "a default argument of called template function instance '" << ovl.Function()->Name ()
        << "' has not yet been parsed");
    return false;
  }
  return true;
}


// check result of looking up a name
bool CCSemExpr::checkDeclsFound (CTree *node, CCNameLookup &nl) const {
  CObjectInfo *info;

  // ���3.4.1 if more than one declaration was 
  // found they shall all belong to functions
  if (false && nl.Objects () > 1) {
    for (unsigned i = 0; i < nl.Objects (); i++) {
      info = nl.Object (i);
      if (info->FunctionInfo ()) 
        continue;
  
      SEM_ERROR (node, "`" << *info->TypeInfo () 
        << "' is not a function,");
      for (unsigned j = 0; j < nl.Objects (); j++) {
        if (j == i)
          continue;
        info = nl.Object (j);
        if (info->Tree () && info->Tree ()->token ())
          err << info->Tree ()->token ()->location ();
        if (i == 0)
          err << "conflict with ";
        else
          err << "              ";
        err << "`" << *info->TypeInfo () << "'" << endMessage;
      }
      nl.reset (current_scope);
      return false;
    }
  }
  return true;
}


bool CCSemExpr::isClassOrEnum (CTypeInfo *type) const {
  return type && (type->isRecord () || type->isEnum ());
}


// check if any sub expression of a given expression
// depends on a template parameter
static bool isDependentInternal (CTree *expr, std::set<CTree*>& inits) {
  CObjectInfo* info;
  unsigned sons;
  CTree* init;

  if (! expr)
    return false;

  // expression has dependent type
  if (expr->Type () && expr->Type ()->isDependent ())
    return true;

  // value of variable is dependent
  if (expr->NodeName () == CT_SimpleName::NodeId ()) {
    info = ((CT_SimpleName*)expr)->Name ()->Object ();
    if (info && info->AttributeInfo ()) {
      init = info->AttributeInfo ()->DefObject ()->Init ();
      if (init && inits.count(init)==0) {
        inits.insert(init);
        if (isDependentInternal (init, inits))
          return true;
      }
    }
  }

  sons = expr->Sons ();
  for (unsigned i = 0; i < sons; i++)
    if (isDependentInternal (expr->Son (i), inits))
      return true;

  return false;
}


bool CCSemExpr::isDependent (CTree *expr) {
  std::set<CTree*> inits;
  return isDependentInternal (expr, inits);
}


bool CCSemExpr::isDependent (CTree *node, bool dependent_base) {
  if (dependent) {
    node->SemValue ()->setTypeRef (&CTYPE_UNDEFINED);
    return true;
  }
  dependent = dependent_base;
  return false;
}


// reset `dependent' status attribute and return former value
bool CCSemExpr::resetDependent () {
  bool dep = dependent;
  dependent = false;
  return dep;
}


// return true if class is derived from a template parameter
bool CCSemExpr::depBaseClass () const {
  CStructure *scope = current_scope;
  while (! scope->FileInfo ()) {
    if (scope->ClassInfo () && scope->ClassInfo ()->hasDepBaseClass ())
      return true;
    if (scope->QualifiedScope () && scope->QualifiedScope ()->ClassInfo () &&
        scope->QualifiedScope ()->ClassInfo ()->hasDepBaseClass ())
      return true;
    scope = scope->Parent ()->Structure ();
  }
  return false;
}


// transform the expression into a call expression with a built-in operator
// as the target function
void CCSemExpr::builtinOpExpr (CTree *node, CTree *base, const char *op, int oper, CTypeInfo *result,
  CTypeInfo *t1, CTypeInfo *t2) const {
  CT_Call *call = node->IsCall ();
  if (! call) {
    SEM_WARNING(node, "internal error, " << node->NodeName () << " is no CT_Call node, but builtin operator");
  }
  else if (!result || *result == CTYPE_UNDEFINED ||
           !t1 || *t1 == CTYPE_UNDEFINED ||
           (t2 && *t2 == CTYPE_UNDEFINED)) {
    SEM_ERROR(node, "undefined type");
  }
  else {
    char name[1000];
    sprintf (name, "operator %s", op);
    CFunctionInfo *fi = current_scope->SemDB ()->BuiltinOperator (name, oper,
      result->Duplicate (), t1->Duplicate (), t2 ? t2->Duplicate () : 0);
    call->Object (fi);
  }
}


bool CCSemExpr::lookupBuiltin (CT_SimpleName* sn, CCNameLookup& nl) {
  // if name prefixed with "__builtin_" then lookup without this prefix
  const char* name = sn->Name ()->Text ();
  if (! nl.Objects () && name && strncmp (name, "__builtin_", 10) == 0) {
    nl.lookup (name+10, current_scope, true, true);
    return nl.Objects ();
  }
  return true;
}


} // namespace Puma
