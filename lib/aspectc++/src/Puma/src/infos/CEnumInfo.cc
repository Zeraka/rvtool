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

#include "Puma/CEnumInfo.h"
#include "Puma/CEnumeratorInfo.h"
#include "Puma/CTemplateInstance.h"
#include "Puma/CStructure.h"
#include "Puma/CTree.h"
#include "Puma/CSemDatabase.h"
#include "Puma/CTypeInfo.h"
#include <string.h>

namespace Puma {


CEnumInfo::CEnumInfo () :
  CScopeRequest (CObjectInfo::ENUM_INFO),
  _UnderlyingType (&CTYPE_INT) {
}


CEnumInfo::~CEnumInfo () {
}

CEnumeratorInfo *CEnumInfo::Enumerator (const char *name) const { 
  for (unsigned i = 0; i < Enumerators (); i++) 
    if (! strcmp (Enumerator (i)->Name (), name)) 
      return Enumerator (i);
  return (CEnumeratorInfo*)0;
}

void CEnumInfo::addEnumerator (CEnumeratorInfo *info) { 
  for (unsigned i = Enumerators (); i > 0; i--) 
    if (Enumerator (i-1) == info) 
      return;
  _Enumerators.append (info); 
}

void CEnumInfo::removeEnumerator (const CEnumeratorInfo *info) { 
  for (unsigned i = Enumerators (); i > 0; i--) 
    if (Enumerator (i-1) == info) {
      _Enumerators.remove (i-1); 
      break;
    }
}

CEnumeratorInfo *CEnumInfo::newEnumerator () { 
  CEnumeratorInfo *info = SemDB() ? SemDB()->newEnumerator() : new CEnumeratorInfo;
  info->Enum (this);
  addEnumerator (info);
  return info;
}

void CEnumInfo::deleteEnumerator (const CEnumeratorInfo *info) { 
  for (unsigned i = Enumerators (); i > 0; i--) 
    if (Enumerator (i-1) == info) {
      Enumerator (i-1)->Enum (0);
      delete (CEnumeratorInfo*)_Enumerators[i-1];
      _Enumerators.remove (i-1); 
      break;
    }
}

bool CEnumInfo::isComplete (unsigned long pos) const {
  CEnumInfo *info = (CEnumInfo*)DefObject ();
  if (info->isDefined ()) { 
    if (pos == 0)
      return true;
    CT_Token *token = info->Tree ()->token_node ();
    if (token && token->Number () <= pos)
      return true;
  }
  return false;
}

bool CEnumInfo::isDefined () const { 
  return Tree () && Tree ()->NodeName () == CT_EnumDef::NodeId (); 
}

CEnumInfo *CEnumInfo::DefObject () const {
  CObjectInfo *o = (CObjectInfo*)this;
  do {
    if (o->Tree () && o->Tree ()->NodeName () == CT_EnumDef::NodeId ())
      if (! o->TemplateInstance () || o->TemplateInstance ()->canInstantiate ())
        return o->EnumInfo ();
    o = o->NextObject ();
  } while (o != (CObjectInfo*)this);
  return (CEnumInfo*)this;
}


} // namespace Puma
