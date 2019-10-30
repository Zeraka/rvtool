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

#ifndef __CTypedefInfo_h__
#define __CTypedefInfo_h__

/** \file 
 *  Semantic information about a typedef. */
 
#include "Puma/CScopeRequest.h"

namespace Puma {


/** \class CTypedefInfo CTypedefInfo.h Puma/CTypedefInfo.h
 *  Semantic information about a typedef. A typedef is a
 *  named type for any underlying type. The type of a typedef  
 *  is the underlying type. */
class CTypedefInfo : public CScopeRequest {
public: 
  /** Constructor. */
  CTypedefInfo ();
  /** Destructor. If the object type is CObjectInfo::TYPEDEF_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
  ~CTypedefInfo ();
};

inline CTypedefInfo::CTypedefInfo () :
  CScopeRequest (CObjectInfo::TYPEDEF_INFO)
 {}


} // namespace Puma

#endif /* __CTypedefInfo_h__ */
