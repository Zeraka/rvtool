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
#include "Puma/CUnionInfo.h"
#include "Puma/CStructure.h"
#include "Puma/CClassInfo.h"
#include "Puma/CScopeInfo.h"
#include "Puma/CEnumInfo.h"
#include "Puma/CFileInfo.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CConstant.h"
#include "Puma/CSemExpr.h"
#include "Puma/CTree.h"
#include "Puma/Limits.h"
#include "Puma/Utf8.h"

// this include shall be removed as soon as ac++ can better deal with
// the include cycle problem
#include "Puma/CSemVisitor.h"

#include <climits>
#include "Puma/WChar.h"    /* wcslen() */
#include <ctype.h>         /* isdigit(), isxdigit() */
#include <string.h>
#include <stdlib.h>        /* strtod() */


namespace Puma {


// print semantic error messages

#define SEM_MSG(node__,mesg__) \
  {if (node__->token ()) err << node__->token ()->location (); err << mesg__ << endMessage;}
  
#define SEM_ERROR(node__, mesg__) \
  SEM_MSG (node__, sev_error << mesg__)

#define SEM_WARNING(node__, mesg__) \
  SEM_MSG (node__, sev_warning << mesg__)

#define SEM_ERROR__undeclared(name__) \
  SEM_ERROR (name__, "`" << name__->token ()->text () << "' undeclared here")

#define SEM_WARNING__out_of_range(node__,what__) \
  SEM_WARNING (node__, what__ << " out of range")
  
#define SEM_WARNING__is_unsigned(node__) \
  SEM_WARNING (node__, \
    "decimal constant is so large that it is unsigned")

#define SEM_WARNING__incompatible_assign(node__) \
  SEM_WARNING (node__, \
    "assignment from incompatible pointer type")

#define SEM_WARNING__init(node__,name__,type__) \
  SEM_WARNING (node__, type__ << " object `" << name__->Name () \
    << "' may not be initialized")

#define SEM_WARNING__type_mismatch(node__,where__) \
  SEM_WARNING (node__, "type mismatch in " << where__)

#define SEM_ERROR__invalid_op(node__,type__,op__) \
  SEM_ERROR (node__, "invalid operand to " << type__ << " `" << op__ << "'")

#define SEM_WARNING__comp_between(node__,what__) \
  SEM_WARNING (node__, "comparison between " << what__)

#define SEM_ERROR__non_lvalue(node__) \
  SEM_ERROR (node__, "non-lvalue in assignment")

#define SEM_ERROR__num_args_too(node__,what__,num__,info__) \
  SEM_ERROR (node__, "too " << what__ << " arguments (" << num__ << ") in call to function `" \
    << info__->Name () << "'")

#define SEM_ERROR__invalid_arg(node__,pos__,info__) \
  SEM_ERROR (node__, "invalid argument " << pos__ \
    << " in call to function `" << info__->Name () << "'")

#define SEM_WARNING__incompatible_arg(node__,pos__,info__) \
  SEM_WARNING (node__, "passing argument " << pos__ \
    << " of `" << info__->Name () << "' from incompatible type")


///////////////////////////////////////////////////////
// expression evaluation //////////////////////////////
///////////////////////////////////////////////////////


CTypeInfo *CSemExpr::resolveExpr (CTree *expr, CTree *base) {
  CTypeInfo *result = expr->Type ();

  // ensure not to evaluate twice
  if (! result || *result == CTYPE_UNDEFINED || 
     expr->IsSimpleName () ||
     expr->NodeName () == CT_BracedExpr::NodeId ()) {
    // call the node type specific resolve function
    if (!(result = expr->resolve (*this, base))) {
      SEM_ERROR (expr, "expression expected");
      result = &CTYPE_UNDEFINED;
      if (expr->SemValue ())
        expr->SemValue ()->setTypeRef (result);
    }
  }

  // implicit lvalue, array and function type conversions
  result = applyImplicitConv (expr, base, result);

  return result;
}


// fallback: unhandled kind of expression
CTypeInfo *CSemExpr::resolve (CTree *node, CTree *base) {
  SEM_ERROR (node, "illegal expression");
  CTypeInfo *result = &CTYPE_UNDEFINED;
  if (node->SemValue ())
    node->SemValue ()->setTypeRef (result);
  return result;
}


///////////////////////////////////////////////////////
// boolean constant ///////////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_Bool *node, CTree *base) {
  CTypeInfo *result;
  U_LONG_LONG v;
  
  v = (strcmp (node->token ()->text (), "true") == 0) ? 1 : 0;
  result = &CTYPE_C_BOOL;

  node->setValue (new CConstant (v, result));
  node->setTypeRef (result);
  return result;
}



///////////////////////////////////////////////////////
// character constant /////////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_Character *node, CTree *base) {
  CTypeInfo *result;
  LONG_LONG v;
  int chars;
  
  chars = scan_character (node, v);
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



///////////////////////////////////////////////////////
// wide character constant ////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_WideCharacter *node, CTree *base) {
  CTypeInfo *result;
  LONG_LONG v;
  int chars;
  
  chars = scan_character (node, v);
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



///////////////////////////////////////////////////////
// string constant ////////////////////////////////////
///////////////////////////////////////////////////////


CTypeInfo *CSemExpr::resolve (CT_String *node, CTree *base) {
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
      i = next_character (s, ucs);
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

  // type `array of char'
  result = new CTypeArray (&CTYPE_CHAR);
  result->TypeArray ()->Dimension (ct+1);
  node->setValue (new CStrLiteral (buf, ct, result));
  node->setType (result);

  return result;
}



///////////////////////////////////////////////////////
// wide string constant ///////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_WideString *node, CTree *base) {
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
      i = next_character (s, ucs);
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

  // type `array of wchar_t'
  result = new CTypeArray (&CTYPE_WCHAR_T);
  result->TypeArray ()->Dimension (ct+1);
  node->setValue (new CWStrLiteral (buf, ct, result));
  node->setType (result);

  return result;
}



///////////////////////////////////////////////////////
// integer constant ///////////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_Integer *node, CTree *) {
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
  long int size = CTypePrimitive::SizeOfNumber(val);
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



///////////////////////////////////////////////////////
// floating point constant ////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_Float *node, CTree *base) {
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
// name ///////////////////////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_SimpleName *node, CTree *base) {
  CObjectInfo *info;
  CTypeInfo *result;
  LONG_LONG v;
    
  // find semantic object to name
  info = lookup (node, NON_TAG, true);
  if (! info || info->TypedefInfo ()) {
    // typedef name or not a implicitely declared function
    if (info || base->NodeName () != CT_CallExpr::NodeId ()) {
      SEM_ERROR__undeclared (node);
    }
    node->setTypeRef (&CTYPE_UNDEFINED);
    return &CTYPE_UNDEFINED;
  }
  
  result = info->TypeInfo ();
  node->Object (info);
  node->setTypeRef (result);
  
  // name of a constant, refer to its value
  if (info->EnumeratorInfo () && info->EnumeratorInfo ()->hasValue ()) {
    v = info->EnumeratorInfo ()->Value ();
    node->setValue (new CConstant (v, result));
  }
  
  return result;
}



///////////////////////////////////////////////////////
// braced expression //////////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_BracedExpr *node, CTree *base) {
  CTypeInfo* result = resolveExpr (node->Expr (), node);

  // handle constants
  passOnConstant (node->Expr (), node, result);
  
  node->setTypeRef (result);
  return result;
}



///////////////////////////////////////////////////////
// binary expression //////////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_BinaryExpr *node, CTree *base) {
  CTypeInfo *result;
  int ctp_old, oper;
  
  result = &CTYPE_UNDEFINED;
  oper = node->Son (1)->token ()->type ();
  
  ctp_old = conv_to_ptr;
  conv_to_ptr = 1;
  
  // don't apply lvalue-to-rvalue conversion if left operand of assignment
  if (oper == TOK_ASSIGN || oper == TOK_MUL_EQ || oper == TOK_DIV_EQ ||
      oper == TOK_MOD_EQ || oper == TOK_ADD_EQ || oper == TOK_SUB_EQ ||
      oper == TOK_RSH_EQ || oper == TOK_LSH_EQ || oper == TOK_AND_EQ ||
      oper == TOK_XOR_EQ || oper == TOK_IOR_EQ)
    conv_to_ptr = 3; 

  // resolve operands
  resolveExpr (node->Son (0), node);
  if (conv_to_ptr == 3)
    conv_to_ptr = 1;
  if (oper == TOK_COMMA)
    conv_to_ptr = ctp_old;
  resolveExpr (node->Son (2), node);

  // assignment expression: op1 = op2; op1 *= op2; op1 /= op2;
  //   op1 %= op2; op1 += op2; op1 -= op2; op1 >>= op2; 
  //   op1 <<= op2; op1 &= op2; op1 ^= op2; op1 |= op2
  if (oper == TOK_ASSIGN || oper == TOK_MUL_EQ || oper == TOK_DIV_EQ ||
      oper == TOK_MOD_EQ || oper == TOK_ADD_EQ || oper == TOK_SUB_EQ ||
      oper == TOK_RSH_EQ || oper == TOK_LSH_EQ || oper == TOK_AND_EQ ||
      oper == TOK_XOR_EQ || oper == TOK_IOR_EQ) 
    result = binary_ass (node);
  // additive expression: op1 + op2; op1 - op2
  else if (oper == TOK_PLUS || oper == TOK_MINUS)
    result = binary_add (node);
  // multiplicative expression: op1 * op2; op1 / op2; op1 % op2
  else if (oper == TOK_MUL || oper == TOK_DIV || oper == TOK_MODULO)
    result = binary_mul (node);
  // shift expression: op1 << op2; op1 >> op2
  else if (oper == TOK_LSH || oper == TOK_RSH) 
    result = binary_shift (node);
  // relational expression: op1 < op2; op1 > op2; op1 <= op2; op1 >= op2
  else if (oper == TOK_LESS || oper == TOK_GREATER || 
           oper == TOK_LEQ || oper == TOK_GEQ) 
    result = binary_rel (node);
  // equality expression: op1 == op2; op1 != op2
  else if (oper == TOK_EQL || oper == TOK_NEQ) 
    result = binary_equ (node);
  // AND expression: op1 & op2
  // exclusive-OR expression: op1 ^ op2
  // inclusive-OR expression: op1 | op2
  else if (oper == TOK_AND || oper == TOK_ROOF || oper == TOK_OR)
    result = binary_and (node);
  // logical-AND expression: op1 && op2
  // logical-OR expression: op1 || op2
  else if (oper == TOK_AND_AND || oper == TOK_OR_OR)
    result = binary_log (node);
  // comma expression: op1 , op2
  else if (oper == TOK_COMMA)
    result = binary_comma (node);

  node->setTypeRef (result);
  return result;
}

