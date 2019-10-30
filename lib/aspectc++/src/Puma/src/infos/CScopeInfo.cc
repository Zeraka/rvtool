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

#include "Puma/CScopeInfo.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CNamespaceInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CFctInstance.h"
#include "Puma/CClassInstance.h"
#include "Puma/CUnionInstance.h"
#include "Puma/CLocalScope.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CSemDatabase.h"

namespace Puma {


CScopeInfo::~CScopeInfo () {
}

bool CScopeInfo::isMethod () const { 
  return FunctionInfo () && FunctionInfo ()->Record (); 
}

bool CScopeInfo::isClassTemplate () const { 
  return TemplateInfo () && (TemplateInfo ()->isClass () ||
                             TemplateInfo ()->isAttribute ()); 
}

bool CScopeInfo::isFctTemplate () const { 
  return TemplateInfo () && TemplateInfo ()->isFunction (); 
}

void CScopeInfo::Parent (const CScopeInfo *info) { 
  if (_Parent != info) {
    if (info != 0) {
      ((CScopeInfo*)info)->addChild (this);
    }
    _Parent = (CScopeInfo*)info;
  }
}

void CScopeInfo::addChild (CScopeInfo *info) { 
  _Children.append (info); 
}

void CScopeInfo::removeChild (const CScopeInfo *info) { 
  for (unsigned i = Children (); i > 0; i--) 
    if (Child (i-1) == info) {
      _Children.remove (i-1); 
      break;
    }
}

CLocalScope *CScopeInfo::newLocalScope () {
  CLocalScope *info = SemDB() ? SemDB()->newLocalScope() : new CLocalScope;
  info->Parent (this);
  return info;
}

CNamespaceInfo *CScopeInfo::newNamespace () {
  CNamespaceInfo *info = SemDB() ? SemDB()->newNamespace() : new CNamespaceInfo;
  info->Parent (this);
  if (Structure ())
    Structure ()->addNamespace (info);
  return info;
}

CTemplateInfo *CScopeInfo::newTemplate () {
  CTemplateInfo *info = SemDB() ? SemDB()->newTemplate() : new CTemplateInfo;
  info->Parent (this);
  return info;
}

CFunctionInfo *CScopeInfo::newFunction (bool inst) {
  CFunctionInfo *info;
  if (inst)
    info = SemDB() ? SemDB()->newFctInstance() : new CFctInstance;
  else
    info = SemDB() ? SemDB()->newFunction() : new CFunctionInfo;
  info->Parent (this);
  if (Structure ())
    Structure ()->addFunction (info);
  return info;
}

CClassInfo *CScopeInfo::newClass (bool inst) {
  CClassInfo *info;
  if (inst)
    info = SemDB() ? SemDB()->newClassInstance() : new CClassInstance;
  else
    info = SemDB() ? SemDB()->newClass() : new CClassInfo;
  info->Parent (this);
  if (Structure ())
    Structure ()->addType (info);
  return info;
}

CUnionInfo *CScopeInfo::newUnion (bool inst) {
  CUnionInfo *info;
  if (inst)
    info = SemDB() ? SemDB()->newUnionInstance() : new CUnionInstance;
  else
    info = SemDB() ? SemDB()->newUnion() : new CUnionInfo;
  info->Parent (this);
  if (Structure ())
    Structure ()->addType (info);
  return info;
}

void CScopeInfo::deleteLocalScope (const CLocalScope *info) {
  for (unsigned i = Children (); i > 0; i--) 
    if (Child (i-1) == (CScopeInfo*)info) {
      delete (CLocalScope*)_Children[i-1];
      _Children.remove (i-1); 
      break;
    }
}

void CScopeInfo::deleteNamespace (const CNamespaceInfo *info) {
  for (unsigned i = Children (); i > 0; i--) 
    if (Child (i-1) == (CScopeInfo*)info) {
      delete (CNamespaceInfo*)_Children[i-1];
      _Children.remove (i-1); 
      break;
    }
}

void CScopeInfo::deleteTemplate (const CTemplateInfo *info) {
  for (unsigned i = Children (); i > 0; i--) 
    if (Child (i-1) == (CScopeInfo*)info) {
      delete (CTemplateInfo*)_Children[i-1];
      _Children.remove (i-1); 
      break;
    }
}

void CScopeInfo::deleteFunction (const CFunctionInfo *info) {
  for (unsigned i = Children (); i > 0; i--) 
    if (Child (i-1) == (CScopeInfo*)info) {
      delete (CFunctionInfo*)_Children[i-1];
      _Children.remove (i-1); 
      break;
    }
}

void CScopeInfo::deleteClass (const CClassInfo *info) {
  for (unsigned i = Children (); i > 0; i--) 
    if (Child (i-1) == (CScopeInfo*)info) {
      delete (CClassInfo*)_Children[i-1];
      _Children.remove (i-1); 
      break;
    }
}

void CScopeInfo::deleteUnion (const CUnionInfo *info) {
  for (unsigned i = Children (); i > 0; i--) 
    if (Child (i-1) == (CScopeInfo*)info) {
      delete (CUnionInfo*)_Children[i-1];
      _Children.remove (i-1); 
      break;
    }
}


} // namespace Puma
