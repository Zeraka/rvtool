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

#ifndef __CUsingInfo_h__
#define __CUsingInfo_h__

/** \file
 *  Semantic information about a using-directive. */
 
#include "Puma/CScopeRequest.h"

namespace Puma {


class CNamespaceInfo;


/** \class CUsingInfo CUsingInfo.h Puma/CUsingInfo.h
 *  Semantic information about a using-directive. 
 *  The using-directive makes names from a namespace 
 *  visible in another namespace or scope.
 *
 *  Example:
 *  \code
 * namespace A {
 *   class X {};
 * }
 * using namespace A;  // make A::X visible in global scope
 * X x;                // resolves to A::X
 *  \endcode */
class CUsingInfo : public CScopeRequest {
  CNamespaceInfo *_Namespace;

public:
  /** Constructor. */
  CUsingInfo ();
  /** Destructor. If the object type is CObjectInfo::USING_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
  ~CUsingInfo ();

  /** Get the semantic information for the namespace whose
   *  names are made visible. */
  CNamespaceInfo *Namespace () const;
  /** Set the semantic information for the namespace whose
   *  names are made visible. 
   *  \param info The namespace. */
  void Namespace (CNamespaceInfo *info);
};

inline CUsingInfo::CUsingInfo () :
  CScopeRequest (CObjectInfo::USING_INFO),
  _Namespace (0)
 {}
 
inline CNamespaceInfo *CUsingInfo::Namespace () const 
 { return _Namespace; }
inline void CUsingInfo::Namespace (CNamespaceInfo *nsinfo)
 { _Namespace = nsinfo; }


} // namespace Puma

#endif /* __CUsingInfo_h__ */
