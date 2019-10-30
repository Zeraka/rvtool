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

#include "Puma/CFunctionInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CTemplateInstance.h"
#include "Puma/CArgumentInfo.h"
#include "Puma/CLabelInfo.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CConstant.h"
#include "Puma/CTree.h"
#include "Puma/CSemDatabase.h"
#include <string.h>

namespace Puma {


CFunctionInfo::~CFunctionInfo () {
}

bool CFunctionInfo::isPureVirtual () const {
  CTree* init = Init ();
  return init && init->Value () && init->Value ()->Constant () &&
         init->Value ()->Constant ()->isNull ();
}

CTypeInfo *CFunctionInfo::ReturnType () const {
  return TypeInfo ()->ReturnType ();
}

bool CFunctionInfo::hasEllipsis () const {
  CTypeList *atl = TypeInfo ()->ArgTypes ();
  return (atl->Entries () && atl->Entry (atl->Entries ()-1)->is_ellipsis ());
}

CArgumentInfo *CFunctionInfo::Argument (const char *name) const { 
  for (unsigned i = 0; i < Arguments (); i++) 
    if (Argument (i)->Name ())
      if (! strcmp (Argument (i)->Name (), name)) 
        return Argument (i);
  return (CArgumentInfo*)0;
}

CLabelInfo *CFunctionInfo::Label (const char *name) const { 
  for (unsigned i = 0; i < Labels (); i++) 
    if (Label (i)->Name ())
      if (! strcmp (Label (i)->Name (), name)) 
        return Label (i);
  return (CLabelInfo*)0;
}

void CFunctionInfo::addArgument (CArgumentInfo *info) { 
  if (! info) return;
  _Arguments.append (info); 
  addObject (info);
}

void CFunctionInfo::addLabel (CLabelInfo *info) { 
  if (! info) return;
  _Labels.append (info); 
}

void CFunctionInfo::removeArgument (const CArgumentInfo *info) { 
  for (unsigned i = Arguments (); i > 0; i--) 
    if (Argument (i-1) == info) {
      _Arguments.remove (i-1); 
      removeObject (info);
      break;
    }
}

void CFunctionInfo::removeLabel (const CLabelInfo *info) { 
  for (unsigned i = Labels (); i > 0; i--) 
    if (Label (i-1) == info) {
      _Labels.remove (i-1); 
      break;
    }
}

CArgumentInfo *CFunctionInfo::newArgument () {
  CArgumentInfo *info = SemDB () ? SemDB ()->newArgument () : new CArgumentInfo;
  info->Scope (this);
  addArgument (info);
  return info;
}

CLabelInfo *CFunctionInfo::newLabel () {
  CLabelInfo *info = SemDB() ? SemDB()->newLabel() : new CLabelInfo;
  info->Scope (this);
  addLabel (info);
  return info;
}

void CFunctionInfo::deleteArgument (const CArgumentInfo *info) { 
  for (unsigned i = Arguments (); i > 0; i--) 
    if (Argument (i-1) == info) {
      removeObject (info);
      Argument (i-1)->Scope (0);
      delete (CArgumentInfo*)_Arguments[i-1];
      _Arguments.remove (i-1); 
      break;
    }
}

void CFunctionInfo::deleteLabel (const CLabelInfo *info) { 
  for (unsigned i = Labels (); i > 0; i--) 
    if (Label (i-1) == info) {
      Label (i-1)->Scope (0);
      delete (CLabelInfo*)_Labels[i-1];
      _Labels.remove (i-1); 
      break;
    }
}

bool CFunctionInfo::isDefined () const { 
  CFunctionInfo *info = DefObject ();
  return (info->Tree () && info->Tree ()->NodeName () == CT_FctDef::NodeId ());
}

bool CFunctionInfo::isFctDef () const { 
  return (Tree () && Tree ()->NodeName () == CT_FctDef::NodeId ());
}

CFunctionInfo *CFunctionInfo::DefObject () const {
  CObjectInfo *o = (CObjectInfo*)this;
  do {
    if (o->Tree () && o->Tree ()->NodeName () == CT_FctDef::NodeId ())
      if (! o->TemplateInstance () || o->TemplateInstance ()->canInstantiate ())
        return o->FunctionInfo ();
    o = o->NextObject ();
  } while (o != (CObjectInfo*)this);
  return (CFunctionInfo*)this;
}

CTree *CFunctionInfo::DefaultArgument (unsigned pos) const {
  CFunctionInfo *f = (CFunctionInfo*)this;
  do {
    if (f->Arguments () > pos && f->Argument (pos)->hasDefaultArg ())
      return f->Argument (pos)->Init ();
    f = f->NextObject ()->FunctionInfo ();
  } while (f != (CFunctionInfo*)this);
  return (CTree*)0;
}

bool CFunctionInfo::hasDefaultArgument (unsigned a) const {
  CFunctionInfo *f = (CFunctionInfo*)this;
  do {
    if (f->Arguments () > a && f->Argument (a)->hasDefaultArg ())
      return true;
    f = f->NextObject ()->FunctionInfo ();
  } while (f != (CFunctionInfo*)this);
  return false;
}

CRecord *CFunctionInfo::Record () const {
  return CObjectInfo::QualifiedScope () ? CObjectInfo::QualifiedScope ()->Record () : (CRecord*)0;
}

CNamespaceInfo *CFunctionInfo::Namespace () const {
  return CObjectInfo::QualifiedScope () ? CObjectInfo::QualifiedScope ()->NamespaceInfo () : (CNamespaceInfo*)0;
}

bool CFunctionInfo::hasSameNameAndArgs (const CFunctionInfo *fi) const {
  // first the names are compared
  if (Name () != fi->Name ())
    return false;

  // now the argument types are checked
  CTypeList *my_types    = TypeInfo ()->TypeFunction ()->ArgTypes ();
  CTypeList *other_types = fi->TypeInfo ()->TypeFunction ()->ArgTypes ();
  if (my_types->Entries () != other_types->Entries ())
    return false;
  for (unsigned i = 0; i < my_types->Entries (); i++) {
    if (*my_types->Entry (i) != *other_types->Entry (i))
      return false;
  }
  // all argument types are identical and the function name as well!
  return true;
}

bool CFunctionInfo::overridesVirtual () const {
  // the function has to be a non-static member function
  if (!isMethod () || isStaticMethod ())
    return false;

  CClassInfo *cls = TypeInfo ()->TypeFunction ()->Record ()->ClassInfo ();
  return cls->overridesVirtual (this);
}

/** Check if this is a default constructor. */
bool CFunctionInfo::isDefaultConstructor () const {
  // §12.1p5
  // A default constructor for a class X is a constructor of class X
  // that can be called without an argument.
  if (! isConstructor ())
    return false;
  // check the parameters
  for (unsigned i = 0; i < Arguments (); i++)
    if (! Argument (i)->hasDefaultArg ())
      return false;
  return true;
}

/** Check if this is a copy constructor. */
bool CFunctionInfo::isCopyConstructor () const {
  // §12.8p2
  // A non-template constructor for class X is a copy constructor if its
  // first parameter is of type X&, const X&, volatile X& or const volatile X&,
  // and either there are no other parameters or else all other parameters have
  // default arguments.
  if (! isConstructor () || isTemplate () || Arguments () == 0)
    return false;

  // get class
  CRecord* c = Record ();
  if (! c)
    return false;

  // check first parameter
  CTypeInfo* arg = Argument ((unsigned)0)->TypeInfo ();
  if (! arg->isAddress () ||
      ! arg->isClassOrUnion () ||
      *arg->VirtualType ()->TypeRecord () != *c->TypeInfo ())
    return false;

  // check the other parameters
  for (unsigned i = 1; i < Arguments (); i++)
    if (! Argument (i)->hasDefaultArg ())
      return false;
  return true;
}

/** Check if this is a copy assignment operator. */
bool CFunctionInfo::isCopyAssignOperator () const {
  // 12.8p9
  // A copy assignment operator X::operator= is a non-static non-template
  // member function of class X with exactly one parameter of type X, X&,
  // const X&, volatile X& or const volatile X&.
  if (! isOperator () || isStatic () || isTemplate () || Arguments () != 1)
    return false;

  // assignment operator
  if (Name ().c_str () && strcmp (Name ().c_str (), "operator =") != 0)
    return false;

  // get class
  CRecord* c = Record ();
  if (! c)
    return false;

  // check parameter
  CTypeInfo* arg = Argument ((unsigned)0)->TypeInfo ();
  if (! arg->isClassOrUnion ())
    return false;
  if (arg->isAddress () && *arg->VirtualType ()->TypeRecord () == *c->TypeInfo ())
    return true;
  if (arg->TypeRecord () && *arg->TypeRecord () == *c->TypeInfo ())
    return true;
  return false;
}

} // namespace Puma
