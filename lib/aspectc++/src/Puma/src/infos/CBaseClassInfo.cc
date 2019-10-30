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

#include "Puma/CBaseClassInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CTree.h"
#include "Puma/CTemplateInstance.h"

namespace Puma {


CBaseClassInfo::~CBaseClassInfo () {
}

CT_BaseSpec *CBaseClassInfo::Tree () const { 
  return (CT_BaseSpec*)CObjectInfo::Tree (); 
}

void CBaseClassInfo::Class (CClassInfo *c) {
// TEMPORARY HACK --->
  // in the --pseudo-instances mode the base class might be
  // a template pseudo instance. In this case the template is returned instead
  // of the (pseudo) template instance. However, the pseudo instance is
  // available, too
  if (c && c->TemplateInstance () && ! c->TemplateInstance ()->canInstantiate ()) {
    _Class = c->TemplateInstance ()->Template ()->ObjectInfo ()->
      TypeInfo ()->TypeClass ()->ClassInfo ();
    _ClassPseudoInstance = c;
  }
  else {
// TEMPORARY HACK <---
    _Class = c;
    _ClassPseudoInstance = 0;
// TEMPORARY HACK --->
  }
// TEMPORARY HACK <---
}

} // namespace Puma
