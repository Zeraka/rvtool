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

#ifndef __RuleTable__
#define __RuleTable__

// Tokens can be deleted or moved by a manipulation so that 
// another manipulation trying to manipulate this token may 
// fail. This class implements a table of rules describing
// how to get the right (valid) pointer to the token (if 
// moved) or NULL (if deleted).

#include "Puma/Array.h"
#include "Puma/List.h"

namespace Puma {


class Token;

class RuleTable : public List {
  // array of tokens that are already checked
  Array<Token *> _checked;

public:
  // Destructor: delete the arrays too.
  ~RuleTable () { clear (); }
        
  // add a new rule to the table (or update an existing)
  void add (Token *, Token * = (Token*)0);
        
  // get the valid pointer to the token depending on the rules
  Token *get (Token *);

private:
  // return true if the token is already checked
  bool checked (Token *) const;

  // get the valid pointer to the token recursively
  Token *getToken (Token *);
};


} // namespace Puma

#endif /* __RuleTable__ */
