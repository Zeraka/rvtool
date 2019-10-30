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

#include "Puma/UnitIterator.h"
#include "Puma/Token.h"
#include "Puma/Unit.h"
#include <assert.h>

namespace Puma {

UnitIterator::UnitIterator() {
  _unit = (Unit*) 0;
  _current = (Token*) 0;
}

UnitIterator::UnitIterator(ListElement &token) {
  _current = (Token*) &token;
  _unit = (Unit*) token.belonging_to();
}

UnitIterator::UnitIterator(Unit &unit) {
  _current = (Token*) unit.first();
  _unit = &unit;
}

Token *UnitIterator::operator *() const {
  return _current;
}

Token *UnitIterator::operator ->() const {
  assert(_current);
  return _current;
}

UnitIterator &UnitIterator::operator =(ListElement& token) {
  _current = (Token*) &token;
  _unit = (Unit*) token.belonging_to();
  return *this;
}

UnitIterator &UnitIterator::operator =(Unit &unit) {
  _current = (Token*) unit.first();
  _unit = &unit;
  return *this;
}

UnitIterator &UnitIterator::operator ++() {
  if (_unit && _current)
    _current = (Token*) _unit->next(_current);
  return *this;
}

UnitIterator &UnitIterator::operator --() {
  if (_unit && _current)
    _current = (Token*) _unit->prev(_current);
  return *this;
}

} // namespace Puma
