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

#include "Puma/CTemplateParamInfo.h"
#include "Puma/CTemplateInstance.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CStructure.h"
#include "Puma/CTree.h"

namespace Puma {


CTemplateParamInfo::~CTemplateParamInfo () {
  if (_ValueType)
    CTypeInfo::Destroy (_ValueType);
  if (_InstanceInfo)
    delete _InstanceInfo;
}

bool CTemplateParamInfo::match (const CTemplateParamInfo& tp) const {
  if (isTemplate () != tp.isTemplate ())
    return false;
  if (isTypeParam () != tp.isTypeParam ())
    return false;
  if (ValueType () && tp.ValueType () && *ValueType () != *tp.ValueType ())
    return false;

  if (isTemplate ()) {
    CTemplateInfo *t1 = TemplateTemplate ();
    CTemplateInfo *t2 = tp.TemplateTemplate ();
    if (! t1 || ! t2 || (t1->Parameters () != t2->Parameters ()))
      return false;
    for (unsigned i = 0; i < t1->Parameters (); i++) {
      if (! t1->Parameter (i)->match (*t2->Parameter (i)))
        return false;
    }
  }
  return true;
}

CTree *CTemplateParamInfo::DefaultArgument () const { 
  CTemplateInfo *tpl = _TemplateInfo;
  int pos = getPosition ();
  if (tpl && pos != -1) {
    do {
      if ((int)tpl->Parameters () > pos) {
        CTemplateParamInfo *param = tpl->Parameter (pos);
        if (param) {
          CT_TemplateParamDecl *tpd = param->Tree ();
          if (tpd && tpd->DefaultArgument ())
            return tpd->DefaultArgument ()->Entry (0);
        }
      }
      CObjectInfo *obj = tpl->ObjectInfo (), *first = obj;
      tpl = 0;
      if (obj) {
        obj = obj->NextObject ();
        while (obj && obj != first) {
          if ((tpl = obj->Template ()))
            break;
          obj = obj->NextObject ();
        }
      }
    } while (tpl && tpl != _TemplateInfo);
  } else {
    // fallback
    CT_TemplateParamDecl *tpd = Tree ();
    if (tpd && tpd->DefaultArgument ())
      return tpd->DefaultArgument ()->Entry (0);
  }
  return (CTree*)0;
}

int CTemplateParamInfo::getPosition () const {
  if (_TemplateInfo)
    for (unsigned i = 0; i < _TemplateInfo->Parameters (); i++) 
      if (*this == *_TemplateInfo->Parameter (i))
        return i;
  return -1;
}


} // namespace Puma
