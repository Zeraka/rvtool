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

#ifndef __CTypeUnion_h__
#define __CTypeUnion_h__

/** \file
 *  Type of a union. */
 
#ifndef __CTypeInfo_h__
#warning !!!      DO NOT INCLUDE THIS FILE      !!!  
#warning !!! INCLUDE FILE "CTypeInfo.h" INSTEAD !!!  
#endif

namespace Puma {


class CUnionInfo;
class CRecord;


/** \class CTypeUnion CTypeUnion.inc Puma/CTypeInfo.h
 * Type of a union.
 * Example:
 * \code
 * union X x;    // x has type 'union X'
 * \endcode
 * \ingroup types */
class CTypeUnion : public CTypeRecord {
public:
  /** Constructor. Type has id CTypeInfo::TYPE_UNION.
   *  \param c The semantic information about the union. */
  CTypeUnion (CRecord *c);
  /** Destructor. */
  ~CTypeUnion ();
  
  /** Get the semantic information about the union. */
  CUnionInfo *UnionInfo () const;
};

inline CTypeUnion::CTypeUnion (CRecord *u) :
  CTypeRecord (CTypeInfo::TYPE_UNION, u)
 {}
inline CTypeUnion::~CTypeUnion ()
 {}


} // namespace Puma

#endif /* __CTypeUnion_h__ */
