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

#ifndef __pre_arg_desc__
#define __pre_arg_desc__

#include "Puma/Array.h"
#include "Puma/MacroUnit.h"

namespace Puma {


class PreArgDesc {
  // Collected arguments of a function-like macro.
  Array<MacroUnit> m_Args;
  // Begin token of the macro call.
  Token *m_Begin;
  // End token of the macro call.
  Token *m_End;

public:
  // Constructor.
  PreArgDesc () : m_Begin (0), m_End (0) {}
        
  Token *beginToken () const { return m_Begin; }
  Token *endToken () const { return m_End; }
  void beginToken (Token *t) { m_Begin = t; }
  void endToken (Token *t) { m_End = t; }
  
  int numArgs () const { return m_Args.length (); }
  void newArg () { m_Args[numArgs ()]; }
  MacroUnit &currArg () const { return (MacroUnit&)m_Args.lookup (numArgs ()-1); }
  MacroUnit &getArg (int i) const { return (MacroUnit&)m_Args.lookup (i); }
};


} // namespace Puma

#endif /* __pre_arg_desc__ */