CTypeInfo *CSemExpr::binary_add (CT_BinaryExpr *node) {
  CTypeInfo *t1, *t2;
  unsigned long pos;
  int oper;

  oper = node->Son (1)->token ()->type ();
  pos = node->Son (1)->token_node ()->Number ();

  // apply integer promotions on operands
  t1 = promote (node->Son (0), node); 
  t2 = promote (node->Son (2), node);

  if (t1->isArithmetic () && t2->isArithmetic ()) {
    return apply_binary_op (node, oper);
  } else if (oper == TOK_PLUS) {
    if (! (t1->isArithmetic () && isPtrToObj (t2, pos)) &&
        ! (t2->isArithmetic () && isPtrToObj (t1, pos))) {
      SEM_ERROR__invalid_op (node, "binary", "+");
    } else
      return t1->isPointer () ? t1 : t2;
  } else { // TOK_MINUS
    if (! (isPtrToObj (t1, pos) && t2->isInteger ()) &&
        ! (isPtrToObj (t1, pos) && isPtrToObj (t2, pos) && compatibleBase (t1, t2))) {
      SEM_ERROR__invalid_op (node, "binary", "-");
    } else                                           /* ptrdiff_t */
      return t1->isPointer () && t2->isPointer () ? CTypeInfo::CTYPE_PTRDIFF_T : t1;
  }
  return &CTYPE_UNDEFINED;
}

CTypeInfo *CSemExpr::binary_mul (CT_BinaryExpr *node) {
  CTypeInfo *t1, *t2;
  const char *op;
  CConstant *v2;
  int oper;

  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();

  // apply integer promotions on operands
  t1 = promote (node->Son (0), node); 
  t2 = promote (node->Son (2), node);

  if (! (t1->isArithmetic () && t2->isArithmetic ()) &&
      ! (oper == TOK_MODULO && t1->isInteger () && t2->isInteger ())) {
    SEM_ERROR__invalid_op (node, "binary", op);
  } else {
    v2 = node->Son (2)->Value () ? node->Son (2)->Value ()->Constant () : 0;
    if ((oper == TOK_MODULO || oper == TOK_DIV) && v2 && v2->isNull ()) 
      SEM_WARNING (node, "division by zero");
    return apply_binary_op (node, oper);
  }
  return &CTYPE_UNDEFINED;
}

CTypeInfo *CSemExpr::binary_shift (CT_BinaryExpr *node) {
  CTypeInfo *t1, *t2;
  const char *op;
  CConstant *v2;
  int oper;

  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();

  // apply integer promotions on operands
  t1 = promote (node->Son (0), node); 
  t2 = promote (node->Son (2), node);

  if (! (t1->isInteger () && t2->isInteger ())) {
    SEM_ERROR__invalid_op (node, "binary", op);
  } else {
    v2 = node->Son (2)->Value () ? node->Son (2)->Value ()->Constant () : 0;
    if (v2 && v2->isNegative ())
      SEM_WARNING (node, "negative shift count");
      
    t2 = apply_binary_op (node, oper);
    v2 = node->Value () ? node->Value ()->Constant () : 0;
    if (v2 && t2 != t1) // must have type of promoted first operand
      node->setValue (v2->cast_to (t1));
    return t1;
  }
  return &CTYPE_UNDEFINED;
}

CTypeInfo *CSemExpr::binary_rel (CT_BinaryExpr *node) {
  CTypeInfo *t1, *t2;
  const char *op;
  CConstant *v1;
  int oper;

  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();

  // apply integer promotions on operands
  t1 = promote (node->Son (0), node); 
  t2 = promote (node->Son (2), node);

  if (t1->isArithmetic () && t2->isArithmetic ()) {
    t1 = apply_binary_op (node, oper);
    v1 = node->Value () ? node->Value ()->Constant () : 0;
    if (v1 && *t1 != CTYPE_INT) // must have type `int'
      node->setValue (v1->cast_to (&CTYPE_INT));
  } else if (! (t1->isPointer () && t2->isPointer () && 
                compatibleBase (t1, t2))) 
    SEM_ERROR__invalid_op (node, "binary", op);
  return &CTYPE_INT;
}

CTypeInfo *CSemExpr::binary_equ (CT_BinaryExpr *node) {
  CTypeInfo *t1, *t2;
  const char *op;
  CConstant *v1;
  int oper;
  bool l;

  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();

  // apply integer promotions on operands
  t1 = promote (node->Son (0), node); 
  t2 = promote (node->Son (2), node);

  if (t1->isArithmetic () && t2->isArithmetic ()) {
    t1 = apply_binary_op (node, oper);
    v1 = node->Value () ? node->Value ()->Constant () : 0;
    if (v1 && *t1 != CTYPE_INT) // must have type `int'
      node->setValue (v1->cast_to (&CTYPE_INT));
  } else if ((t1->isInteger () && t2->isPointer ()) ||
             (t2->isInteger () && t1->isPointer ())) {
    l = t1->isInteger () ? true : false;
    v1 = node->Son (l?0:2)->Value () ? node->Son (l?0:2)->Value ()->Constant () : 0;
    if (v1 && v1->isNull ()) // null pointer constant
      cast_to (l?t2:t1, node->Son (l?0:2), node);
    else
      SEM_WARNING__comp_between (node, "pointer and integer");
  } else if (t1->isPointer () && t2->isPointer () &&
             (t1->VirtualType ()->BaseType ()->isVoid () || 
              t2->VirtualType ()->BaseType ()->isVoid ())) {
    if (*t1 != *t2) {
      l = t1->VirtualType ()->BaseType ()->isVoid () ? true : false;
      cast_to (l?t1:t2, node->Son (l?2:0), node);
    }
  } else if (t1->isPointer () && t2->isPointer ()) {
    if (! compatibleBase (t1, t2))
      SEM_WARNING__comp_between (node, "distinct pointer types");
  } else
    SEM_ERROR__invalid_op (node, "binary", op);
  return &CTYPE_INT;
}

CTypeInfo *CSemExpr::binary_and (CT_BinaryExpr *node) {
  CTypeInfo *t1, *t2;
  const char *op;
  int oper;

  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();

  // apply integer promotions on operands
  t1 = promote (node->Son (0), node); 
  t2 = promote (node->Son (2), node);

  if (! (t1->isInteger () && t2->isInteger ())) {
    SEM_ERROR__invalid_op (node, "binary", op);
  } else 
    return apply_binary_op (node, oper);
  return &CTYPE_UNDEFINED;
}

CTypeInfo *CSemExpr::binary_log (CT_BinaryExpr *node) {
  CTypeInfo *t1, *t2;
  const char *op;
  CConstant *v1;
  int oper;

  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();

  // apply integer promotions on operands
  t1 = promote (node->Son (0), node); 
  t2 = promote (node->Son (2), node);

  if (! (t1->isScalar () && t2->isScalar ())) {
    SEM_ERROR__invalid_op (node, "binary", op);
  } else if (t1->isArithmetic () && t2->isArithmetic ()) {
    t1 = apply_binary_op (node, oper);
    v1 = node->Value () ? node->Value ()->Constant () : 0;
    if (v1 && *t1 != CTYPE_INT) // must have type `int'
      node->setValue (v1->cast_to (&CTYPE_INT)); 
  }
  return &CTYPE_INT;
}

CTypeInfo *CSemExpr::binary_comma (CT_BinaryExpr *node) {
//  CExprValue *value;

//  value = node->Son (2)->Value ();
//  if (value)
//    node->setValueRef (value);
  return node->Son (2)->Type ();
}

CTypeInfo *CSemExpr::binary_ass (CT_BinaryExpr *node) {
  CTypeInfo *t1, *t2;
  unsigned long pos;
  const char *op;
  CConstant *v2;
  int oper;

  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();
  pos = node->Son (1)->token_node ()->Number ();
  t1 = node->Son (0)->Type (); 
  t2 = node->Son (2)->Type (); 
  v2 = node->Son (2)->Value () ? node->Son (2)->Value ()->Constant () : 0;

  if (! isLvalue (node->Son (0))) {
    SEM_ERROR__non_lvalue (node);
  } else {
    if (! isModifiable (t1, pos))
      SEM_WARNING (node, "assignment of unmodifiable location");
    if (t1->isArithmetic () && t2->isArithmetic ()) {
      if (oper == TOK_ASSIGN) {
        if (*t1 != *t2)
          cast_to (t1->VirtualType (), node->Son (2), node);
      } else {
        if ((oper == TOK_RSH_EQ || oper == TOK_LSH_EQ || 
             oper == TOK_MOD_EQ || oper == TOK_AND || 
             oper == TOK_ROOF || oper == TOK_OR) &&
            ! (t1->isInteger () && t2->isInteger ()))
          SEM_ERROR__invalid_op (node, "binary", op);
        if (v2) {
          if ((oper == TOK_MOD_EQ || oper == TOK_DIV_EQ) && v2->isNull ()) {
            SEM_WARNING (node, "division by zero");
          } else if ((oper == TOK_RSH_EQ || oper == TOK_LSH_EQ) && 
                   v2->isNegative ()) {
            SEM_WARNING (node, "negative shift count");
          }
        }
      }
      return t1->VirtualType ();
    } else if (oper == TOK_ASSIGN) {
      if (! compatible (t1, t2, v2)) 
        SEM_WARNING__incompatible_assign (node);
      if (*t1 != *t2)
        cast_to (t1->VirtualType (), node->Son (2), node);
      return t1->VirtualType ();
    } else if (oper == TOK_ADD_EQ || oper == TOK_SUB_EQ) {
      if (! (isPtrToObj (t1, pos) && t2->isInteger ())) { 
        SEM_ERROR__invalid_op (node, "binary", op);
      } else
        return t1->VirtualType ();
    } else
      SEM_ERROR__invalid_op (node, "binary", op);
  }
  return &CTYPE_UNDEFINED;
}

