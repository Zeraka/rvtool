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

#ifndef PUMA_MacroUnit_H
#define PUMA_MacroUnit_H

/** \file
 * Macro expansion unit implementation. */

#include "Puma/Unit.h"
#include "Puma/Token.h"

namespace Puma {

/** \class MacroUnit MacroUnit.h Puma/MacroUnit.h
 * Specialized unit for preprocessor macro expansions.
 * \ingroup common */
class MacroUnit : public Unit {
  Unit *m_callingUnit;
  Token *m_startToken;
  Token *m_endToken;

public:
  /** Construct an empty macro unit. */
  MacroUnit()
      : m_callingUnit(0), m_startToken(0), m_endToken(0) {
  }
  /** Construct a macro unit.
   * \param unit The unit containing the macro expansion. */
  MacroUnit(const Unit &unit)
      : Unit(unit), m_callingUnit(0), m_startToken(0), m_endToken(0) {
  }
  /** Destructor. */
  virtual ~MacroUnit() {
  }
  /** Check if this is a macro expansion unit.
   * \return Always returns true. */
  virtual bool isMacroExp() const {
    return true;
  }
  /** Set the unit containing the macro call.
   * \param unit The unit. */
  void CallingUnit(Unit *unit) {
    m_callingUnit = unit;
  }
  /** Get the unit containing the macro call.
   * \return A pointer to the unit, or NULL if not set. */
  Unit *CallingUnit() const {
    return (Unit*) m_callingUnit;
  }
  /** Set the first token of the macro call in the calling unit.
   * \param token The first token. */
  void MacroBegin(Token *token) {
    m_startToken = token;
  }
  /** Set the last token of the macro call in the calling unit.
   * \param token The last token. */
  void MacroEnd(Token *token) {
    m_endToken = token;
  }
  /** Get the first token of the macro call in the calling unit.
   * \return A pointer to the token. */
  Token *MacroBegin() const {
    return m_startToken;
  }
  /** Get the last token of the macro call in the calling unit.
   * \return A pointer to the token. */
  Token *MacroEnd() const {
    return m_endToken;
  }
  /** Get the first token of the whole macro expansion in the source
   * file if the given token is the first token of this expansion or
   * nested expansion (macro generated macro call).
   * \param token The token.
   * \return The first token, or NULL otherwise. */
  Token *ExpansionBegin(Token *token) const;
  /** Get the last token of the whole macro expansion in the source
   * file if the given token is the last token of this expansion or
   * nested expansion (macro generated macro call).
   * \param token The token.
   * \return The last token, or NULL otherwise. */
  Token *ExpansionEnd(Token *token) const;
};

} // namespace Puma

#endif /* PUMA_MacroUnit_H */
