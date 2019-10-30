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

#include "Puma/CRecord.h"
#include "Puma/CClassInfo.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CAttributeInfo.h"
#include "Puma/CTemplateInstance.h"
#include "Puma/CBaseClassInfo.h"
#include "Puma/CTree.h"
#include "Puma/CSemDatabase.h"
#include <string.h>

namespace Puma {


CRecord::~CRecord () {
////  for (unsigned i = 0; i < Users (); i++) 
////    User (i)->removeType (this);
//  CScopeInfo *parent = Parent ();
//  if (parent && parent->Structure ())
//    parent->Structure ()->removeType (this);
//  if (AssignedScope ())
//    AssignedScope ()->removeType (this);
//  if (_TemplateInfo)
//    _TemplateInfo->ObjectInfo ((CObjectInfo*)0);
}

bool CRecord::isComplete (unsigned long pos) const {
  CRecord *record = (CRecord*)DefObject ();
  if (record->isDefined ()) { 
    if (pos == 0)
      return true;
    CObjectInfo *info = record;
    if (info->isTemplateInstance ()) {
      info = info->TemplateInstance ()->Template ()->ObjectInfo ();
    }
    CT_Token *token = info->Tree ()->token_node ();
    if (token && token->Number () <= pos)
      return true;
  }
  return false;
}

bool CRecord::isDefined () const { 
  return Tree () && (Tree ()->NodeName () == CT_ClassDef::NodeId () ||
                     Tree ()->NodeName () == CT_UnionDef::NodeId ()); 
}

/** Check if this is a trivial class/union */
bool CRecord::isTrivial () const {
  return hasTrivialCtor () && hasTrivialCopy ();
}

/** Yields true if the class has a trivial copy assignment operator. */
bool CRecord::hasTrivialAssign () const {
  // §12.8p11
  // A copy assignment operator for class X is trivial if it is implicitly declared and if:
  // — class X has no virtual functions (10.3) and no virtual base classes (10.1), and
  // — each direct base class of X has a trivial copy assignment operator, and
  // — for all the nonstatic data members of X that are of class type (or array thereof),
  //   each such class type has a trivial copy assignment operator.
  bool oper_found = false;
  unsigned i, num = Functions ();
  for (i = 0; i < num; i++) {
    CFunctionInfo* mf = Function (i);
    // virtual function
    if (mf->isVirtual ())
      return false;
    // implicit assignment operator
    if (! oper_found && mf->isBuiltin () && mf->isCopyAssignOperator ())
      oper_found = true;
  }
  // no implicit assignment operator found
  if (! oper_found)
    return false;

  // check if base classes have a trivial assignment operator and are not virtual
  CClassInfo* c = ClassInfo ();
  num = c ? c->BaseClasses () : 0;
  for (i = 0; i < num; i++) {
    CBaseClassInfo* bc = c->BaseClass (i);
    if (bc->isVirtual () || ! bc->Class ()->hasTrivialAssign ())
      return false;
  }

  // check non-static data members
  num = Attributes ();
  for (i = 0; i < num; i++) {
    CAttributeInfo* ai = Attribute (i);
    if (! ai->isStatic ()) {
      CTypeInfo* type = ai->TypeInfo ();
      // get underlying type of array
      while (type->isArray ())
        type = type->VirtualType ()->BaseType ();
      // check if data member has trivial assignment operator
      if (type->isClassOrUnion ()) {
        CTypeRecord* tr = type->VirtualType ()->TypeRecord ();
        if (tr && tr->Record () && ! tr->Record ()->hasTrivialAssign ()) {
          // data member with non-trivial assignment operator found
          return false;
        }
      }
    }
  }
  return true;
}

/** Yields true if the class has a trivial copy constructor. */
bool CRecord::hasTrivialCopy () const {
  // §12.8p6
  // A copy constructor for class X is trivial if it is implicitly declared and if:
  // — class X has no virtual functions (10.3) and no virtual base classes (10.1), and
  // — each direct base class of X has a trivial copy constructor, and
  // — for all the non-static data members of X that are of class type (or array thereof),
  //   each such class type has trivial copy constructor.
  bool ctor_found = false;
  unsigned i, num = Functions ();
  for (i = 0; i < num; i++) {
    CFunctionInfo* mf = Function (i);
    // virtual function
    if (mf->isVirtual ())
      return false;
    // implicit copy constructor
    if (! ctor_found && mf->isBuiltin () && mf->isCopyConstructor ())
      ctor_found = true;
  }
  // no implicit default ctor found
  if (! ctor_found)
    return false;

  // check if base classes have a trivial copy constructor and are not virtual
  CClassInfo* c = ClassInfo ();
  num = c ? c->BaseClasses () : 0;
  for (i = 0; i < num; i++) {
    CBaseClassInfo* bc = c->BaseClass (i);
    if (bc->isVirtual () || ! bc->Class ()->hasTrivialCopy ())
      return false;
  }

  // check non-static data members
  num = Attributes ();
  for (i = 0; i < num; i++) {
    CAttributeInfo* ai = Attribute (i);
    if (! ai->isStatic ()) {
      CTypeInfo* type = ai->TypeInfo ();
      // get underlying type of array
      while (type->isArray ())
        type = type->VirtualType ()->BaseType ();
      // check if data member has trivial copy ctor
      if (type->isClassOrUnion ()) {
        CTypeRecord* tr = type->VirtualType ()->TypeRecord ();
        if (tr && tr->Record () && ! tr->Record ()->hasTrivialCopy ()) {
          // data member with non-trivial copy constructor found
          return false;
        }
      }
    }
  }
  return true;
}

/** Yields true if the class has a trivial default constructor. */
bool CRecord::hasTrivialCtor () const {
  // §12.1p5
  // A constructor is trivial if it is an implicitly-declared default constructor and if:
  // — its class has no virtual functions (10.3) and no virtual base classes (10.1), and
  // — all the direct base classes of its class have trivial constructors, and
  // — for all the nonstatic data members of its class that are of class type (or array
  //   thereof), each such class has a trivial constructor.
  bool ctor_found = false;
  unsigned i, num = Functions ();
  for (i = 0; i < num; i++) {
    CFunctionInfo* mf = Function (i);
    // virtual function
    if (mf->isVirtual ())
      return false;
    // implicit default constructor
    if (! ctor_found && mf->isBuiltin () && mf->isDefaultConstructor ())
      ctor_found = true;
  }
  // no implicit default ctor found
  if (! ctor_found)
    return false;

  // check if base classes have a trivial constructor and are not virtual
  CClassInfo* c = ClassInfo ();
  num = c ? c->BaseClasses () : 0;
  for (i = 0; i < num; i++) {
    CBaseClassInfo* bc = c->BaseClass (i);
    if (bc->isVirtual () || ! bc->Class ()->hasTrivialCtor ())
      return false;
  }

  // check non-static data members
  num = Attributes ();
  for (i = 0; i < num; i++) {
    CAttributeInfo* ai = Attribute (i);
    if (! ai->isStatic ()) {
      CTypeInfo* type = ai->TypeInfo ();
      // get underlying type of array
      while (type->isArray ())
        type = type->VirtualType ()->BaseType ();
      // check if data member has trivial ctor
      if (type->isClassOrUnion ()) {
        CTypeRecord* tr = type->VirtualType ()->TypeRecord ();
        if (tr && tr->Record () && ! tr->Record ()->hasTrivialCtor ()) {
          // data member with non-trivial constructor found
          return false;
        }
      }
    }
  }
  return true;
}

/** Yields true if the class has a trivial destructor. */
bool CRecord::hasTrivialDtor () const {
  // §12.4p3
  // A destructor is trivial if it is an implicitly-declared destructor and if:
  // — all of the direct base classes of its class have trivial destructors and
  // — for all of the non-static data members of its class that are of class type
  //   (or array thereof), each such class has a trivial destructor.
  unsigned i, num = Functions ();
  for (i = 0; i < num; i++) {
    CFunctionInfo* mf = Function (i);
    if (mf->isDestructor () && mf->isBuiltin ())
      break;
  }
  // no implicit destructor found
  if (i == num)
    return false;

  // check if base classes have a trivial destructor
  CClassInfo* c = ClassInfo ();
  num = c ? c->BaseClasses () : 0;
  for (i = 0; i < num; i++)
    if (! c->BaseClass (i)->Class ()->hasTrivialDtor ())
      return false;

  // check non-static data members
  num = Attributes ();
  for (i = 0; i < num; i++) {
    CAttributeInfo* ai = Attribute (i);
    if (! ai->isStatic ()) {
      CTypeInfo* type = ai->TypeInfo ();
      // get underlying type of array
      while (type->isArray ())
        type = type->VirtualType ()->BaseType ();
      // check if data member has trivial dtor
      if (type->isClassOrUnion ()) {
        CTypeRecord* tr = type->VirtualType ()->TypeRecord ();
        if (tr && tr->Record () && ! tr->Record ()->hasTrivialDtor ()) {
          // data member with non-trivial destructor found
          return false;
        }
      }
    }
  }
  return true;
}

void CRecord::rejectParseDelayed () {
  long n = _DelayedLength.top ();
  for (long i = _Delayed.length (); i > n; i--)
    _Delayed.remove (i-1);
  _DelayedLength.pop ();
}

void CRecord::removeDelayed (CTree* tree) {
  _InstantiateDelayed.erase (tree->token());
  for (long i = 0; i < _Delayed.length (); i++) {
    if (_Delayed.lookup (i) == tree) {
      _Delayed.remove (i);
      break;
    }
  }
}


} // namespace Puma
