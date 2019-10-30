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

#ifndef __CTypeBitField_h__
#define __CTypeBitField_h__

/** \file
 *  Type of a bit-field. */
 
#ifndef __CTypeInfo_h__
#warning !!!      DO NOT INCLUDE THIS FILE      !!!  
#warning !!! INCLUDE FILE "CTypeInfo.h" INSTEAD !!!  
#endif

namespace Puma {


/** \class CTypeBitField CTypeBitField.inc Puma/CTypeInfo.h
 * Type of a bit-field.
 * Example:
 * \code
 * class X {
 *   int i : 10;      // i has type 'bit-field of size 10'
 *                    // type structure:
 *                    // CTypeBitField dim=10
 *                    //   CTypePrimitive int
 * };
 * \endcode
 * \ingroup types */
class CTypeBitField : public CTypeInfo {
  long int _Size;

public:
  /** Constructor. Type has id CTypeInfo::TYPE_BIT_FIELD.
   *  \param base The base type. */
  CTypeBitField (CTypeInfo *base);
  /** Destructor. */
  ~CTypeBitField ();

  /** Set the dimension of the bit-field.
   *  \param dim The dimension. */
  void Dimension (long int dim);
  /** Get the dimension of the bit-field. */
  long int Dimension () const;
};

inline CTypeBitField::CTypeBitField (CTypeInfo *base) :
  CTypeInfo (base, CTypeInfo::TYPE_BIT_FIELD),
  _Size (0)
 {}
inline CTypeBitField::~CTypeBitField ()
 {}
 
inline void CTypeBitField::Dimension (long int v) 
 { _Size = v; }
inline long int CTypeBitField::Dimension () const
 { return _Size; }


} // namespace Puma

#endif /* __CTypeBitField_h__ */
