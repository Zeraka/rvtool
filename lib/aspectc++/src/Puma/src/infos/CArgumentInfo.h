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

#ifndef __CArgumentInfo_h__
#define __CArgumentInfo_h__

/** \file 
 *  Semantic information about a function parameter. */

#include "Puma/CScopeRequest.h"

namespace Puma {


/** \class CArgumentInfo CArgumentInfo.h Puma/CArgumentInfo.h
 *  Semantic information about a function parameter. */
class CArgumentInfo : public CScopeRequest {
  bool _hasDefaultArg; // can be set to true even without an init tree
                       // (needed for inline member functions)
public: 
  /** Constructor. */
  CArgumentInfo ();
  /** Destructor. If the object type is CObjectInfo::ARGUMENT_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
  ~CArgumentInfo ();

  /** Return \e true if the parameter has a default argument. */
  bool hasDefaultArg () const;  
  /** Set whether the parameter has a default argument. 
   *  \param v True for yes, false for no. */
  void hasDefaultArg (bool v);  
};

inline CArgumentInfo::CArgumentInfo () :
  CScopeRequest (CObjectInfo::ARGUMENT_INFO), _hasDefaultArg (false)
 {}

inline bool CArgumentInfo::hasDefaultArg () const
 { return (_hasDefaultArg || Init ()); }

inline void CArgumentInfo::hasDefaultArg (bool has_arg)
 { _hasDefaultArg = has_arg; }

} // namespace Puma

#endif /* __CArgumentInfo_h__ */
