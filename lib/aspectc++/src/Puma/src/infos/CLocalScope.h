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

#ifndef __CLocalScope_h__
#define __CLocalScope_h__

/** \file 
 *  Semantic information about a local scope in a function body. */

#include "Puma/CStructure.h"

namespace Puma {


class CFunctionInfo;

/** \class CLocalScope CLocalScope.h Puma/CLocalScope.h
 *  Semantic information about a local scope in a function
 *  body, also called block scope. 
 *
 *  Examples for local scopes:
 *  \code
 * { <local scope> }
 * if (...) <local scope>;
 * while (...) { <local scope> } 
 *  \endcode */
class CLocalScope : public CStructure {
public:
  /** Constructor. */
  CLocalScope ();
  /** Destructor. If the object type is CObjectInfo::LOCAL_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
  ~CLocalScope ();

  /** Get the outermost local scope in a nested hierarchy
   *  of local scopes. */
  CLocalScope* Top () const;
  /** Get the function containing the local scope. 
   *  \return The function or NULL if no function found. */
  CFunctionInfo* Function () const;
};

inline CLocalScope::CLocalScope () :
  CStructure (CObjectInfo::LOCAL_INFO)
 {}
inline CLocalScope::~CLocalScope ()
 {}
 

} // namespace Puma

#endif /* __CLocalScope_h__ */
