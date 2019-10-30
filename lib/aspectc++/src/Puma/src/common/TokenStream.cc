// This file is part of PUMA.
// Copyright (C) 1999-2016  The PUMA developer team.
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

#include "Puma/TokenStream.h"
#include "Puma/Token.h"

namespace Puma {

// Push a unit on stack.
void TokenStream::push(Unit *unit, Token *curr) {
  if (unit) {
    UnitTokenSrc *src = new UnitTokenSrc(*unit);
    append(src);
    if (curr) {
      src->current(curr);
    }
  }
}

// Get the top unit on stack.
Unit *TokenStream::top() const {
  if (length()) {
    return lookup(length() - 1)->unit();
  }
  return (Unit*) 0;
}

// Get the current token of the top unit.
Token *TokenStream::current() const {
  if (length()) {
    return lookup(length() - 1)->current();
  }
  return (Token*) 0;
}

// Get the next token of the top unit.
Token *TokenStream::next() {
  long pos = length() - 1;

  // No more units left. Return NULL to show it the caller.
  if (pos < 0)
    return (Token*) 0;

  // Get the next token.
  Token *token = lookup(pos)->next();
  if (token)
    return token;

  // Top unit is done. Take it from stack.
  delete get(pos);
  remove(pos);

  // Get the next token of the "new" top unit.
  return next();
}

// Check if there are more tokens in the current top unit.
bool TokenStream::topHasMore() const {
  long pos = length() - 1;

  // No more units left.
  if (pos < 0)
    return false;

  // Return true if there is a next token in the top unit.
  return lookup(pos)->hasNext();
}

} // namespace Puma
