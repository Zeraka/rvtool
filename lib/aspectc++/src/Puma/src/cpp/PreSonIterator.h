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

#ifndef __son_iterator__
#define __son_iterator__

// An iterator for sons in a syntax tree.

#include "Puma/PreTreeIterator.h"

namespace Puma {


class PreSonIterator : public PreTreeIterator {
public:
  PreSonIterator (PreTree *node, bool dir = true) : 
    PreTreeIterator (node, dir) {}
                
  ~PreSonIterator () {}
        
  // Return true if the iterator has all done.
  bool isDone () const
    { return _dir ? (_current >= _tree->sons ()) : (_current < 0); }
                                      
  // Return the current item or NULL if all is done.
  PreTree *currentItem () const
    { return isDone () ? (PreTree*) 0 : _tree->son (_current); }
};


} // namespace Puma

#endif /* __son_iterator__ */
