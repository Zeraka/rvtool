// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2003  The 'ac++' developers (see aspectc.org)
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

#include "PumaTrackerDog.h"
#include "IntroductionUnit.h"
#include "ModelBuilder.h"

#include "Puma/CFunctionInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CRecord.h"
#include "Puma/CTree.h"

using namespace Puma;

CObjectInfo *PumaTrackerDog::unique_object (CObjectInfo *obj) {
  unsigned long fpos = 0L;
  CObjectInfo *first = 0;
  CObjectInfo *curr = obj;
  do {
    // builtin objects might have a declaration, but the generated obj is ret.
    if (curr->isBuiltin ())
      return curr;
    CTree *tree = curr->Tree ();
    unsigned long cpos = tree->token_node ()->Number ();
    // if the current node is not a forward declaration, it will be returned
    if (tree->NodeName () == CT_ClassDef::NodeId () ||
        tree->NodeName () == CT_FctDef::NodeId ())
      return curr;
    // check if the current forward declaration was the first
    if (!first || cpos < fpos) {
      fpos = cpos;
      first = curr;
    }
    curr = curr->NextObject ();
  } while (curr != obj);
  // if there were only forward declarations, return the first
  return first;  
}


CFunctionInfo *PumaTrackerDog::filter_func (CFunctionInfo* fi) {
  CRecord *cls = fi->ClassScope ();
  
  if (fi->isBuiltin () || fi->isTemplate () ||
      (cls && cls->isTemplate ()) ||
      strncmp (fi->Name (), "%a", 2) == 0 ||
      strncmp (fi->Name (), "__a", 3) == 0 ||
      strcmp (fi->Name (), "aspectof") == 0 ||
      strcmp (fi->Name (), "aspectOf") == 0 ||
      fi != unique_object (fi))
    return 0;
  
  if (strcmp (fi->SourceInfo ()->FileName (), "<anonymous unit>") == 0)
    return 0;
  
  if (cls &&
      (strcmp (cls->QualName (), "__JoinPoint") == 0 ||
       strcmp (cls->QualName (), "AC::Action") == 0))
    return 0;

//  CSemDatabase &db = *(CSemDatabase*)fi->SemDB();
//  if (db.PointcutInfo (fi))
//    return 0;

  return fi;
}

void PumaTrackerDog::pre_visit (CTree *node) {
  const char *id = node->NodeName ();

  if (id == CT_Token::NodeId ())
    return;

  if (id == CT_FctDef::NodeId ())
    pre_FctDef ((CT_FctDef *)node);
  else if (id == CT_InitDeclarator::NodeId ())
    pre_InitDeclarator ((CT_InitDeclarator*)node);
  else if (id == CT_MembInitList::NodeId ())
    in_memb_init = true;
  else if (id == CT_MembPtrExpr::NodeId () ||
           id == CT_MembRefExpr::NodeId ())
    pre_MembPtrExpr ((CT_MembPtrExpr*)node);
  else if (node->IsSimpleName ())
    pre_SimpleName (node->IsSimpleName ());
  else if (id == CT_NamespaceDef::NodeId ())
    pre_NamespaceDef ((CT_NamespaceDef*)node);
  else if (id == CT_ClassDef::NodeId ())
    pre_ClassDef ((CT_ClassDef*)node);
}

void PumaTrackerDog::post_visit (CTree *node) {
  const char *id = node->NodeName ();
  CT_Call *call;
  if ((call = node->IsCall ()) != 0)
    pre_Call (call);
  else if (id == CT_FctDef::NodeId ())
    post_FctDef ((CT_FctDef *)node);
  else if (id == CT_InitDeclarator::NodeId ())
    post_InitDeclarator ((CT_InitDeclarator *)node);
  else if (id == CT_MembInitList::NodeId ())
    in_memb_init = false;
  else if (id == CT_ClassDef::NodeId ())
    post_ClassDef ((CT_ClassDef*)node);
}

void PumaTrackerDog::pre_ClassDef (CT_ClassDef *node) {
  if (node->Object () &&
      strcmp (node->Object ()->QualName (), "JoinPoint") == 0)
    prune ();
    
  if (node->BaseIntros ())
    visit (node->BaseIntros ());
}

void PumaTrackerDog::post_ClassDef (CT_ClassDef *node) {
  if (node->IntroMembers ())
    visit (node->IntroMembers ());
}

