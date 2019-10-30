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

#include "Puma/CClassInfo.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CFctInstance.h"
#include "Puma/CAttributeInfo.h"
#include "Puma/CClassInstance.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CTemplateInstance.h"
#include "Puma/CTokens.h"
#include "Puma/CTree.h"
#include "Puma/Token.h"
#include "Puma/CSemDatabase.h"
#include <string.h>

namespace Puma {


CClassInfo::~CClassInfo () {
}

CBaseClassInfo *CClassInfo::BaseClass (const char *name) const { 
  for (unsigned i = 0; i < BaseClasses (); i++) 
    if (! strcmp (BaseClass (i)->Class ()->Name (), name)) 
      return BaseClass (i);
  return (CBaseClassInfo*)0;
}

CClassInfo *CClassInfo::DerivedClass (const char *name) const { 
  for (unsigned i = 0; i < DerivedClasses (); i++) 
    if (! strcmp (DerivedClass (i)->Name (), name)) 
      return DerivedClass (i);
  return (CClassInfo*)0;
}

void CClassInfo::addBaseClass (CBaseClassInfo *info) { 
  for (unsigned i = 0; i < BaseClasses (); i++) 
    if (BaseClass (i) == info) 
      return;
  info->Class ()->addDerivedClass ((CClassInfo*)this);
  _BaseClasses.append (info); 
}

void CClassInfo::addDerivedClass (CClassInfo *info) { 
  for (unsigned i = 0; i < DerivedClasses (); i++) 
    if (DerivedClass (i) == info) 
      return;
  _Derived.append (info); 
}

void CClassInfo::removeBaseClass (const CBaseClassInfo *info) { 
  for (unsigned i = 0; i < BaseClasses (); i++) 
    if (BaseClass (i) == info) {
      BaseClass (i)->Scope ((CScopeInfo*)0);
      //delete _BaseClasses[i];
      _BaseClasses.remove (i); 
      break;
    }
}

void CClassInfo::removeBaseClass (const CClassInfo *info) { 
  for (unsigned i = 0; i < BaseClasses (); i++) 
    if (BaseClass (i)->Class () == info) {
      BaseClass (i)->Scope ((CScopeInfo*)0);
      //delete _BaseClasses[i];
      _BaseClasses.remove (i); 
    }
}

void CClassInfo::removeDerivedClass (const CClassInfo *info) { 
  for (unsigned i = 0; i < DerivedClasses (); i++) 
    if (DerivedClass (i) == info) {
      _Derived.remove (i); 
      break;
    }
}

bool CClassInfo::isBaseClass (const CClassInfo *info, bool recursive) const { 
  for (unsigned i = 0; i < BaseClasses (); i++) 
    if (BaseClass (i)->Class () == info ||
        (recursive && BaseClass (i)->Class ()->isBaseClass (info, true)))
      return true;
  return false;
}

bool CClassInfo::isDerivedClass (const CClassInfo *info, bool recursive) const { 
  for (unsigned i = 0; i < DerivedClasses (); i++) 
    if (DerivedClass (i) == info ||
        (recursive && DerivedClass (i)->isDerivedClass (info, true)))
      return true;
  return false;
}

CBaseClassInfo *CClassInfo::newBaseClass (CClassInfo *cinfo) { 
  CBaseClassInfo *info = SemDB() ? SemDB()->newBaseClass() : new CBaseClassInfo;
  info->Scope (this);
  info->Class (cinfo);
  if (cinfo)
    info->Class ()->addDerivedClass ((CClassInfo*)this);
  _BaseClasses.append (info); 
  return info;
}

bool CClassInfo::isStruct () const { 
  if (Tree ()) {
    Token *token = Tree ()->token ();
    return token && token->type () == TOK_STRUCT;
  }
  return false; 
}

CClassInfo *CClassInfo::DefObject () const {
  CObjectInfo *o = (CObjectInfo*)this;
  if (o->NextObject() != o) {
    do {
      CTree* tree = o->Tree();
      if (tree && tree->NodeName () == CT_ClassDef::NodeId ()) {
        CClassInstance* ci = o->ClassInstance();
        if (! ci || ci->TemplateInstance ()->canInstantiate ()) {
          return o->ClassInfo ();
        }
      }
      o = o->NextObject ();
    } while (o != (CObjectInfo*)this);
  }
  return (CClassInfo*)this;
}

CProtection::Type CClassInfo::Accessibility (CObjectInfo *oi) const {

  // oi should be the declaration if there is a separate definition
  if (oi->NextObject () != oi && oi == oi->DefObject ())
    oi = oi->NextObject ();
    
  // TODO: Check for using declarations etc., which change the accessibility
  CProtection::Type result = CProtection::PROT_NONE;

  // lookup the object info in this class
  CScopeInfo *scope = oi->Scope ();
  if (oi->isTemplateInstance ())
    scope = oi->TemplateInstance ()->Template ()->Scope ();
  if (scope == this) {
    // found it here, the accessibility is the protection of the declaration
    result = oi->Protection ();
  }
  else {
    // no success in this class, let's try the base classes
    for (unsigned i = 0; i < BaseClasses (); i++) {
      CBaseClassInfo *base = BaseClass (i);
      // 11.2 Accessibility of base classes and base class members
      // If a class is declared to be a base class (clause 10) for another
      // class using the public access specifier, the public members of the
      // base class are accessible as public members of the derived class and
      // protected members of the base class are accessible as protected
      // members of the derived class. If a class is declared to be a base
      // class for another class using the protected access specifier, the
      // public and protected members of the base class are accessible as
      // protected members of the derived class. If a class is declared to be
      // a base class for another class using the private access specifier,
      // the public and protected members of the base class are accessible as
      // private members of the derived class99).
      CProtection::Type inheritance = base->Protection ();
      CProtection::Type base_access = base->Class ()->Accessibility (oi);
      if (base_access == CProtection::PROT_PRIVATE) {
        base_access = CProtection::PROT_NONE;
      }
      else if (base_access == CProtection::PROT_PROTECTED) {
        if (inheritance == CProtection::PROT_PRIVATE)
          base_access = CProtection::PROT_PRIVATE;
      }
      else if (base_access == CProtection::PROT_PUBLIC) {
        if (inheritance == CProtection::PROT_PRIVATE)
          base_access = CProtection::PROT_PRIVATE;
        else if (inheritance == CProtection::PROT_PROTECTED)
          base_access = CProtection::PROT_PROTECTED;
      }
      
      // 11.7.1 Multiple access
      // If a name can be reached by several paths through a multiple
      // inheritance graph, the access is that of the path that gives most
      // access.
      if (result == CProtection::PROT_NONE)
        result = base_access;
      else if (base_access < result)
        result = base_access;
    }
  }
  return result;
}

bool CClassInfo::overridesVirtual (const CFunctionInfo *fi) const {
  // first check all member functions of this class
  for (unsigned f = 0; f < Functions (); f++) {
    CFunctionInfo *tested = Function (f);
    if (tested->isVirtual () && tested->hasSameNameAndArgs (fi))
      return true;
  }
  // if we were not successful, the base classes have to be searched
  for (unsigned b = 0; b < BaseClasses (); b++) {
    if (BaseClass (b)->Class ()->overridesVirtual (fi))
      return true;
  }
  // nothing found
  return false;
}

// 8.5.1 Aggregates
// An aggregate is an array or a class (clause 9) with no userdeclared
// constructors (12.1), no private or protected nonstatic data members
// (clause 11), no base classes (clause 10), and no virtual functions (10.3).
bool CClassInfo::isAggregate () const {
  // check for base classes
  if (BaseClasses () > 0)
    return false;

  // check the functions
  for (unsigned f = 0; f < Functions (); f++) {
    CFunctionInfo *func = Function (f);
    if (func->isConstructor () && !func->isBuiltin ())
      return false;
    if (func->isVirtual ())
      return false;
  }

  // check the attributes
  for (unsigned a = 0; a < Attributes(); a++) {
    CAttributeInfo *attr = Attribute (a);
    if (!attr->isStatic () &&
        (attr->Protection () == CProtection::PROT_PRIVATE ||
         attr->Protection () == CProtection::PROT_PROTECTED))
      return false;
  }

  // this IS an aggregate
  return true;
}

/** Yields true if the class has no non-static data members other than
 *  bit-fields of length 0, no virtual member functions, no virtual
 *  base classes, and no base class B for which isEmpty() is false.
 *  \return true if empty class. */
bool CClassInfo::isEmpty () const {
  unsigned num = Attributes ();
  for (unsigned i = 0; i < num; i++) {
    CAttributeInfo* ai = Attribute (i);
    if (! ai->isStatic ()) {
      CTypeBitField* type = ai->TypeInfo ()->TypeBitField ();
      if (! type || type->Dimension () != 0)
        return false;
    }
  }
  num = Functions ();
  for (unsigned i = 0; i < num; i++) {
    if (Function (i)->isVirtual ())
      return false;
  }
  num = BaseClasses ();
  for (unsigned i = 0; i < num; i++) {
    CBaseClassInfo* bc = BaseClass (i);
    if (bc->isVirtual () || ! bc->Class ()->isEmpty ())
      return false;
  }
  return true;
}

/** Yields true if the class declares or inherits a virtual function. */
bool CClassInfo::isPolymorphic () const {
  unsigned num = Functions ();
  for (unsigned i = 0; i < num; i++) {
    if (Function (i)->isVirtual ())
      return true;
  }
  num = BaseClasses ();
  for (unsigned i = 0; i < num; i++) {
    if (BaseClass (i)->Class ()->isPolymorphic ())
      return true;
  }
  return false;
}

/** Yields true if the class has a pure virtual function. */
bool CClassInfo::isAbstract () const {
  unsigned num = Functions ();
  for (unsigned i = 0; i < num; i++) {
    if (Function (i)->isPureVirtual ())
      return true;
  }
  return false;
}

/** Yields true if the class has a virtual destructor. */
bool CClassInfo::hasVirtualDtor () const {
  unsigned num = Functions ();
  for (unsigned i = 0; i < num; i++) {
    CFunctionInfo* mf = Function (i);
    if (mf->isVirtual () && mf->isDestructor ())
      return true;
  }
  return false;
}


} // namespace Puma
