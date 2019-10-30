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

#include "Puma/CCAssocScopes.h"
#include "Puma/CTree.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CEnumInfo.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CRecord.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CTemplateParamInfo.h"
#include "Puma/CNamespaceInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CClassInstance.h"
#include "Puma/CUnionInstance.h"
#include "Puma/CFctInstance.h"

namespace Puma {


// §3.4.2 argument-dependent name lookup
void CCAssocScopes::determineAssocScopes (CT_ExprList *args) {
  CTypeInfo *type;
  int num;
  
  num = args->Entries ();
  for (int i = 0; i < num; i++) {
    type = args->Entry (i)->Type ();
    if (type)
      determineAssocScopes (type);    
  }
}


// §3.4.2.2 determine associated classes and namespaces
void CCAssocScopes::determineAssocScopes (CTypeInfo *type) {
  CTypeFunction *ftype;
  CTemplateInfo *tinfo;
  CStructure *info;
  CEnumInfo *einfo;
  long num;
  
  if (type->isPointerOrArray ())
    type = type->VirtualType ()->BaseType ();
  type = type->VirtualType ();
  if (type->isClass ()) {
    info = type->TypeClass ()->ClassInfo ();
    if (info) {
      if (info->ClassInstance ()) {
        tinfo = info->TemplateInstance ()->Template ();
        if (tinfo) {
          if (tinfo->Parent ()->ClassInfo ())
            classes.append (tinfo->Parent ()->ClassInfo ());
          determineAssocScopes (tinfo, info->TemplateInstance ()->PointOfInstantiation ());
        }
      } else {
        num = classes.length ();
        addBaseClasses (info->ClassInfo ());
        for (int i = num; i < classes.length (); i++)
          addEnclosingNamespace (classes[i]);
      }
    }
  } else if (type->isUnion ()) {
    info = type->TypeUnion ()->UnionInfo ();
    if (info) {
      if (info->UnionInstance ()) {
        tinfo = info->TemplateInstance ()->Template ();
        if (tinfo->Parent ()->ClassInfo ())
          classes.append (tinfo->Parent ()->ClassInfo ());
        determineAssocScopes (tinfo, info->TemplateInstance ()->PointOfInstantiation ());
        return;
      } else if (info->Parent ()->ClassInfo ())
        classes.append (info->Parent ()->ClassInfo ());
      addEnclosingNamespace (info);
    }
  } else if (type->isEnum ()) {
    einfo = type->TypeEnum ()->EnumInfo ();
    if (einfo) { 
      info = einfo->Scope ()->Structure ();
      if (info->ClassInfo ())
        classes.append (info->ClassInfo ());
      addEnclosingNamespace (info);
    }
  } else if (type->isFunction ()) {
    ftype = type->TypeFunction ();
    info = ftype->FunctionInfo ();
    if (info && info->FctInstance ()) {
      tinfo = info->TemplateInstance ()->Template ();
      if (tinfo->ObjectInfo ()->FunctionInfo ()->Record ())
        classes.append (tinfo->ObjectInfo ()->FunctionInfo ()->Record ());
      determineAssocScopes (tinfo, info->TemplateInstance ()->PointOfInstantiation ());
      return;        
    }
    determineAssocScopes (ftype->ReturnType ());
    determineAssocScopes (ftype->ArgTypes ());
  } else if (type->isMemberPointer ()) {
    info = type->TypeMemberPointer ()->Record ();
    if (info)
      determineAssocScopes (info->TypeInfo ());
    determineAssocScopes (type->VirtualType ()->
                          TypeMemberPointer ()->BaseType ());
  } 
}


// §3.4.2.2 determine associated scopes of template-ids
void CCAssocScopes::determineAssocScopes (CTemplateInfo *tinfo, CTree *n) {
  CTemplateParamInfo *tpinfo;
  CT_TemplateArgList *al;
  CTemplateInfo *ttpinfo;
  CT_TemplateName *name;
  CTypeInfo *type;
  unsigned num;

  addEnclosingNamespace (tinfo);

  if (n->NodeName () != CT_TemplateName::NodeId ())
    return;
  
  name = (CT_TemplateName*)n;
  al = name->Arguments ();
  num = (unsigned)al->Entries ();
  for (unsigned i = 0; i < tinfo->TemplateParams (); i++) {
    tpinfo = tinfo->TemplateParam (i);
    if (num > i)
      type = al->Entry ((int)i)->Type ();
    else if (tpinfo->DefaultArgument ())
      type = tpinfo->DefaultArgument ()->Type ();
    else
      continue;
    if (! type)
      continue;
      
    // template template argument
    if (tpinfo->isTemplate ()) { 
      ttpinfo = type->TypeRecord ()->Record ()->TemplateInfo ();
      addEnclosingNamespace (ttpinfo);
      if (ttpinfo->Parent ()->ClassInfo ())
        classes.append (ttpinfo->Parent ()->ClassInfo ());
    // type template argument
    } else if (tpinfo->isTypeParam ())
      determineAssocScopes (type);
  }
}


void CCAssocScopes::determineAssocScopes (CTypeList *argtypes) {
  for (unsigned i = 0; i < argtypes->Entries (); i++) 
    determineAssocScopes (argtypes->Entry (i));    
}


void CCAssocScopes::addEnclosingNamespace (CStructure *info) {
  if (info != 0) {
    while (! info->isNamespace () || info->NamespaceInfo ()->aroundInstantiation ())
      info = info->Parent ()->Structure ();
    addNamespace (info->NamespaceInfo ());
  }
}


void CCAssocScopes::addNamespace (CNamespaceInfo *info, bool add_inline_parent) {
  for (unsigned i = 0; i < Namespaces (); i++) 
    if (Namespace (i) == info)
      return;

  namespaces.append (info);
  
  // In argument-dependent lookup, when a namespace is added to the set of 
  // associated namespaces, its inline namespaces are added as well, and if 
  // an inline namespace is added to the list of associated namespaces, its 
  // enclosing namespace is added as well.
  if (info->isInline ()) {
    if (add_inline_parent)
      addEnclosingNamespace (info->Parent ()->Structure ());
  } else {
    for (unsigned i = 0; i < info->Namespaces (); i++) 
      if (info->Namespace (i)->isInline ())
        addNamespace (info->Namespace (i), false);
  }
}


void CCAssocScopes::addBaseClasses (CClassInfo *info) {
  classes.append (info);
  for (unsigned i = 0; i < info->BaseClasses (); i++)
    addBaseClasses (info->BaseClass (i)->Class ());
}


} // namespace Puma
