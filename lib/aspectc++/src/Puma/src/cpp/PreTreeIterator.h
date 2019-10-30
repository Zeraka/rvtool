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

#ifndef __syntax_tree_iterator__
#define __syntax_tree_iterator__

// Abstract base class of an iterator for syntax trees.

#include "Puma/PreTree.h"

namespace Puma {


class PreTreeIterator {
protected:
  PreTree *_tree;
  long     _current;
  bool     _dir;

  PreTreeIterator (PreTree *node, bool dir = true) :
    _tree (node), _current (0), _dir (dir) {}
        
public:
  virtual ~PreTreeIterator () {};
        
  // Initialize iterator.
  void first () { _current = _dir ? 0 : _tree->sons (); }
        
  // Go to next item.
  void next () { _current = _dir ? _current + 1 : _current - 1; }
        
  // Return true if current item is the last item.
  virtual bool isDone () const = 0;
        
  // Return the current item or NULL if iterator has all done.
  virtual PreTree *currentItem () const = 0;
};


} // namespace Puma

#endif /* __syntax_tree_iterator__ */