// apply binary operator
CTypeInfo *CSemExpr::apply_binary_op (CTree *node, int oper) {
  CTypeInfo *t1, *t2, *result;
  CConstant *v1, *v2;
  CTree *op1, *op2;

  op1 = node->Son (0); 
  op2 = node->Son (2);
  t1 = op1->Type (); 
  t2 = op2->Type ();
  
  // cast operands if needed
  result = t1;
  if (*t1 > *t2) {
    cast_to (t1, op2, node);
    op2 = node->Son (2);
  } else if (*t1 < *t2) {
    result = cast_to (t2, op1, node);
    op1 = node->Son (0);
  }
  
  // apply operator on constant values if any
  v1 = op1->Value () ? op1->Value ()->Constant () : (CConstant*)0; 
  v2 = op2->Value () ? op2->Value ()->Constant () : (CConstant*)0; 
  if (v1 && v2 && node->SemValue ())
    node->SemValue ()->setValue (v1->compute (oper, v2));

  return result;
}



///////////////////////////////////////////////////////
// unary expression ///////////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_UnaryExpr *node, CTree *base) {
  CTypeInfo *result, *t1;
  unsigned long pos;
  const char *op;
  CConstant *v1;
  CTree *op1;
  int oper;
  
  result = &CTYPE_UNDEFINED;
  op = node->Son (0)->token ()->text ();
  oper = node->Son (0)->token ()->type ();
  
  conv_to_ptr = 1;

  // don't apply lvalue-to-rvalue conversion
  if (oper == TOK_DECR || oper == TOK_INCR)
    conv_to_ptr = 3;

  // resolve operands
  t1 = resolveExpr (node->Son (1), node); 
  op1 = node->Son (1);
  pos = node->Son (1)->token_node ()->Number ();
  v1 = op1->Value () ? op1->Value ()->Constant () : (CConstant*)0;
    
  // ++ op1; -- op1
  if (oper == TOK_DECR || oper == TOK_INCR) {
    if (! isLvalue (op1)) { 
      SEM_ERROR__non_lvalue (node);
    } else {
      if (! isModifiable (t1, pos)) {
        if (oper == TOK_DECR) {
          SEM_WARNING (node, "decrement of unmodifiable location");
        } else {
          SEM_WARNING (node, "increment of unmodifiable location");
        }
      }
      if (! t1->isScalar ()) {
        SEM_ERROR__invalid_op (node, "unary", op);
      } else
        result = t1->VirtualType ();
    }
  }
  // + op1; - op1; ~ op1
  else if (oper == TOK_PLUS || oper == TOK_MINUS || oper == TOK_TILDE) {
    t1 = promote (op1, node);
    if (! t1->isArithmetic () || (oper == TOK_TILDE && ! t1->isInteger ())) {
      SEM_ERROR__invalid_op (node, "unary", op);
    } else {
      if (v1) 
        node->setValue (v1->compute (oper));
      result = t1;
    }
  }
  // ! op1
  else if (oper == TOK_NOT) {
    if (! t1->isScalar ()) {
      SEM_ERROR__invalid_op (node, "unary", op);
    } else {
      if (v1) {
        node->setValue (v1->compute (oper));
        if (*t1 != CTYPE_INT) // must have type `int'
          node->setValue (v1->cast_to (&CTYPE_INT)); 
      }
      result = &CTYPE_INT;
    }
  }

  node->setTypeRef (result);
  return result;
}

CTypeInfo *CSemExpr::resolve (CT_AddrExpr *node, CTree *) {
  CTypeInfo *result, *t1;
  CObjectInfo *info;
  CTree *op1, *base;
  
  result = &CTYPE_UNDEFINED;

  // do not apply standard array and function type conversions
  conv_to_ptr = 0;
  
  // resolve operand
  t1 = resolveExpr (node->Son (1), node); 
  op1 = node->Son (1);

  base = op1;
  while (true) {
    if (base->NodeName () == CT_BracedExpr::NodeId ())
      base = base->Son (1);
    else if (base->NodeName () == CT_ImplicitCast::NodeId ())
      base = base->Son (0);
    else
      break; 
  }

  info = findObject (op1);
  if (t1->isFunction () || 
      t1->isArray () ||
      base->NodeName () == CT_DerefExpr::NodeId () ||
      base->NodeName () == CT_IndexExpr::NodeId () ||
      (isLvalue (op1) && (info ? ! info->TypeInfo ()->TypeBitField () : true))) {
    result = new CTypePointer (t1->Duplicate ());
    cast_to (result, op1, node, false);
  } else {
    SEM_ERROR__invalid_op (node, "unary", "&");
  }
  
  node->setTypeRef (result);
  return result;
}

CTypeInfo *CSemExpr::resolve (CT_DerefExpr *node, CTree *base) {
  CTypeInfo *result, *t1;
  unsigned long pos;
  int ctp_old;

  result = &CTYPE_UNDEFINED;
  
  ctp_old = conv_to_ptr;
  conv_to_ptr = 1;

  // resolve operand
  t1 = resolveExpr (node->Son (1), node); 
  conv_to_ptr = ctp_old;
  pos = node->Son (1)->token_node ()->Number ();

  if (! t1->isPointer ()) {
    SEM_ERROR__invalid_op (node, "unary", "*");
  } else {
    if (! isPtrToObj (t1, pos))
      SEM_WARNING (node, "dereferencing pointer to incomplete type");
    result = t1->VirtualType ()->BaseType ();
  }

  node->setTypeRef (result);
  return result;
}



///////////////////////////////////////////////////////
// postfix expression /////////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_PostfixExpr *node, CTree *base) {
  CTypeInfo *result, *t1;
  unsigned long pos;
  const char *op;
  int oper;
  
  conv_to_ptr = 1;

  result = &CTYPE_UNDEFINED;
  op = node->Son (1)->token ()->text ();
  oper = node->Son (1)->token ()->type ();
  pos = node->Son (1)->token_node ()->Number ();
  
  // don't apply lvalue-to-rvalue conversion
  if (oper == TOK_DECR || oper == TOK_INCR)
    conv_to_ptr = 3;

  // resolve operand
  t1 = resolveExpr (node->Son (0), node); 
    
  // postfix in-/decrement: op1 ++; op1 --
  if (! isLvalue (node->Son (0))) { 
    SEM_ERROR__non_lvalue (node);
  } else {
    if (! isModifiable (t1, pos)) {
      if (oper == TOK_DECR) {
        SEM_WARNING (node, "decrement of unmodifiable location");
      } else {
        SEM_WARNING (node, "increment of unmodifiable location");
      }
    }
    if (! t1->isScalar ()) {
      SEM_ERROR__invalid_op (node, "unary", op);
    } else
      result = t1->VirtualType ();
  }

  node->setTypeRef (result);
  return result;
}



///////////////////////////////////////////////////////
// index expression ///////////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_IndexExpr *node, CTree *base) {
  CTypeInfo *result, *t1, *t2;
  unsigned long pos;
  int ctp_old;
  
  result = &CTYPE_UNDEFINED;
  
  ctp_old = conv_to_ptr;
  conv_to_ptr = 1;

  // resolve operands
  t1 = resolveExpr (node->Son (0), node); 
  t2 = resolveExpr (node->Son (2), node); 
  conv_to_ptr = ctp_old;
  pos = node->Son (0)->token_node ()->Number ();
    
  // array subscripting: op1 [ op2 ]
  if (! (isPtrToObj (t1, pos) && t2->isInteger ())) { 
    SEM_ERROR__invalid_op (node, "array subscript", "[]");
  } else 
    result = t1->VirtualType ()->BaseType ();

  node->setTypeRef (result);
  return result;
}



///////////////////////////////////////////////////////
// sizeof expression //////////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_SizeofExpr *node, CTree *base) {
  CTypeInfo *result, *t1, *unqual;
  CObjectInfo *info;
  U_LONG_LONG size;

  result = CTypeInfo::CTYPE_SIZE_T; // size_t
  size = 0;

  // resolve operand; can be either expression or named type
  if (node->TypeName ()) { // named type
    info = node->TypeName ()->Object ();
    t1 = info ? info->TypeInfo () : 0;
  } else {
    // do not convert array/function type to pointer type
    conv_to_ptr = 0;
    t1 = resolveExpr (node->Expr (), node);
    info = findObject (node->Expr ());
  }

  // sizeof op1; sizeof ( op1 )
  if (! t1 || t1->isFunction () || (info && info->TypeInfo ()->TypeBitField ())) {
    SEM_ERROR__invalid_op (node, "unary", "sizeof");
  } else {
    unqual = t1->UnqualType();
    if (unqual->is_char() || unqual->is_signed_char() || unqual->is_unsigned_char()) {
      size = 1; // always 1 byte
    } else {
      size = t1->Size () / 8; // size in bytes
      node->setValue (new CConstant (size, result));
    }
  }

  node->setTypeRef (result);
  return result;
}



///////////////////////////////////////////////////////
// alignof expression //////////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_AlignofExpr *node, CTree *base) {
  CTypeInfo *result, *t1;
  CObjectInfo *info;
  U_LONG_LONG align;

  result = CTypeInfo::CTYPE_SIZE_T; // size_t

  // resolve operand; can be either expression or named type
  if (node->TypeName ()) { // named type
    info = node->TypeName ()->Object ();
    t1 = info ? info->TypeInfo () : 0;
  } else {
    // do not convert array/function type to pointer type
    conv_to_ptr = 0;
    t1 = resolveExpr (node->Expr (), node);
    //info = findObject (node->Expr ());
  }

  // alignof op1; alignof ( op1 )
  if (! t1) {
    SEM_ERROR__invalid_op (node, "unary", "alignof");
  } else {
    align = t1->Align (); // alignment in bytes
    if (align) {
      node->setValue (new CConstant (align, result));
    }
  }

  node->setTypeRef (result);
  return result;
}



