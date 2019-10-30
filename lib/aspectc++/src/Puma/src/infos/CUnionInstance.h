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

#ifndef __CUnionInstance_h__
#define __CUnionInstance_h__

/** \file 
 *  Semantic information about an instance of a union template. */

#include "Puma/CTemplateInstance.h"
#include "Puma/CUnionInfo.h"

namespace Puma {


/** \class CUnionInstance CUnionInstance.h Puma/CUnionInstance.h
 *  Semantic information about an instance of a union template. */
class CUnionInstance : public CUnionInfo {
  CTemplateInstance _InstanceInfo;

public: 
  /** Constructor. */
  CUnionInstance ();
  /** Destructor. If the object type is CObjectInfo::UNION_INSTANCE_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
  ~CUnionInstance ();

  /** Get the semantic information about the template instance. */
  CTemplateInstance *TemplateInstance () const;
};

inline CUnionInstance::CUnionInstance () :
  CUnionInfo (CObjectInfo::UNION_INSTANCE_INFO)
 {}
inline CUnionInstance::~CUnionInstance ()
 {}

inline CTemplateInstance *CUnionInstance::TemplateInstance () const
 { return (CTemplateInstance*)(&_InstanceInfo); }


} // namespace Puma

#endif /* __CUnionInstance_h__ */
