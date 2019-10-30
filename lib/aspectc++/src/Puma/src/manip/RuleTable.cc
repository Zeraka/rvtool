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

#include "Puma/RuleTable.h"
#include "Puma/Token.h"
#include "Puma/Rule.h"
#include <assert.h>

namespace Puma {


// add a new rule to the table (or update an existing)
void RuleTable::add (Token *oldT, Token *newT) {
  assert (oldT);
  Rule *curr = (Rule*) first ();
  Rule *nextr = 0;
    
  // search for an existing rule
  for (; curr; curr = nextr) {
    nextr = (Rule*) next(curr);
     
    // existing rule found 
    if (curr->_old == oldT) {
      // delete the rule if the old and new tokens 
      // are the same, to avoid endless loops
      if (oldT == newT)
        kill (curr);
      else // update the rule
        curr->_new = newT;
      return;
    }
  }
   
  // create a new rule
  append (*(new Rule (oldT, newT)));
}
        
// get the valid pointer to the token depending on the rules
Token *RuleTable::get (Token *token) {
  // Get the corresponding token.
  token = getToken (token);

  _checked.reset();
  if (token)
    assert (token->belonging_to ());
  return token;
}

// get the valid pointer to the token depending on the rules
Token *RuleTable::getToken (Token *oldT) {
  if (! oldT || checked (oldT))
    return oldT;

  Rule *curr = (Rule*) first ();
    
  // get the wanted rule
  for (; curr; curr = (Rule*) next (curr)) {
    // rule found
    if (curr->_old == oldT) {
      // mark the token as already checked
      _checked.append (oldT);
            
      // now check whether there is a rule for the new token too
      // example: a->b && b->c => a->c
      return get (curr->_new);
    }
  }

  return oldT;
}

// return true if the token is already checked
bool RuleTable::checked (Token *token) const {
  for (int i = 0; i < _checked.length (); i++)
    if (_checked.lookup (i) == token)
      return true;
  return false;
}


} // namespace Puma
