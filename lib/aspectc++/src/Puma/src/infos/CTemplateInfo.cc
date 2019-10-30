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
#include "Puma/CTemplateInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CAttributeInfo.h"
#include "Puma/CStructure.h"
#include "Puma/CTemplateInstance.h"
#include "Puma/CSemDatabase.h"
#include "Puma/CNamespaceInfo.h"
#include "Puma/Builder.h"

namespace Puma {


CTemplateInfo::~CTemplateInfo () {
  for (unsigned i = _TreeFragments.length (); i > 0; i--)
    Builder::destroy (_TreeFragments.lookup (i-1));
}

void CTemplateInfo::addTreeFragment (CTree* fragment) {
  if (fragment)
    _TreeFragments.append (fragment);
}

void CTemplateInfo::BaseTemplate (CTemplateInfo *info) { 
  _Base = info; 
  info->addSpecialization ((CTemplateInfo*)this);
}

void CTemplateInfo::addSpecialization (CTemplateInfo *info) { 
  if (! info)
    return;
    
  for (unsigned i = Specializations (); i > 0; i--) 
    if (Specialization (i-1) == info) 
      return;
  _Specializations.append (info); 
}

void CTemplateInfo::removeSpecialization (const CTemplateInfo *info) { 
  for (unsigned i = Specializations (); i > 0; i--) 
    if (Specialization (i-1) == info) {
      _Specializations.remove (i-1); 
      break;
    }
}

void CTemplateInfo::removeLinks () {
  CStructure *scope;

  if (_ObjectInfo) {
    scope = Parent ()->Structure ();
    while (scope && scope->TemplateInfo () && scope != this)
      scope = scope->Parent ()->Structure ();
    if (_ObjectInfo->Record ()) {
      _ObjectInfo->Record ()->TemplateInfo ((CTemplateInfo*)0);
      if (scope) 
        scope->removeType (_ObjectInfo);
    } else if (_ObjectInfo->FunctionInfo ()) {
      _ObjectInfo->FunctionInfo ()->TemplateInfo ((CTemplateInfo*)0);
      if (scope) 
        scope->removeFunction (_ObjectInfo->FunctionInfo ());
    } else if (_ObjectInfo->AttributeInfo ()) {
      _ObjectInfo->AttributeInfo ()->TemplateInfo ((CTemplateInfo*)0);
      if (scope) 
        scope->removeAttribute (_ObjectInfo->AttributeInfo ());
    }
  }
}

CTree *CTemplateInfo::DefaultArgument (unsigned n) const {
  CTemplateParamInfo *pinfo = Parameter (n);
  if (pinfo && pinfo->DefaultArgument ())
    return pinfo->DefaultArgument ();
  return 0;
}

CTemplateParamInfo *CTemplateInfo::newTemplateParam (bool add) {
  CTemplateParamInfo *info = SemDB() ? SemDB()->newTemplateParam() : new CTemplateParamInfo;
  info->TemplateInfo (this);
  addTemplateParam (info);
  if (add) {
    _Params.append (info);
  }
  return info;
}

void CTemplateInfo::deleteTemplateParam (const CTemplateParamInfo *info) {
  CTemplateParamInfo *tinfo;
  for (unsigned i = TemplateParams (); i > 0; i--) 
    if (TemplateParam (i-1) == info) {
      tinfo = TemplateParam (i-1);
      removeTemplateParam (tinfo); 
      if (tinfo->TemplateInfo () == this) {
        for (unsigned j = Parameters (); j > 0; j--) 
          if (Parameter (j) == tinfo) {
            _Params.remove (j);
            break;
          }
        tinfo->TemplateInfo (0);
        delete tinfo;
      }
      break;
    }
}

void CTemplateInfo::removeInstance (const CObjectInfo *info) {
  for (unsigned i = Instances (); i > 0; i--) {
    CObjectInfo* instance = Instance (i-1);
    if (instance == info) {
      _Instances.remove (i-1);
      Parent ()->Structure ()->removeNamespace (instance->Scope ()->NamespaceInfo ());
      instance->TemplateInstance ()->isInstantiated (false);
      instance->TemplateInstance ()->canInstantiate (false);
      //instance->Tree (0);
      //instance->Template (0);
      //instance->Object (0);
      break;
    }
  }
}

void CTemplateInfo::linkInstance (CObjectInfo *info) {
  CObjectInfo *obj;
  CTemplateInfo *curr;
  CTemplateInstance *inst1 = info->TemplateInstance (), *inst2;
  if (! inst1)
    return;

  curr = this;
  do {
    for (unsigned i = curr->Instances (); i > 0; i--) {
      obj = curr->Instance (i-1);
      inst2 = obj->TemplateInstance ();
      if (inst2 && inst1 != inst2 && equalArguments (inst1, inst2)) {
        obj->NextObject (info);
        return;
      }
    }
    curr = nextTemplate (curr);
  } while (curr && curr != this);
}

bool CTemplateInfo::equalArguments (CTemplateInstance *inst1, CTemplateInstance *inst2, bool matchTemplateParams) {
  if (inst1 && inst2 && inst1->InstantiationArgs () == inst2->InstantiationArgs ()) {
    for (unsigned i = inst1->InstantiationArgs (); i > 0; i--)
      if (! inst1->InstantiationArg (i-1)->equals (*inst2->InstantiationArg (i-1), matchTemplateParams))
        return false;
    return true;
  }
  return false;
}

bool CTemplateInfo::equalParameters (CTemplateInfo *t1, CTemplateInfo *t2) {
  if (t1 && t2 && t1->Parameters () == t2->Parameters ()) {
    for (unsigned i = t1->Parameters (); i > 0; i--)
      if (! t1->Parameter (i-1)->match (*t2->Parameter (i-1)))
        return false;
    return true;
  }
  return false;
}

CTemplateInfo *CTemplateInfo::nextTemplate (CTemplateInfo *tinfo) const { 
  CObjectInfo *next = tinfo->ObjectInfo ()->NextObject ();
  if (next->TemplateInstance ()) 
    return next->TemplateInstance ()->Template ();
  // pseudo instance
  return next->Record () ? next->Record ()->TemplateInfo () :
         next->FunctionInfo () ? next->FunctionInfo ()->TemplateInfo () : 
         next->TemplateParamInfo () ? next->TemplateParamInfo ()->TemplateInfo () : 
         (CTemplateInfo*)0; 
}


} // namespace Puma
