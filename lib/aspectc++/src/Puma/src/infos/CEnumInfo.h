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

#ifndef __CEnumInfo_h__
#define __CEnumInfo_h__

/** \file 
 *  Semantic information about an enumeration. */

#include "Puma/CEnumeratorInfo.h"
#include "Puma/Array.h"
#include "Puma/CScopeRequest.h"

namespace Puma {

class CTypeEnum;


/** \class CEnumInfo CEnumInfo.h Puma/CEnumInfo.h
 *  Semantic information about an enumeration. */
class CEnumInfo : public CScopeRequest {
  Array<CEnumeratorInfo*> _Enumerators;
  CTypeInfo *_UnderlyingType;

public: 
  /** Constructor. */
  CEnumInfo ();
  /** Destructor. If the object type is CObjectInfo::ENUM_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
  ~CEnumInfo ();

  /** Get the number of enumerators in this enumeration. */
  unsigned Enumerators () const;
  /** Get the n-th enumerator.
   *  \param n The index of the enumerator.
   *  \return The enumerator or NULL if \e n is invalid. */
  CEnumeratorInfo *Enumerator (unsigned n) const;
  /** Get the enumerator with the given name. 
   *  \param name The name of the enumerator.
   *  \return The enumerator or NULL if no enumerator with that name. */
  CEnumeratorInfo *Enumerator (const char *name) const;

  /** Get the semantic object of the enumeration definition. 
   *  \see CObjectInfo::DefObject() */
  CEnumInfo *DefObject () const;
  /** Get the type information for the enumeration. */
  CTypeEnum *TypeInfo () const;

  /** Add a new enumerator to the enumeration.
   *  \param ei The enumerator. */
  void addEnumerator (CEnumeratorInfo *ei);
  /** Remove the given enumerator from the enumeration.
   *  \param ei The enumerator. */
  void removeEnumerator (const CEnumeratorInfo *ei);

  /** Create a semantic object for an enumerator. The new 
   *  enumerator is added to the enumeration. */
  CEnumeratorInfo *newEnumerator ();
  /** Destroy the given enumerator. The enumerator is
   *  removed from the enumeration.
   *  \param ei The enumerator. */
  void deleteEnumerator (const CEnumeratorInfo *ei);
  
  /** Check if the enumeration is defined. */
  bool isDefined () const;
  /** Check if the enumeration is complete (defined)
   *  at the given source code position. The position
   *  is specified by the unique number of the CT_Token
   *  tree node representing the name of the enumeration.
   *  \param pos The source code position. */
  bool isComplete (unsigned long pos = 0) const;

  /** Return the underlying data type of the enumeration. 
   *  This is implementation-defined and defaults to \e int. */
  CTypeInfo *UnderlyingType () const;
  /** Set the underlying data type of the enumeration.
   *  This is implementation-defined and defaults to \e int.
   *  \param type The underlying type. */
  void UnderlyingType (CTypeInfo *type);
};

inline unsigned CEnumInfo::Enumerators () const
 { return _Enumerators.length (); }
inline CEnumeratorInfo *CEnumInfo::Enumerator (unsigned n) const
 { return _Enumerators.lookup (n); }

inline CTypeEnum *CEnumInfo::TypeInfo () const
 { return (CTypeEnum*)CObjectInfo::TypeInfo (); }
//inline CT_EnumSpec *CEnumInfo::Tree () const
// { return (CT_EnumSpec*)CObjectInfo::Tree (); }

inline CTypeInfo *CEnumInfo::UnderlyingType () const
 { return (CTypeInfo*)_UnderlyingType; }
inline void CEnumInfo::UnderlyingType (CTypeInfo *type)
 { _UnderlyingType = type; }


} // namespace Puma

#endif /* __CEnumInfo_h__ */
