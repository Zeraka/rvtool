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

#ifndef __CBaseClassInfo_h__
#define __CBaseClassInfo_h__

/** \file 
 *  Semantic information about a base class of a class. */

#include "Puma/CScopeRequest.h"
#include "Puma/CProtection.h"

namespace Puma {

class CClassInfo;
class CT_BaseSpec;

/** \class CBaseClassInfo CBaseClassInfo.h Puma/CBaseClassInfo.h
 *  Semantic information about a base class of a class. The base
 *  class is specified by a base class specifier in the base class
 *  list of a class definition. It can have several qualifiers 
 *  like \e virtual, \e public, and so on. */
class CBaseClassInfo : public CScopeRequest {
  /** The semantic object of the base class. */
  CClassInfo        *_Class;
  /** The semantic object of the pseudo instance 
   *  if the base class is a pseudo instance. */
  CClassInfo        *_ClassPseudoInstance;
  
public: 
  /** Constructor. */
  CBaseClassInfo ();
  /** Destructor. If the object type is CObjectInfo::BASECLASS_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
  ~CBaseClassInfo ();
  
  /** Get the semantic information for the base class. */
  CClassInfo *Class () const;
  /** Get the semantic information for the template pseudo instance 
   *  if the base class is a template pseudo instance. */
  CClassInfo *ClassPseudoInstance () const;
  /** Set the semantic information for the base class. */
  void Class (CClassInfo *);

  /** Get the syntax tree node representing the base class specifier. */
  CT_BaseSpec *Tree () const;
};

inline CBaseClassInfo::CBaseClassInfo () :
  CScopeRequest (CObjectInfo::BASECLASS_INFO),
  _Class ((CClassInfo*)0)
 {}


inline CClassInfo *CBaseClassInfo::Class () const {
  return _Class;
}

inline CClassInfo *CBaseClassInfo::ClassPseudoInstance () const {
  return _ClassPseudoInstance;
}

} // namespace Puma

#endif /* __CBaseClassInfo_h__ */
