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

#ifndef __CSemScope_h__
#define __CSemScope_h__

/** \file
 *  Scope information for the syntax tree. */

namespace Puma {


class CStructure;

/** \class CSemScope CSemScope.h Puma/CSemScope.h
 *  Scope information object for syntax tree nodes. 
 *  Some syntactic constructs open own scopes, e.g.
 *  class definitions, function bodies, and compound 
 *  statements. */
class CSemScope {
  CStructure *_scope;

public:
  /** Constructor. */
  CSemScope () : _scope ((CStructure*)0) {}
  /** Set the scope. 
   *  \param s The scope. */
  void Scope (CStructure *s) { _scope = s; }
  /** Get the scope. */
  CStructure *Scope () const { return _scope; }
};


} // namespace Puma

#endif /* __CSemScope_h__ */
