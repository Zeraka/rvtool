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

#ifndef __CLabelInfo_h__
#define __CLabelInfo_h__

/** \file 
 *  Semantic information about a jump label. */

#include "Puma/CScopeRequest.h"

namespace Puma {


/** \class CLabelInfo CLabelInfo.h Puma/CLabelInfo.h
 *  Semantic information about a jump label. Jump labels
 *  are used as argument of goto-statements. */
class CLabelInfo : public CScopeRequest {
public: 
  /** Constructor. */
  CLabelInfo ();
  /** Destructor. If the object type is CObjectInfo::LABEL_INFO, 
   *  CObjectInfo::CleanUp() is called. */
  ~CLabelInfo ();
};

inline CLabelInfo::CLabelInfo () :
  CScopeRequest (CObjectInfo::LABEL_INFO)
 {}


} // namespace Puma

#endif /* __CLabelInfo_h__ */