///////////////////////////////////////////////////////
// offsetof expression ////////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_OffsetofExpr *node, CTree *base) {
  CTypeInfo *result;
  U_LONG_LONG offset;
  
  // TODO: calculate the offset in bytes of the given member
  offset = 0;
  result = CTypeInfo::CTYPE_SIZE_T; // size_t
  
  node->setValue (new CConstant (offset, result));
  node->setTypeRef (result);
  return result;
}



///////////////////////////////////////////////////////
// if-then expression /////////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::cond_expr_type (CT_IfThenExpr *node,
    CTypeInfo *t1, CTree *op1, CTypeInfo *t2, CTree *op2,
    CTypeInfo *t3, CTree *op3) {
  
  bool l;
  CConstant *v1;
  CTypeInfo *result = &CTYPE_UNDEFINED;

  if (! t1->isScalar ()) {
    SEM_ERROR__invalid_op (node, "conditional", "?:");
  } else if ((t2->isVoid () && t3->isVoid ()) ||
             (t2->isRecord () && t3->isRecord () && compatible (t2, t3))) {
    result = t2;
  } else if ((t2->isInteger () && t3->isPointer ()) ||
             (t3->isInteger () && t2->isPointer ())) {
    l = t2->isInteger () ? true : false;
    v1 = (l?op2:op3)->Value () ? (l?op2:op3)->Value ()->Constant () : 0;
    if (! v1 || ! v1->isNull ()) // null pointer constant
      SEM_WARNING__type_mismatch (node, "conditional expression");
    result = l?t3:t2;
  } else if (t2->isPointer () && t3->isPointer () && 
             ((t2->VirtualType ()->BaseType ()->isVoid () && 
               ! t3->VirtualType ()->BaseType ()->isVoid ()) ||
              (t3->VirtualType ()->BaseType ()->isVoid () && 
               ! t2->VirtualType ()->BaseType ()->isVoid ()))) {
    l = t2->VirtualType ()->BaseType ()->isVoid () ? false : true;
    if (t2->isConst () != t3->isConst () ||
        t2->isVolatile () != t3->isVolatile () ||
        t2->isRestrict () != t3->isRestrict ())
      result = new CTypeQualified ((l?t3:t2)->VirtualType ()->BaseType ()->Duplicate (), 
        t2->isConst () || t3->isConst (),
        t2->isVolatile () || t3->isVolatile (),
        t2->isRestrict () || t3->isRestrict ());
    else
      result = (l?t3:t2);
  } else if (t2->isPointer () && t3->isPointer ()) {
    if (! compatibleBase (t2, t3)) {
      SEM_WARNING__type_mismatch (node, "conditional expression");
      result = t2;
    } else if (t2->isConst () != t3->isConst () ||
               t2->isVolatile () != t3->isVolatile () ||
               t2->isRestrict () != t3->isRestrict ()) {
      result = new CTypeQualified (t2->VirtualType ()->BaseType ()->Duplicate (), 
        t2->isConst () || t3->isConst (),
        t2->isVolatile () || t3->isVolatile (),
        t2->isRestrict () || t3->isRestrict ());
    } else if (t2->PtrToFct () && t3->PtrToFct ()) {
      result = t2->PtrToFct ()->ArgTypes ()->Entries () ? t2 : t3;
    } else {
      result = t2;
    }
  } else if (t2->isArithmetic () && t3->isArithmetic ()) {
    result = t2;
    if (*t2 < *t3) {
      result = t3;
    }
  } else
    SEM_ERROR__invalid_op (node, "conditional", "?:");

  return result;
}

CTypeInfo *CSemExpr::resolve (CT_IfThenExpr *node, CTree *base) {
  CTypeInfo *result, *t1, *t2, *t3;
  CTree *op1, *op2, *op3;
  CConstant *v1;

  conv_to_ptr = 1;

  // resolve operands
  t1 = resolveExpr (node->Son (0), node);
  t2 = resolveExpr (node->Son (2), node);
  t3 = resolveExpr (node->Son (4), node);
  op1 = node->Son (0);
  op2 = node->Son (2);
  op3 = node->Son (4);

  // determine the result type
  result = cond_expr_type (node, t1, op1, t2, op2, t3, op3);

  // apply arithmetic conversions if needed
  if (t2->isArithmetic () && t3->isArithmetic ()) {
    if (*t2 > *t3) {
      result = cast_to (t2, op3, node);
      op3 = node->Son (4);
    } else if (*t2 < *t3) {
      result = cast_to (t3, op2, node);
      op2 = node->Son (2);
    } else
      result = t2;
  }

  // handle constants
  v1 = op1->Value () ? op1->Value ()->Constant () : 0;
  if (v1) { 
    if (v1->isNull ()) {
      if (op3->Value ())
        node->setValueRef (op3->Value ()); 
    } else {
      if (op2->Value ())
        node->setValueRef (op2->Value ()); 
    }
  }
  
  if (!result || result == t2 || result == t3 || *result == CTYPE_UNDEFINED)
    node->setTypeRef (result);
  else
    node->setType (result);
  return result;
}


///////////////////////////////////////////////////////
// cast expression ////////////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_CastExpr *node, CTree *base) {
  CTypeInfo *result, *t1, *t2;
  CExprValue *value;

  conv_to_ptr = 1;
  result = &CTYPE_UNDEFINED;

  // resolve operands
  t1 = node->TypeName ()->Object () ?
       node->TypeName ()->Object ()->TypeInfo () : 0;
  t2 = resolveExpr (node->Expr (), node);

  // cast operator; ( op1 ) op2
  if (t1 && ((t1->isScalar () && t2->isScalar ()) || t1->isVoid ())) {
    // cast constant value if any
    value = node->Expr ()->Value ();
    if (value) {
      if (value->Constant () && *t1 != *t2) {
        value = value->Constant ()->cast_to (t1);
        node->setValue (value);
      } else
        node->setValueRef (value);
    }
    result = t1;
  // cast to union: ( union ) op2
  } else if (t1 && t1->isUnion ()) {
    // as an extension to ISO C, the cast is valid if the
    // union has a member with exactly the type of op2
    CUnionInfo* u = t1->VirtualType ()->TypeUnion ()->UnionInfo ();
    if (u) {
      unsigned n = u->Attributes ();
      for (unsigned i = 0; i < n; i++) {
        if (*u->Attribute (i)->TypeInfo () == *t2) {
          result = t1;
          break;
        }
      }
    }
  }

  if (result == &CTYPE_UNDEFINED)
    SEM_ERROR (node, "invalid cast");

  node->setTypeRef (result);
  return result;
}



///////////////////////////////////////////////////////
// structure/union member access expression ///////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_MembPtrExpr *node, CTree *base) {
  CT_SimpleName *member;
  CTypeInfo *t1, *t2;
  CObjectInfo *info;
  CStructure *sinfo;
  unsigned long pos;
  const char *name;
  int ctp_old, oper;
  
  oper = node->Son (1)->token ()->type ();
  pos = node->Son (1)->token_node ()->Number ();
  sinfo = 0;
  
  ctp_old = conv_to_ptr;
  conv_to_ptr = 1;

  // don't apply lvalue-to-rvalue conversion
  if (oper == TOK_DOT)
    conv_to_ptr = 3;

  // resolve operands
  t1 = resolveExpr (node->Son (0), node); 
  conv_to_ptr = ctp_old;
  
  // op1 -> op2
  if (oper == TOK_PTS) {
    if (! (isPtrToObj (t1, pos) && t1->VirtualType ()->BaseType ()->isRecord ())) { 
      SEM_ERROR__invalid_op (node, "binary", "->");
    } else {
      t2 = t1->VirtualType ()->BaseType ()->VirtualType ();
      sinfo = t2->TypeRecord ()->Record ();
    }
  // op1 . op2
  } else {
    if (! (t1->isRecord () && t1->isComplete (pos))) { 
      SEM_ERROR__invalid_op (node, "binary", ".");
    } else {
      t2 = t1->VirtualType ();
      sinfo = t2->TypeRecord ()->Record ();
    }
  }

  if (sinfo) {
    sinfo = sinfo->DefObject ()->Record ();
    member = (CT_SimpleName*)node->Son (2);
    name = member->token ()->text ();
    info = sinfo->Attribute (name);
    if (! info) {
      SEM_ERROR (node, (sinfo->ClassInfo () ? "`struct ": "`union ") << 
        sinfo->Name () << "' has no member named `" << name << "'");
      node->setTypeRef (&CTYPE_UNDEFINED);
    } else {
      t2 = info->TypeInfo ();
      if (t1->TypeQualified () && 
          (t1->isConst () != t2->isConst () ||
           t1->isVolatile () != t2->isVolatile () ||
           t1->isRestrict () != t2->isRestrict ()))
        node->setType (new CTypeQualified (t2->VirtualType ()->Duplicate (), 
          t1->isConst () || t2->isConst (),
          t1->isVolatile () || t2->isVolatile (),
          t1->isRestrict () || t2->isRestrict ()));
      else 
        node->setTypeRef (t2);
      
      member->setTypeRef (t2);
      member->Object (info);
      node->Object (info);
      if (t2->isConst ()) {
        CTree *init = info->Init ();
        if (init && init->Value ())
          member->setValueRef (init->Value ());
      }
    }
  } else
    node->setTypeRef (&CTYPE_UNDEFINED);
    
  return node->Type ();
}



