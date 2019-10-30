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

#include "Puma/CObjectInfo.h"
#include "Puma/CScopeInfo.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CFileInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CEnumInfo.h"
#include "Puma/CLabelInfo.h"
#include "Puma/CLocalScope.h"
#include "Puma/CTypedefInfo.h"
#include "Puma/CEnumInfo.h"
#include "Puma/CEnumeratorInfo.h"
#include "Puma/CAttributeInfo.h"
#include "Puma/CArgumentInfo.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CClassInstance.h"
#include "Puma/CFctInstance.h"
#include "Puma/CUnionInstance.h"
#include "Puma/CTemplateParamInfo.h"
#include "Puma/CMemberAliasInfo.h"
#include "Puma/CBaseClassInfo.h"
#include "Puma/CUsingInfo.h"
#include "Puma/CNamespaceInfo.h"
#include "Puma/CTree.h"
#include "Puma/Array.h"
#include "Puma/StrCol.h"
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

namespace Puma {


CObjectInfo::~CObjectInfo () {
  CTypeInfo::Destroy (_TypeInfo);
  if (_QualName)
    delete[] _QualName;
}

void CObjectInfo::Unregister (CStructure *scope) { 
  for (unsigned i = 0; i < (unsigned)_Registered.length (); i++) 
    if (_Registered.lookup (i) == scope)
      _Registered.remove (i); 
}

bool CObjectInfo::isRegistered (const CStructure *scope) const {
  for (unsigned i = 0; i < (unsigned)_Registered.length (); i++) 
    if (_Registered.lookup (i) == scope)
      return true;
  return false;
}

void CObjectInfo::NextObject (CObjectInfo *o) { 
  if (o && o != this) { 
    o->_Next = _Next; 
    o->_Prev = this;
    _Next->_Prev = o; 
    _Next = o; 
  } 
}

void CObjectInfo::PrevObject (CObjectInfo *o) { 
  if (o && o != this) { 
    o->_Prev = _Prev; 
    o->_Next = this;
    _Prev->_Next = o; 
    _Prev = o; 
  } 
}

void CObjectInfo::Unlink () { 
  // remove from qualified scope
  if (QualifiedScope ()) {
    QualifiedScope ()->removeObject (this);
  }
  // remove from assigned scope
  if (AssignedScope ()) {
    AssignedScope ()->removeObject (this);
  }

  // cleanup template instance
  if (TemplateInstance () && TemplateInstance ()->Template ()) {
    TemplateInstance ()->Template ()->removeInstance (this);
  }
  // cleanup CArgumentInfo
  if (ArgumentInfo ()) {
    if (ArgumentInfo ()->Scope () && ArgumentInfo ()->Scope ()->FunctionInfo ())
      ArgumentInfo ()->Scope ()->FunctionInfo ()->removeArgument (ArgumentInfo ());
  }
  // cleanup CAttributeInfo
  if (AttributeInfo ()) {
    if (AttributeInfo ()->Scope () && AttributeInfo ()->Scope ()->Structure ())
      AttributeInfo ()->Scope ()->Structure ()->removeAttribute (AttributeInfo ());
  }
  // cleanup CBaseClassInfo
  if (BaseClassInfo ()) {
    if (BaseClassInfo ()->Scope () && BaseClassInfo ()->Scope ()->ClassInfo ()) {
      CClassInfo *cinfo = BaseClassInfo ()->Scope ()->ClassInfo ();
      if (BaseClassInfo ()->Class ())
        BaseClassInfo ()->Class ()->removeDerivedClass (cinfo);
      cinfo->removeBaseClass (BaseClassInfo ());
    }
  }
  // cleanup CClassInfo
  if (ClassInfo ()) {
    for (unsigned i = 0; i < ClassInfo ()->BaseClasses (); i++) {
      CBaseClassInfo *info = ClassInfo ()->BaseClass (i);
      info->Class ()->removeDerivedClass (ClassInfo ());
    }
    for (unsigned i = 0; i < ClassInfo ()->DerivedClasses (); i++)
      ClassInfo ()->DerivedClass (i)->removeBaseClass (ClassInfo ());
  }
  // cleanup CEnumInfo
  if (EnumInfo ()) {
    if (EnumInfo ()->Scope () && EnumInfo ()->Scope ()->Structure ())
      EnumInfo ()->Scope ()->Structure ()->removeType (EnumInfo ());
  }
  // cleanup CEnumeratorInfo
  if (EnumeratorInfo ()) {
    if (EnumeratorInfo ()->Enum ())
      EnumeratorInfo ()->Enum ()->removeEnumerator (EnumeratorInfo ());
  }
  // cleanup CFunctionInfo
  if (FunctionInfo ()) {
    CScopeInfo *parent = FunctionInfo ()->Parent ();
    if (parent && parent->Structure ())
      parent->Structure ()->removeFunction (FunctionInfo ());
    if (FunctionInfo ()->AssignedScope ())
      FunctionInfo ()->AssignedScope ()->removeFunction (FunctionInfo ());
  }
  // cleanup CLabelInfo
  if (LabelInfo ()) {
    if (LabelInfo ()->Scope () && LabelInfo ()->Scope ()->FunctionInfo ())
      LabelInfo ()->Scope ()->FunctionInfo ()->removeLabel (LabelInfo ());
  }
  // cleanup CMemberAliasInfo
  if (MemberAliasInfo ()) {
    if (MemberAliasInfo ()->Scope () && MemberAliasInfo ()->Scope ()->Structure ())
      MemberAliasInfo ()->Scope ()->Structure ()->removeMemberAlias (MemberAliasInfo ());
  }
  // cleanup CTypedefInfo
  if (TypedefInfo ()) {
    if (TypedefInfo ()->Scope () && TypedefInfo ()->Scope ()->Structure ())
      TypedefInfo ()->Scope ()->Structure ()->removeType (TypedefInfo ());
  }
  // cleanup CUsingInfo
  if (UsingInfo ()) {
    if (UsingInfo ()->Scope () && UsingInfo ()->Scope ()->Structure ())
      UsingInfo ()->Scope ()->Structure ()->removeUsing (UsingInfo ());
  }
  // cleanup CTemplateParamInfo
  if (TemplateParamInfo ()) {
    if (TemplateParamInfo ()->TemplateInfo ())
      TemplateParamInfo ()->TemplateInfo ()->removeTemplateParam (TemplateParamInfo ());
  }
  // cleanup CTemplateInfo
  if (TemplateInfo ()) {
    CTemplateInfo *templ = TemplateInfo ();
    if (templ->isSpecialization () && templ->BaseTemplate ())
      templ->BaseTemplate ()->removeSpecialization (templ);
  }
  // cleanup CRecord
  if (Record ()) {
    CScopeInfo *parent = Record ()->Parent ();
    if (parent && parent->Structure ())
      parent->Structure ()->removeType (Record ());
    if (Record ()->AssignedScope ())
      Record ()->AssignedScope ()->removeType (Record ());
  }
  // cleanup CNamespaceInfo
  if (NamespaceInfo ()) {
    CScopeInfo *parent = NamespaceInfo ()->Parent ();
    if (parent && parent->Structure ())
      parent->Structure ()->removeNamespace (NamespaceInfo ());
  }
  // cleanup CScopeInfo
  if (ScopeInfo ()) {
    if (ScopeInfo ()->Parent () && ScopeInfo ()->Parent () != ScopeInfo ())
      ScopeInfo ()->Parent ()->removeChild (ScopeInfo ());
  }
  // cleanup CStructure
  if (Structure ())
    Structure ()->removeRegisterEntry ();

  // cleanup CObjectInfo
  _Prev->_Next = _Next;
  _Next->_Prev = _Prev;
  _Next = this;
  _Prev = this;
  _Tree = 0;

  // cleanup symbol table
  if (! Name().empty())
    for (unsigned i = _Registered.length (); i > 0; i--)
      _Registered.lookup (i-1)->removeNamedObject (Name (), this);
}

CTemplateInstance *CObjectInfo::TemplateInstance () const { 
  if (ClassInstance ())
    return ClassInstance ()->TemplateInstance ();
  else if (FctInstance ())
    return FctInstance ()->TemplateInstance ();
  else if (UnionInstance ())
    return UnionInstance ()->TemplateInstance ();
  else if (TemplateParamInfo ())
    return TemplateParamInfo ()->TemplateInstance ();
  return (CTemplateInstance*)0; 
}

bool CObjectInfo::isTemplateInstance () const {
  return (ClassInstance () || UnionInstance () || FctInstance () ||
          (TemplateParamInfo () && TemplateParamInfo ()->TemplateInstance ()));
}

bool CObjectInfo::isClassMember () const {
  return AttributeInfo () ? AttributeInfo ()->Record () :
         FunctionInfo () ? FunctionInfo ()->Record () : (CRecord*)0;
}

CRecord *CObjectInfo::ClassScope () const {
  if (AttributeInfo ())
    return AttributeInfo ()->Record ();
  if (FunctionInfo ())
    return FunctionInfo ()->Record ();
  return (CRecord*)0;
}

bool CObjectInfo::operator ==(const CObjectInfo &info) const {
  CObjectInfo *oi1, *oi2;
  oi1 = (CObjectInfo*)this;
  oi2 = (CObjectInfo*)(&info);
  do {
    if (oi1 == oi2)
      return true;
    oi1 = oi1->NextObject ();
  } while (oi1 != (CObjectInfo*)this);
  return false;
}

CScopeInfo *CObjectInfo::Scope () const { 
  if (ScopeInfo ())
    return ScopeInfo ()->Parent ();
  else 
    return TypedefInfo () ? TypedefInfo ()->Scope () :
           EnumInfo () ? EnumInfo ()->Scope () :
           MemberAliasInfo () ? MemberAliasInfo ()->Scope () :
           EnumeratorInfo () ? EnumeratorInfo ()->Scope () :
           AttributeInfo () ? AttributeInfo ()->Scope () :
           TemplateParamInfo () ? TemplateParamInfo ()->TemplateInfo () :
           ArgumentInfo () ? ArgumentInfo ()->Scope () : 
           LabelInfo () ? LabelInfo ()->Scope () : 
           (CScopeInfo*)0; // Should never be reached.
}

bool CObjectInfo::isTemplate () const { 
  return Record () ? Record ()->isTemplate () :
         FunctionInfo () ? FunctionInfo ()->isTemplate () : 
         TemplateParamInfo () ? TemplateParamInfo ()->isTemplate () : false; 
}

void CObjectInfo::TypeInfo (CTypeInfo *info) { 
  _TypeInfo = info; 
}

void CObjectInfo::Name (const DString& name) {
  CObjectInfo *scope = this->AssignedScope ();
  if (! scope) {
    scope = this->Scope ();
  }
  CStructure *scopeStruct = 0;
  if (scope) {
    scopeStruct = scope->Structure ();
  }

  if (scopeStruct && ! _Name.empty ()) {
    scopeStruct->removeNamedObject (name, this);
  }
  _Name = name;
  if (scopeStruct) {
    scopeStruct->addNamedObject (this);
  }
}

const char *CObjectInfo::QualName (bool abs, bool tdef, bool unnamed) {
  if ((! _QualName && ! _Name.empty()) || abs != _abs || tdef != _tdef || unnamed != _unnamed) {
    CObjectInfo *obj = this;
    // get the scope object
    CScopeInfo *info = obj->QualifiedScope ();
    if (!info) info = obj->AssignedScope ();
    if (!info) info = obj->Scope ();

    // if this an anonymous (generated) template scope, skip it
    while (info && info->isNamespace () && info->isAnonymous () &&
           !info->GlobalScope () &&
           (!unnamed || strcmp(info->Name (), "<unnamed>") != 0)) {
      info = info->Scope ();
    }

    std::ostringstream qname;
    if (info->TypeInfo () && info->TypeInfo ()->isRecord ()) {
      // if the scope has an associated type we can use CTypeInfo::TypeText ()
      info->TypeInfo ()->TypeText (qname, 0, abs, tdef, false, unnamed);
      qname << "::";
    }
    else {
      // if the scope is not a type we run through all the namespace
      Array<const char*> scopes;
      while (info && ((unnamed && strcmp(info->Name(), "<unnamed>") == 0) ||
             ! info->isAnonymous ()) &&
             (info->Record () ||
              (info->NamespaceInfo () && ! info->FileInfo ()))) {
        scopes.append (info->Name ());
        info = info->Scope ();
      }
      if (abs)
        qname << "::";
      for (long i = scopes.length () - 1; i >= 0; i--)
        qname << scopes.lookup (i) << "::";
    }
    qname << _Name.c_str ();
    _QualName = StrCol::dup (qname.str ().c_str ());

    // remember these parameters, because on a different request the name
    // has to be regenerated
    _abs = abs;
    _tdef = tdef;
    _unnamed = unnamed;
  }
  return _QualName;
}

CObjectInfo *CObjectInfo::DefObject () const { 
  if (ClassInfo ())
    return ClassInfo ()->DefObject ();
  else if (UnionInfo ())
    return UnionInfo ()->DefObject ();
  else if (FunctionInfo ())
    return FunctionInfo ()->DefObject ();
  else if (EnumInfo ())
    return EnumInfo ()->DefObject ();
  return ObjectInfo (); 
}

CTemplateInfo *CObjectInfo::Template () const { 
  return Record () ? Record ()->TemplateInfo () :
         FunctionInfo () ? FunctionInfo ()->TemplateInfo () : 
         TemplateParamInfo () ? TemplateParamInfo ()->TemplateInfo () : 
         (CTemplateInfo*)0; 
}

bool CObjectInfo::isType () const {
  if (_Id == MEMBERALIAS_INFO)
    return MemberAliasInfo ()->Member () &&
           MemberAliasInfo ()->Member ()->isType ();
  if (_Id == TEMPLATE_PARAM_INFO)
    return TemplateParamInfo ()->isTemplate () ||
           TemplateParamInfo ()->isTypeParam ();
  return _Id == UNION_INFO ||
         _Id == CLASS_INFO ||
         _Id == ENUM_INFO ||
         _Id == TYPEDEF_INFO ||
         _Id == CLASS_INSTANCE_INFO ||
         _Id == UNION_INSTANCE_INFO;
}

bool CObjectInfo::isObject () const {
  if (_Id == MEMBERALIAS_INFO)
    return MemberAliasInfo ()->Member () &&
           MemberAliasInfo ()->Member ()->isObject ();
  if (_Id == TEMPLATE_PARAM_INFO)
    return ! TemplateParamInfo ()->isTemplate () &&
           ! TemplateParamInfo ()->isTypeParam ();
  return _Id == FUNCTION_INFO ||
         _Id == ENUMERATOR_INFO ||
         _Id == ATTRIBUTE_INFO ||
         _Id == FCT_INSTANCE_INFO ||
         _Id == ARGUMENT_INFO;
}

/** Check if the entity is local. An entity is local if it was 
 *  declared in a local scope. */
bool CObjectInfo::isLocal () const {
  if (LocalScope ())
    return true;
  if (FileInfo ())
    return false;
  CScopeInfo* scope = Scope ();
  return scope && scope->isLocal ();
}

CTree *CObjectInfo::Init () const {
  if (_Id == ATTRIBUTE_INFO)
    return ((CAttributeInfo*)this)->Init ();

  CT_ExprList* el = 0;
  CTree* tree = Tree ();
  if (tree) {
    if (_Id == FUNCTION_INFO || _Id == FCT_INSTANCE_INFO) {
      if (tree->NodeName () == CT_InitDeclarator::NodeId ())
        el = ((CT_InitDeclarator*)tree)->Initializer ();
    } else if (_Id == ENUMERATOR_INFO) {
      if (tree->NodeName () == CT_Enumerator::NodeId ())
        el = ((CT_Enumerator*)tree)->Initializer ();
    } else if (_Id == ARGUMENT_INFO) {
      if (tree->NodeName () == CT_ArgDecl::NodeId ())
        el = ((CT_ArgDecl*)tree)->Initializer ();
    }
  }
  return el && el->Entries () > 0 ? el->Entry (0) : 0;
}


} // namespace Puma
