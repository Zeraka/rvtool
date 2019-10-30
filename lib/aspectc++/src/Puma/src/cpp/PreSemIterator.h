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

#ifndef __daughter_iterator__
#define __daughter_iterator__

// An iterator for daughters in a syntax tree.

#include "Puma/PreTreeIterator.h"

namespace Puma {


class PreSemIterator : public PreTreeIterator {
public:
  PreSemIterator (PreTree *node, bool dir = true) : 
    PreTreeIterator (node, dir) {};
                
  ~PreSemIterator () {};
        
  // Return true if iterator has all done.
  bool isDone () const
    { return _dir ? (_current >= _tree->daughters ()) : (_current < 0); }
        
  // Return current item or NULL if iterator has all done.
  PreTree *currentItem () const 
    { return isDone () ? (PreTree*) 0 : _tree->daughter (_current); }
};


} // namespace Puma

#endif /* __daughter_iterator__ */
