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

#ifndef __ACIntroducer_h__
#define __ACIntroducer_h__

// Interface class that is used for introductions during a parser run.
// The semantics object has a reference to an ACIntroducer and uses
// the interface at some well defined (join) points.

namespace Puma {
  class CT_ClassDef;
  class CT_Program;
  class Token;
}

namespace Puma {

class ACIntroducer {
public:
  virtual ~ACIntroducer () {}
  
  // called when a new class/union/struct/aspect is created, current scope
  // is the global scope
  virtual void class_before (CT_ClassDef*) {}

  // called when a new class/union/struct/aspect body is parsed
  virtual void class_begin (CT_ClassDef*) {}

  // called when a new class/union/struct/aspect definition ends
  // (still in the class scope)
  virtual void class_end (CT_ClassDef*) {}
  
  // called after the parser tried to parse a base clause
  virtual void base_clause_end (CT_ClassDef*, Token*) {}
  
  // called at the beginning of the parse process
  virtual void trans_unit_begin () {}

  // called after the program has been parsed completely
  virtual void trans_unit_end (CT_Program*) {}
};

} // namespace Puma

#endif /* __ACIntroducer_h__ */
