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

#include "Puma/CConstant.h"
#include "Puma/CTokens.h"

namespace Puma {


bool CConstant::operator== (const CConstant &other) const {
  return value_type == INT_VAL  ? (value.int_val == other.convert_to_int ()) :
         value_type == UINT_VAL ? (value.uint_val == other.convert_to_uint ()) :
                                  (value.float_val == other.convert_to_float ());
}


bool CConstant::operator!= (const CConstant &other) const {
  return ! (*this == other);
}


CConstant *CConstant::duplicate () const {
  if (value_type == INT_VAL) 
    return new CConstant (value.int_val, Type ());
  else if (value_type == UINT_VAL) 
    return new CConstant (value.uint_val, Type ());
  else
    return new CConstant (value.float_val, Type ());
}

CConstant *CConstant::compute (int oper) const {
  CConstant *result;
  if (value_type == INT_VAL) 
    result = new CConstant (cast_to_real_type (
      compute (oper, value.int_val), Type ()), Type ());
  else if (value_type == UINT_VAL) 
    result = new CConstant (cast_to_real_type (
      compute (oper, value.uint_val), Type ()), Type ());
  else
    result = new CConstant (cast_to_real_type (
      compute (oper, value.float_val), Type ()), Type ());
  return result;
}

CConstant *CConstant::compute (int oper, const CConstant *v) const {
  CConstant *result;
  CTypeInfo *type = *Type () > *v->Type () ? Type () : v->Type ();
  if (type->is_signed ())
    result = new CConstant (cast_to_real_type (
      compute (convert_to_int (), oper, v->convert_to_int ()), type), type);
  else if (type->is_unsigned ())
    result = new CConstant (cast_to_real_type (
      compute (convert_to_uint (), oper, v->convert_to_uint ()), type), type);
  else
    result = new CConstant (cast_to_real_type (
      compute (convert_to_float (), oper, v->convert_to_float ()), type), type);
  return result;
}

CConstant *CConstant::cast_to (CTypeInfo *type) const {
  CConstant *result;
  if (value_type == INT_VAL) 
    result = new CConstant (cast_to_real_type (value.int_val, type), type);
  else if (value_type == UINT_VAL) 
    result = new CConstant (cast_to_real_type (value.uint_val, type), type);
  else
    result = new CConstant (cast_to_real_type (value.float_val, type), type);
  return result;
}

LONG_LONG CConstant::convert_to_int () const {
  if (value_type == INT_VAL) 
    return value.int_val;
  else if (value_type == UINT_VAL) 
    return (LONG_LONG) value.uint_val;
  else 
    return (LONG_LONG) value.float_val;
}

U_LONG_LONG CConstant::convert_to_uint () const {
  if (value_type == INT_VAL) 
    return (U_LONG_LONG) value.int_val;
  else if (value_type == UINT_VAL) 
    return value.uint_val;
  else 
    return (U_LONG_LONG) value.float_val;
}

long double CConstant::convert_to_float () const {
  if (value_type == INT_VAL) 
    return (long double) value.int_val;
  else if (value_type == UINT_VAL) 
    return (long double) value.uint_val;
  else 
    return value.float_val;
}

LONG_LONG CConstant::compute (int oper, LONG_LONG v) const {
  switch (oper) {
    case TOK_NOT: return !v;
    case TOK_PLUS: return +v;
    case TOK_MINUS: return -v;
    case TOK_TILDE: return ~v;
    default: return v; /* error */
  }
}

U_LONG_LONG CConstant::compute (int oper, U_LONG_LONG v) const {
  switch (oper) {
    case TOK_NOT: return !v;
    case TOK_PLUS: return +v;
    case TOK_MINUS: return -(LONG_LONG)v;
    case TOK_TILDE: return ~v;
    default: return v; /* error */
  }
}

long double CConstant::compute (int oper, long double v) const {
  switch (oper) {
    case TOK_NOT: return !v;
    case TOK_PLUS: return +v;
    case TOK_MINUS: return -v;
    default: return v; /* error */
  }
}

LONG_LONG CConstant::compute (LONG_LONG v1, int oper, LONG_LONG v2) const {
  switch (oper) {
    case TOK_AND: return v1 & v2;
    case TOK_MUL: return v1 * v2;
    case TOK_DIV: return (v1 && v2) ? v1 / v2 : (LONG_LONG)0;
    case TOK_EQL: return v1 == v2;
    case TOK_NEQ: return v1 != v2;
    case TOK_LEQ: return v1 <= v2;
    case TOK_GEQ: return v1 >= v2;
    case TOK_LSH: return v1 << v2;
    case TOK_RSH: return v1 >> v2;
    case TOK_OR: return v1 | v2;
    case TOK_LESS: return v1 < v2;
    case TOK_PLUS: return v1 + v2;
    case TOK_ROOF: return v1 ^ v2;
    case TOK_MINUS: return v1 - v2;
    case TOK_OR_OR: return v1 || v2;
    case TOK_MODULO: return (v1 && v2) ? v1 % v2 : (LONG_LONG)0;
    case TOK_GREATER: return v1 > v2;
    case TOK_AND_AND: return v1 && v2;
    default: return v1; /* error */
  }
}

U_LONG_LONG CConstant::compute (U_LONG_LONG v1, int oper, U_LONG_LONG v2) const {
  switch (oper) {
    case TOK_AND: return v1 & v2;
    case TOK_MUL: return v1 * v2;
    case TOK_DIV: return (v1 && v2) ? v1 / v2 : (U_LONG_LONG)0;
    case TOK_EQL: return v1 == v2;
    case TOK_NEQ: return v1 != v2;
    case TOK_LEQ: return v1 <= v2;
    case TOK_GEQ: return v1 >= v2;
    case TOK_LSH: return v1 << v2;
    case TOK_RSH: return v1 >> v2;
    case TOK_OR: return v1 | v2;
    case TOK_LESS: return v1 < v2;
    case TOK_PLUS: return v1 + v2;
    case TOK_ROOF: return v1 ^ v2;
    case TOK_MINUS: return v1 - v2;
    case TOK_OR_OR: return v1 || v2;
    case TOK_MODULO: return (v1 && v2) ? v1 % v2 : (U_LONG_LONG)0;
    case TOK_GREATER: return v1 > v2;
    case TOK_AND_AND: return v1 && v2;
    default: return v1; /* error */
  }
}

long double CConstant::compute (long double v1, int oper, long double v2) const {
  switch (oper) {
    case TOK_MUL: return v1 * v2;
    case TOK_DIV: return (v1 && v2) ? v1 / v2 : (long double)0.0;
    case TOK_EQL: return v1 == v2;
    case TOK_NEQ: return v1 != v2;
    case TOK_LEQ: return v1 <= v2;
    case TOK_GEQ: return v1 >= v2;
    case TOK_LESS: return v1 < v2;
    case TOK_PLUS: return v1 + v2;
    case TOK_MINUS: return v1 - v2;
    case TOK_OR_OR: return v1 || v2;
    case TOK_GREATER: return v1 > v2;
    case TOK_AND_AND: return v1 && v2;
    default: return v1; /* error */
  }
}

LONG_LONG CConstant::cast_to_real_type (LONG_LONG v, CTypeInfo *t) const {
  CTypeInfo *type = t->VirtualType ();
  if (type->is_bool ()) return (LONG_LONG)((bool)v);
  else if (type->is_char ()) return (LONG_LONG)((char)v);
  else if (type->is_wchar_t ()) return (LONG_LONG)((wchar_t)v);
  else if (type->is_short ()) return (LONG_LONG)((short)v);
  else if (type->is_int ()) return (LONG_LONG)((int)v);
  else if (type->is_long ()) return (LONG_LONG)((long)v);
  else if (type->is_long_long ()) return (LONG_LONG)((long long)v);
  else if (type->is_signed_char ()) return (LONG_LONG)((signed char)v);
  else if (type->is_unsigned_char ()) return (LONG_LONG)((unsigned char)v);
  else if (type->is_unsigned_short ()) return (LONG_LONG)((unsigned short)v);
  else if (type->is_unsigned_int ()) return (LONG_LONG)((unsigned int)v);
  else if (type->is_unsigned_long ()) return (LONG_LONG)((unsigned long)v);
  else if (type->is_unsigned_long_long ()) return (LONG_LONG)((unsigned long long)v);
  else if (type->is_float ()) return (LONG_LONG)((float)v);
  else if (type->is_double ()) return (LONG_LONG)((double)v);
  else if (type->is_long_double ()) return (LONG_LONG)((long double)v);
  return v; 
}

U_LONG_LONG CConstant::cast_to_real_type (U_LONG_LONG v, CTypeInfo *t) const {
  CTypeInfo *type = t->VirtualType ();
  if (type->is_bool ()) return (U_LONG_LONG)((bool)v);
  else if (type->is_char ()) return (U_LONG_LONG)((char)v);
  else if (type->is_wchar_t ()) return (U_LONG_LONG)((wchar_t)v);
  else if (type->is_short ()) return (U_LONG_LONG)((short)v);
  else if (type->is_int ()) return (U_LONG_LONG)((int)v);
  else if (type->is_long ()) return (U_LONG_LONG)((long)v);
  else if (type->is_long_long ()) return (U_LONG_LONG)((long long)v);
  else if (type->is_signed_char ()) return (U_LONG_LONG)((signed char)v);
  else if (type->is_unsigned_char ()) return (U_LONG_LONG)((unsigned char)v);
  else if (type->is_unsigned_short ()) return (U_LONG_LONG)((unsigned short)v);
  else if (type->is_unsigned_int ()) return (U_LONG_LONG)((unsigned int)v);
  else if (type->is_unsigned_long ()) return (U_LONG_LONG)((unsigned long)v);
  else if (type->is_unsigned_long_long ()) return (U_LONG_LONG)((unsigned long long)v);
  else if (type->is_float ()) return (U_LONG_LONG)((float)v);
  else if (type->is_double ()) return (U_LONG_LONG)((double)v);
  else if (type->is_long_double ()) return (U_LONG_LONG)((long double)v);
  return v; 
}

long double CConstant::cast_to_real_type (long double v, CTypeInfo *t) const {
  CTypeInfo *type = t->VirtualType ();
  if (type->is_bool ()) return (long double)((bool)v);
  else if (type->is_char ()) return (long double)((char)v);
  else if (type->is_wchar_t ()) return (long double)((wchar_t)v);
  else if (type->is_short ()) return (long double)((short)v);
  else if (type->is_int ()) return (long double)((int)v);
  else if (type->is_long ()) return (long double)((long)v);
  else if (type->is_long_long ()) return (long double)((long long)v);
  else if (type->is_signed_char ()) return (long double)((signed char)v);
  else if (type->is_unsigned_char ()) return (long double)((unsigned char)v);
  else if (type->is_unsigned_short ()) return (long double)((unsigned short)v);
  else if (type->is_unsigned_int ()) return (long double)((unsigned int)v);
  else if (type->is_unsigned_long ()) return (long double)((unsigned long)v);
  else if (type->is_unsigned_long_long ()) return (long double)((unsigned long long)v);
  else if (type->is_float ()) return (long double)((float)v);
  else if (type->is_double ()) return (long double)((double)v);
  else if (type->is_long_double ()) return (long double)((long double)v);
  return v; 
}

bool CConstant::isNull () const {
  if ((value_type == INT_VAL && value.int_val == 0) ||
      (value_type == UINT_VAL && value.uint_val == 0) ||
      (value_type == FLOAT_VAL && value.float_val == 0.0))
    return true;
  return false;
}

bool CConstant::isPositive () const {
  if ((value_type == UINT_VAL) ||
      (value_type == INT_VAL && value.int_val >= 0) ||
      (value_type == FLOAT_VAL && value.float_val >= 0.0))
    return true;
  return false;
}

bool CConstant::isNegative () const {
  if ((value_type == INT_VAL && value.int_val < 0) ||
      (value_type == FLOAT_VAL && value.float_val < 0.0))
    return true;
  return false;
}

void CConstant::print (std::ostream &out) const {
  if (Type ()->VirtualType ()->is_bool ())
    out << (convert_to_int () ? "true" : "false");
  else if (isSigned ())
    out << convert_to_int ();
  else if (isUnsigned ())
    out << convert_to_uint ();
  else if (isFloat ())
    out << convert_to_float ();
}

} // namespace Puma
