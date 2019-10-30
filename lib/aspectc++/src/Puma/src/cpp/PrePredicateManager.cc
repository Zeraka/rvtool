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

#include "Puma/PrePredicateManager.h"
#include "Puma/PrePredicate.h"

namespace Puma {

// Destroy the predicates too.
PrePredicateManager::~PrePredicateManager () {
  // iterate over all stored predicates and delete the objects
  for (iterator iter = begin (); iter != end (); ++iter)
    delete (*iter).second;
}


// Add given predicate to predicate table.
void PrePredicateManager::addPredicate (PrePredicate* predicate) { 
  assert (predicate);
  insert (PMap::value_type (predicate->getName (), predicate));
}
        

// Remove predicate `name' from predicate table.
void PrePredicateManager::removePredicate (const char *name) {
  assert (name);
  iterator entry = find (name);
  if (entry != end ()) {
    delete (*entry).second;
    erase (entry);
  }
}

// Get predicate `name'.
PrePredicate* PrePredicateManager::getPredicate (const char* name) const { 
  const_iterator entry = find (name);
  return entry == end () ? 0 : (*entry).second;
}


} // namespace Puma
