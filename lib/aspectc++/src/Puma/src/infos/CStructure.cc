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

#include "Puma/CStructure.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CAttributeInfo.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CTemplateParamInfo.h"
#include "Puma/CEnumInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CTypedefInfo.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CNamespaceInfo.h"
#include "Puma/CArgumentInfo.h"
#include "Puma/CFileInfo.h"
#include "Puma/CNamespaceInfo.h"
#include "Puma/CMemberAliasInfo.h"
#include "Puma/CUsingInfo.h"
#include "Puma/CSemDatabase.h"

#include <string.h>
#include <assert.h>
#include <iostream>

namespace Puma {


void CStructure::setShared (CStructure *info) {
  _Shared = info->_Shared;
}

void CStructure::removeRegisterEntry () {
  ObjectsByName::iterator i;
  ObjectInfoList::iterator j;

  for (i = _Shared->_ObjectMap.begin (); i != _Shared->_ObjectMap.end (); ++i) {
    ObjectInfoList &entry = i->second;
    for (j = entry.begin(); j != entry.end(); ++j)
      (*j)->Unregister (_Shared);
  }
}

void CStructure::removeNamedObject (const DString& name, CObjectInfo *info) {
  CStructure::ObjectsByName::iterator sameNameListIter = _Shared->_ObjectMap.find(name);
  if (sameNameListIter != _Shared->_ObjectMap.end()) {
    CStructure::ObjectInfoList& sameNameList = sameNameListIter->second;
    for (CStructure::ObjectInfoList::iterator iter = sameNameList.begin(); iter != sameNameList.end(); iter++) {
      if (*iter == info) {
        sameNameList.erase(iter);
        break;
      }
    }
  }
}

void CStructure::addNamedObject (CObjectInfo *info) {
  // This check is only needed if called externally from CObjectInfo->Name(..)
  if (_Shared->_ObjectSet.find(info) != _Shared->_ObjectSet.end()) {
    if (! info->Name().empty()) {
      _Shared->_ObjectMap[info->Name()].push_back(info);
    }
    if (! info->isRegistered(_Shared))
      info->Register (_Shared);
  }
}

void CStructure::addObject (CObjectInfo *info) {
  _Shared->_ObjectSet.insert(info);
  _Objects.append (info); 
  _Shared->addNamedObject(info); 
}

CStructure::~CStructure () {
}

unsigned CStructure::Functions (const DString& name) const {
  unsigned n = 0;
  ObjectsByName::const_iterator objs = _Shared->_ObjectMap.find(name);
  if (objs != _Shared->_ObjectMap.end()) {
    CStructure::ObjectInfoList::const_reverse_iterator obj = objs->second.rbegin();
    for (; obj != objs->second.rend(); ++obj)
      if ((*obj)->FunctionInfo())
        n++;
  }
  return n;
}

unsigned CStructure::Types (const DString& name) const {
  unsigned n = 0;
  ObjectsByName::const_iterator objs = _Shared->_ObjectMap.find(name);
  if (objs != _Shared->_ObjectMap.end()) {
    CStructure::ObjectInfoList::const_reverse_iterator obj = objs->second.rbegin();
    for (; obj != objs->second.rend(); ++obj)
      if ((*obj)->TypedefInfo () || (*obj)->Record () || (*obj)->EnumInfo ())
        n++;
  }
  return n;
}

unsigned CStructure::Objects (const DString& name) const {
  unsigned n = 0;
  ObjectsByName::const_iterator objs = _Shared->_ObjectMap.find(name);
  if (objs != _Shared->_ObjectMap.end())
    n = objs->second.size();
  return n;
}

unsigned CStructure::Attributes (const DString& name) const {
  unsigned n = 0;
  ObjectsByName::const_iterator objs = _Shared->_ObjectMap.find(name);
  if (objs != _Shared->_ObjectMap.end()) {
    CStructure::ObjectInfoList::const_reverse_iterator obj = objs->second.rbegin();
    for (; obj != objs->second.rend(); ++obj)
      if ((*obj)->AttributeInfo())
        n++;
  }
  return n;
}

unsigned CStructure::TemplateParams (const DString& name) const {
  unsigned n = 0;
  ObjectsByName::const_iterator objs = _Shared->_ObjectMap.find(name);
  if (objs != _Shared->_ObjectMap.end()) {
    CStructure::ObjectInfoList::const_reverse_iterator obj = objs->second.rbegin();
    for (; obj != objs->second.rend(); ++obj)
      if ((*obj)->TemplateParamInfo())
        n++;
  }
  return n;
}

unsigned CStructure::Usings (const DString& name) const {
  unsigned n = 0;
  ObjectsByName::const_iterator objs = _Shared->_ObjectMap.find(name);
  if (objs != _Shared->_ObjectMap.end()) {
    CStructure::ObjectInfoList::const_reverse_iterator obj = objs->second.rbegin();
    for (; obj != objs->second.rend(); ++obj)
      if ((*obj)->UsingInfo())
        n++;
  }
  return n;
}

unsigned CStructure::Namespaces (const DString& name) const {
  unsigned n = 0;
  ObjectsByName::const_iterator objs = _Shared->_ObjectMap.find(name);
  if (objs != _Shared->_ObjectMap.end()) {
    CStructure::ObjectInfoList::const_reverse_iterator obj = objs->second.rbegin();
    for (; obj != objs->second.rend(); ++obj)
      if ((*obj)->NamespaceInfo())
        n++;
  }
  return n;
}

unsigned CStructure::Friends (const DString& name) const {
  unsigned n = 0;
  for (unsigned i = 0; i < Friends (); i++) {
    CObjectInfo* info = Friend (i);
    if (! info->Name ().empty() && info->Name () == name)
      n++;
  }
  return n;
}

CFunctionInfo *CStructure::Function (const DString& name, unsigned pos) const {
  ObjectsByName::const_iterator objs = _Shared->_ObjectMap.find(name);
  if (objs != _Shared->_ObjectMap.end()) {
    CStructure::ObjectInfoList::const_reverse_iterator obj = objs->second.rbegin();
    for (unsigned n = 0; obj != objs->second.rend(); ++obj) {
      if ((*obj)->FunctionInfo()) {
        if (n == pos)
          return (CFunctionInfo*)*obj;
        n++;
      }
    }
  }
  return (CFunctionInfo*)0;
}

CObjectInfo *CStructure::Type (const DString& name, unsigned pos) const {
  ObjectsByName::const_iterator objs = _Shared->_ObjectMap.find(name);
  if (objs != _Shared->_ObjectMap.end()) {
    CStructure::ObjectInfoList::const_reverse_iterator obj = objs->second.rbegin();
    for (unsigned n = 0; obj != objs->second.rend(); ++obj) {
      if ((*obj)->TypedefInfo () || (*obj)->Record () || (*obj)->EnumInfo ()) {
        if (n == pos)
          return (CObjectInfo*)*obj;
        n++;
      }
    }
  }
  return (CObjectInfo*)0;
}

CTypedefInfo *CStructure::Typedef (const DString& name, unsigned pos) const {
  ObjectsByName::const_iterator objs = _Shared->_ObjectMap.find(name);
  if (objs != _Shared->_ObjectMap.end()) {
    CStructure::ObjectInfoList::const_reverse_iterator obj = objs->second.rbegin();
    for (unsigned n = 0; obj != objs->second.rend(); ++obj) {
      if ((*obj)->TypedefInfo ()) {
        if (n == pos)
          return (CTypedefInfo*)*obj;
        n++;
      }
    }
  }
  return (CTypedefInfo*)0;
}

CObjectInfo *CStructure::Object (const DString& name, unsigned pos) const {
  ObjectsByName::const_iterator objs = _Shared->_ObjectMap.find(name);
  if (objs != _Shared->_ObjectMap.end()) {
    CStructure::ObjectInfoList::const_reverse_iterator obj = objs->second.rbegin();
    for (unsigned n = 0; obj != objs->second.rend(); ++obj, ++n)
      if (n == pos)
        return *obj;
  }
  return (CObjectInfo*)0;
}

CAttributeInfo *CStructure::Attribute (const DString& name, unsigned pos) const {
  ObjectsByName::const_iterator objs = _Shared->_ObjectMap.find(name);
  if (objs != _Shared->_ObjectMap.end()) {
    CStructure::ObjectInfoList::const_reverse_iterator obj = objs->second.rbegin();
    for (unsigned n = 0; obj != objs->second.rend(); ++obj) {
      if ((*obj)->AttributeInfo()) {
        if (n == pos)
          return (CAttributeInfo*)*obj;
        n++;
      }
    }
  }
  return (CAttributeInfo*)0;
}

CTemplateParamInfo *CStructure::TemplateParam (const DString& name, unsigned pos) const {
  ObjectsByName::const_iterator objs = _Shared->_ObjectMap.find(name);
  if (objs != _Shared->_ObjectMap.end()) {
    CStructure::ObjectInfoList::const_reverse_iterator obj = objs->second.rbegin();
    for (unsigned n = 0; obj != objs->second.rend(); ++obj) {
      if ((*obj)->TemplateParamInfo()) {
        if (n == pos)
          return (CTemplateParamInfo*)*obj;
        n++;
      }
    }
  }
  return (CTemplateParamInfo*)0;
}

CUsingInfo *CStructure::Using (const DString& name, unsigned pos) const {
  unsigned n = 0;
  for (unsigned i = Usings (); i > 0; i--) {
    CUsingInfo* info = Using (i-1);
    if (! info->Name ().empty() && info->Name () == name) {
      if (n == pos)
        return info;
      n++;
    }
  }
  return (CUsingInfo*)0;
}

CNamespaceInfo *CStructure::Namespace (const DString& name, unsigned pos) const {
  ObjectsByName::const_iterator objs = _Shared->_ObjectMap.find(name);
  if (objs != _Shared->_ObjectMap.end()) {
    CStructure::ObjectInfoList::const_reverse_iterator obj = objs->second.rbegin();
    for (unsigned n = 0; obj != objs->second.rend(); ++obj) {
      if ((*obj)->NamespaceInfo()) {
        if (n == pos)
          return (CNamespaceInfo*)*obj;
        n++;
      }
    }
  }
  return (CNamespaceInfo*)0;
}

CRecord *CStructure::Friend (const DString& name, unsigned pos) const {
  unsigned n = 0;
  for (unsigned i = Friends (); i > 0; i--) {
    CRecord* info = Friend (i-1);
    if (! info->Name ().empty() && info->Name () == name) {
      if (n == pos)
        return info;
      n++;
    }
  }
  return (CRecord*)0;
}

CFunctionInfo *CStructure::Function (const DString& name, CTypeInfo *type) const {
  ObjectsByName::const_iterator objs = _Shared->_ObjectMap.find(name);
  if (objs != _Shared->_ObjectMap.end()) {
    CStructure::ObjectInfoList::const_reverse_iterator obj = objs->second.rbegin();
    for (; obj != objs->second.rend(); ++obj)
      if ((*obj)->FunctionInfo() && *(*obj)->TypeInfo() == *type)
        return (CFunctionInfo*)*obj;
  }
  return (CFunctionInfo*)0;
}

void CStructure::addUsing (CUsingInfo *info) { 
  for (unsigned i = Usings (); i > 0; i--) 
    if (Using (i-1) == info) 
      return;
  _Usings.append (info); 
}

void CStructure::addNamespace (CNamespaceInfo *info) { 
  if (info->isRegistered(Structure()))
    return;
  _Namespaces.append (info); 
  addObject (info);
}

void CStructure::addFriend (CRecord *info) { 
  for (unsigned i = Friends (); i > 0; i--) 
    if (Friend (i-1) == info) 
      return;
  _Friends.append (info); 
}

void CStructure::addType (CObjectInfo *info) { 
  if (info->isRegistered(Structure()))
    return;
  _Types.append (info); 
  addObject (info);
}

void CStructure::addAttribute (CAttributeInfo *info) { 
  if (info->isRegistered(Structure()))
    return;
  _Attributes.append (info); 
  addObject (info);
}

void CStructure::addTemplateParam (CTemplateParamInfo *info) { 
  if (info->isRegistered(Structure()))
    return;
  _TemplateParams.append (info); 
  addObject (info);
}

void CStructure::addFunction (CFunctionInfo *info) {
  if (info->isRegistered(Structure()))
    return;
  _Functions.append (info); 
  addObject (info);
}

void CStructure::removeAttribute (const CAttributeInfo *info) { 
  CMemberAliasInfo *mainfo;
  if ((mainfo = MemberAlias (info)))
    removeMemberAlias (mainfo);
  else
    for (unsigned i = Attributes (); i > 0; i--) 
      if (Attribute (i-1) == info) {
        _Attributes.remove (i-1); 
        removeObject (info);
        break;
      }
}

void CStructure::removeTemplateParam (const CTemplateParamInfo *info) { 
  for (unsigned i = TemplateParams (); i > 0; i--) 
    if (TemplateParam (i-1) == info) {
      _TemplateParams.remove (i-1); 
      removeObject (info);
      break;
    }
}

void CStructure::removeFunction (const CFunctionInfo *info) { 
  CMemberAliasInfo *mainfo;
  if ((mainfo = MemberAlias (info)))
    removeMemberAlias (mainfo);
  else
    for (unsigned i = Functions (); i > 0; i--) 
      if (Function (i-1) == info) {
        _Functions.remove (i-1); 
        removeObject (info);
        break;
      }
}

void CStructure::removeType (const CObjectInfo *info) { 
  CMemberAliasInfo *mainfo;
  if ((mainfo = MemberAlias (info)))
    removeMemberAlias (mainfo);
  else
    for (unsigned i = Types (); i > 0; i--) 
      if (Type (i-1) == info) {
        _Types.remove (i-1); 
        removeObject (info);
        break;
      }
}

void CStructure::removeObject (const CObjectInfo *info) {
  for (unsigned i = Objects (); i > 0; i--) {
    if (Object (i-1) == info) {
      const CObjectInfo* real = (info->MemberAliasInfo() != 0) ? info->MemberAliasInfo()->Member() : info;
      if (real->Name()) {
        ObjectsByName::iterator iter = _Shared->_ObjectMap.find(real->Name());
        if (iter != _Shared->_ObjectMap.end()) {
          for (ObjectInfoList::iterator liter = iter->second.begin(); liter != iter->second.end(); liter++) {
            if (*liter == info) {
              iter->second.erase(liter);
              break;
            }
          }
        }
      }
      _Shared->_ObjectSet.erase((CObjectInfo*)info);
      _Objects.remove (i-1);
      ((CObjectInfo*)info)->Unregister(Structure());
      break;
    }
  }
}

void CStructure::removeUsing (const CUsingInfo *info) { 
  for (unsigned i = Usings (); i > 0; i--) 
    if (Using (i-1) == info) {
      _Usings.remove (i-1); 
      break;
    }
}

void CStructure::removeNamespace (const CNamespaceInfo *info) { 
  for (unsigned i = Namespaces (); i > 0; i--) 
    if (Namespace (i-1) == info) {
      _Namespaces.remove (i-1); 
      removeObject (info);
      break;
    }
}

void CStructure::removeFriend (const CRecord *info) { 
  for (unsigned i = Friends (); i > 0; i--) 
    if (Friend (i-1) == info) {
      _Friends.remove (i-1); 
      break;
    }
}

void CStructure::removeMemberAlias (const CMemberAliasInfo *info) { 
  for (unsigned i = Objects (); i > 0; i--) 
    if (Object (i-1) == info->ObjectInfo ()) {
      removeObject (Object (i-1)); 
      for (unsigned j = (unsigned)_Aliases.length (); j > 0; j--) 
        if (_Aliases.lookup (j-1) == info) {
          _Aliases.remove (j-1); 
          break;
        }
      if (info->Member ()) {
        if (info->Member ()->FunctionInfo ()) {
          for (unsigned j = Functions (); j > 0; j--) 
            if (Function (j-1) == info->Member ()) {
              _Functions.remove (j-1); 
              break;
            }
        } else if (info->Member ()->AttributeInfo ()) {
          for (unsigned j = Attributes (); j > 0; j--) 
            if (Attribute (j-1) == info->Member ()) {
              _Attributes.remove (j-1); 
              break;
            }
        } else if (info->Member ()->TypedefInfo () || 
                   info->Member ()->Record () || 
                   info->Member ()->EnumInfo ()) {
          for (unsigned j = Types (); j > 0; j--) 
            if (Type (j-1) == info->Member ()) {
              _Types.remove (j-1); 
              break;
            }
        }
      }
      break;
    }
}

CAttributeInfo *CStructure::newAttribute () {
  CAttributeInfo *info = SemDB() ? SemDB()->newAttribute() : new CAttributeInfo;
  info->Scope (this);
  addAttribute (info);
  return info;
}

CEnumInfo *CStructure::newEnum () {
  CEnumInfo *info = SemDB() ? SemDB()->newEnum() : new CEnumInfo;
  info->Scope (this);
  addType (info);
  return info;
}

CUsingInfo *CStructure::newUsing () {
  CUsingInfo *info = SemDB() ? SemDB()->newUsing() : new CUsingInfo;
  info->Scope (this);
  addUsing (info);
  return info;
}

CTypedefInfo *CStructure::newTypedef () {
  CTypedefInfo *info = SemDB() ? SemDB()->newTypedef() : new CTypedefInfo;
  info->Scope (this);
  addType (info);
  return info;
}

CMemberAliasInfo *CStructure::newMemberAlias (CObjectInfo *oinfo, bool direct_member) {
  CMemberAliasInfo *info = SemDB() ? SemDB()->newMemberAlias() : new CMemberAliasInfo;
  info->Scope (this);
  addObject (info);
  _Aliases.append (info);
  if (direct_member) {
    if (oinfo->FunctionInfo ()) 
      _Functions.append (oinfo->FunctionInfo ()); 
    else if (oinfo->AttributeInfo ()) 
      _Attributes.append (oinfo->AttributeInfo ()); 
    else if (oinfo->TypedefInfo () || oinfo->Record () || oinfo->EnumInfo ())
      _Types.append (oinfo);
    addObject (oinfo);
  }
  return info;
}

void CStructure::deleteAttribute (const CAttributeInfo *info) {
  for (unsigned i = Attributes (); i > 0; i--) 
    if (Attribute (i-1) == info) {
      removeObject (info);
      Attribute (i-1)->Scope (0);
      if (! Attribute (i-1)->EnumeratorInfo ())
        delete (CAttributeInfo*)_Attributes[i-1];
      _Attributes.remove (i-1); 
      break;
    }
}
 
void CStructure::deleteEnum (const CEnumInfo *info) {
  for (unsigned i = Types (); i > 0; i--) 
    if (Type (i-1) == (CObjectInfo*)info) {
      removeObject (info);
      ((CEnumInfo*)Type (i-1))->Scope (0);
      delete (CEnumInfo*)_Types[i-1];
      _Types.remove (i-1); 
      break;
    }
}

void CStructure::deleteUsing (const CUsingInfo *info) {
  for (unsigned i = Usings (); i > 0; i--) 
    if (Using (i-1) == (CUsingInfo*)info) {
      Using (i-1)->Scope (0);
      delete (CUsingInfo*)_Usings[i-1];
      _Usings.remove (i-1); 
      break;
    }
}

void CStructure::deleteTypedef (const CTypedefInfo *info) {
  for (unsigned i = Types (); i > 0; i--) 
    if (Type (i-1) == (CObjectInfo*)info) {
      removeObject (info);
      ((CTypedefInfo*)Type (i-1))->Scope (0);
      delete (CTypedefInfo*)_Types[i-1];
      _Types.remove (i-1); 
      break;
    }
}

void CStructure::deleteMemberAlias (const CMemberAliasInfo *info) {
  CMemberAliasInfo *mainfo;
  for (unsigned k = Objects (); k > 0; k--) 
    if (Object (k-1) == (CObjectInfo*)info) {
      mainfo = (CMemberAliasInfo*)Object (k-1);
      mainfo->Scope (0);
      if (mainfo->Member ()) {
        if (mainfo->Member ()->FunctionInfo ()) {
          for (unsigned i = Functions (); i > 0; i--) 
            if (Function (i-1) == mainfo->Member ()) {
              _Functions.remove (i-1); 
              break;
            }
        } else if (mainfo->Member ()->AttributeInfo ()) {
          for (unsigned i = Attributes (); i > 0; i--) 
            if (Attribute (i-1) == mainfo->Member ()) {
              _Attributes.remove (i-1); 
              break;
            }
        } else if (mainfo->Member ()->TypedefInfo () || 
                   mainfo->Member ()->Record () || 
                   mainfo->Member ()->EnumInfo ()) {
          for (unsigned i = Types (); i > 0; i--) 
            if (Type (i-1) == mainfo->Member ()) {
              _Types.remove (i-1); 
              break;
            }
        }
      }
      removeObject (Object (k-1)); 
      for (unsigned j = (unsigned)_Aliases.length (); j > 0; j--) 
        if (_Aliases.lookup (j-1) == info) {
          _Aliases.remove (j-1); 
          break;
        }
      delete mainfo;
      break;
    }
}

CMemberAliasInfo *CStructure::MemberAlias (const CObjectInfo *info) const {
  CMemberAliasInfo *mainfo;
  for (unsigned i = (unsigned)_Aliases.length (); i > 0; i--) {
    mainfo = _Aliases.lookup (i-1);
    if (mainfo->Member () == (CObjectInfo*)info)
      return mainfo;
  }
  return (CMemberAliasInfo*)0;  
}


} // namespace Puma
