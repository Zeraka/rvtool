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

#include "Puma/Builder.h"
#include "Puma/CObjectInfo.h"
#include "Puma/CScopeInfo.h"

namespace Puma {


unsigned long int Builder::token_counter = 0;

const char *Builder::Container::NodeId () { 
  return "Container"; 
}

void Builder::destroy (CTree *t, bool recursive) {
  if (t) {
    if (recursive)
      for (int s = 0; s < t->Sons (); s++)
        destroy (t->Son (s), true);
    delete t;
  }
}

void Builder::destroyNode (CTree *node) {
  if (node)
    delete node;
}

void Builder::Delete () {
  CTree *tree = Top ();
  if (tree) 
    destroy (tree);
}

// put all children into a list node
CTree *Builder::list (CT_List *l) const {
  for (int n = 0; n < nodes (); n++)
    l->AddSon (get_node (n));
  return l;
}

// put all sons of the container into a list node
CTree *Builder::copy_list (CT_List *l, Container* c) const {
  for (int n = 0; n < c->Sons (); n++)
    l->AddSon (c->Son (n));
  return l;
}

// Helper functions
CTree *Builder::error () { 
  return new CT_Error; 
}

CTree *Builder::token (Token *t) { 
  return new CT_Token (t, ++token_counter); 
}


} // namespace Puma
