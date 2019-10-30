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

#include "Puma/CTemplateInstance.h"
#include "Puma/CCInstantiation.h"
#include "Puma/CSemDatabase.h"
#include "Puma/Builder.h"

namespace Puma {


CTemplateInstance::~CTemplateInstance () {
  if (_IsSpecialization)
    for (unsigned i = 0; i < InstantiationArgs (); i++)
      delete InstantiationArg (i);
  for (unsigned i = 0; i < DeducedArgs (); i++)
    delete DeducedArg (i);
}


bool CTemplateInstance::instantiate (CStructure *scope) {
  if (_Pseudo && _CanInstantiate) {
    CCInstantiation inst(_TemplateInfo->ClassDB()->Project()->err());
    _Pseudo = false;
    inst.instantiate(this, scope);
    hasDelayedParseProblem (inst.detectedDelayedParseProblem ());
    _Pseudo = ! isInstantiated();
  }
  return ! _Pseudo;
}


} // namespace Puma
