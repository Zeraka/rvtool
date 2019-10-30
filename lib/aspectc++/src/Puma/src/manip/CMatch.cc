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

#include "Puma/CMatch.h"
#include <string.h>
#include <stdio.h>

namespace Puma {


// delete the sub-matches
CMatch::~CMatch () {
  for (int i = 0; i < _matches.length (); i++)
    delete _matches.get (i);
}

// clear the object
void CMatch::clear () {
  for (int i = 0; i < _matches.length (); i++)
    delete _matches.get (i);

  _matches.reset ();
}

// combine two CMatch objects
void CMatch::combine (CMatch *matchObj, bool root) {
  if (! matchObj) 
    return;

  Array<CSubMatch *> &matches = matchObj->matches ();
  int start = 0;
  if (! _root && ! root)
    if (matches.length ())
      if (strcmp (matches[0]->name (), "root") == 0)
        start = 1;

  for (int i = start; i < matches.length (); i++)
    for (int j = 0; j < (int)matches[i]->Trees (); j++)
      add (matches[i]->Tree (j), matches[i]->name ());
}

// get the index of the named sub-tree
int CMatch::index (const char *name) const {
  if (! name) 
    return -1;
    
  int len = _matches.length ();

  for (int i = 0; i < len; i++)
    if (_matches.lookup (i)->name ())
      if (strcmp (_matches.lookup (i)->name (), name) == 0)
        return i;

  return -1;
}

// get a sub-tree by its associated name
Array<CTree *> &CMatch::get (const char *name) const {
  int pos = index (name);

  if (pos == -1)
    return (Array<CTree *>&)_empty;

  return _matches.lookup (pos)->trees ();
}
 
// get the named sub-match
CSubMatch *CMatch::SubMatch (const char *name) const {
  int pos = index (name);
    
  if (pos == -1)
    return (CSubMatch*)0;

  return _matches.lookup (pos);
}
 
// add a new sub-tree
void CMatch::add (CTree *tree, const char *name) {
  int pos;

  if (! tree || ! name) 
    return;

  if (! get (name).length ()) {
    pos = _matches.length ();
    _matches.get (pos) = new CSubMatch (name);
  } else
    pos = index (name);
        
  _matches.get (pos)->trees ().append (tree);
}


} // namespace Puma