///////////////////////////////////////////////////////
// call expression ////////////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_CallExpr *node, CTree *base) {
  CTypeInfo *result, *t1, *ta, *tp;
  int pnum, anum, ctp_old;
  CTypeFunction *ftype;
  CFunctionInfo *finfo;
  CTypeList *ptypes;
  CT_ExprList *args;
  CConstant *v;
  CTree *arg;
  bool skip;
  
  result = &CTYPE_UNDEFINED;
  args = node->Arguments ();
  anum = args->Entries ();

  ctp_old = conv_to_ptr;
  conv_to_ptr = 1;
  
  // resolve operands
  t1 = resolveExpr (node->Expr (), node); 
  // resolve arguments
  for (int i = 0; i < anum; i++)
    resolveExpr (args->Entry (i), args);
  conv_to_ptr = ctp_old;
  
  // function call: op1 ( op2 , .. )
  if (t1->isPointer () && t1->VirtualType ()->BaseType ()->isFunction ()) {
    ftype = t1->VirtualType ()->BaseType ()->VirtualType ()->TypeFunction ();
    result = ftype->ReturnType ();
    ptypes = ftype->ArgTypes ();
    finfo = ftype->FunctionInfo ();
    skip = false;

    if (finfo && ptypes) { // should always be true ;-)
      node->Object (finfo);
      pnum = ptypes->Entries ();
      // compare number of arguments and number of parameters
      if (ftype->hasPrototype ()) {
        if (pnum) {
          if (ptypes->Entry (0)->isVoid ()) {
            if (anum) 
              SEM_ERROR__num_args_too (node, "many", anum, finfo);
          } else if (anum < (pnum - (ptypes->Entry (pnum-1)->is_ellipsis () ? 1 : 0)))
            SEM_ERROR__num_args_too (node, "few", anum, finfo);
        } else
          skip = true;
      } else
        skip = true;
      
      // analyse argument types
      for (int i = 0; i < anum; i++) {
        arg = args->Entry (i);
        ta = arg->Type ();
        
        // apply the default argument promotions
        if (skip || ! ftype->hasPrototype ()) {
          ta = promote (arg, args);
          if (ta->VirtualType ()->is_float ())
            ta = cast_to (&CTYPE_DOUBLE, arg, args);
        } 

        // validate argument type
        if (! skip) {
          // argument excess
          if (pnum < i+1) { 
            SEM_ERROR__num_args_too (node, "many", (i+1), finfo);
            skip = true;
            continue;
          }
          tp = ptypes->Entry (i);
          // variable number of arguments 
          if (tp->is_ellipsis ()) { 
            skip = true; // types of trailing arguments are not checked
            continue;
          }
    
          // convert argument type to the unqualified parameter type
          v = arg->Value () ? arg->Value ()->Constant () : 0;
          if (tp->isScalar ()) {
            if (! compatible (tp, ta, v))
              SEM_WARNING__incompatible_arg (arg, i+1, finfo);
            if (*tp != *ta)
              cast_to (tp->VirtualType (), arg, args);
          } else if (tp->isRecord ()) {
            if (! compatible (tp, ta, v))
              SEM_ERROR__invalid_arg (arg, i+1, finfo);
          } else
            SEM_ERROR__invalid_arg (arg, i+1, finfo);
        }
      }
    }
  } else if (t1->is_undefined ()) {
    // object not resolved, so it is an implicitely
    // declared function; the return type defaults
    // to int
    result = &CTYPE_INT;
  } else {
    // object could be resolved but is not a function,
    SEM_ERROR (node, "called object is not a function");
  }

  node->setTypeRef (result);
  return result;
}



///////////////////////////////////////////////////////
// compound literal ///////////////////////////////////
///////////////////////////////////////////////////////



CTypeInfo *CSemExpr::resolve (CT_CmpdLiteral *node, CTree *base) {
  bool const_init, ctp_old;
  CObjectInfo *info;
  CExprValue *value;
  CTypeInfo *result;
  
  ctp_old = conv_to_ptr;
  conv_to_ptr = 1;
  
  result = &CTYPE_UNDEFINED;
  info = node->TypeName ()->Object ();
  if (info) {
    result = info->TypeInfo ();
    const_init = (! info->Scope ()->insideFunction ());
    resolveInit (info, node->Initializer (), node, const_init);
    
    // handle constants
    value = node->Initializer ()->Value ();
    if (value)
      node->setValueRef (value);
  }
  node->setTypeRef (result);
 
  conv_to_ptr = ctp_old;

  return result;
}


///////////////////////////////////////////////////////
// initializer evaluation /////////////////////////////
///////////////////////////////////////////////////////


void CSemExpr::resolveInit (CObjectInfo *info, CTree *node, CTree *base, 
 bool is_const) {
  Stack<SubObj> objs;
  objs.push (SubObj (info, info->TypeInfo (), 0));
  resolveInit (info, node, base, is_const, objs);
}

void CSemExpr::resolveInit (CObjectInfo *info, CTree *node, CTree *base, 
 bool is_const, Stack<SubObj> &objs) {
  bool braced, designations;
  CObjectInfo *curr, *obj;
  CT_DesignatorSeq *ds;
  CStructure *sinfo;
  unsigned long pos;
  const char *name;
  CT_ExprList *el;
  CTypeInfo *type;
  CTree *item, *d;
  int items, size;
  CConstant *v;

  if (objs.length () == 0)
    return;

  // validate type of object to be initialized
  curr = obj = info;
  type = objs.top ().type;
  pos = node->token_node ()->Number ();
  if (type->isVarArray () || (! type->isObject (pos) &&
      ! (type->isArray () && ! type->isComplete () && 
         type->VirtualType ()->BaseType ()->isObject (pos)))) {
    if (type->isFunction ()) {
      SEM_WARNING__init (node, info, "function-type");
    } else if (! type->isComplete (pos)) {
      SEM_WARNING__init (node, info, "incomplete-type");
    } else {
      SEM_WARNING__init (node, info, "variable-sized");
    }
  }

  // init_clause: [=] { init_list }
  if (node->NodeName () == CT_ExprList::NodeId () &&
      node->end_token ()->type () == TOK_CLOSE_CURLY) { 
    el = (CT_ExprList*)node;
    el->setTypeRef (type);
    designations = false;

    // init_list: init_list_item [, init_list_item]*
    items = el->Entries ();
    for (int i = 0; i < items; i++) { 
      item = el->Entry (i);
      base = el;

      // init_list_item: designation init_clause
      if (item->NodeName () == CT_BinaryExpr::NodeId () && 
          item->Son (0)->NodeName () == CT_DesignatorSeq::NodeId ()) {
        designations = true;
        ds = (CT_DesignatorSeq*)item->Son (0);
        base = item;
        item = item->Son (2); // init_clause
        braced = (item->NodeName () == CT_ExprList::NodeId ());

        curr = obj = info;
        while (objs.top ().info != info && objs.length () > 0) 
          objs.pop ();
	if (objs.top ().info != info) {
	  SEM_WARNING(node, "designators not analyzed");
	  return;
	}
        type = objs.top ().type;

        // designation: [designator]+ =
        for (int j = 0; j < ds->Sons (); j++) {
          d = ds->Son (j);
          if (obj != curr)
            curr = obj;

          // designator: . identifier
          if (d->Sons () == 2) { 
            name = d->Son (1)->token ()->text ();
            if (! type->isRecord ()) {
              SEM_WARNING (d, "`" << curr->Name () 
                << "' is not a struct or union type object");
              SEM_MSG (d, "and cannot be used to access a member named `" 
                << name << "'");
              break;
            }  
            sinfo = type->VirtualType ()->TypeRecord ()->Record ();
            sinfo = sinfo->DefObject ()->Record ();
            obj = sinfo->Attribute (name);
            if (! obj) {
              SEM_WARNING (d, "`" << name << "' is not a member of `" 
                << (sinfo->ClassInfo () ? "struct " : "union ")
                << sinfo->Name () << "'");
              break;
            }
            type = obj->TypeInfo ();
            objs.push (SubObj (obj, type, 0));
            ((CT_MembDesignator*)d)->setTypeRef (type);
            ((CT_SimpleName*)d->Son (1))->setTypeRef (type);
          // designator: [ const_expr ]
          } else {
            resolveExpr (d->Son (1), d); // compute element index
            if (! type->isArray ()) {
              SEM_WARNING (d, "`" << curr->Name () << "' is not an array type object");
              SEM_MSG (d, "and cannot be initialized like array type objects");
              break;
            }  
            if (! isConstIntExpr (d)) {
              SEM_WARNING (d, "element index is not a valid integer constant expression");
              break;
            }
            v = d->Son (1)->Value ()->Constant ();
            if (v->isNegative ()) {
              SEM_WARNING (d, "negative element index");
              break;
            }
            size = v->convert_to_uint () + 1;
            if (objs.top ().size < size)
              objs.top ().size = size;
            if (type->isFixedArray () && type->Dimension () < size) {
              SEM_WARNING (d, "element index out of range for fixed-size array `"
                << curr->Name () << "'");
              break;
            } else if (! type->isFixedArray () && type->Dimension () < size) {
              type->VirtualType ()->TypeArray ()->Dimension (size);
            }
            pushSubObj (type, obj, objs);
            ((CT_IndexDesignator*)d)->setTypeRef (type);
          }
        }
        ds->setTypeRef (ds->Son (ds->Sons () - 1)->Type ());
        base->SemValue ()->setTypeRef (ds->Type ());
      // init_list_item: init_clause
      } else {
        braced = (item->NodeName () == CT_ExprList::NodeId ());
        if (! designations && braced) {
          while (objs.top ().info != info)
            objs.pop ();
          type = objs.top ().type;
          curr = obj = info;
        }
      }

      // initialize first member
      if (i == 0)
        findFirst (item, type, curr, obj, info, objs, braced);
      // initialize next member
      else 
        findNext (item, type, curr, obj, info, objs, braced);

      // initialize subobject
      resolveInit (obj, item, base, is_const, objs);
    }

    if (el->Type ()->isScalar () && el->Entry (0)->Value ())
      el->setValueRef (el->Entry (0)->Value ());
  // = ass_expr
  } else if (node->NodeName () == CT_ExprList::NodeId ()) {
    singleExprInit (node->Son (1), node, type, is_const);
    node->SemValue ()->setTypeRef (type);
    if (node->Son (1)->Value ())
      node->SemValue ()->setValueRef (node->Son (1)->Value ());
  // init_clause: ass_expr
  } else
    singleExprInit (node, base, type, is_const);
}

