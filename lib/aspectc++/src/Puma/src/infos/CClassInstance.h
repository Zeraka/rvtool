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

#ifndef __CClassInstance_h__
#define __CClassInstance_h__

/** \file 
 *  Semantic information about an instance of a class template. */

#include "Puma/CTemplateInstance.h"
#include "Puma/CClassInfo.h"

namespace Puma {


/** \class CClassInstance CClassInstance.h Puma/CClassInstance.h
 *  Semantic information about an instance of a class template. */
class CClassInstance : public CClassInfo {
  CTemplateInstance _InstanceInfo;

public: 
  /** Constructor. */
  CClassInstance ();
  /** Destructor. If the object type is CObjectInfo::CLASS_INSTANCE_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
  ~CClassInstance ();

  /** Get the semantic information about the template instance. */
  CTemplateInstance *TemplateInstance () const;
};

inline CClassInstance::CClassInstance () :
  CClassInfo (CObjectInfo::CLASS_INSTANCE_INFO)
 {}
inline CClassInstance::~CClassInstance ()
 {}

inline CTemplateInstance *CClassInstance::TemplateInstance () const
 { return (CTemplateInstance*)(&_InstanceInfo); }


} // namespace Puma

#endif /* __CClassInstance_h__ */
