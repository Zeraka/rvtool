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

#ifndef PUMA_UnitTokenSrc_H
#define PUMA_UnitTokenSrc_H

/** \file
 * Token source for source files. */

#include "Puma/TokenSource.h"
#include "Puma/Token.h"
#include "Puma/Unit.h"

namespace Puma {

/** \class UnitTokenSrc UnitTokenSrc.h Puma/UnitTokenSrc.h
 * Token unit iterator implementing the token source interface.
 * \ingroup common */
class UnitTokenSrc : public TokenSource {
  Unit &_unit;
  Token *_curr;
  bool _first;

public:
  /** Constructor.
   * \param unit The unit to iterate. */
  UnitTokenSrc(Unit &unit);
  /** Set the current token in the unit that is iterated.
   * \param token The new current token. */
  void current(Token* token);
  /** Get the next token in the unit.
   * \return A pointer to the next token. */
  Token *next();
  /** Get the current token in the unit.
   * \return A pointer to the current token. */
  Token *current() const;
  /** Get the token being iterated.
   * \return A pointer to the unit. */
  Unit *unit() const;
  /** Check if the current token is not the last in the unit.
   * \return True, if the end of the unit isn't reached yet. */
  bool hasNext() const;
};

inline UnitTokenSrc::UnitTokenSrc(Unit &u)
    : _unit(u) {
  _curr = (Token*) 0;
  _first = true;
}

inline void UnitTokenSrc::current(Token* c) {
  _curr = c;
  _first = !c;
}

inline Token *UnitTokenSrc::next() {
  if (_curr) {
    _curr = (Token*) _unit.next(_curr);
  } else if (_first) {
    _curr = (Token*) _unit.first();
    _first = false;
  }
  return _curr;
}

inline Token *UnitTokenSrc::current() const {
  return _curr;
}

inline Unit *UnitTokenSrc::unit() const {
  return &_unit;
}

inline bool UnitTokenSrc::hasNext() const {
  return (bool) (_curr ? _unit.next(_curr) : _first ? _unit.first() : 0);
}

} // namespace Puma

#endif /* PUMA_UnitTokenSrc_H */
