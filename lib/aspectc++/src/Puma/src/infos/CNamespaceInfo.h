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

#ifndef __CNamespaceInfo_h__
#define __CNamespaceInfo_h__

/** \file 
 *  Semantic information about a namespace. */

#include "Puma/CStructure.h"

namespace Puma {


/** \class CNamespaceInfo CNamespaceInfo.h Puma/CNamespaceInfo.h
 *  Semantic information about a user-defined namespace. 
 *  There are two kinds of namespaces, original namespaces and 
 *  namespace aliases. */
class CNamespaceInfo : public CStructure {
  bool _isAlias;
  bool _aroundInstantiation;
  
protected:
  /** Constructor.
   *  \param id The object type. */
  CNamespaceInfo (ObjectId id);

public:
  /** Constructor. */
  CNamespaceInfo ();
  /** Destructor. If the object type is CObjectInfo::NAMESPACE_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
 ~CNamespaceInfo ();

  /** Check if this is a namespace alias. */
  bool isAlias () const; 
  /** Set whether this is a namespace alias. 
   *  \param v \e true if it is a namespace alias. */
  void isAlias (bool v);
  
  /** Check if this is a special internal namespace
   *  around template instantiations. */
  bool aroundInstantiation () const;
  /** Set whether this is a special internal namespace around
   *  template instantiations.
   *  \param v \e true if it is such an internal namespace. */
  void aroundInstantiation (bool v);
};

inline CNamespaceInfo::CNamespaceInfo (CObjectInfo::ObjectId id) :
  CStructure (id),
  _isAlias (false),
  _aroundInstantiation (false)
 {}
inline CNamespaceInfo::CNamespaceInfo () :
  CStructure (CObjectInfo::NAMESPACE_INFO),
  _isAlias (false),
  _aroundInstantiation (false)
 {}
 
inline bool CNamespaceInfo::isAlias () const
 { return _isAlias; }
inline void CNamespaceInfo::isAlias (bool v)
 { _isAlias = v; }

inline bool CNamespaceInfo::aroundInstantiation () const
 { return _aroundInstantiation; }
inline void CNamespaceInfo::aroundInstantiation (bool v)
 { _aroundInstantiation = v; }


} // namespace Puma

#endif /* __CNamespaceInfo_h__ */
