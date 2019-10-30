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

#ifndef __CTypePointer_h__
#define __CTypePointer_h__

/** \file
 *  Type of a pointer. */
 
#ifndef __CTypeInfo_h__
#warning !!!      DO NOT INCLUDE THIS FILE      !!!  
#warning !!! INCLUDE FILE "CTypeInfo.h" INSTEAD !!!  
#endif

namespace Puma {


/** \class CTypePointer CTypePointer.inc Puma/CTypeInfo.h
 * Type of a pointer.
 * Examples:
 * \code
 * int* ip = 0;          // ip has type 'pointer to int'
 *                       // type structure:
 *                       // CTypePointer
 *                       //   CTypePrimitive int
 * const char* s = 0;    // s has type 'pointer to const char'
 *                       // type structure:
 *                       // CTypePointer
 *                       //   CTypeQualified const
 *                       //     CTypePrimitive char
 * \endcode
 * \ingroup types */
class CTypePointer : public CTypeInfo {
protected:
  /** Constructor.
   *  \param base The base type. 
   *  \param id The type identifier. */
  CTypePointer (CTypeInfo *base, CTypeInfo::TypeId id);

public:
  /** Constructor. Type has id CTypeInfo::TYPE_POINTER.
   *  \param base The base type. */
  CTypePointer (CTypeInfo *base);
  /** Destructor. */
  ~CTypePointer ();
};

inline CTypePointer::CTypePointer (CTypeInfo *base) :
  CTypeInfo (base, CTypeInfo::TYPE_POINTER)
 {}
inline CTypePointer::CTypePointer (CTypeInfo *base, CTypeInfo::TypeId id) :
  CTypeInfo (base, id)
 {}
inline CTypePointer::~CTypePointer ()
 {}
 

} // namespace Puma

#endif /* __CTypePointer_h__ */
