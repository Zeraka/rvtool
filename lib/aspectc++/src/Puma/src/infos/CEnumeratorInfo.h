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

#ifndef __CEnumeratorInfo_h__
#define __CEnumeratorInfo_h__

/** \file 
 *  Semantic information about an enumeration constant. */

#include "Puma/CAttributeInfo.h"
#include "Puma/Limits.h"

namespace Puma {


class CEnumInfo;
class CT_Enumerator;
class CT_ExprList;


/** \class CEnumeratorInfo CEnumeratorInfo.h Puma/CEnumeratorInfo.h
 *  Semantic information about an enumeration constant. 
 *  An enumeration constant also is called enumerator. */
class CEnumeratorInfo : public CAttributeInfo {
  CEnumInfo *_Enum;
  LONG_LONG _Value;
  bool _HasValue;

public: 
  /** Constructor. */
  CEnumeratorInfo ();
  /** Destructor. If the object type is CObjectInfo::ENUMERATOR_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
  ~CEnumeratorInfo ();
  
  /** Get the syntax tree node of the enumerator constant. */
  CT_Enumerator *Tree () const;

  /** Get the underlying value type. This is implementation-defined
   *  and defaults to \e int.*/
  CTypeInfo *UnderlyingType () const;

  /** Get the enumeration containing this enumerator. */
  CEnumInfo *Enum () const;
  /** Set the enumeration containing this enumerator.
   *  \param ei The enumeration. */
  void Enum (CEnumInfo *ei);

  /** Check if enumerator has a value. */
  bool hasValue () const;
  /** Set whether enumerator has a value.
   *  \param has True or false. */
  void hasValue (bool has);

  /** Get the value of the enumerator. */
  LONG_LONG Value () const;
  /** Set the value of the enumerator. 
   *  \param v The value. */
  void Value (LONG_LONG v);
};

inline CEnumeratorInfo::CEnumeratorInfo () :
  CAttributeInfo (CObjectInfo::ENUMERATOR_INFO),
  _Enum ((CEnumInfo*)0),
  _Value (0),
  _HasValue (false)
 {}

inline void CEnumeratorInfo::Enum (CEnumInfo *e) 
 { _Enum = e; }
inline void CEnumeratorInfo::Value (LONG_LONG v) 
 { _Value = v; }

inline CEnumInfo *CEnumeratorInfo::Enum () const 
 { return _Enum; }
inline LONG_LONG CEnumeratorInfo::Value () const 
 { return _Value; }

inline bool CEnumeratorInfo::hasValue () const
 { return _HasValue; }
inline void CEnumeratorInfo::hasValue (bool has)
 { _HasValue = has; }


} // namespace Puma

#endif /* __CEnumeratorInfo_h__ */
