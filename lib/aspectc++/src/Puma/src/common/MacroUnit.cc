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

#include "Puma/MacroUnit.h"
#include "Puma/Token.h"

namespace Puma {

Token *MacroUnit::ExpansionBegin(Token *token) const {
  if (token && token == (Token*) first() && MacroBegin()) {
    // macro generated macro call
    if (MacroBegin()->is_macro_generated()) {
      Unit *unit = (Unit*) MacroBegin()->belonging_to();
      if (unit && unit->isMacroExp()) {
        return ((MacroUnit*) unit)->ExpansionBegin(MacroBegin());
      }
      // initial macro call - this is what we searched for
    } else {
      return MacroBegin();
    }
  }
  return 0;
}

Token *MacroUnit::ExpansionEnd(Token *token) const {
  if (token && token == (Token*) last() && MacroEnd()) {
    // macro generated macro call
    if (MacroEnd()->is_macro_generated()) {
      Unit *unit = (Unit*) MacroEnd()->belonging_to();
      if (unit && unit->isMacroExp()) {
        return ((MacroUnit*) unit)->ExpansionEnd(MacroEnd());
      }
      // initial macro call - this is what we searched for
    } else {
      return MacroEnd();
    }
  }
  return 0;
}

} // namespace Puma
