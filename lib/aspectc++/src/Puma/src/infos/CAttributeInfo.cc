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

#include "Puma/CAttributeInfo.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CStructure.h"
#include "Puma/CTree.h"

namespace Puma {


CAttributeInfo::~CAttributeInfo () {
}

CAttributeInfo *CAttributeInfo::DefObject () const {
  CAttributeInfo *ainfo = (CAttributeInfo*)this;
  do {
    if (ainfo->Init ())
      return ainfo;
    ainfo = ainfo->NextObject ()->AttributeInfo ();
  } while (ainfo != (CAttributeInfo*)this);
  return (CAttributeInfo*)this;
}

CT_InitDeclarator *CAttributeInfo::Tree () const {
  return (CT_InitDeclarator*)CObjectInfo::Tree ();
}

CTree *CAttributeInfo::Init () {
  if (! _Init && CObjectInfo::Tree ()) {
    CTree* tree = CObjectInfo::Tree ();
    CT_ExprList* el = 0;
    if (tree->NodeName () == CT_InitDeclarator::NodeId ())
      el = ((CT_InitDeclarator*)tree)->Initializer ();
    else if (tree->NodeName () == CT_Enumerator::NodeId ())
      el = ((CT_Enumerator*)tree)->Initializer ();
    _Init = el && el->Entries () > 0 ? el->Entry (0) : 0;
  }
  return _Init;
}

CRecord *CAttributeInfo::Record () const {
  return CObjectInfo::QualifiedScope () ? CObjectInfo::QualifiedScope ()->Record () : (CRecord*)0;
}

CNamespaceInfo *CAttributeInfo::Namespace () const {
  return CObjectInfo::QualifiedScope () ? CObjectInfo::QualifiedScope ()->NamespaceInfo () : (CNamespaceInfo*)0;
}


} // namespace Puma
