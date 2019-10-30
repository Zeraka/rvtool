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

#include "Puma/Unit.h"
#include "Puma/Token.h"
#include "Puma/StrCol.h"
#include <stdlib.h>
#include <sstream>

namespace Puma {

Unit::~Unit() {
  if (m_name)
    delete[] m_name;
  clear();
}

void Unit::name(const char *n) {
  if (m_name)
    delete[] m_name;
  m_name = n ? StrCol::dup(n) : (char*) 0;
}

void Unit::print(std::ostream &os) const {
  const Token *token = (const Token*) first();

  while (token) {
    os << *token;
    token = (const Token*) next(token);
  }
}

// Build a string from an unit.
char *Unit::toString() const {
  Token *token;
  std::ostringstream str;

  // Fill the return string buffer.
  for (token = (Token*) first(); token; token = (Token*) next(token))
    str << token->text();

  // Duplicate and return the string.
  return StrCol::dup(str.str().c_str());
}

} // namespace Puma
