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

#include "Puma/CVisitor.h"
#include "Puma/CTree.h"

namespace Puma {


void CVisitor::visit (CTree *node) {
  if (! node) 
    return;

  pre_visit (node);
  
  if (_pruned) { 
    _pruned = false;
    return;
  }
  if (_aborted)
    return;
  
  visit_sons (node);
  
  if (_aborted)
    return;
  
  post_visit (node);
}

void CVisitor::visit_sons (CTree *node) {
  for (int i = 0; i < node->Sons (); i++) {
    if (_aborted) 
      return;
    visit (node->Son (i));
  }
}

void CVisitor::pre_visit (CTree *node) {
  // do something
}

void CVisitor::post_visit (CTree *node) {
  // do something
}


} // namespace Puma