void CSemExpr::findFirst (CTree *item, CTypeInfo *&type, CObjectInfo *&curr, 
 CObjectInfo *&obj, CObjectInfo *info, Stack<SubObj> &objs, bool braced) const {
  CStructure *sinfo;
  int pos, size;
  unsigned num;

  if (objs.length () == 0)
    return;

  if (type->isScalar ()) {
    if (braced) {
      size = objs.top ().size;
      //SEM_WARNING (item, "too many braces around scalar initializer");
    } else
      size = ++objs.top ().size;
    //if (size > 1)
    //  SEM_WARNING (item, "too many elements in scalar initializer");
    return;
  } else if (type->isArray ()) {
    if ((type->VirtualType ()->BaseType ()->VirtualType ()->is_char () && 
         (braced?item->Son (1):item)->NodeName () == CT_String::NodeId ()) ||
        (type->VirtualType ()->BaseType ()->VirtualType ()->is_wchar_t () &&
         (braced?item->Son (1):item)->NodeName () == CT_WideString::NodeId ())) {
      //if (braced)
      //  SEM_WARNING (item, "too many braces around initializer-string");
      return;
    } else {
      size = ++objs.top ().size;
      if (type->isFixedArray () && type->Dimension () < size) {
        //SEM_WARNING (item, "too many elements in fixed-size array initializer");
      } else if (! type->isFixedArray () && type->Dimension () < size)
        type->VirtualType ()->TypeArray ()->Dimension (size);
      pushSubObj (type, obj, objs);
    }
    if (braced)
      return;
  } else if (type->isRecord ()) {
    // unnamed union members are not initialized
    if (type->isUnion ()) {
      pos = -1;
      sinfo = type->VirtualType ()->TypeUnion ()->UnionInfo ()->DefObject ();
      num = sinfo->Attributes ();
      for (unsigned i = 0; i < num; i++)
        if (! sinfo->Attribute (i)->isAnonymous ()) {
          pos = i;
          break;
        }
    } else {
      pos = 0;
      sinfo = type->VirtualType ()->TypeClass ()->ClassInfo ()->DefObject ();
    }
    if (type->isUnion () ? pos >= 0 : sinfo->Attributes ()) {
      curr = obj;
      obj = sinfo->Attribute ((unsigned)pos);
      type = obj->TypeInfo ();
      objs.push (SubObj (obj, type, 0));
      if (braced)
        return;
    } else { // curr has no members (?)
      if (*type->VirtualType () == *info->TypeInfo ()->VirtualType ()) {
        //SEM_WARNING (item, "too many elements in record initializer");
      } else {
        popSubObj (type, curr, obj, objs);
        findNext (item, type, curr, obj, info, objs, braced);
      }
      return;
    } 
  } else 
    return;

  findFirst (item, type, curr, obj, info, objs, braced);
}

void CSemExpr::findNext (CTree *item, CTypeInfo *&otype, CObjectInfo *&curr, 
 CObjectInfo *&obj, CObjectInfo *info, Stack<SubObj> &objs, bool braced) const {
  CObjectInfo *tmp = 0;
  CStructure *sinfo;
  CTypeInfo *type;
  int size;

  if (objs.length () == 0)
    return;

  type = objs.top ().type;
  if (type->isScalar ()) {
    //if (braced)
    //  SEM_WARNING (item, "too many braces around scalar initializer");
    // TODO: the following line produces masses of warnings even if the code
    //       is correct -> needs to be fixed
    // SEM_WARNING (item, "excess elements in scalar initializer");
    objs.pop ();
  } else if (type->isArray ()) {
    /*if (braced &&
        ((type->VirtualType ()->BaseType ()->VirtualType ()->is_char () &&
          item->Son (1)->NodeName () == CT_String::NodeId ()) ||
         (type->VirtualType ()->BaseType ()->VirtualType ()->is_wchar_t () &&
          item->Son (1)->NodeName () == CT_WideString::NodeId ())))
      SEM_WARNING (item, "too many braces around initializer-string");*/
    size = objs.top ().size + 1;
    if (type->isFixedArray () && type->Dimension () < size) {
      if (*type->VirtualType () == *info->TypeInfo ()->VirtualType ()) {
        //SEM_WARNING (item, "too many elements in fixed-size array initializer");
        return;
      } else
        popSubObj (otype, curr, obj, objs);
    } else { 
      objs.top ().size = size;
      if (! type->isFixedArray () && type->Dimension () < size)
        type->VirtualType ()->TypeArray ()->Dimension (size);
      pushSubObj (type, obj, objs);
      otype = objs.top ().type;
      if (! braced)
        findFirst (item, otype, curr, obj, info, objs, braced);
      return;
    }
  } else if (type->isRecord ()) {
    // only the first member of a union is initialized
    if (! type->isUnion ()) {
      sinfo = type->VirtualType ()->TypeRecord ()->Record ();
      sinfo = sinfo->DefObject ()->Record ();
      tmp = nextMember (/*obj->Scope ()->Structure ()*/sinfo, obj);
    }
    if (tmp) {
      obj = tmp;
      otype = obj->TypeInfo ();
      objs.push (SubObj (obj, otype, 0));
      if (! braced)
        findFirst (item, otype, curr, obj, info, objs, braced);
      return;
    } else { // all members of curr initialized
      if (*type->VirtualType () == *info->TypeInfo ()->VirtualType ()) {
        //SEM_WARNING (item, "too many elements in record initializer");
        return;
      } else
        popSubObj (otype, curr, obj, objs);
    }
  } else
    return;

  findNext (item, otype, curr, obj, info, objs, braced);
}

void CSemExpr::singleExprInit (CTree *node, CTree *base, CTypeInfo *type, bool is_const) {
  CTypeInfo *t2;
  CConstant *v;

  // do not convert a (wide) string constant to pointer to char/wchar_t
  if (type->isArray ())
    conv_to_ptr = 2;
  else
    conv_to_ptr = 1;

  unsigned idx = (base->Sons () > 0) ? base->Sons () - 1 : 0;
  for (; idx > 0 && base->Son (idx) != node; idx--);

  t2 = resolveExpr (node, base);
  conv_to_ptr = 1;

  // get node, could have been replaced due to implicit conversions
  node = base->Son (idx);

  //if (is_const && ! isConstExpr (node)) 
  //  SEM_WARNING (node, "initializer element is not constant");

  v = node->Value () ? node->Value ()->Constant () : 0;
  if (type->isScalar ()) {
    if (type->isArithmetic () && t2->isArithmetic ()) {
      if (*type != *t2)
        cast_to (type->VirtualType (), node, base);
    } else if (! compatible (type, t2, v)) {
      //SEM_WARNING (node, "invalid initializer");
    } else if (*type != *t2)
      cast_to (type->VirtualType (), node, base);
  } else if (type->isArray ()) {
    t2 = t2->VirtualType ();
    if (t2->isArray () && t2->Dimension () &&
        ((t2->BaseType ()->VirtualType ()->is_char () && 
          node->NodeName () == CT_String::NodeId ()) ||
         (t2->BaseType ()->VirtualType ()->is_wchar_t () && 
          node->NodeName () == CT_WideString::NodeId ()))) {
      if (type->isFixedArray ()) { 
        //if (type->Dimension () < t2->Dimension ()) 
        //  SEM_WARNING (node, "initializer-string for array is too long");
      } else
        type->VirtualType ()->TypeArray ()->Dimension (t2->Dimension ());
    } //else 
      //SEM_WARNING (node, "invalid initializer");
  } else if (type->isRecord ()) {
    //if (! compatible (type, t2, v))
      //SEM_WARNING (node, "invalid initializer");
  } //else
    //SEM_WARNING (node, "invalid initializer");
}

void CSemExpr::popSubObj (CTypeInfo *&type, CObjectInfo *&curr, CObjectInfo *&obj, Stack<SubObj> &objs) const {
  obj = objs.top ().info;
  type = objs.top ().type;
  objs.pop ();
  curr = objs.top ().info;
}

void CSemExpr::pushSubObj (CTypeInfo *&type, CObjectInfo *&obj, Stack<SubObj> &objs) const {
  type = type->BaseType ();
  if (type->isRecord ())
    obj = type->VirtualType ()->TypeRecord ()->Record ()->DefObject ()->Record ();
  objs.push (SubObj (obj, type, 0));
}

CObjectInfo *CSemExpr::nextMember (CStructure *info, CObjectInfo *curr) const {
  unsigned members = info->Attributes ();
  for (unsigned i = 0; i < members; i++)
    if (info->Attribute (i) == curr) {
      if (i+1 < members)
        return info->Attribute (i+1);
      break;
    }
  return (CObjectInfo*)0;
}



///////////////////////////////////////////////////////
// constant expression ////////////////////////////////
///////////////////////////////////////////////////////



// integer constant expression
bool CSemExpr::isConstIntExpr (CTree *node) { 
  if (isConstExpr (node, true))
    if (node->Type () && node->Type ()->isInteger () && 
        node->Value () && node->Value ()->Constant ())
      return true;
  return false;
}

