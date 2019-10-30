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

#ifndef PUMA_TokenStream_H
#define PUMA_TokenStream_H

/** \file
 * Input token stream. */

#include "Puma/Array.h"
#include "Puma/Unit.h"
#include "Puma/UnitTokenSrc.h"

namespace Puma {

/** \class TokenStream TokenStream.h Puma/TokenStream.h
 * Continuous token stream from a stack of token units, such
 * as include files and files included by these files (nested
 * includes).
 * \ingroup common */
class TokenStream : public TokenSource, public Array<UnitTokenSrc*> {
public:
  /** Constructor. */
  TokenStream()
      : Array<UnitTokenSrc*>(20) {
  }
  /** Destructor. */
  ~TokenStream() {
  }

  /** Push a token unit on the token unit stack.
   * \param unit The token unit.
   * \param curr Optional current token in the token unit, defaults to the first token in the unit. */
  void push(Unit *unit, Token *curr = 0);

  /** Get the top token unit on token unit stack.
   * \return The unit on top of the stack, or NULL if stack is empty. */
  Unit *top() const;

  /** Get the next token from the top token unit. If the end of the top token
   * unit is reached, that token unit is removed from the stack and the
   * next token in the new top token unit is returned.
   * \return A pointer to the token, or NULL if stack is empty. */
  Token *next();

  /** Get the current token of the top token unit.
   * \return A pointer to the token, or NULL if stack is empty. */
  Token *current() const;

  /** Check if there are tokens after the current token in the top unit.
   * \return True, if the end of the top unit isn't reached yet. */
  bool topHasMore() const;
};

} // namespace Puma

#endif /* PUMA_TokenStream_H */
