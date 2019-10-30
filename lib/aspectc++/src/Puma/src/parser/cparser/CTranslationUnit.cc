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

#include "Puma/CTranslationUnit.h"
#include "Puma/PreTree.h"
#include "Puma/CTree.h"

namespace Puma {


CTranslationUnit::~CTranslationUnit () {
  destroy (_syntax_tree);
  destroy (_cpp_tree);
  // destroy the unit if this is a translation unit
  // of a template instance -- this is a on-the-fly
  // unit, not a primary
  if (_unit && _unit->isTemplateInstance())
    delete _unit;
}

void CTranslationUnit::destroy (PreTree *tree) {
  if (! tree) return;
  for (int s = 0; s < tree->sons (); s++)
    destroy (tree->son (s));
  for (int s = 0; s < tree->daughters (); s++)
    delete tree->daughter (s);
  delete tree;
}


} // namespace Puma
