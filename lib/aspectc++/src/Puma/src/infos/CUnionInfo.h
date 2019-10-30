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

#ifndef __CUnionInfo_h__
#define __CUnionInfo_h__

/** \file 
 *  Semantic information about a union. */

#include "Puma/CRecord.h"

namespace Puma {


class CTypeUnion;


/** \class CUnionInfo CUnionInfo.h Puma/CUnionInfo.h
 *  Semantic information about a union. */
class CUnionInfo : public CRecord {
protected:
  /** Constructor.
   *  \param id The object type. */
  CUnionInfo (ObjectId id);

public: 
  /** Constructor. */
  CUnionInfo ();
  /** Destructor. If the object type is CObjectInfo::UNION_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
  ~CUnionInfo ();

  /** Get the semantic object of the union definition. 
   *  \see CObjectInfo::DefObject() */
  CUnionInfo *DefObject () const;
  /** Get the type information for the union. */
  CTypeUnion *TypeInfo () const;
//  CT_UnionDef *Tree () const;
};

inline CUnionInfo::CUnionInfo (ObjectId id) :
  CRecord (id)
 {}
inline CUnionInfo::CUnionInfo () :
  CRecord (CObjectInfo::UNION_INFO)
 {}
inline CUnionInfo::~CUnionInfo () 
 {}
 
inline CTypeUnion *CUnionInfo::TypeInfo () const
 { return (CTypeUnion*)CObjectInfo::TypeInfo (); }
//inline CT_UnionDef *CUnionInfo::Tree () const
// { return (CT_UnionDef*)CObjectInfo::Tree (); }


} // namespace Puma

#endif /* __CUnionInfo_h__ */
