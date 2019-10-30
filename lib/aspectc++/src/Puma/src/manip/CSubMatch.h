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

#ifndef __CSubMatch__
#define __CSubMatch__

// A sub-match consists of a list of matched trees and a name.

#include "Puma/CTree.h"
#include "Puma/Array.h"

namespace Puma {


class CSubMatch {
  Array<CTree *> _trees; // matched sub-trees
  const char *_name;         // associated name for the sub-match

public:
  CSubMatch (const char *);
  ~CSubMatch () { delete[] _name; }

  // get the matched sub-trees
  Array<CTree *> &trees () const { return (Array<CTree *> &)_trees; }
  unsigned Trees () const { return _trees.length (); }
  CTree *Tree (unsigned i) const { return _trees.lookup (i); }
        
  // get the name of the sub-match
  const char *name () const { return _name; }
};


} // namespace Puma

#endif /* __CSubMatch__ */
