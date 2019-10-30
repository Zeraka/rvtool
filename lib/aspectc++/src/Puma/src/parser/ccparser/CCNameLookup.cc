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

#include "Puma/CCNameLookup.h"
#include "Puma/CTemplateParamInfo.h"
#include "Puma/CTemplateInstance.h"
#include "Puma/CNamespaceInfo.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CSemDatabase.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CEnumInfo.h"
#include "Puma/CUsingInfo.h"
#include "Puma/CMemberAliasInfo.h"
#include "Puma/CTree.h"
#include <string.h>     /* strcmp(), strncmp() */
#include <iostream>     /* cerr     */
#include <assert.h>     /* assert() */

namespace Puma {



CCNameLookup::CCNameLookup (ErrorSink &e, CStructure* curr_scope, bool allow_inst) : err(e) {
  pos           = 0;
  name          = 0;
  sname         = 0;
  flags         = 0;
  virtualbase   = 0;
  current_scope = curr_scope;
  allow_instantiate = allow_inst;
}



bool CCNameLookup::isAnonymous () const {
  return name && strncmp(name, "%anon", 5) == 0;
}



///////////////////////////////////////////////////////
// §3.4 name lookup ///////////////////////////////////
///////////////////////////////////////////////////////



void CCNameLookup::lookup (CT_SimpleName *sn, CStructure *scope,
                           bool nested, bool base_usings) { 
  virtualbase = 0;
  unsigned long num = Objects ();

  // §3.4.3 qualified name lookup
  if (sn->NodeName () == CT_QualName::NodeId () ||
      sn->NodeName () == CT_RootQualName::NodeId ()) {
    sname = sn->Name ();
    name = sname->Text ();
    pos = sname->token_node () ? sname->token_node ()->Number () : 0;
    CStructure *name_scope = getNameScope (sn, scope);
    if (!isAnonymous())
      lookup (name_scope, false, true);
    // §3.4.1 unqualified name lookup
  } else {
    sname = sn;
    name = sname->Text ();
    pos = sname->token_node () ? sname->token_node ()->Number () : 0;
    if (!isAnonymous())
      lookup (scope, nested, base_usings); 
  }

  // filter names hidden by dominating declarations
  // (§10.1.4 virtual inheritance)
  findMostDominant (num);

  // clean up
  flags = 0;
  virtualbase = 0;
}



// §10.1.4 virtual inheritance, find most dominant name
void CCNameLookup::findMostDominant (unsigned long num) {
  // start a simple tournament to filter names dominated by others
  for (long i = (long)Objects (); i > (long)num; i--) {
    CObjectInfo *champion = Object (i-1);
    CClassInfo *champion_vb = virtualbases.lookup (i-1);
    // let the champion face all other candidates 
    if (champion_vb && champion->isClassMember ()) {
      for (long j = (long)Objects (); j > (long)num; j--) {
        CObjectInfo *challenger = Object (j-1);
        CObjectInfo *challenger_os = objscopes.lookup (j-1);
        // check if challenger dominates the champion
        if (challenger != champion && challenger_os && 
            challenger_os->ClassInfo () && challenger->isClassMember ()) {
          if (dominates (challenger_os, champion_vb)) {
            // challenger has won, remove the champion
            objects.remove (i-1);
            virtualbases.remove (i-1);
            objscopes.remove (i-1);
            i--; j--;
            break;
          }
        }
      }
    }
  }
}



// check if the first candidate dominates the second
bool CCNameLookup::dominates (CObjectInfo *object_scope, CObjectInfo *virtual_base) {
  // if the second candidate is in a subobject of the first 
  // candidate, the first candidate hides the second
  if (isSameSubObject ((CClassInfo*)object_scope, (CClassInfo*)virtual_base)) {
    return true;
  }
  return false;
}



bool CCNameLookup::isSameSubObject (CClassInfo *ci, CClassInfo *bc1) {
  if (ci && bc1) {
    for (unsigned i = ci->BaseClasses (); i > 0; i--) {
      CBaseClassInfo *bci = ci->BaseClass (i-1);
      CClassInfo *bc2 = bci->Class ();
      if (bc2) {
        if (bci->isVirtual () && (*bc1 == *bc2)) {
          return true;
        } else if (isSameSubObject (bc2, bc1)) {
          return true;
        }
      }
    }
  }
  return false;
}



CStructure *CCNameLookup::getNameScope (CT_SimpleName *sn, CStructure *scope) const {
  CObjectInfo *info;
  int entries = sn->Entries ();
  if (entries > 1) {        // class or namespace scope
    info = ((CT_SimpleName*)sn->Entry (entries-2))->Object ();
    if (info) {
      if (info->isTemplateInstance () && ! info->TemplateInstance ()->canInstantiate ())
        info = info->TemplateInstance ()->Template ()->ObjectInfo ();
      if (info->Record () || info->NamespaceInfo ())
        scope = (CStructure*)info->DefObject ();
      else if (info->TypedefInfo () && info->TypeInfo ()->VirtualType ()->TypeRecord ())
        scope = (CStructure*)info->TypeInfo ()->VirtualType ()->TypeRecord ()->Record ()->DefObject ();
    }
  } else {                 // file scope
    while (scope->Parent () != scope)
      scope = scope->Parent ()->Structure ();
  }
  return scope;
}



// §3.4.1 unqualified name lookup
void CCNameLookup::lookup (CStructure *scope, bool nested, bool base_usings) {
  CClassInfo *cinfo, *bcinfo;
  CBaseClassInfo *baseclass;
  CObjectInfo *info;
  unsigned num;

  // consider scopes that are pseudo template instances
  CTemplateInstance* inst = scope->TemplateInstance ();
  if (inst && inst->isPseudoInstance () && inst->Template ()->ObjectInfo ()->Structure ()) {
    if (inst->canInstantiate () && allow_instantiate) {
      inst->instantiate (current_scope ? current_scope : scope);
    } else {
      scope = inst->Template ()->ObjectInfo ()->Structure ();
    }
  }

  // lookup unqualified name in current scope
  num = Objects ();
  unqualifiedLookup (scope, nested, base_usings);

  // handle template scope as if it wouldn't be an own scope
  if (scope->TemplateInfo ())
    num = Objects ();
  // name found, stop here
  else if (Objects () > num)
    return;

  // §9.1.2 the name of a class or union is also considered
  // a member of that class or union
  if (! (flags & IS_NAMESPACE) && scope->isRecord () &&
      ! scope->Name ().empty () && scope->Name () == name) {
    // template instances are not found by name lookup,
    // so add the template instead of the instance
    if (scope->isTemplateInstance ())
      scope = scope->TemplateInstance ()->Template ()->ObjectInfo ()->Structure ();
    addObject (scope, scope);
  }

  // §10.2 member name lookup
  // §10.2.2 lookup in base classes (if any) only if there 
  // is no declaration in current scope that hides potential 
  // declarations in base class sub-objects
  else if (scope->isClass () && base_usings) {
    // each of the base classes is looked up; the result is 
    // the set of declarations found in one or more base classes
    cinfo = scope->DefObject ()->ClassInfo ();
    for (unsigned i = cinfo->BaseClasses (); i > 0; i--) {
      baseclass = cinfo->BaseClass (i-1);
      // §10.1.4 virtual base classes, do not lookup twice
      // in the same subobject
      if (isDistinctSubObject(baseclass) &&
          (! (flags & IS_FRIEND) || ! baseclass->ClassPseudoInstance ())) {
        bcinfo = baseclass->Class ();
        if (bcinfo != cinfo) {
          CClassInfo *oldvirtualbase = virtualbase;
          if (baseclass->isVirtual ()) {
            virtualbase = bcinfo;
          }
          lookup (bcinfo, false, base_usings);
          virtualbase = oldvirtualbase;
        }
      }
    }
  }

  if (Objects () == num) {
    // §9.3.5 lookup in member function's class/namespace if function 
    // is defined outside of the class/namespace's body
    if (scope->QualifiedScope () && scope->QualifiedScope () != scope && nested)
      lookup (scope->QualifiedScope (), true, base_usings); 
  }

  if (Objects () == num) {
    // §14.6.1.7 template parameters can be hidden only by names 
    // of base classes or (direct and inherited) class members
    info = scope->TemplateParam (name);
    if (! (flags & IS_NAMESPACE) && info)
      addObject (info, scope);
  }

  // continue in parent scopes if nested and nothing was found here
  if (Objects () == num && nested) {
    info = scope; scope = scope->Parent ()->Structure ();
    if (scope && scope != info)
      // if looking up a friend search only until the nearest
      // non-class non-template scope
      if (! (flags & IS_FRIEND) || info->Record () || info->TemplateInfo () ||
          (info->NamespaceInfo () && info->NamespaceInfo ()->aroundInstantiation ()))
        lookup (scope, nested, base_usings);
  }
}



// §10.1.4 virtual base classes
bool CCNameLookup::isDistinctSubObject (CBaseClassInfo *bcinfo) {
  // check if the base class is a distinct sub-object of a class,
  // i.e. there is at least one non-virtual base class of the 
  // same class type
  bool is_virtual = bcinfo->isVirtual ();
  CClassInfo *cinfo = bcinfo->Class ();
  // get the previous base class declaration (if there is any)
  BaseClassMap::iterator prev = baseclasses.find (cinfo);
  if (prev != baseclasses.end ()) {
    if (is_virtual) {
      // if both, this and the previous base class declaration, 
      // are virtual, this sub-object is not distinct but 
      // the same as for the previous base class declaration
      if (prev->second == true) {
        // sub-object not distinct
        return false;
      }
      // remember that there is already a virtual base class
      // of that class type
      prev->second = true;
    }
  } else {
    // first occurrence of this base class during lookup 
    baseclasses[cinfo] = is_virtual;
  }
  // sub-object is distinct
  return true;
}



///////////////////////////////////////////////////////
// usual unqualified name lookup in current scope /////
///////////////////////////////////////////////////////



void CCNameLookup::unqualifiedLookup (CStructure *scope, bool nested, bool base_usings) {
  CObjectInfo *last = 0, *tmp;
  CStructure *ns;
  unsigned num;

  num = Objects ();
  if (flags & IS_TYPE) {
    unqualTypeLookup (scope);
  } else {
    if (scope->isNamespace ()) {
      tmp = unqualLookup (scope, nested);
      // name hiding mechanism
      if (Objects () > num)
        last = 0;
      else if (! last)
        last = tmp;
    } else
      last = unqualLookup (scope, nested);

    // visible class, union, or enum name 
    if (last)
      addObject (last, scope);
  }

  // §3.4.1.2 consider declarations in namespaces nominated 
  // by using-directives (if any) in namespace and local scope
  if (base_usings) {
    if (scope->isNamespace ()) {
      ns = scope;
      do { // consider the namespace extensions too
        lookupUsings (ns);
        ns = (CStructure*)ns->NextObject ();
      } while (ns != scope);
    } else if (scope->isLocalScope ())
      lookupUsings (scope);
  }
}



// usual unqualified lookup
CObjectInfo *CCNameLookup::unqualLookup (CStructure *scope, bool nested) {
  CObjectInfo *info, *last = 0, *pinfo;
  CFunctionInfo *finfo;
  CTemplateInfo *tpl;
  bool is_class;

  is_class = scope->isClass ();
  tpl = scope->TemplateInfo ();
  scope = scope->DefObject ()->Structure ();

  ObjectsIter sameNameListIter = scope->ObjectInfoMap ().find (name);

  if (sameNameListIter != scope->ObjectInfoMap ().end ()) {
    CStructure::ObjectInfoList &sameNameList = sameNameListIter->second;

    for (ObjectListRIter riter = sameNameList.rbegin (); 
         riter != sameNameList.rend (); riter++) {
      info = pinfo = *riter;

      // template instances are never found by name lookup
      if (info->isTemplateInstance ())
        continue;

      // do not find templates in template scope; templates are introduced
      // e.g. into the surrounding scope and will be found there
      if (tpl && tpl->ObjectInfo () == info)
        continue;

      // §7.3.3.12 base class member functions introduced in a derived 
      // class by a using declaration can be hidden by direct member 
      // functions of the derived class that have the same name and
      // parameter types
      if (info->MemberAliasInfo ()) {
        if (flags & NO_MEMBER_ALIAS)
          continue;

        info = info->MemberAliasInfo ()->Member ();
        if (is_class && info->FunctionInfo ()) {
          // check for direct member functions with the same parameter types
          bool hidden = false;
          for (ObjectListRIter rfiter = sameNameList.rbegin (); rfiter != sameNameList.rend (); rfiter++) {
            finfo = (*rfiter)->FunctionInfo ();
            if (finfo && finfo->hasSameNameAndArgs ((CFunctionInfo*)info)) {
              hidden = true;
              break;
            }
          }
          if (hidden)
            continue;
        }
      // members defined qualified in a scope will not be introduced
      // in this scope, i.e. they are not visible here
      } else if (info->QualifiedScope () && *info->QualifiedScope () != *scope)
        continue;

      if (info->Name ().empty () || /*info->Name () != name ||*/ ! knownHere (pinfo)) 
        continue;

      // §3.3.7 name hiding
      if (! info->Record () && ! info->EnumInfo ()) {
        last = 0;

        if (! info->FunctionInfo ()) {
          // match namespaces only if searching for a namespace
          if ((flags & IS_NAMESPACE) && ! info->NamespaceInfo ())
            continue;

          // lookup succeeded; break
          addObject (info, scope);
          break;
        // don't break due to be possibly overloaded by other 
        // function declarations
        // §12.1.2 constructors (and destructors) have no name
        // and therefore are never found by name lookup
        // §13.3.1.2.3 ignore class member functions when looking
        // up unqualified operator names (non-member candidates)
        } else if (! info->FunctionInfo ()->isConstructor () &&
                   /*! info->FunctionInfo ()->isDestructor () &&*/
                   ! ((flags & NO_MEMBER_FCTS) && info->isClassMember ())) {
          // §7.3.1.2.3 friend functions declared first in class scope
          // are not found unless explicitly declared in namespace scope
          // except if a previous declaration is looked up (=> ! nested)
          // or associated namespaces are considered
          if (((flags & ASS_NS_LOOKUP) && ! nested && scope->NamespaceInfo ()) || 
              ! invisibleFriend (info)) {
            addObject (info, scope);
          }
        }
      // don't break due to be possibly hidden by an object, 
      // function, or enumerator
      // skip forward declarations of the form `... class-key identifier ...'
      // because they don't hide entities declared in enclosing scopes
      } else if (! last && (info->Record () ? 
                 ! info->Record ()->isHiddenForwardDecl () : true)) {
        last = info;
      }
    }
  }

  return last;
}



// unqualified lookup for type names
void CCNameLookup::unqualTypeLookup (CStructure *scope) {
  if (flags & IS_NAMESPACE)
    return;

  CObjectInfo *info, *pinfo;
  bool is_class = scope->isClass ();

  scope = scope->DefObject ()->Structure ();

  ObjectsIter sameNameListIter = scope->ObjectInfoMap ().find (name);
  if (sameNameListIter != scope->ObjectInfoMap ().end ()) {
    CStructure::ObjectInfoList& sameNameList = sameNameListIter->second;

    for (ObjectListRIter riter = sameNameList.rbegin (); 
         riter != sameNameList.rend (); riter++) {
      info = pinfo = *riter;

      // §7.3.3.12 names of base class members introduced in a derived 
      // class by a using declaration can be hidden by direct members 
      // of the derived class
      if (info->MemberAliasInfo ()) {
        if (is_class)
          continue;
        info = info->MemberAliasInfo ()->Member ();
      // members defined qualified in a scope will not be introduced
      // in this scope, i.e. they are not visible here
      } else if (info->QualifiedScope () && info->QualifiedScope () != scope)
        continue;

      // consider types only
      if (! (info->Record () || info->EnumInfo () || info->TypedefInfo ()))
        continue;
      if (! info->Name ())
        continue;
      if (info->Name () != name)
        continue;
      if (! knownHere (pinfo))
        continue;

      // §3.3.7 name hiding; names of types otherwise hidden 
      // are successfully looked up by not consider non-type 
      // declarations
      addObject (info, scope);
      return;
    }
  }

  // §7.3.3.12 lookup member alias names (introduced by using 
  // declarations) if not hidden by direct class members
  if (is_class) {

    if (sameNameListIter != scope->ObjectInfoMap ().end ()) {
      CStructure::ObjectInfoList &sameNameList = sameNameListIter->second;

      for (ObjectListRIter riter = sameNameList.rbegin (); 
           riter != sameNameList.rend (); riter++) {
        info = pinfo = *riter;

        // consider member aliases only
        if (! info->MemberAliasInfo ())
          continue;

        info = info->MemberAliasInfo ()->Member ();
        if ((info->Record () || info->EnumInfo () || info->TypedefInfo ()) &&
            ! info->Name ().empty () && info->Name () == name &&
            knownHere (pinfo)) {
          addObject (info, scope);
          return;
        }
      }
    }

  }
}



CStructure* CCNameLookup::getNonTemplateScope (CStructure* scope) const {
  // skip all template and template instance scopes
  scope = scope->Parent ()->Structure ();
  while (scope->TemplateInfo () ||
         (scope->NamespaceInfo () && scope->NamespaceInfo ()->aroundInstantiation ()))
    scope = scope->Parent ()->Structure ();
  return scope;
}



bool CCNameLookup::invisibleFriend (CObjectInfo *info) const {
  // §7.3.1.2.3 friend functions declared first in class scope
  // are not found unless explicitly declared in namespace scope
  CFunctionInfo *finfo = (CFunctionInfo*)info;
  bool is_friend = false, visible = false;
  do {
    if (getNonTemplateScope (finfo)->isNamespace ())
      visible = true;
    if (finfo->AssignedScope ())
      is_friend = true;
    finfo = (CFunctionInfo*)finfo->NextObject ();
  } while (finfo != info);
  return is_friend && !visible;
}



bool CCNameLookup::knownHere (CObjectInfo *info) const {
  CT_Token *token;
  // if in template instance, then it is known
  CObjectInfo *curr = info;
  while (curr) {
    if (curr->isTemplateInstance ())
      return true;
    if (curr == curr->Scope ())
      break;
    curr = curr->Scope ();
  }
  // is known if declared before the current position
  if (pos && info->Tree ()) {
    token = info->Tree ()->token_node ();
    if (token && token->Number () > pos)
      return false;
  }
  return true;
}



///////////////////////////////////////////////////////
// §3.4.1.2 namespace member lookup ///////////////////
///////////////////////////////////////////////////////



void CCNameLookup::lookupUsings (CStructure *scope) {
  CObjectInfo *info;
  CUsingInfo *uinfo;

  // §7.3.4.4 lookup name in ALL nominated namespaces
  for (unsigned i = scope->Usings (); i > 0; i--) {
    uinfo = scope->Using (i-1);
    if (! knownHere (uinfo))
      continue;

    info = uinfo->Namespace ();
    if (! namespaceVisited (info))
      lookup (info->Structure (), false, true);

    // same name declared in different namespaces and not all 
    // of it are functions => ambiguous use of that name
    // !!! CORRELATING CHECK TO BE IMPLEMENTED !!!
  }
}



bool CCNameLookup::namespaceVisited (CObjectInfo *nsinfo) {
  for (long i = namespaces.length (); i > 0; i--)
    if (namespaces[i-1] == nsinfo)
      return true;
  namespaces.append (nsinfo);
  return false;
}


} // namespace Puma
