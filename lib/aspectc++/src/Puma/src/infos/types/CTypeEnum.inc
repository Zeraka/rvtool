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

#ifndef __CTypeEnum_h__
#define __CTypeEnum_h__

/** \file
 *  Type of an enumeration. */
 
#ifndef __CTypeInfo_h__
#warning !!!      DO NOT INCLUDE THIS FILE      !!!  
#warning !!! INCLUDE FILE "CTypeInfo.h" INSTEAD !!!  
#endif

namespace Puma {


class CEnumInfo;


/** \class CTypeEnum CTypeEnum.inc Puma/CTypeInfo.h
 * Type of an enumeration.
 * Examples:
 * \code
 * enum E { A,B } e;  // e has type 'enum E'
 * enum { C,D } a;    // a has type 'enum <anonymous>'
 * \endcode
 * \ingroup types */
class CTypeEnum : public CTypeInfo {
  CEnumInfo *_Enum;

public:
  /** Constructor. Type has id CTypeInfo::TYPE_ENUM.
   *  \param e The semantic information about the enumeration. */
  CTypeEnum (CEnumInfo *e);
  /** Destructor. */
  ~CTypeEnum ();

  /** Get the semantic information about the enumeration. */
  CEnumInfo *EnumInfo () const;

  /** Check if the enumeration is complete.
   *  \param pos Optional source code position. */
  bool isComplete (unsigned long pos = 0) const;
  
  /** Get the implementation-defined underlying data
   *  type of the enumeration (defaults to \e int). */
  CTypeInfo *UnderlyingType () const;
};

inline CTypeEnum::CTypeEnum (CEnumInfo *e) :
  CTypeInfo (&CTYPE_EMPTY, CTypeInfo::TYPE_ENUM),
  _Enum (e)
 {}
inline CTypeEnum::~CTypeEnum ()
 {}
 
inline CEnumInfo *CTypeEnum::EnumInfo () const 
 { return _Enum; }


} // namespace Puma

#endif /* __CTypeEnum_h__ */
