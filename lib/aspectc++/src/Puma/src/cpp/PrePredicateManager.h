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

#ifndef __pre_predicate_manager__
#define __pre_predicate_manager__

// The predicate manager manages all preprocessor predicates defined
// through the `#assert' directive in the source code.

#include <map>
#include <string>

namespace Puma {


class PrePredicate;

class PrePredicateManager : private std::map<std::string, PrePredicate*> {

  typedef std::map<std::string, PrePredicate*> PMap;
  
public:

  // Destroy the predicates too.
  ~PrePredicateManager ();

  // Add given predicate to predicate table.
  void addPredicate (PrePredicate *);
        
  // Remove predicate `name' from predicate table.
  void removePredicate (const char *name);
        
  // Get predicate `name'.
  PrePredicate *getPredicate (const char *) const;
};


} // namespace Puma

#endif /* __pre_predicate_manager__ */
