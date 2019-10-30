// This file is part of PUMA.
// Copyright (C) 1999-2015  The PUMA developer team.
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

#ifndef PUMA_Unit_H
#define PUMA_Unit_H

/** \file
 * Token chain abstraction. */

#include "Puma/List.h"
#include "Puma/Printable.h"
#include "Puma/UnitState.h"

namespace Puma {

class Token;

/** \class Unit Unit.h Puma/Unit.h
 * Token chain abstraction of a source file or string.
 *  
 * A unit is the result of scanning a file or string
 * containing recognizable source code. The unit consists
 * of a list of tokens that directly map to the characters
 * and words of the source code.
 *
 * A unit can have different states, such as being modified.
 * After changing something on a unit, its state
 * should be set to being modified.
 *
 * All source code manipulations resp. transformations provided
 * by PUMA are based on manipulating the token chain. Tokens
 * can be added, moved, removed, and so on.
 *
 * Translating the token chain back to the source code text is
 * done by printing the unit using method Puma::Unit::print()
 * or as shown in the following example.
 * \code
 * ...
 * std::ofstream file("code.cc");
 * file << unit;
 * file.close();
 * \endcode
 * \ingroup common */
class Unit : public List, public Printable {
  char *m_name;
  UnitState m_state;

public:
  /** Constructor. */
  Unit() :
      List(), m_name((char*) 0) {
  }
  /** Copy-constructor.
   * \param list The token chain to copy. */
  Unit(const List &list) :
      List(list), m_name((char*) 0) {
  }
  /** Destructor. Destroys the tokens. */
  virtual ~Unit();
  /** Set the name of the unit (usually the file name).
   * \param name The name. */
  virtual void name(const char *name);
  /** Get the name of the unit (usually the file name).
   * \return The name of the unit. */
  char *name() const {
    return m_name;
  }
  /** Print the tokens of the unit on the given stream.
   *  \param os The output stream. */
  virtual void print(std::ostream &os) const;
  /** Check if this is a unit for a file.
   * \return True if the tokens belong to a file. */
  virtual bool isFile() const {
    return false;
  }
  /** Check if this is a unit for a macro expansion.
   * \return True if the tokens belong to a macro expansion. */
  virtual bool isMacroExp() const {
    return false;
  }
  /** Check if this is a unit for a template instance.
   * \return True if the tokens belong to a template instance. */
  virtual bool isTemplateInstance() const {
    return false;
  }
  /** Get the state of the unit.
   * \return A reference to the unit state. */
  UnitState &state() {
    return m_state;
  }
  /** Serialize the tokens of the unit.
   * \return A new string containing the tokens of the unit. */
  char *toString() const;

  /** Get the first token in the unit. 
   * \return The first token, or NULL if unit is empty. */
  Token *first() const {
    return (Token*) List::first();
  }
  /** Get the last token in the unit. 
   * \return The last token or NULL if unit is empty. */
  Token *last() const {
    return (Token*) List::last();
  }
  /** Get the next token of the given token. 
   * \return The next token, or NULL if the given token is NULL. */
  Token *next(const Token *token) const {
    return (Token*) List::next((const ListElement*) token);
  }
  /** Get the previous token of the given token. 
   * \return The previous token, or NULL if the given token is NULL. */
  Token *prev(const Token *token) const {
    return (Token*) List::prev((const ListElement*) token);
  }
};

} // namespace Puma

#endif /* PUMA_Unit_H */
