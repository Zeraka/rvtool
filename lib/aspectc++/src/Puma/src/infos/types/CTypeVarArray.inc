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

#ifndef __CTypeVarArray_h__
#define __CTypeVarArray_h__

/** \file
 *  Type of a variable length array. */
 
#ifndef __CTypeInfo_h__
#warning !!!      DO NOT INCLUDE THIS FILE      !!!  
#warning !!! INCLUDE FILE "CTypeInfo.h" INSTEAD !!!  
#endif

namespace Puma {


/** \class CTypeVarArray CTypeVarArray.inc Puma/CTypeInfo.h
 * Type of a variable length array.
 * Example:
 * \code
 * void foo(int len) {
 *   int i[len];      // i has type 'variable length array of int'
 * }
 * \endcode
 * \ingroup types */
class CTypeVarArray : public CTypeArray {
public:
  /** Constructor. Type has id CTypeInfo::TYPE_VAR_ARRAY.
   *  \param base The base type of the array.
   *  \param quals The type qualifiers. */
  CTypeVarArray (CTypeInfo *base, CTypeQualified *quals = 0);
  /** Destructor. */
  ~CTypeVarArray ();
};

inline CTypeVarArray::CTypeVarArray (CTypeInfo *base, CTypeQualified *quals) :
  CTypeArray (base, quals, CTypeInfo::TYPE_VAR_ARRAY)
 {}
inline CTypeVarArray::~CTypeVarArray ()
 {}


} // namespace Puma

#endif /* __CTypeVarArray_h__ */
