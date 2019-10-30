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

#ifndef __CTypeAddress_h__
#define __CTypeAddress_h__

/** \file
 *  Type of a reference. */
 
#ifndef __CTypeInfo_h__
#warning !!!      DO NOT INCLUDE THIS FILE      !!!  
#warning !!! INCLUDE FILE "CTypeInfo.h" INSTEAD !!!  
#endif

namespace Puma {


/** \class CTypeAddress CTypeAddress.inc Puma/CTypeInfo.h
 * Type of a reference.
 * Examples:
 * \code
 * int& i = i0;      // i has type 'reference to int'
 *                   // type structure:
 *                   // CTypeAddress
 *                   //   CTypePrimitive int
 * const X& x = x0;  // x has type 'reference to const X'
 *                   // type structure:
 *                   // CTypeAddress
 *                   //   CTypeQualified const
 *                   //     CTypeClass X
 * \endcode
 * \ingroup types */
class CTypeAddress : public CTypeInfo {
public:
  /** Constructor. Type has id CTypeInfo::TYPE_ADDRESS.
   *  \param base The base type. */
  CTypeAddress (CTypeInfo *base);
  /** Destructor. */
  ~CTypeAddress ();
};

inline CTypeAddress::CTypeAddress (CTypeInfo *base) :
  CTypeInfo (base, CTypeInfo::TYPE_ADDRESS)
 {}
inline CTypeAddress::~CTypeAddress ()
 {}


} // namespace Puma

#endif /* __CTypeAddress_h__ */
