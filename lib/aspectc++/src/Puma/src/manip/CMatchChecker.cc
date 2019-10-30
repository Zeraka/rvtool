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

#include "Puma/CMatchChecker.h"
#include "Puma/CTree.h"

namespace Puma {


// A match is found.
void CMatchChecker::action (CMatch *matchObj) {
  if (! matchObj) return;
  CTree *currRoot;
    
  if (_mode == CTreeMatcher::MATCH_ALL) {
    // Save all matches.
    _foundMatch = true;
    _matchObj->combine (matchObj);        
  } else if (_mode == CTreeMatcher::MATCH_FIRST) {
    // First match found.
    _foundMatch = true;
    _correct = false;
    _matchObj->combine (matchObj); 
  } else if (_mode == CTreeMatcher::MATCH_LAST) {
    // First match found.
    if (! _foundMatch) {
      _foundMatch = true;
      _last.clear ();
      _last.combine (_matchObj, true); 
    // Next match found, could be the last.
    } else {
      _matchObj->clear ();
      _matchObj->combine (&_last, true);
    }

    _matchObj->combine (matchObj);
  } else if (_mode == CTreeMatcher::MATCH_PRUNE) {
    // Match found. 
    _foundMatch = true;
    _matchObj->combine (matchObj);
        
    // Don't dive into the sons.
    if (matchObj->get ("root").length ())
      prune ();
  } else if (_mode == CTreeMatcher::MATCH_LEAF) {
    if (matchObj->get ("root").length ())
      currRoot = matchObj->get ("root")[0];
    else 
      currRoot = (CTree*)0;
            
    // Match found. Check whether the current match is
    // a sub-match of a "higher" match. If so, eject the 
    // "higher" match.
    if (_foundMatch)
      if (deeper (currRoot, _lastRoot)) { // Match in sub-tree?
        _matchObj->clear ();
        _matchObj->combine (&_last, true);
      }
        
    _lastRoot = currRoot;
    _foundMatch = true;
    _last.clear ();
    _last.combine (_matchObj, true); 
    _matchObj->combine (matchObj);
  }

  // Don't need it anymore.
  delete matchObj;                
}


// Return true if the first node lays "under" the second.
bool CMatchChecker::deeper (CTree *first, CTree *second) {
  if (! second)
    return false;

  if (first == second) 
    return true;

  for (int i = 0; i < second->Sons (); i++)
    if (deeper (first, second->Son (i)))
      return true;
  return false;
}


// Stop visiting the sons on MATCH_FIRST, otherwise return true.
bool CMatchChecker::correct (CTree *node) { 
  if (! _correct)
    prune ();
    
  return _correct;
}


// Has the tree matched?
bool CMatchChecker::matchFound () const { 
  return _foundMatch; 
}


} // namespace Puma
