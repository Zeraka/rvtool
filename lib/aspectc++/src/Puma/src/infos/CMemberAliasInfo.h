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

#ifndef __CMemberAliasInfo_h__
#define __CMemberAliasInfo_h__

/** \file 
 *  Semantic information about a member alias. */

#include "Puma/CScopeRequest.h"

namespace Puma {


/** \class CMemberAliasInfo CMemberAliasInfo.h Puma/infos/CMemberAliasInfo.h
 *  Semantic information about a member alias. A member alias 
 *  is created by a using-declaration. */
class CMemberAliasInfo : public CScopeRequest {
  CObjectInfo *_Member;

public: 
  /** Constructor. */
  CMemberAliasInfo ();
  /** Destructor. If the object type is CObjectInfo::MEMBERALIAS_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
  ~CMemberAliasInfo ();
  
  /** Get the semantic information about the original member 
   *  for which this is an alias. */
  CObjectInfo *Member () const;
  /** Set the original member for which this is an alias. 
   *  \param info The original member. */
  void Member (CObjectInfo *info);
};

inline CMemberAliasInfo::CMemberAliasInfo () :
  CScopeRequest (CObjectInfo::MEMBERALIAS_INFO),
  _Member ((CObjectInfo*)0)
 {}

inline CObjectInfo *CMemberAliasInfo::Member () const
 { return _Member; }
inline void CMemberAliasInfo::Member (CObjectInfo *m)
 { _Member = m; }


} // namespace Puma

#endif /* __CMemberAliasInfo_h__ */