bool CSemExpr::isConstExpr (CTree *node, bool integer) {
  const char *id = node->NodeName ();
  int oper;
  
  if (id == CT_BinaryExpr::NodeId ()) {
    oper = node->Son (1)->token ()->type ();
    if (oper == TOK_ASSIGN &&
        node->Son (0)->NodeName () == CT_DesignatorSeq::NodeId ())
      return isConstExpr (node->Son (2), integer);
    if (oper == TOK_ASSIGN || oper == TOK_MUL_EQ || oper == TOK_DIV_EQ ||
        oper == TOK_MOD_EQ || oper == TOK_ADD_EQ || oper == TOK_SUB_EQ ||
        oper == TOK_RSH_EQ || oper == TOK_LSH_EQ || oper == TOK_AND_EQ ||
        oper == TOK_XOR_EQ || oper == TOK_IOR_EQ || oper == TOK_COMMA)
      return false; 
    return isConstExpr (node->Son (0), integer) && 
           isConstExpr (node->Son (2), integer);
  } else if (id == CT_PostfixExpr::NodeId ()) {
    oper = node->Son (1)->token ()->type ();
    if (oper == TOK_DECR || oper == TOK_INCR)
      return false;
    return isConstExpr (node->Son (0), integer);
  } else if (id == CT_UnaryExpr::NodeId ()) {
    oper = node->Son (0)->token ()->type ();
    if (oper == TOK_DECR || oper == TOK_INCR)
      return false;
    return isConstExpr (node->Son (1), integer);
  } else if (id == CT_QualName::NodeId () || 
             id == CT_RootQualName::NodeId ()) {
    return isConstExpr (((CT_QualName*)node)->Name (), integer);
  } else if (id == CT_BracedExpr::NodeId ()) {
    return isConstExpr (node->Son (1), integer);
  } else if (id == CT_ExprList::NodeId ()) {
    int n = ((CT_ExprList*)node)->Entries ();
    for (int i = 0; i < n; i++) { 
      CTree* expr = ((CT_ExprList*)node)->Entry (i);
      if (! isConstExpr (expr, integer))
        return false;
    }
  } else if (id == CT_IfThenExpr::NodeId ()) {
    if (isConstIntExpr (node->Son (0)))
        return (node->Son (0)->Value ()->Constant ()->isNull () ?
           isConstExpr (node->Son (4), integer) :
           isConstExpr (node->Son (2), integer));
    return false;
  } else if (id == CT_ImplicitCast::NodeId ()) {
    return isConstExpr (node->Son (0), integer);
  } else if (id == CT_CastExpr::NodeId ()) {
    CT_CastExpr *cast = (CT_CastExpr*)node;
    return isConstExpr (cast->Expr (), integer);
  } else if (id == CT_CmpdLiteral::NodeId ()) {
    CT_CmpdLiteral *cmpd_literal = (CT_CmpdLiteral*)node;
    return isConstExpr (cmpd_literal->Initializer (), integer);
  } else if (id == CT_AddrExpr::NodeId ()) {
    CT_AddrExpr *ae = (CT_AddrExpr*)node;
    CTypeInfo *type = ae->Type ();
    if (integer || ! type->isPointerOrArray ())
      return false;
    if (type->isPointer () && type->VirtualType ()->BaseType ()->isFunction ())
      return true;
    if (isConstantMembPtr (ae->Expr ()))
      return true;
    CObjectInfo *info = findObject (ae->Expr (), true);
    if (! info || (info->Storage () != CStorage::CLASS_STATIC &&
                   info->Storage () != CStorage::CLASS_THREAD))
      return false;
    return info->TypeInfo ()->isArray () || isLvalue (ae->Expr ());
  } else if (id == CT_SimpleName::NodeId ()) {
    CT_SimpleName *name = (CT_SimpleName*)node;
    CTypeInfo *type = node->Type ();
    CObjectInfo *obj = name->Object ();
    return ((obj && (obj->EnumeratorInfo () || obj->LabelInfo ())) ||
            (! integer && (type->PtrToFct () || type->PtrToArray () ||
                           type->isFunction () || type->isArray ())));
  } else if (id == CT_SizeofExpr::NodeId () ||
             id == CT_AlignofExpr::NodeId () ||
             id == CT_OffsetofExpr::NodeId ()) {
    return true;
  } else if (id == CT_Bool::NodeId () ||
             id == CT_Character::NodeId () ||
             id == CT_WideCharacter::NodeId () ||
             id == CT_Integer::NodeId () ||
             id == CT_Float::NodeId ()) {
    return true;
  } else if (id == CT_String::NodeId () ||
             id == CT_WideString::NodeId ()) {
    return ! integer;
  } else if (id == CT_CallExpr::NodeId ()) {
    return node->Value(); // implicit constant expression
  } else if (id == CT_MembPtrExpr::NodeId () ||
             id == CT_MembRefExpr::NodeId ()) {
    return isConstExpr (node->Son (0), integer);
  } else
    return false;
  return true;
}

bool CSemExpr::isConstantMembPtr (CTree *node) {
  const char *id = node->NodeName ();
  if (id == CT_BracedExpr::NodeId () ||
      id == CT_DerefExpr::NodeId () ||
      id == CT_AddrExpr::NodeId ()) {
    return isConstantMembPtr (node->Son (1));
  } else if (id == CT_ImplicitCast::NodeId ()) {
    return isConstantMembPtr (node->Son (0));
  } else if (id == CT_CastExpr::NodeId ()) {
    return isConstantMembPtr (node->Son (3));
  } else if (id == CT_BinaryExpr::NodeId ()) {
    int oper = node->Son (1)->token ()->type ();
    if (oper == TOK_COMMA)
      return isConstantMembPtr (node->Son (2));
  } else if (id == CT_MembPtrExpr::NodeId () ||
             id == CT_MembRefExpr::NodeId ()) {
    if (node->Son(0)->Value())
      return true;
    return isConstantMembPtr (node->Son (0));
  }
  return false;
}



///////////////////////////////////////////////////////
// helper /////////////////////////////////////////////
///////////////////////////////////////////////////////



bool CSemExpr::compatible (CTypeInfo *t1, CTypeInfo *t2, CConstant* value) {
  if (! t1 || ! t2)
    return false;

  t1 = t1->VirtualType ();
  t2 = t2->VirtualType ();

  if (t1->isArithmetic () && t2->isArithmetic ()) {
    // arithmetic types are compatible
    return true;
  }
  if (t1->is_bool () && t2->isScalar ()) {
    // boolean and scalar types are compatible
    return true;
  }
  if ((t1->isPointer () && t2->isInteger ()) ||
      (t1->isInteger () && t2->isPointer ())) {
    // pointer and integer types are compatible
    return true;
  } 
  if (t1->isPointer () && t2->isPointer ()) {
    // null pointer constant is compatible to any pointer
    if (value && value->isNull ()) {
      return true;
    } 
    // pointer to void is compatible to pointer to char type
    if ((t1->BaseType ()->isVoid () && t2->BaseType ()->VirtualType ()->is_char ()) ||
        (t2->BaseType ()->isVoid () && t1->BaseType ()->VirtualType ()->is_char ())) {
      return true;
    }
    // other pointer types are compatible to pointer to void
    if (t1->BaseType ()->isVoid () || t2->BaseType ()->isVoid ()) {
      return true;
    }
    // base types and qualifier have to be compatible
    return compatibleBase (t1, t2) &&
      (t2->isConst () ? t1->isConst () : true) &&
      (t2->isVolatile () ? t1->isVolatile () : true) &&
      (t2->isRestrict () ? t1->isRestrict () : true);
  } 
  if (t1->isFunction () && t2->isFunction ()) {
    // return type and parameter list have to be compatible
    // NO FUNCTION PARAMETER PROMOTION AT THE MOMENT
    unsigned nargs1 = t1->TypeFunction ()->ArgTypes ()->Entries ();
    unsigned nargs2 = t2->TypeFunction ()->ArgTypes ()->Entries ();
    return compatibleBase (t1, t2) && (nargs1 == 0 || nargs2 == 0 || nargs1 == nargs2);
  }
  if (*t1 == *t2) {
    // types are equal
    return true; 
  }

  return false;
}


CTypeInfo* CSemExpr::applyImplicitConv (CTree* expr, CTree* base, CTypeInfo* type) {
  // conv_to_ptr is 0 if argument of address or sizeof expression
  // conv_to_ptr is 2 if initializer of array
  // conv_to_ptr is 3 if left operand of assignment, ++, --, or . operator
  // conv_to_ptr is 1 otherwise
  const char *id = expr->NodeName ();

  // implicit array and function type conversions
  if (type->isArray () || type->isFunction ()) {
    if (conv_to_ptr > 0 && ! (conv_to_ptr == 2 && 
        (id == CT_String::NodeId () || id == CT_WideString::NodeId ()))) {
      type = type->isArray () ? type->VirtualType ()->BaseType () : type;
      type = new CTypePointer (type->Duplicate ());
      cast_to (type, expr, base, false);
      conv_to_ptr = 1;
    }
  // implicit lvalue to rvalue conversions
  } else if (conv_to_ptr != 3) {
    if (type->isQualified ()) {
      type = type->UnqualType ()->Duplicate ();
      cast_to (type, expr, base, false);
    }
    conv_to_ptr = 1;
  } else 
    conv_to_ptr = 1;
  
  return type;
}



void CSemExpr::passOnConstant (CTree *node, CTree *base, CTypeInfo *t1) {
  CTypeInfo *t2;
  CConstant *value;
  
  t2 = node->Type ();
  value = node->Value () ? node->Value ()->Constant () : 0;
  if (value && base->SemValue ()) {
    if (t1 && t2 && *t1 != *t2) {
      value = value->cast_to (t1);
      base->SemValue ()->setValue (value);
    } else
      base->SemValue ()->setValueRef (value);
  }
}



