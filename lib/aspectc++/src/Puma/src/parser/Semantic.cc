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

#include "Puma/Unit.h"
#include "Puma/CTree.h"
#include "Puma/Array.h"
#include "Puma/PtrStack.h"
#include "Puma/ErrorSink.h"
#include "Puma/Semantic.h"
#include "Puma/ErrorSeverity.h"
#include "Puma/CSemDeclarator.h"
#include "Puma/CSemDeclSpecs.h"
#include "Puma/CFileInfo.h"
#include "Puma/CEnumInfo.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CClassInstance.h"
#include "Puma/CUnionInstance.h"
#include "Puma/CFctInstance.h"
#include "Puma/CClassInfo.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CLabelInfo.h"
#include "Puma/CLocalScope.h"
#include "Puma/CTypedefInfo.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CArgumentInfo.h"
#include "Puma/CAttributeInfo.h"
#include "Puma/CEnumeratorInfo.h"
#include "Puma/CMemberAliasInfo.h"
#include "Puma/CSemDatabase.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <iostream>

namespace Puma {


/*DEBUG*/int TRACE_UNDO=0;
/*DEBUG*/int TRACE_OBJS=0;
/*DEBUG*/int TRACE_FCT=0;
/*DEBUG*/int TRACE_SCOPES=0;
/*DEBUG*/int TRACE_TYPE=0;
/*DEBUG*/int SEM_DECL_SPECS=0;

Semantic::~Semantic () { 
  while (in_decl ())
    decl_end ();
}

void Semantic::init (CSemDatabase &db, Unit &unit) {
  _db = &db;
  _Anonymous = 0;
  _sem_decl_specs.reset ();
  _decl_specs.reset ();
  _in_decl.reset ();
  _in_decl.push (false);
  _in_param_decl_clause.reset ();
  _in_param_decl_clause.push (false);
  current_enum = (CEnumInfo*)0; 
  current_fct = (CStructure*)0; 
/*DEBUG*/if (TRACE_FCT) std::cout<<"FCT NIL(0): "<<(void*)current_fct<<std::endl;

  char *name = new char[strlen (unit.name ()) + 20];
  sprintf (name, "%%file %s", unit.name ());
  unsigned f = 0;
  for (; f < db.FileInfos (); f++)
    if (strcmp (db.FileInfo (f)->Name (), name) == 0)
      break;
  if (f < db.FileInfos ())      // File already parsed!?
    _file = db.FileInfo (f);
  else {
    _file = db.newFile ();
    _file->SourceInfo ()->FileInfo (_file);
    _file->Primary (&unit);
    _file->ScopeInfo ()->Parent (_file);
    _file->Name (name);
  }
  delete[] name;
  current_scope = _file;
  enter_scope (_file);
}

/*DEBUG*/void Semantic::Push (CObjectInfo *info) {
/*DEBUG*/  if (TRACE_OBJS) {
/*DEBUG*/    std::cerr<<"INSERT: "<<(void*)info<<" ("
/*DEBUG*/        <<(info->LocalScope()?"LocalScope":info->FunctionInfo()?"Function":
/*DEBUG*/           info->ClassInfo()?"Class":info->UnionInfo()?"Union":
/*DEBUG*/           info->AttributeInfo()?"Attribute":info->ArgumentInfo()?"Argument":
/*DEBUG*/           info->EnumInfo()?"Enum":info->EnumeratorInfo()?"Enumerator":
/*DEBUG*/           info->LabelInfo()?"Label":info->TypedefInfo()?"Typedef":
/*DEBUG*/           info->MemberAliasInfo()?"MemberAlias":info->TemplateInfo()?"Template":
/*DEBUG*/           info->FileInfo()?"File":"Namespace")
/*DEBUG*/        <<" "<<info->Name()<<")"<<std::endl;
/*DEBUG*/  }
/*DEBUG*/  PtrStack<CObjectInfo>::Push (info);
/*DEBUG*/}

void Semantic::Delete () {
  // Unlink the top semantic object
  CObjectInfo* info = Top ();
  if (info)
    info->Unlink ();
}

// Generate and create a new private (anonymous) name.
CTree *Semantic::PrivateName () {
  char name[100];
  sprintf (name, "%%anon%ld", _Anonymous);
  _Anonymous++;
  return new CT_PrivateName (name); 
}

void Semantic::undo (CTree *tree) {
  if (! tree) return;
  for (int i = 0; i < tree->Sons (); i++) 
    undo (tree->Son (i));

  /*DEBUG*/if (TRACE_UNDO) {
  /*DEBUG*/  std::cerr<<"UNDO: "<<tree->NodeName();
  /*DEBUG*/  if (tree->NodeName()==CT_Token::NodeId() && tree->token())
  /*DEBUG*/    std::cerr<<" '"<<tree->token()->text()<<"'";
  /*DEBUG*/  std::cerr<<"  ("<<(void*)tree<<")"<<std::endl;
  /*DEBUG*/}
  if (tree->NodeName () == CT_DeclSpecSeq::NodeId ()) {
    if (in_decl () && (sem_decl_specs ()->DeclSpecSeq () == tree))
      decl_end ();
  } 
}

void Semantic::common_settings (CObjectInfo *info, CTree *tree) {
  info->SourceInfo ()->FileInfo (_file);
  info->SourceInfo ()->StartToken (tree->token_node ());
  info->Tree (tree);
}

void Semantic::common_settings (CObjectInfo *info, CTree *tree, CSemDeclarator *csd) {
  common_settings (info, tree);
  if (info->Name ().empty () || info->Name () != DString(csd->Name ()->Text ()))
    info->Name (csd->Name ()->Text ());
  if (info->FctInstance ())
    info->FctInstance ()->RealTypeInfo (csd->Type ());
  else
    info->TypeInfo (csd->Type ());
  csd->Name ()->Object (info);
  csd->Name ()->setTypeRef (info->TypeInfo ());
  csd->Type ((CTypeInfo*)0);
}

/*****************************************************************************/
/*                                                                           */
/*                           State information                               */
/*                                                                           */
/*****************************************************************************/

void Semantic::decl_begin (CSemDeclSpecs *dss) { 
  /*DEBUG*/SEM_DECL_SPECS++;
  _sem_decl_specs.push (dss);
  _in_decl.push (true); 
}

void Semantic::decl_end () { 
  /*DEBUG*/SEM_DECL_SPECS--;
  delete sem_decl_specs ();
  _sem_decl_specs.pop ();
  _in_decl.pop (); 
}

void Semantic::finish_declarator () { 
  if (! _in_param_decl_clause.top ()) {
/*DEBUG*/if (TRACE_FCT) std::cout<<"FCT NIL(4): "<<(void*)current_fct<<std::endl;
    current_fct = 0;
  }
}

/*****************************************************************************/
/*                                                                           */
/*                           Scope information                               */
/*                                                                           */
/*****************************************************************************/

void Semantic::enter_scope (CStructure *scp) { 
  /*DEBUG*/if (TRACE_SCOPES) std::cerr<<(void*)this<<": +SCOPE: "<<(void*)scp<<" ("
  /*DEBUG*/    <<(scp->TemplateInfo()?"Template":scp->LocalScope()?"Local":scp->FunctionInfo()?"Function":
  /*DEBUG*/       scp->ClassInfo()?"Class":scp->UnionInfo()?"Union":"File")
  /*DEBUG*/    <<(scp->isTemplateInstance()?"Instance ":" ")<<(scp->Name()?scp->Name().c_str():"<?>")<<")"<<std::endl;
  scp->Parent (current_scope);
  current_scope = scp;
  Push (scp);
}

void Semantic::reenter_scope (CStructure *scp) { 
  /*DEBUG*/if (TRACE_SCOPES) std::cerr<<(void*)this<<": +SCOPE: "<<(void*)scp<<" ("
  /*DEBUG*/    <<(scp->TemplateInfo()?"Template":scp->LocalScope()?"Local":scp->FunctionInfo()?"Function":
  /*DEBUG*/       scp->ClassInfo()?"Class":scp->UnionInfo()?"Union":"File")
  /*DEBUG*/    <<(scp->isTemplateInstance()?"Instance ":" ")<<(scp->Name()?scp->Name().c_str():"<?>")<<")"<<std::endl;
  current_scope = scp;
}

void Semantic::leave_scopes () {
  // Leave all scopes entered up to here automatically!
  long len = Length ();
  for (long i = 0; i < len; i++) {
    CObjectInfo *info = Get (i); // get n'th item
    if (info && info->ScopeInfo () && info == current_scope) {
      /*DEBUG*/if (TRACE_SCOPES) std::cerr<<(void*)this<<": -SCOPE: "<<(void*)info<<" ("
      /*DEBUG*/    <<(info->TemplateInfo()?"Template":info->LocalScope()?"Local":info->FunctionInfo()?"Function":
      /*DEBUG*/       info->ClassInfo()?"Class":info->UnionInfo()?"Union":"File")
      /*DEBUG*/    <<(info->isTemplateInstance()?"Instance ":" ")<<(info->Name()?info->Name().c_str():"<?>")<<")"<<std::endl;
      current_scope = info->ScopeInfo ()->Parent ()->Structure ();
      break; // outermost scope found
    }
  }
  Forget ();
}

void Semantic::enter_local_scope () { 
  // create and enter new local (block) scope
  CLocalScope *info = _db->newLocalScope();
  enter_scope (info);
}

void Semantic::enter_param_decl_clause () { 
  // create and enter new function prototype scope
  CFunctionInfo *info = _db->newFunction();
  enter_scope (info);
  _in_param_decl_clause.push (true);
}

void Semantic::leave_param_decl_clause () { 
  // save function prototype scope if this 
  // is the outermost param_decl_clause
  _in_param_decl_clause.pop ();
  if (! _in_param_decl_clause.top () && ! current_fct) {
    current_fct = current_scope;
    /*DEBUG*/if (TRACE_FCT) std::cout<<"FCT SET(0): "<<(void*)current_fct<<std::endl;
  }
}

bool Semantic::in_param_decl_clause () {
  return _in_param_decl_clause.top ();
}


} // namespace Puma
