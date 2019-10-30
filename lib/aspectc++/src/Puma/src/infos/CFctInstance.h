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

#ifndef __CFctInstance_h__
#define __CFctInstance_h__

/** \file 
 *  Semantic information about an instance of a function template. */

#include "Puma/CTemplateInstance.h"
#include "Puma/CFunctionInfo.h"

namespace Puma {


/** \class CFctInstance CFctInstance.h Puma/CFctInstance.h
 *  Semantic information about an instance of a function template. */
class CFctInstance : public CFunctionInfo {
  CTemplateInstance _InstanceInfo;
  CTypeInfo* _PseudoType;  // pseudo instance type, replaced by type of real instantiation

public: 
  /** Constructor. */
  CFctInstance ();
  /** Destructor. If the object type is CObjectInfo::FCT_INSTANCE_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
  ~CFctInstance ();

  /** Get the semantic information for the template instance. */
  CTemplateInstance *TemplateInstance () const;
  
  /** Set the type of the real instantiation. 
   *  \param type The function type. */
  void RealTypeInfo (CTypeInfo* type);
};

inline CFctInstance::CFctInstance () :
  CFunctionInfo (CObjectInfo::FCT_INSTANCE_INFO),
  _PseudoType (0)
 {}

inline CTemplateInstance *CFctInstance::TemplateInstance () const
 { return (CTemplateInstance*)(&_InstanceInfo); }


} // namespace Puma

#endif /* __CFctInstance_h__ */