CTypeInfo *CSemExpr::cast_to (CTypeInfo *type, CTree *node, CTree *base, bool ref) {
  CT_ImplicitCast *cast;
  CExprValue *value;

  // cast implicitly if types differ  
  if (*node->Type () != *type) {
    cast = new CT_ImplicitCast (node);
    if (ref)
      cast->setTypeRef (type);    // refer to a shared type
    else
      cast->setType (type);       // type is new (to be deleted by this node)
    base->ReplaceSon (node, cast);
    
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
  return type;    
}

bool CSemExpr::isLvalue (CTree *node) {
  CObjectInfo *info;
  CTypeInfo *type;
  const char *id;
  
  id = node->NodeName ();
  type = node->Type ();
  if (type->isUndefined () || type->isVoid () || type->isFunction () || type->isArray ())
    return false;
  if (type->isPointer () && type->VirtualType ()->BaseType ()->isArray ()) {
    CTree *tn = node;
    while (true) {
      if (tn->NodeName () == CT_BracedExpr::NodeId ())
        tn = tn->Son (1);
      else if (tn->NodeName () == CT_ImplicitCast::NodeId ())
        return false;
      else
        break;
    }
  }

  if (id == CT_SimpleName::NodeId ()) {
    info = ((CT_SimpleName*)node)->Object ();
    if (info) {
      if (info->ArgumentInfo ()) 
        return true;
      if (info->EnumeratorInfo ())
        return false;
      type = info->TypeInfo ();
      return type && ! type->isFunction ();
    }
  } else if (id == CT_QualName::NodeId () || id == CT_RootQualName::NodeId ()) {
    return isLvalue (((CT_QualName*)node)->Name ());
  } else if (id == CT_BracedExpr::NodeId ()) {
    return isLvalue (node->Son (1));
  } else if (id == CT_MembRefExpr::NodeId ()) {
    return isLvalue (node->Son (0));
  } else if (id == CT_BinaryExpr::NodeId ()) {
    int oper = node->Son (1)->token ()->type ();
    if (oper == TOK_ASSIGN || oper == TOK_MUL_EQ || oper == TOK_DIV_EQ ||
        oper == TOK_MOD_EQ || oper == TOK_ADD_EQ || oper == TOK_SUB_EQ ||
        oper == TOK_RSH_EQ || oper == TOK_LSH_EQ || oper == TOK_AND_EQ ||
        oper == TOK_XOR_EQ || oper == TOK_IOR_EQ || oper == TOK_COMMA)
      return false;
  } else if (id == CT_DerefExpr::NodeId ()) {
    unsigned long pos = node->token_node ()->Number ();
    if (node->Type ()->isObject (pos))
      return true;
  } else if (id == CT_String::NodeId () || 
             id == CT_WideString::NodeId () ||
             id == CT_MembPtrExpr::NodeId () || 
             id == CT_CmpdLiteral::NodeId () ||
             id == CT_IndexExpr::NodeId ()) {
    return true;
//  } else if (id == CT_AddrExpr::NodeId () || 
//             id == CT_CastExpr::NodeId () ||
//             id == CT_IfThenExpr::NodeId () ||
//             id == CT_SizeofExpr::NodeId () ||
//             id == CT_AlignofExpr::NodeId () ||
//             id == CT_OffsetofExpr::NodeId () ||
//             id == CT_Integer::NodeId () ||
//             id == CT_Character::NodeId () ||
//             id == CT_WideCharacter::NodeId () ||
//             id == CT_Float::NodeId () ||
//             id == CT_Bool::NodeId () ||
//             id == CT_CallExpr::NodeId ()) {
//    return false;
  } else if (id == CT_ImplicitCast::NodeId ()) {
    return isLvalue (node->Son (0));
  } 
  return false;
}

CObjectInfo *CSemExpr::findObject (CTree *node, bool const_expr) {
  const char *id = node->NodeName ();
  if (id == CT_BracedExpr::NodeId () ||
      id == CT_DerefExpr::NodeId () ||
      id == CT_AddrExpr::NodeId () ||
      id == CT_UnaryExpr::NodeId ()) {
    return findObject (node->Son (1), const_expr);
  } else if (id == CT_ImplicitCast::NodeId () ||
             id == CT_PostfixExpr::NodeId () ||
             id == CT_IndexExpr::NodeId ()) {
    return findObject (node->Son (0), const_expr);
  } else if (id == CT_CastExpr::NodeId ()) {
    return findObject (node->Son (3), const_expr);
  } else if (id == CT_BinaryExpr::NodeId ()) {
    int oper = node->Son (1)->token ()->type ();
    if (oper == TOK_COMMA)
      return findObject (node->Son (2), const_expr);
    if (oper == TOK_ASSIGN || oper == TOK_MUL_EQ || oper == TOK_DIV_EQ ||
        oper == TOK_MOD_EQ || oper == TOK_ADD_EQ || oper == TOK_SUB_EQ ||
        oper == TOK_RSH_EQ || oper == TOK_LSH_EQ || oper == TOK_AND_EQ ||
        oper == TOK_XOR_EQ || oper == TOK_IOR_EQ)
      return findObject (node->Son (0), const_expr);
  } else if (id == CT_QualName::NodeId () ||
             id == CT_RootQualName::NodeId ()) {
    return findObject (((CT_QualName*)node)->Name (), const_expr);
  } else if (id == CT_SimpleName::NodeId () ||
             id == CT_PrivateName::NodeId ()) {
    return ((CT_SimpleName*)node)->Object ();
  } else if (id == CT_MembPtrExpr::NodeId ()) {
    return ((CT_MembPtrExpr*)node)->Object ();
  } else if (id == CT_MembRefExpr::NodeId ()) {
    if (const_expr)
      return findObject (node->Son (0), const_expr);
    return ((CT_MembPtrExpr*)node)->Object ();
  } else if (id == CT_CallExpr::NodeId ()) {
    return ((CT_CallExpr*)node)->Object ();
  }
  return (CObjectInfo*)0;
}

// apply integer promotions if needed
CTypeInfo *CSemExpr::promote (CTree *expr, CTree *base) {
  CT_ImplicitCast *cast;
  CExprValue *v;
  CTypeInfo *t;
  
  t = expr->Type ()->VirtualType ();
  if (! t->isInteger () || (t->conv_rank () >= CTYPE_INT.conv_rank ()))
    return t;
  
  // apply integer promotions
  if (! t->is_unsigned_int () && ! t->is_int ()) {
    t = &CTYPE_INT;
    v = expr->Value ();
    cast = new CT_ImplicitCast (expr);
    if (v && v->Constant ())
      cast->setValue (v->Constant ()->cast_to (t));
    cast->setTypeRef (t);
    base->ReplaceSon (expr, cast);
  }
  return t;
}

int CSemExpr::scan_character (CTree *node, LONG_LONG &v) const {
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
    ch = next_character (s, ucs);
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

LONG_LONG CSemExpr::next_character (const char *&s, bool &ucs) const {
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

void CSemExpr::overflow (CTree *node, int base, bool &uns) const {
  if (base == 10) 
    SEM_WARNING__is_unsigned (node);
  uns = true;
}


///////////////////////////////////////////////////////
// name lookup ////////////////////////////////////////
///////////////////////////////////////////////////////


CObjectInfo *CSemExpr::lookup (CT_SimpleName *id, SemObjType type,
                               bool nested) const {
  return lookup (id->Text (), current_scope, type, nested, 
                 id->Sons () ? ((CT_Token*)id->Son (0))->Number () : 0);
}


CObjectInfo *CSemExpr::lookup (const char *id, CStructure *scope, 
 SemObjType type, bool nested, unsigned long int pos) const {
  CObjectInfo *result = (CObjectInfo*)0;
  switch (type) {
    case CLASS:
    case UNION:
    case ENUM: 
    case TYPEDEF:
    case TAG:
      result = Type (id, scope, type, pos);
      break;
    case FCT: 
      result = Function (id, scope, pos);
      break;
    case ATTRIB: 
      result = Attribute (id, scope, pos);
      break;
    case ARG: 
      if (scope->FunctionInfo ())
        result = scope->FunctionInfo ()->Argument (id);
      if (result && ! known_here (result, pos))
        result = (CObjectInfo*)0;
      break;
    case NON_TAG:
      {
        CStructure::ObjectsByName::iterator objs = scope->ObjectInfoMap().find(id);
        if (objs != scope->ObjectInfoMap().end()) {
          CStructure::ObjectInfoList::reverse_iterator obj = objs->second.rbegin();
          for (; obj != objs->second.rend(); ++obj) {
            result = *obj;
            if (! result->Record () && ! result->EnumInfo () && known_here (result, pos))
              break;
            result = (CObjectInfo*)0;
          }
        }
      }
      break;
    case ANY:
      break;
  }

  if (nested && ! result && scope != scope->Parent ())
    return lookup (id, scope->Parent ()->Structure (), type, nested, pos);
  return result;
}

CObjectInfo *CSemExpr::Type (const char *id, CStructure *scope, 
 SemObjType type, unsigned long int pos) const {
  CObjectInfo *result;
  for (unsigned i = scope->Types (); i > 0; i--) {
    result = scope->Type (i-1);
    // correct type?
    if ((type == CLASS   && ! result->ClassInfo ())   ||
        (type == UNION   && ! result->UnionInfo ())   ||
        (type == ENUM    && ! result->EnumInfo ())    ||
        (type == TYPEDEF && ! result->TypedefInfo ()) ||
        (type == TAG     &&   result->TypedefInfo ()))
      continue;
    // correct name?
    if (strcmp (result->Name (), id) != 0)
      continue;
    // correct declaration?
    if (! known_here (result, pos))
      continue;
    return result;
  }
  return (CObjectInfo*)0;
}

CObjectInfo *CSemExpr::Attribute (const char *id, CStructure *scope, 
 unsigned long int pos) const {
  CObjectInfo *result;
  for (unsigned i = scope->Attributes (); i > 0; i--) {
    result = scope->Attribute (i-1);
    // correct name?
    if (strcmp (result->Name (), id) != 0)
      continue;
    // correct declaration?
    if (! known_here (result, pos))
      continue;
    return result;
  }
  return (CObjectInfo*)0;
}

CObjectInfo *CSemExpr::Function (const char *id, CStructure *scope, 
 unsigned long int pos) const {
  CObjectInfo *result;
  for (unsigned i = scope->Functions (); i > 0; i--) {
    result = scope->Function (i-1);
    // correct name?
    if (strcmp (result->Name (), id) != 0)
      continue;
    // correct declaration?
    if (! known_here (result, pos))
      continue;
    return result;
  }
  return (CObjectInfo*)0;
}

bool CSemExpr::known_here (CObjectInfo *info, unsigned long int pos) const {
  if (pos && 
      info->Tree () && 
      info->Tree ()->token_node () &&
      info->Tree ()->token_node ()->Number () > pos)
    return false;
  return true;
}



} // namespace Puma
