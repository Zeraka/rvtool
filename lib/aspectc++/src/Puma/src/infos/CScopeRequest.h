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

#ifndef __CScopeRequest_h__
#define __CScopeRequest_h__

/** \file
 *  Additional scope information. */
 
#include "Puma/CObjectInfo.h"

namespace Puma {


class CScopeInfo;


/** \class CScopeRequest CScopeRequest.h Puma/CScopeRequest.h
 *  Provides additional scope information for semantic 
 *  objects that are not scopes itself (like objects). */
class CScopeRequest : public CObjectInfo {
  CScopeInfo *_Scope;

protected:
  /** Constructor.
   *  \param id The object type. */
  CScopeRequest (ObjectId id);

public: 
  /** Destructor. */
  ~CScopeRequest ();
  
  /** Get the enclosing scope. */
  CScopeInfo *Scope () const;
  /** Set the enclosing scope.
   *  \param scope The scope. */
  void Scope (CScopeInfo *scope);
};

inline CScopeRequest::CScopeRequest (CObjectInfo::ObjectId id) :
  CObjectInfo (id),
  _Scope ((CScopeInfo*)0)
 {}
inline CScopeRequest::~CScopeRequest ()
 {}

inline CScopeInfo *CScopeRequest::Scope () const
 { return _Scope; }
inline void CScopeRequest::Scope (CScopeInfo *s)
 { _Scope = s; }


} // namespace Puma

#endif /* __CScopeRequest_h__ */
