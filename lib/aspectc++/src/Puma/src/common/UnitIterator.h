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

#ifndef PUMA_UnitIterator_H
#define PUMA_UnitIterator_H

/** \file
 * Token unit iterator. */

#include "Puma/Unit.h"
#include "Puma/ListElement.h"

namespace Puma {

class Token;

/** \class UnitIterator UnitIterator.h Puma/UnitIterator.h
 * Token unit iterator supporting iteration from the beginning
 * of a unit or any token within that unit.
 * \ingroup common */
class UnitIterator {
  Unit *_unit;           // Unit to iterate.
  Token *_current;        // The current token.

public:
  /** Construct an empty iterator. */
  UnitIterator();
  /** Constructor. Iterates a unit starting at its first token.
   * \param unit The unit to iterate. */
  UnitIterator(Unit &unit);
  /** Constructor. Iterates a unit starting at the given token.
   * \param token The token from which to start to iterate. */
  UnitIterator(ListElement &token);

  /** Reset the iterator to iterate a unit starting at its first token.
   * \param unit The unit to iterate.
   * \return A reference to this iterator. */
  UnitIterator &operator =(Unit &unit);
  /** Reset the iterator to iterate a unit starting at the given token.
   * \param token The token from which to start to iterate.
   * \return A reference to this iterator. */
  UnitIterator &operator =(ListElement &token);

  /** Move on to the next token in the unit.
   * \return A reference to this iterator. */
  UnitIterator &operator ++();
  /** Go back to the previous token in the unit.
   * \return A reference to this iterator. */
  UnitIterator &operator --();

  /** Get the current token in the unit.
   * \return A pointer to the current token. */
  Token *operator *() const;
  /** Call a member on the current token.
   * \return A pointer to the current token on which the member is called. */
  Token *operator ->() const;
};

} // namespace Puma

#endif /* PUMA_UnitIterator_H */
