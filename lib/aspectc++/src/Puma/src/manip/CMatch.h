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

#ifndef __CMatch__
#define __CMatch__

// A match consists of a list of matched trees

#include "Puma/CSubMatch.h"
#include "Puma/Array.h"

namespace Puma {


class CMatch {
  bool _root;
        
  Array<CSubMatch*> _matches; // sub-matches
  Array<CTree *> _empty;      // to return in error case

public:
  CMatch (bool root = false) : _root (root) {}
  ~CMatch ();

  // combine two CMatch objects
  void combine (CMatch *, bool = false);

  // get the sub-trees with the given name
  Array<CTree *> &get (const char *) const;

  // add a new sub-tree
  void add (CTree *, const char *);
        
  // get the sub-matches
  Array<CSubMatch *> &matches () { return (Array<CSubMatch *>&) _matches; }
  unsigned SubMatches () const { return _matches.length (); }
  CSubMatch *SubMatch (unsigned i) const { return _matches.lookup (i); }
  CSubMatch *SubMatch (const char *) const;
        
  // clear the object
  void clear ();
        
private:
  // get the index of the named sub-trees
  int index (const char *) const;
};


} // namespace Puma

#endif /* __CMatch__ */