void PumaTrackerDog::pre_Call (CT_Call *node) {
  // ignore calls via function pointer, which cannot be resolved
  if (!node->Object ())
     return;
   
  CFunctionInfo *called = node->Object ()->FunctionInfo ();
  called = (CFunctionInfo*)unique_object (called);

  // conditions to ignore this call (join point):
  // - constructor/destructor calls (not yet implemented)
  // - calls to generated or special ac++ code
  if (called->isConstructor () || called->isDestructor () ||
      !filter_func (called))
    return;

  // for introduced code check the target file
  Unit *unit = (Unit*)node->token ()->belonging_to ();
  IntroductionUnit *intro_unit = IntroductionUnit::cast (unit);
  if (intro_unit) unit = intro_unit->final_target_unit ();

  // conditions to ignore this call (join point) ... continued:
  // - the calling code is not part of the project
  if (!db ().Project ()->isBelow (unit))
    return;

  CObjectInfo *caller = func_stack.top ();
  if (caller) {
    // call inside a function
    // TODO: in_member_init no longer used here. Was is really useless?
    _jpm.register_call (called, node, caller->DefObject (), local_id);
    local_id++;
  }
  else if (last_init_declarator) {
    // call outside a function: this must be a declaration in global scope with initializer
    caller = last_init_declarator->Object ();
    _jpm.register_call (called, node, caller->DefObject (), -1);
  }
  else {
    // here we must have a caller, otherwise there is a strange kind of call
    assert(false);
  }
}

void PumaTrackerDog::pre_FctDef (CT_FctDef *node) {
  CObjectInfo *func = node->Object ();

  if (func && func->FunctionInfo ()) {
    func_stack.push (func->FunctionInfo ());
    local_id = 0;
  }
  else
    prune ();
}

void PumaTrackerDog::post_FctDef (CT_FctDef *node) {
  func_stack.pop ();
}

void PumaTrackerDog::pre_InitDeclarator (CT_InitDeclarator *node) {
  last_init_declarator = node;
}

void PumaTrackerDog::post_InitDeclarator (CT_InitDeclarator *node) {
  last_init_declarator = 0;
}

bool PumaTrackerDog::register_access (CObjectInfo *obj, CTree *node) {
  // only track attributes if the tracker dog is configured to do it.
  if (!_track_attrs)
    return false;

  // is a relevent attribute accessed?
  if (!obj || !obj->AttributeInfo () || !obj->Scope () || 
      obj->Scope ()->isLocalScope () || obj->isAnonymous () ||
      (strncmp (obj->QualName (), "JoinPoint::", 11) == 0 ||
      strcmp (obj->Name (), "this") == 0) ||
      strcmp (obj->QualName (), "tjp") == 0)
    return false;

  if (last_init_declarator && last_init_declarator->Object () == obj)
    return false;

  Unit *unit = (Unit*)node->token ()->belonging_to ();
  while (unit->isMacroExp ())
    unit = ((MacroUnit*)unit)->CallingUnit ();

  if (!(IntroductionUnit::cast (unit) ||
      db ().Project ()->isBelow (unit)))
    return false;

  // register the accessing syntax tree
//#ifndef ACMODEL
//  // TODO: has to be implemented
//  _jpm.register_attr_access (obj->AttributeInfo (), node);
//#endif
  
  return true;
}

void PumaTrackerDog::pre_MembPtrExpr (CT_MembPtrExpr *node) {
  CObjectInfo *obj = node->Object ();
  
  // register the object if it is relevant
  if (!register_access (obj, node))
    return;
    
  // foo.x -> visit foo here
  visit (node->Son (0));
  
  CTree *name = node->Son (2);
  const char *id = name->NodeName ();
  if (id == CT_QualName::NodeId () ||
      id == CT_RootQualName::NodeId ()) {
    CT_QualName *qual_name = (CT_QualName*)name;
    for (int i = 0; i < qual_name->Entries () - 1; i++)
      visit (qual_name->Entry (i));
  }
  
  // don't visit all the children again
  prune ();
}

void PumaTrackerDog::pre_SimpleName (CT_SimpleName *node) {

  // register this access if it is relevant
  if (!register_access (node->Object (), node))
    return;

  const char *id = node->NodeName ();
  if (id == CT_QualName::NodeId () ||
      id == CT_RootQualName::NodeId ()) {
    CT_QualName *qual_name = (CT_QualName*)node;
    for (int i = 0; i < qual_name->Entries () - 1; i++)
      visit (qual_name->Entry (i));
    prune ();
  }
}

void PumaTrackerDog::pre_NamespaceDef (CT_NamespaceDef *node) {
  CObjectInfo *obj = node->Object ();
  if (obj && strcmp (obj->QualName (), "AC") == 0)
    prune ();
}
