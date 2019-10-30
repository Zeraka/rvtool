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

#ifndef __Rule__
#define __Rule__

// A rule of the rule table. Consists of two pointers. 
// The first is an invalid pointer to a token and the 
// second the valid one that shall be used instead. 

#include "Puma/Token.h"
#include "Puma/ListElement.h"

namespace Puma {


class RuleTable;

class Rule : public ListElement {
  friend class RuleTable;

  Token *_old;
  Token *_new;

public:
  Rule (Token *o, Token *n) : _old (o), _new (n) {}
  virtual ~Rule () {}
            
  ListElement *duplicate () { return new Rule (_old, _new); }
};


} // namespace Puma

#endif /* __Rule__ */
