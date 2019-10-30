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

#ifndef __CConstant_h__
#define __CConstant_h__

/** \file
 *  Semantic information for arithmetic constants. */

#include "Puma/Limits.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CExprValue.h"

namespace Puma {


/** \class CConstant CConstant.h Puma/CConstant.h
 *  Semantic information object for arithmetic constants. */
class CConstant : public CExprValue {
  union Value {
    LONG_LONG    int_val;    // [signed] char|wchar_t|short|int|long|long long
    U_LONG_LONG  uint_val;   // bool|unsigned (char|short|int|long|long long)
    long double  float_val;  // float|double|long double
    Value () { int_val = 0; uint_val = 0; float_val = 0.0; }
  } value;
  
  enum {
    INT_VAL,
    UINT_VAL,
    FLOAT_VAL
  } value_type;
  
public:
  /** Constructor.
   *  \param v The value.
   *  \param t The type of the value. */
  CConstant (LONG_LONG v, CTypeInfo *t) : CExprValue (t)
   { value.int_val = v; value_type = INT_VAL; }
  /** Constructor.
   *  \param v The value.
   *  \param t The type of the value. */
  CConstant (U_LONG_LONG v, CTypeInfo *t) : CExprValue (t)
   { value.uint_val = v; value_type = UINT_VAL; }
  /** Constructor.
   *  \param v The value.
   *  \param t The type of the value. */
  CConstant (long double v, CTypeInfo *t) : CExprValue (t)
   { value.float_val = v; value_type = FLOAT_VAL; }
  /** Destructor. */
  virtual ~CConstant () {}
   
  /** Cast the value to the given type. 
   *  \param t The type to which to cast.
   *  \return A new object with the casted value.
   *          Has to be destroyed by the caller. */
  CConstant *cast_to (CTypeInfo *t) const;

  /** Compute the resulting value when applying the given 
   *  unary operator to the value. 
   *  \param oper The operator to apply to the value (token type).
   *  \return A new object with the computed value.
   *          Has to be destroyed by the caller. */
  CConstant *compute (int oper) const;
  /** Compute the resulting value when applying the given 
   *  binary operator to this and the given value. 
   *  \param oper The operator to apply to the values (token type).
   *  \param v The other operand to the operator.
   *  \return A new object with the computed value.
   *          Has to be destroyed by the caller. */
  CConstant *compute (int oper, const CConstant *v) const;

  /** Convert the value to a signed integer. */
  LONG_LONG convert_to_int () const;
  /** Convert the value to an unsiged integer. */
  U_LONG_LONG convert_to_uint () const;
  /** Convert the value to floating point. */
  long double convert_to_float () const;

  /** Get this. */
  CConstant *Constant () const { return (CConstant*)this; }
  /** Duplicate this. 
   *  \return A newly allocated duplicate of this.
   *          Has to be destroyed by the caller. */ 
  CConstant *duplicate () const;

  /** Compare the value of this and the given constant.
   *  Does not compare the types.
   *  \param c The value to compare to. 
   *  \return True if the values are equal. */
  bool operator== (const CConstant &c) const;
  /** Compare the value of this and the given constant.
   *  Does not compare the types.
   *  \param c The value to compare to. 
   *  \return True if the values are not equal. */
  bool operator!= (const CConstant &c) const;

  /** Check if the constant value is 0, regardless of the type. */
  bool isNull () const;
  /** Check if the value is positive (>= 0). */
  bool isPositive () const;
  /** Check if the value is negative (< 0). */
  bool isNegative () const;
  /** Check if the value is signed. */
  bool isSigned () const    { return value_type == INT_VAL; }
  /** Check if the value is unsigned. */
  bool isUnsigned () const  { return value_type == UINT_VAL; }
  /** Check is the value is a floating point value. */
  bool isFloat () const     { return value_type == FLOAT_VAL; }

  /** Print the value on the given output stream.
   *  \param out The output stream. */
  virtual void print (std::ostream &out) const;
  
private:
  LONG_LONG compute (int, LONG_LONG) const;
  LONG_LONG compute (LONG_LONG, int, LONG_LONG) const;
  U_LONG_LONG compute (int, U_LONG_LONG) const;
  U_LONG_LONG compute (U_LONG_LONG, int, U_LONG_LONG) const;
  long double compute (int, long double) const;
  long double compute (long double, int, long double) const;

  LONG_LONG cast_to_real_type (LONG_LONG, CTypeInfo *) const;
  U_LONG_LONG cast_to_real_type (U_LONG_LONG, CTypeInfo *) const;
  long double cast_to_real_type (long double, CTypeInfo *) const;
};


} // namespace Puma

#endif /* __CConstant_h__ */
