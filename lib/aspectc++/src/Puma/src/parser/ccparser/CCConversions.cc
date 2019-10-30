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

#include "Puma/CCConversions.h"
#include "Puma/CCOverloading.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CCConvSeq.h"
#include "Puma/CCSemExpr.h"
#include "Puma/CConstant.h"
#include "Puma/CTree.h"

namespace Puma {


////////////////////////////////////////////////////////////
// arithmetic promotions/conversions ///////////////////////
////////////////////////////////////////////////////////////


// �4.5 integral promotion
CTypeInfo *CCConversions::integralPromotion (CTypeInfo *type) {
  CTypeInfo *t = type->UnqualType ();

  if (t->isInteger ()) {
    if (t->is_char () || t->is_signed_char () || t->is_unsigned_char () ||
        t->is_short () || t->is_unsigned_short () || t->is_wchar_t () || 
        t->is_bool ())
      return &CTYPE_INT;
    else if (t->isEnum ())
      return t->VirtualType ()->TypeEnum ()->UnderlyingType ();
    else if (t->isBitField () && t->BaseType ()->isInteger () &&
             (*t->BaseType ()->UnqualType () < CTYPE_INT))
      return &CTYPE_INT;
    return type;
  }
  
  return 0;
}


// �4.5, �4.6 integral/floating point promotion
CTypeInfo *CCConversions::arithmeticPromotion (CTypeInfo *type) {
  CTypeInfo *t = type->UnqualType ();
  
  // �4.6 floating point promotion
  if (t->isReal ()) {
    if (t->is_float ())
      return &CTYPE_DOUBLE;
    return type;
  }

  // �4.5 integral promotion
  return integralPromotion (type);
}


// �5.9 usual arithmetic conversions
CTypeInfo *CCConversions::usualArithmeticConv (CTypeInfo *type1, CTypeInfo *type2) {
  CTypeInfo *t1 = type1->UnqualType ();
  CTypeInfo *t2 = type2->UnqualType ();

  if (! t1->isArithmetic () || ! t2->isArithmetic ())
    return 0;
  if (*t1 == *t2)
    return t1;

  if (t1->is_long_double () && ! t2->is_long_double ())
    t2 = &CTYPE_LONG_DOUBLE;
  else if (t2->is_long_double () && ! t1->is_long_double ())
    t1 = &CTYPE_LONG_DOUBLE;
  else if (t1->is_double () && ! t2->is_double ())
    t2 = &CTYPE_DOUBLE;
  else if (t2->is_double () && ! t1->is_double ())
    t1 = &CTYPE_DOUBLE;
  else if (t1->is_float () && ! t2->is_float ())
    t2 = &CTYPE_FLOAT;
  else if (t2->is_float () && ! t1->is_float ())
    t1 = &CTYPE_FLOAT;
  else { // integral promotion of both operands
    t1 = integralPromotion (t1);
    t2 = integralPromotion (t2);
  }  
  
  if ((t1->is_long () && t2->is_unsigned_int ()) ||
      (t2->is_long () && t1->is_unsigned_int ())) {
    t1 = &CTYPE_UNSIGNED_LONG;
    t2 = &CTYPE_UNSIGNED_LONG;
  } else if (t1->is_long () && ! t2->is_long ())
    t2 = &CTYPE_LONG;
  else if (t2->is_long () && ! t1->is_long ())
    t1 = &CTYPE_LONG;
  else if (t1->is_unsigned_int () && ! t2->is_unsigned_int ())
    t2 = &CTYPE_UNSIGNED_INT;
  else if (t2->is_unsigned_int () && ! t1->is_unsigned_int ())
    t1 = &CTYPE_UNSIGNED_INT;

  return t1;
}


////////////////////////////////////////////////////////////
// �13.3.3.1 implicit conversion sequences /////////////////
////////////////////////////////////////////////////////////


// �8.5.14 initialization
CCConvSeq *CCConversions::implicitConversions (CTypeInfo *ptype, 
 CTypeInfo *atype, CTree *arg_expr, CTree *base, bool user_def) {
  if (! atype || ! ptype)
    return 0;

  // argument passing is a copy-initialization
  if (atype->isAddress ())
    atype = atype->BaseType ();
  if (ptype->isAddress ())
    return referenceBinding (ptype, atype, arg_expr, base, user_def);
  if (user_def && (ptype->isRecord () || atype->isRecord ()))
    return userDefinedConversions (ptype, atype, arg_expr, base);
  return standardConversions (ptype, atype, arg_expr, base);
}


// �13.3.3.1.1 (and �4) standard conversions
CCConvSeq *CCConversions::standardConversions (CTypeInfo *ptype, 
 CTypeInfo *atype, CTree *arg_expr, CTree *base) {
  CTypeInfo *param = ptype->UnqualType ();
  CCConvSeq *seq = new CCStandardConvSeq;
  bool conv = false, is_lvalue, null_ptr_conv = false, del = false;

  // �4.7 a bit field is converted according to its type
  atype = atype->isBitField () ? atype->BaseType () : atype;
  CTypeInfo *arg = atype;
  arg = arg->UnqualType ();

  is_lvalue = arg_expr ? CCSemExpr::isLvalue (arg_expr) : arg->isAddress ();

  // �4.1 lvalue-to-rvalue conversion
  if (is_lvalue && ! arg->isFunction () && ! arg->isArray ()) {
    seq->addConversion (new CCLvalueToRvalueConv (param, arg, arg_expr, base));
  // �4.2 array-to-pointer conversion
  } else if (arg->isArray ()) {
    if (arg_expr && (arg_expr->NodeName () == CT_String::NodeId () ||
                     arg_expr->NodeName () == CT_WideString::NodeId ())) {
      // string and wide-string constants shall only be converted
      // when an appropriated target type exists
      if (param->isPointer () &&
          (param->BaseType ()->UnqualType ()->is_void () ||
           (arg_expr->NodeName () == CT_String::NodeId () &&
            param->BaseType ()->UnqualType ()->is_char ()) ||
           (arg_expr->NodeName () == CT_WideString::NodeId () &&
            param->BaseType ()->UnqualType ()->is_wchar_t ()))) {
        // convert to "pointer to char" resp. to "pointer to wchar_t"
        seq->addConversion (new CCArrayToPointerConv (param, arg, arg_expr, base));
        seq->addConversion (new CCQualificationConv (param, arg, arg_expr, base));
        arg = arg->BaseType ()->UnqualType ();
        arg = new CTypePointer (arg->Duplicate ()); del = true;
      }
    } else {
      seq->addConversion (new CCArrayToPointerConv (param, arg, arg_expr, base));
      arg = arg->BaseType ();
      arg = new CTypePointer (arg->Duplicate ()); del = true;
    }
  // �4.3 function-to-pointer conversion
  } else if (is_lvalue && arg->isFunction ()) {
    seq->addConversion (new CCFunctionToPointerConv (param, arg, arg_expr, base));
    arg = new CTypePointer (arg->Duplicate ()); del = true;
  }

  // same types, ignoring top level qualification
  if (arg->equalsUnqualified (*param)) {
    seq->addConversion (new CCIdentityConv (param, arg, arg_expr, base)); conv = true;
  // base and derived class objects considered equal
  } else if (param->isClass () && arg->isClass () && baseClass (param, arg)) {
    seq->addConversion (new CCQualificationConv (param, arg, arg_expr, base)); conv = true;
  }

  if (conv) {
    if (del) CTypeInfo::Destroy (arg);
    return seq;
  }

  // �4.12 boolean conversion
  if (param->is_bool () && arg->isScalar ()) {
    seq->addConversion (new CCBooleanConv (param, arg, arg_expr, base)); conv = true;
  // �4.5 integral promotions
  } else if (arg->isInteger () && param->isInteger ()) {
    // promote only if argument type is less precise than parameter type
    if (*param == CTYPE_INT && arg->rank () <= CTYPE_WCHAR_T.rank ()) {
      seq->addConversion (new CCIntegralPromotion (param, arg, arg_expr, base)); conv = true;
      if (del) CTypeInfo::Destroy (arg);
      arg = &CTYPE_INT; del = false;
    // promote enumeration type
    } else if (arg->isEnum () && *param == *arg->VirtualType ()->TypeEnum ()->UnderlyingType ()) {
      seq->addConversion (new CCIntegralPromotion (param, arg, arg_expr, base)); conv = true;
      if (del) CTypeInfo::Destroy (arg);
      arg = param; del = false;
    // �4.7 integral conversion
    } else if (! param->isEnum ()) {
      seq->addConversion (new CCIntegralConv (param, arg, arg_expr, base)); conv = true;
    }
  // �4.6 floating point promotion
  } else if (arg->isReal () && param->isReal ()) {
    if (arg->is_float ()) {
      seq->addConversion (new CCFloatingPointPromotion (param, arg, arg_expr, base)); conv = true;
      if (del) CTypeInfo::Destroy (arg);
      arg = &CTYPE_DOUBLE; del = false;
    // �4.8 floating point conversion
    } else {
      seq->addConversion (new CCFloatingPointConv (param, arg, arg_expr, base)); conv = true;
    }
  // �4.9 floating-integral conversion
  } else if ((arg->isReal () && param->isInteger ()) ||
             (arg->isInteger () && param->isReal ())) {
    if (! param->isEnum ()) {
      seq->addConversion (new CCFloatingIntegralConv (param, arg, arg_expr, base)); conv = true;
    }
  // �4.10 pointer conversions
  } else if (param->isPointer () && (arg->isPointer () || arg->isInteger ())) {
    CClassInfo *ci1, *ci2;
    bool null_ptr = (! is_lvalue && arg_expr && arg_expr->Value () &&
                     arg_expr->Value ()->Constant () &&
                     arg_expr->Value ()->Constant ()->isNull ());
    if (! param->isMemberPointer () && (null_ptr ||
        (arg->isPointer () && ! arg->isMemberPointer ()))) {
      if (null_ptr && (! arg->isPointer () || similarTypes (param, arg))) {
        seq->addConversion (new CCPointerConv (param, arg, arg_expr, base)); conv = null_ptr_conv = true;
      } else if (param->BaseType ()->UnqualType ()->is_void ()) {
        seq->addConversion (new CCPointerConv (param, arg, arg_expr, base)); conv = true;
        if (del) CTypeInfo::Destroy (arg);
        arg = &CTYPE_VOID;
        if (! null_ptr && param->BaseType ()->isQualified ()) {
          arg = new CTypeQualified (arg, param->BaseType ()->isConst (),
                                         param->BaseType ()->isVolatile (),
                                         param->BaseType ()->isRestrict ());
        }
        arg = new CTypePointer (arg); del = true;
      } else if (param->BaseType ()->UnqualType ()->TypeClass () &&
                 arg->BaseType ()->UnqualType ()->TypeClass ()) {
        ci1 = param->BaseType ()->UnqualType ()->TypeClass ()->ClassInfo ();
        ci2 = arg->BaseType ()->UnqualType ()->TypeClass ()->ClassInfo ();
        if (baseClass (ci1, ci2)) {
          seq->addConversion (new CCPointerConv (param, arg, arg_expr, base)); conv = true;
        }
      }
    // �4.11 pointer to member conversion
    } else if (param->isMemberPointer () && (null_ptr || arg->isMemberPointer ())) {
      if (null_ptr) {
        seq->addConversion (new CCPointerToMemberConv (param, arg, arg_expr, base)); conv = true;
      } else if (param->TypeMemberPointer ()->Record () &&
                 arg->TypeMemberPointer ()->Record ()) {
        ci1 = param->TypeMemberPointer ()->Record ()->ClassInfo ();
        ci2 = arg->TypeMemberPointer ()->Record ()->ClassInfo ();
        if (baseClass (ci2, ci1) && *param->BaseType () == *arg->BaseType ()) {
          seq->addConversion (new CCPointerToMemberConv (param, arg, arg_expr, base)); conv = true;
        }
      }
    }
  }

  // �4.4 qualification conversion
  if (! null_ptr_conv && similarTypes (param, arg)) {
    seq->addConversion (new CCQualificationConv (param, arg, arg_expr, base)); conv = true;
  }

  if (! conv) {
    delete seq;
    seq = (CCStandardConvSeq*)0;
  }
  if (del) CTypeInfo::Destroy (arg);
  return seq;
}


// �13.3.3.1.2 (and �8.5.14) user-defined conversions
CCConvSeq *CCConversions::userDefinedConversions (CTypeInfo *ptype, 
 CTypeInfo *atype, CTree *arg_expr, CTree *base) {
  CCConvSeq *seq2, *seq = 0;
  CTypeInfo *arg = atype->UnqualType ();
  CTypeInfo *param = ptype->UnqualType ();
  CTypeInfo *ctype;
  CRecord *c1, *c2;

  c1 = arg->isRecord () ? 
       arg->VirtualType ()->TypeRecord ()->Record () : (CRecord*)0;
  c2 = param->UnqualType ()->TypeRecord () ? 
       param->UnqualType ()->TypeRecord ()->Record () : (CRecord*)0;
  if (c1)
    c1 = c1->DefObject ()->Record ();
  if (c2)
    c2 = c2->DefObject ()->Record ();
  
  if (c2) {
    // �13.3.3.1.6 initialization of class type T with T
    if (c1 && *c1 == *c2) {
      seq = new CCStandardConvSeq;
      seq->addConversion (new CCIdentityConv (param, arg, arg_expr, base));
    // �13.3.1.3 initialization of class type T class derived from T
    } else {
      CCOverloading ovl (err);
      if (c1 && baseClass (c2->ClassInfo (), c1->ClassInfo ())) {
        // candidates are converting constructors of c2
        ovl.collectConstructors (c2);
        // select the best constructor
        ovl.resolve (base, arg_expr);
        if (isValidConversion (arg_expr, ptype, atype, ovl)) {
          seq = new CCUserDefinedConvSeq;
          // 1. initial standard conversion sequence
          seq->addConversion (new CCDerivedToBaseConv (param, arg, arg_expr, base));
          // 2. user-defined conversion
          seq->addConversion (new CCUserDefinedConv (param, arg, ovl.Function (), arg_expr, base));
          // 3. second standard conversion sequence
          seq2 = new CCStandardConvSeq;
          seq2->addConversion (new CCIdentityConv (param, arg, arg_expr, base));
          seq->addConversion (seq2);
        }
      // �13.3.1.4 initialization of class type with a (non-)class type
      } else {
        if (c1) {
          // candidates are conversion functions of c1 and its base 
          // classes that are visible in c1 and yield a type whose 
          // cv-unqualified version is the same type as c2 or is a 
          // derived class thereof (including functions returning 
          // "reference to c2" => treated as "c2")
          ovl.collectConvFunctions (c1);
          ovl.ObjectType (arg);
          for (unsigned i = ovl.Candidates (); i > 0; i--) {
            ctype = ovl.Candidate (i-1)->Function ()->ConversionType ();
            if (! (*param->UnqualType () == *ctype->VirtualType () ||
                   (ctype->isClass () && baseClass (param, ctype)))) 
              ovl.removeCandidate (i-1);
          }
        }
        // candidates are converting constructors of c2
        ovl.collectConstructors (c2);
        // select the best conversion function (no user-defined conversions)
        ovl.resolve (base, arg_expr, false);
        if (isValidConversion (arg_expr, ptype, atype, ovl)) {
          seq = new CCUserDefinedConvSeq;
          // 1. initial standard conversion sequence (need not to 
          // be correct, not relevant to the algorithm at all)
          seq->addConversion (new CCIdentityConv (param, arg, arg_expr, base));
          // 2. user-defined conversion
          seq->addConversion (new CCUserDefinedConv (param, arg, ovl.Function (), arg_expr, base));
          // 3. second standard conversion sequence
          seq2 = new CCStandardConvSeq;
          if (! ovl.Function ()->isConstructor () &&
              baseClass (param, ovl.Function ()->ConversionType ()))
            seq2->addConversion (new CCDerivedToBaseConv (param, 
                                 ovl.Function ()->ConversionType (), arg_expr, base));
          else 
            seq2->addConversion (new CCIdentityConv (param, 
                                 ovl.Function ()->ConversionType (), arg_expr, base));
          seq->addConversion (seq2);
        }
      }
    }
  // �13.3.1.5 initialization of non-class type with a class type
  } else if (c1) {
    // candidates are conversion functions of c1 and its base 
    // classes that are visible in c1 and yield a type whose 
    // cv-unqualified version is the same type as c2 or can be
    // standard converted to c2 (including functions returning 
    // "reference to c2" => treated as "c2")
    CCOverloading ovl (err, true);
    ovl.ObjectType (arg);
    ovl.collectConvFunctions (c1);
    for (unsigned i = ovl.Candidates (); i > 0; i--) {
      ctype = ovl.Candidate (i-1)->Function ()->ConversionType ();
      seq2 = standardConversions (param, ctype->VirtualType (), 0, base);
      if (! seq2)
        ovl.removeCandidate (i-1);
      else
        ovl.Candidate (i-1)->SecondConvSeq (seq2);
    }
    // select the best conversion function (no user-defined conversions)
    ovl.resolve (base, arg_expr, false);
    if (isValidConversion (arg_expr, ptype, atype, ovl)) {
      seq = new CCUserDefinedConvSeq;
      // 1. initial standard conversion sequence (need not to 
      // be correct, not relevant to the algorithm at all)
      seq->addConversion (new CCIdentityConv (param, arg, arg_expr, base));
      // 2. user-defined conversion
      seq->addConversion (new CCUserDefinedConv (param, arg, ovl.Function (), arg_expr, base));
      // 3. second standard conversion sequence
      seq2 = standardConversions (param, ovl.Function ()->ConversionType ()->
                                  VirtualType (), 0, base);
      seq->addConversion (seq2);
    }
  }
  
  return seq;
}


// �13.3.3.1.3 ellipsis conversions
CCConvSeq *CCConversions::ellipsisConversions () {
  CCConvSeq *seq = new CCEllipsisConvSeq;
  return seq;
}


// �13.3.3.1.4 (and �8.5.3) reference binding
CCConvSeq *CCConversions::referenceBinding (CTypeInfo *ptype, 
 CTypeInfo *atype, CTree *arg_expr, CTree *base, bool user_def) {
  CTypeInfo *param = ptype->UnqualType ();
  CTypeInfo *arg = atype;
  CCConvSeq *seq = 0, *seq2;
  bool conv = false;
  CTypeInfo *ctype;
  CRecord *c;

  if (param->isAddress ()) {
    param = param->BaseType ();
    
    // �8.5.3.5 direct reference binding to argument expression lvalue
    if (CCSemExpr::isLvalue (arg_expr) && ! arg->isBitField () &&
        referenceCompatible (param, arg)) {
      seq = new CCStandardConvSeq;
      // �13.3.3.1.4.1
      if ((param->isClass () && arg->isClass () && baseClass (param, arg)))
        seq->addConversion (new CCDerivedToBaseConv (param, arg, arg_expr, base));
      else if (arg->isConst () != param->isConst () || arg->isVolatile () != param->isVolatile ())
        seq->addConversion (new CCQualificationConv (param, arg, arg_expr, base));
      else
        seq->addConversion (new CCIdentityConv (param, arg, arg_expr, base));
      conv = true;
    } else if (arg->UnqualType ()->TypeRecord ()) {
      CCOverloading ovl (err);
      c = arg->UnqualType ()->TypeRecord ()->Record ();
      // candidates are conversion functions of c and its 
      // base classes that are visible in c and yield a type 
      // "reference to T" where underlying parameter type is
      // reference-compatible with T
      ovl.ObjectType (arg);
      ovl.collectConvFunctions (c);
      for (unsigned i = ovl.Candidates (); i > 0; i--) {
        ctype = ovl.Candidate (i-1)->Function ()->ConversionType ();
        if (! ctype->isAddress () || 
            ! referenceCompatible (param, ctype->UnqualType ()->BaseType ()))
          ovl.removeCandidate (i-1);
      }
      // select the best conversion function
      ovl.resolve (base, arg_expr);
      if (isValidConversion (arg_expr, ptype, atype, ovl)) {
        seq = new CCUserDefinedConvSeq;
        // 1. initial standard conversion sequence (need not to 
        // be correct, not relevant to the algorithm at all)
        seq->addConversion (new CCIdentityConv (param, arg, arg_expr, base));
        // 2. user-defined conversion
        seq->addConversion (new CCUserDefinedConv (param, arg, ovl.Function (), arg_expr, base));
        // 3. second standard conversion sequence
        seq2 = new CCStandardConvSeq;
        ctype = ovl.Function ()->ConversionType ();
        if (param->isClass () && ctype->isClass () && baseClass (param, ctype))
          seq2->addConversion (new CCDerivedToBaseConv (param, ctype, arg_expr, base));
        else
          seq2->addConversion (new CCIdentityConv (param, ctype, arg_expr, base));
        seq2->isReferenceBinding (true);
        seq->addConversion (seq2);
        conv = true;
      }
    }
    
    // �8.5.3.5 parameter type is reference to non-volatile const type
    if (! conv && param->isQualified () && param->isConst () &&
        ! param->isVolatile ()) {
      if (CCSemExpr::isRvalue (arg_expr) && arg->UnqualType ()->TypeRecord () &&
          referenceCompatible (param, arg)) {
        seq = new CCStandardConvSeq;
        // �13.3.3.1.4.2 not really clear which kind of conversion
        // this is; is it right to distinguish like above?
        if ((param->isClass () && arg->isClass () && baseClass (param, arg)))
          seq->addConversion (new CCDerivedToBaseConv (param, arg, arg_expr, base));
        else if (arg->isConst () != param->isConst () || arg->isVolatile () != param->isVolatile ())
          seq->addConversion (new CCQualificationConv (param, arg, arg_expr, base));
        else
          seq->addConversion (new CCIdentityConv (param, arg, arg_expr, base));
      // �13.3.3.1.4.2 reference not bound directly to argument expression
      } else if (! (referenceRelated (param, arg) && 
                    ! equalOrMoreQualified (param, arg))) {
        // 1. create temporary of underlying parameter type
        // 2. non-reference copy-initialization of temporary
        // 3. temporary is bound to reference
        seq = implicitConversions (param, arg, arg_expr, base, user_def);
      }
    }
  }
  if (seq)
    seq->isReferenceBinding (true);
  return seq;
}


// �8.5.3.4 reference-related types
bool CCConversions::referenceRelated (CTypeInfo *t1, CTypeInfo *t2) {
  // compare unqualified types (top-level qualifiers removed)
  t1 = t1->VirtualType ();
  t2 = t2->VirtualType ();
  if (t1->equalsUnqualified (*t2))
    return true;
  return (t1->isClass () && t2->isClass () && baseClass (t1, t2));
}


// �8.5.3.4 reference-compatible types
bool CCConversions::referenceCompatible (CTypeInfo *t1, CTypeInfo *t2) {
  if (! referenceRelated (t1, t2))
    return false;
  return (equalOrMoreQualified (t1, t2));
}


bool CCConversions::equalOrMoreQualified (CTypeInfo *t1, CTypeInfo *t2) {
  unsigned t1_qualification, t2_qualification;
  CTypeQualified *tq;

  while (t1->TypeArray () && t2->TypeArray ()) {
    t1 = t1->BaseType ();
    t2 = t2->BaseType ();
  }

  if (! t2->isQualified ())
    return true;
  else if (! t1->isQualified ())
    return false;

  tq = t1->TypeQualified ();
  t1_qualification = (tq->isConst ()?2:0) + (tq->isVolatile ()?1:0);
  tq = t2->TypeQualified ();
  t2_qualification = (tq->isConst ()?2:0) + (tq->isVolatile ()?1:0);
  if (t1_qualification >= t2_qualification)
    return true;
  return false;
}


bool CCConversions::baseClass (CTypeInfo *t1, CTypeInfo *t2) {
  t1 = t1->VirtualType ()->TypeClass ();
  t2 = t2->VirtualType ()->TypeClass ();
  if (t1 && t2)
    return baseClass (t1->TypeClass ()->ClassInfo (), 
                      t2->TypeClass ()->ClassInfo ());
  return false;
}


bool CCConversions::baseClass (CClassInfo *c1, CClassInfo *c2) {
  CClassInfo *binfo;
  if (! (c1 && c2))
    return false;
  
  c2 = c2->DefObject ()->ClassInfo ();
  for (unsigned i = 0; i < c2->BaseClasses (); i++) {
    binfo = c2->BaseClass (i)->Class ();
    if (*c1 == *binfo || baseClass (c1, binfo))
      return true;
  }
  return false;
}


// �4.4.4 qualification conversion; similar pointer types
bool CCConversions::similarTypes (CTypeInfo *t1, CTypeInfo *t2) {
  bool all_const = true;
  CTypeInfo *p1, *p2;
  CClassInfo *ci1, *ci2;
  
  if ((t1->isMemberPointer () && t2->isMemberPointer ()) ||
      (t1->isPointer () && t2->isPointer ())) {
    p1 = t2->VirtualType ()->TypePointer ();
    p2 = t1->VirtualType ()->TypePointer ();
    while (true) {
      if ((p1->isMemberPointer () && ! p2->isMemberPointer ()) ||
          (p2->isMemberPointer () && ! p1->isMemberPointer ()))
        break;
      if (p1->isMemberPointer () && 
          p1->TypeMemberPointer ()->Record () &&
          p2->TypeMemberPointer ()->Record () &&
          ! (*p1->TypeMemberPointer ()->Record () ==
             *p2->TypeMemberPointer ()->Record ()))
        break;
      p1 = p1->BaseType ();
      p2 = p2->BaseType ();
      if ((p1->UnqualType ()->isPointer () && ! p2->UnqualType ()->isPointer ()) ||
          (p2->UnqualType ()->isPointer () && ! p1->UnqualType ()->isPointer ()))
        break;

      if (p1->isQualified () && ! p2->isQualified ())
        break;
      if (p1->isQualified ()) { // => p2->isQualified ()
        if ((p1->isVolatile () && ! p2->isVolatile ()) ||
            (p1->isConst () && ! p2->isConst ()))
          break;
        if (((p1->isVolatile ()?1:0)+(p1->isConst ()?2:0)) !=
            ((p2->isVolatile ()?1:0)+(p2->isConst ()?2:0)) &&
            ! all_const)
          break;
      } else if (p2->isQualified () && ! all_const)
        break;
      if (! p2->isQualified () || ! p2->isConst ())
        all_const = false;

      if (! p1->UnqualType ()->isPointer ()) {
        if (p1->equalsUnqualified (*p2))
          return true; // types are similar
        if (p1->UnqualType ()->TypeClass () && p2->UnqualType ()->TypeClass ()) {
          ci1 = p1->UnqualType ()->TypeClass ()->ClassInfo ();
          ci2 = p2->UnqualType ()->TypeClass ()->ClassInfo ();
          if (baseClass (ci2, ci1))
            return true; // types are similar
        }
        break;
      }

      if (p1->isQualified ())
        p1 = p1->BaseType ();
      if (p2->isQualified ())
        p2 = p2->BaseType ();
    }
  }
  return false;
}


bool CCConversions::isValidConversion (CTree *arg_expr, CTypeInfo *to, 
 CTypeInfo *from, CCOverloading &ovl) const {
  if (ovl.isAmbiguous ())
    return false;
  // no candidates
  else if (! ovl.Function ())
    return false;
  return true;
}


////////////////////////////////////////////////////////////
// �13.3.3.2 ranking implicit conversion sequences /////////
////////////////////////////////////////////////////////////


// �13.3.3.2 ranking implicit conversion sequences
// return 1 if seq1 is better than seq2
// return 0 if seq1 and seq2 are indistinguishable
// return -1 if seq1 is worse than seq2
int CCConversions::compareConvSeq (CCConvSeq *seq1, CCConvSeq *seq2) {
  CCConversion *c1, *c2;
  unsigned rank1, rank2;
  bool v1, v2;

  // �13.3.3.2.2 compare basic forms of conversion sequences
  if (seq1->isStandardConvSeq () || seq2->isStandardConvSeq ()) {
    if (seq1->isStandardConvSeq () && ! seq2->isStandardConvSeq ())
      return 1;
    if (seq2->isStandardConvSeq () && ! seq1->isStandardConvSeq ())
      return -1;
  } else if (seq1->isUserDefinedConvSeq () || seq2->isUserDefinedConvSeq ()) {
    if (seq1->isUserDefinedConvSeq () && ! seq2->isUserDefinedConvSeq ())
      return 1;
    if (seq2->isUserDefinedConvSeq () && ! seq1->isUserDefinedConvSeq ())
      return -1;
  // ellipsis conversion sequences are indistinguishable
  } else // if (seq1->isEllipsisConvSeq () && seq2->isEllipsisConvSeq ())
    return 0;
  
  // �13.3.3.2.3 ranking conversion sequences of the same form
  if (seq1->isStandardConvSeq ()) { // => seq2->isStandardConvSeq ()
    // a sequence is better than another if it is 
    // a proper subsequence of that sequence
    if (properSubsequence (seq1, seq2))
      return 1;
    if (properSubsequence (seq2, seq1))
      return -1;
    
    // compare conversion sequence ranks
    rank1 = convSequenceRank (seq1);
    rank2 = convSequenceRank (seq2);
    if (rank1 > rank2)
      return 1;
    if (rank2 > rank1)
      return -1;
    
    // compare conversion sequences with the same rank
    rank1 = compareSameRankConvSeq (seq1, seq2);
    if (rank1 != 0)
      return rank1;

    // compare qualification signature
    c1 = seq1->Conversion (seq1->Conversions ()-1);
    c2 = seq2->Conversion (seq2->Conversions ()-1);
    if (c1->isQualificationConv () || c2->isQualificationConv ()) {
      v1 = similarTypes (c2->ToType (), c1->ToType ());  // c1 similar to c2?
      v2 = similarTypes (c1->ToType (), c2->ToType ());  // c2 similar to c1?
      if (v1 && ! v2)
        return 1;
      if (v2 && ! v1)
        return -1;
      if (c1->isQualificationConv () && ! c2->isQualificationConv ())
        return -1;
      if (c2->isQualificationConv () && ! c1->isQualificationConv ())
        return 1;
    }

    // compare reference bindings
    if (seq1->isReferenceBinding () && seq2->isReferenceBinding ()) {
      c1 = seq1->Conversion (0);
      c2 = seq2->Conversion (0);
      if (*c1->ToType ()->UnqualType () == 
          *c2->ToType ()->UnqualType ()) {
        v1 = equalOrMoreQualified (c1->ToType (), c2->ToType ());
        v2 = equalOrMoreQualified (c2->ToType (), c1->ToType ());
        if (! v1 && v2)
          return 1;
        if (! v2 && v1)
          return -1;
      }
    }
  } else if (seq1->isUserDefinedConvSeq ()) { // => seq2->isUserDefinedConvSeq ()
    // same user-defined conversion function/constructor used
    if ((*seq1->Conversion (1)->ConvFunction () == *seq2->Conversion (1)->ConvFunction ()) ||
        (seq1->Conversion (1)->ConvFunction ()->isConstructor () &&
         seq1->Conversion (1)->ConvFunction ()->isConstructor ()))
      // compare second standard conversion sequences
      return compareConvSeq ((CCConvSeq*)seq1->Conversion (2), (CCConvSeq*)seq2->Conversion (2));
  }

  return 0;
}


// �13.3.3.1.1.3 determine rank of a standard conversion sequence
// return 3 if exact match rank
// return 2 if promotion rank
// return 1 if conversion rank
unsigned CCConversions::convSequenceRank (CCConvSeq *seq) {
  bool promo = false, conv = false;
  CCConversion *c;
  
  for (unsigned i = 0; i < seq->Conversions (); i++) {
    c = seq->Conversion (i);

    if (c->isIntegralPromotion () || c->isFloatingPointPromotion ())
      promo = true;
    else if (c->isFloatingPointConv () || c->isIntegralConv () ||
             c->isFloatingIntegralConv () || c->isPointerConv () ||
             c->isPointerToMemberConv () || c->isBooleanConv () ||
             c->isDerivedToBaseConv ())
      conv = true;
  }

  return conv ? 1 : promo ? 2 : 3;
}


// �13.3.3.2.4 compare standard conversion sequences with the same rank
// return 1 if seq1 is better than seq2
// return 0 if seq1 and seq2 are indistinguishable
// return -1 if seq1 is worse than seq2
int CCConversions::compareSameRankConvSeq (CCConvSeq *seq1, CCConvSeq *seq2) {
  CCConversion *c1 = 0, *c2 = 0;
  CClassInfo *f1 = 0, *f2 = 0, *t1 = 0, *t2 = 0;
  CTypeInfo *param1, *param2, *arg1, *arg2;
  
  for (unsigned i = 0; i < seq1->Conversions (); i++) {
    c1 = seq1->Conversion (i);
    if (! c1->isLvalueToRvalueConv () &&
        ! c1->isArrayToPointerConv () &&
        ! c1->isFunctionToPointerConv ())
      break;
  }
  for (unsigned i = 0; i < seq2->Conversions (); i++) {
    c2 = seq2->Conversion (i);
    if (! c2->isLvalueToRvalueConv () &&
        ! c2->isArrayToPointerConv () &&
        ! c2->isFunctionToPointerConv ())
      break;
  }

  arg1 = c1->FromType ();
  arg2 = c2->FromType ();
  
  // any conversion is better than conversion from pointer, or 
  // pointer to member, to bool
  if ((c2->isBooleanConv () && arg2->isPointer ()) && 
      (! c1->isBooleanConv () || ! arg1->isPointer ()))
    return 1;
  if ((c1->isBooleanConv () && arg1->isPointer ()) && 
      (! c2->isBooleanConv () || ! arg2->isPointer ()))
    return -1;
    
  param1 = c1->ToType ()->UnqualType ();
  param2 = c2->ToType ()->UnqualType ();

  f1 = extractClass (arg1, seq1->isReferenceBinding ());
  f2 = extractClass (arg2, seq2->isReferenceBinding ());
  t1 = extractClass (param1, seq1->isReferenceBinding ());
  t2 = extractClass (param2, seq2->isReferenceBinding ());

  // B is derived from A and C is derived from B => C->B->A

  if (c1->isPointerConv () && c2->isPointerConv ()) {
    // B* to A* better than B* to void*
    if (f1 && f2 && *f1 == *f2) {
      if (! t2 && param2->BaseType ()->isVoid () && baseClass (t1, f1))
        return 1;
      if (! t1 && param1->BaseType ()->isVoid () && baseClass (t2, f2))
        return -1;
    }
    
    // A* to void* better than B* to void*
    if (! t1 && param1->BaseType ()->isVoid () && 
        ! t2 && param2->BaseType ()->isVoid ()) {
      if (baseClass (f1, f2))
        return 1;
      if (baseClass (f2, f1))
        return -1;
    }
  } 
  
  if (! (f1 && f2 && t1 && t2))
    return 0; // indistinguishable

  if (c1->isPointerToMemberConv () && c2->isPointerToMemberConv ()) {
    // A::* to B::* better than A::* to C::*
    if (*f1 == *f2) {
      if (baseClass (f1, t1) && baseClass (t1, t2))
        return 1;
      if (baseClass (f2, t2) && baseClass (t2, t1))
        return -1;
    }
    // B::* to C::* better than A::* to C::*
    if (*t1 == *t2) {
      if (baseClass (f1, t1) && baseClass (f2, f1))
        return 1;
      if (baseClass (f2, t2) && baseClass (f1, f2))
        return -1;
    }
  } else {
    // C* to B* better than C* to A*
    // C  to B& better than C  to A&
    // C  to B  better than C  to A
    if (*f1 == *f2) {
      if (baseClass (t1, f1) && baseClass (t2, t1))
        return 1;
      if (baseClass (t2, f2) && baseClass (t1, t2))
        return -1;
    }

    // B* to A* better than C* to A*
    // B  to A& better than C  to A&
    // B  to A  better than C  to A
    if (*t1 == *t2) {
      if (baseClass (t1, f1) && baseClass (f1, f2))
        return 1;
      if (baseClass (t2, f2) && baseClass (f2, f1))
        return -1;
    }    
  }
  
  return 0; // indistinguishable
}


CClassInfo *CCConversions::extractClass (CTypeInfo *type, bool ref_bind) {
  type = type->VirtualType ();
  if (! ref_bind) {
    if (type->TypeMemberPointer () && type->TypeMemberPointer ()->Record ())
      return type->TypeMemberPointer ()->Record ()->ClassInfo ();
    else if (type->TypePointer ())
      type = type->BaseType ()->UnqualType ();
  }
  return type->TypeClass () ? type->TypeClass ()->ClassInfo () : (CClassInfo*)0;
}


// return true if seq1 is a proper subsequence of seq2
bool CCConversions::properSubsequence (CCConvSeq *seq1, CCConvSeq *seq2) {
  CCConversion *c1, *c2;
  unsigned num_conv1 = seq1->Conversions ();
  unsigned num_conv2 = seq2->Conversions ();
  
  // ignore any lvalue-to-rvalue conversions
  for (unsigned i = 0; i < num_conv2; i++) {
    c1 = seq1->Conversion (i);
    if (c1->isLvalueToRvalueConv () ||
        c1->isArrayToPointerConv () ||
        c1->isFunctionToPointerConv ())
      continue;
      
    // ignore any lvalue-to-rvalue conversions
    for (unsigned j = 0; j < num_conv2; j++) {
      c2 = seq2->Conversion (j);
      if (c2->isLvalueToRvalueConv () ||
          c2->isArrayToPointerConv () ||
          c2->isFunctionToPointerConv ())
        continue;

      // the identity conversion sequence is a subsequence 
      // of any other non-identity conversion sequence
      if ((++j) < num_conv2 && c1->isIdentityConv ())
        return true;

      // now compare the subsequences
      for (unsigned k = j; k < num_conv2; k++) {
        // subsequence of seq2 is longer than subsequence of seq1
        if ((num_conv2-k) > (num_conv1-i))
          continue;
        
        // subsequence of seq2 is shorter than subsequence of seq1
        // so it cannot be a proper subsequence of seq2
        if ((num_conv2-k) < (num_conv1-i))
          return false;
          
        // compare seq1 with the remainder of seq2; must be equal
        for (unsigned l = k; l < num_conv2; l++) {
          c2 = seq2->Conversion (l);
        
          // not the same type of conversion 
          if (c1->Id () != c2->Id ())
            return false;
        
          // same type of conversions; but also same parameter
          // and argument types?
          if (*c1->FromType () != *c2->FromType () ||
              *c1->ToType () != *c2->ToType ())
            return false;
        }        
        return true;
      }
      return false;
    }
    return false;
  }
  return false;
}


} // namespace Puma
