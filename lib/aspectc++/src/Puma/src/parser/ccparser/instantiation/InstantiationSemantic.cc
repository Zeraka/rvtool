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

#include "Puma/InstantiationSemantic.h"
#include "Puma/CTree.h"
#include "Puma/DelayedParseTree.h"
#include "Puma/Unit.h"
#include "Puma/Array.h"
#include "Puma/PtrStack.h"
#include "Puma/ErrorSink.h"
#include "Puma/ErrorSeverity.h"
#include "Puma/CCSemDeclarator.h"
#include "Puma/CSemDeclSpecs.h"
#include "Puma/CFileInfo.h"
#include "Puma/CEnumInfo.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CUsingInfo.h"
#include "Puma/CLabelInfo.h"
#include "Puma/CLocalScope.h"
#include "Puma/CTypedefInfo.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CArgumentInfo.h"
#include "Puma/CAttributeInfo.h"
#include "Puma/CEnumeratorInfo.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CTemplateParamInfo.h"
#include "Puma/CClassInstance.h"
#include "Puma/CUnionInstance.h"
#include "Puma/CFctInstance.h"
#include "Puma/CSyntax.h"
#include "Puma/CCSyntax.h"
#include "Puma/CSemDatabase.h"
#include "Puma/CCNameLookup.h"
#include "Puma/CMemberAliasInfo.h"
#include "Puma/CNamespaceInfo.h"
#include "Puma/CCSemExpr.h"
#include "Puma/CConstant.h"
#include "Puma/CCInstantiation.h"
#include "Puma/FileUnit.h"
#include "Puma/MacroUnit.h"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>

namespace Puma {

///*DEBUG*/extern int TRACE_FCT;
///*DEBUG*/extern int TRACE_OBJS;
///*DEBUG*/extern int TRACE_SCOPES;

// print semantic error messages

#define SEM_MSG(loc__,mesg__) \
  *_err << loc__->token ()->location () << mesg__ << endMessage

#define SEM_ERROR(loc__,mesg__) \
  SEM_MSG (loc__, sev_error << mesg__)

#define SEM_WARNING(loc__,mesg__) \
  SEM_MSG (loc__, sev_warning << mesg__)

#define SEM_MSG__prev_loc(info__) \
  if (info__->SourceInfo ()->StartToken ()) \
    *_err << info__->SourceInfo ()->StartToken ()->token ()->location () \
          << "previously defined here" << endMessage

#define SEM_ERROR__already_defined(loc__,name__,info__) \
  SEM_ERROR (loc__, "`" << *name__ << "' already defined"); \
  SEM_MSG__prev_loc(info__)

#define SEM_ERROR__duplicate(loc__,type__,name__) \
  SEM_ERROR (loc__, "duplicate " << type__ << " `" << *name__ << "'")

#define SEM_ERROR__invalid_in_param(loc__,what__,name__) \
  SEM_ERROR (loc__, what__ << " invalid in declaration of parameter `" \
    << *name__ << "'")

#define SEM_ERROR__redefinition(loc__,name__,info__) \
  SEM_ERROR (loc__, "redefinition of `" << *name__ << "'"); \
  SEM_MSG__prev_loc(info__)

#define SEM_ERROR__not_member(loc__,name__,class__) \
  SEM_ERROR (loc__, "`" << *name__ \
    << "' is not a member of `" << class__ << "'")

#define SEM_ERROR__no_matching_decl(loc__,name__,class__) \
  SEM_ERROR (loc__, "no matching declaration found for `" << *name__ \
    << "' in class `" << class__ << "'")

#define SEM_ERROR__unknown_ns(loc__,name__) \
  SEM_ERROR (loc__, "unknown namespace `" << *name__ << "'")

#define SEM_ERROR__not_class(loc__,name__) \
  SEM_ERROR (loc__, "base type `" << *name__ \
    << "' fails to be a struct or class type")

#define SEM_ERROR__invalid_param(loc__,what__,name__) \
  SEM_ERROR (loc__, "invalid " << what__ << \
    " in declaration of parameter `" << *name__ << "'")

#define SEM_ERROR__ambiguous_use(loc__,name__) \
  SEM_ERROR (loc__, "use of `" << *name__ << "' is ambiguous")

#define SEM_ERROR__cannot_declare(loc__,name__,scope__) \
  SEM_ERROR (loc__, "cannot declare `" << *name__ << \
    "' within `" << scope__->Name () << "'")

#define SEM_ERROR__conflicting_types(loc__,info__) \
  SEM_ERROR (loc__, "conflicting types for `" << \
    info__->Name () << "'"); \
  SEM_MSG__prev_loc(info__)

#define SEM_ERROR__not_base_class(loc__,class__) \
  SEM_ERROR (loc__, "`" << class__->Name () << \
    "' is not a base class of `" << current_scope->Name () << "'")



void InstantiationSemantic::parse_delayed(bool (CCSyntax::*rule)(), CTree *tree) {
//  CTree* node = getParent(syntax().provider().node());
  CTree* node = getParent((CTree*) syntax().provider().current_context());
  if (node && node->IsDelayedParse()) {
    CT_DelayedParse* dp = (CT_DelayedParse*) createDelayedParse (rule, tree);
    _outermost_class_def->addInstantiateDelayed(syntax().provider().current(), dp);
  } else {
    CCSemantic::parse_delayed(rule, tree);
  }
}

CTree* InstantiationSemantic::getParent(CTree* node, int n) {
  for (int i = 0; i < n && node; i++)
    node = node->Parent();
  return node;
}

CTree *InstantiationSemantic::non_type_param_without_init() {
  finish_declarator();
  if (!in_decl())
    return (CTree*)0;

  CT_NonTypeParamDecl *td = (CT_NonTypeParamDecl*) builder().non_type_param();
  CSemDeclSpecs *dsi = sem_decl_specs();
  CCSemDeclarator csd(_err, dsi->make_type(), td->Declarator());
  if (!csd.Name()) {
    builder().destroyNode(td);
    return (CTree*)0;
  }

  CTypeInfo *type = csd.Type();
  if (type->isFunction() || type->isArray()) {
    // template non-type parameter adjustment
    csd.Type(new CTypePointer(type->isArray() ? type->VirtualType()->BaseType() : type));
  }
  else if (! (type->isInteger() || type->isPointer() ||
              type->isAddress() || type->isMemberPointer() ||
              type->isTemplateParam())) {
    SEM_ERROR__invalid_param(td, "type", csd.Name());
    builder().destroyNode(td);
    return (CTree*)0;
  }

  csd.Name()->setType(csd.Type());
  csd.Type(0);

  if (dsi)
    decl_end();

  return td;
}

bool InstantiationSemantic::is_class_name() {
//  CTree* node = getParent(syntax().provider().node());
  CTree* node = getParent((CTree*) syntax().provider().current_context());
  if (is<CT_SimpleName>(node)) {
    CObjectInfo* obj = node->SemObject()->Object();
    if (obj && !obj->Record() && !obj->TypedefInfo() &&
        !(obj->TemplateParamInfo() && obj->TemplateParamInfo()->isTypeParam()))
      return false;
  }
  return true;
}

bool InstantiationSemantic::is_enum_name() {
//  CTree* node = getParent(syntax().provider().node());
  CTree* node = getParent((CTree*) syntax().provider().current_context());
  if (is<CT_SimpleName>(node)) {
    CObjectInfo* obj = node->SemObject()->Object();
    if (obj && !obj->EnumInfo())
      return false;
  }
  return true;
}

bool InstantiationSemantic::is_typedef_name() {
//  CTree* node = getParent(syntax().provider().node());
  CTree* node = getParent((CTree*) syntax().provider().current_context());
  if (is<CT_SimpleName>(node)) {
    CObjectInfo* obj = node->SemObject()->Object();
    if (obj && !obj->TypedefInfo() &&
        !(obj->TemplateParamInfo() && obj->TemplateParamInfo()->isTypeParam()))
      return false;
  }
  return true;
}

bool InstantiationSemantic::is_template_name() {
//  CTree* node = getParent(syntax().provider().node());
  CTree* node = getParent((CTree*) syntax().provider().current_context());
  if (is<CT_SimpleName>(node)) {
    CObjectInfo* obj = node->SemObject()->Object();
    if (obj && !obj->isTemplate() && !obj->isTemplateInstance())
      return false;
  }
  return true;
}

bool InstantiationSemantic::is_class_template_name() {
//  CTree* node = getParent(syntax().provider().node());
  CTree* node = getParent((CTree*) syntax().provider().current_context());
  if (is<CT_SimpleName>(node)) {
    CObjectInfo* obj = node->SemObject()->Object();
    if (obj && ((!obj->isTemplate() && !obj->isTemplateInstance()) || obj->FunctionInfo()))
      return false;
  }
  return true;
}

bool InstantiationSemantic::is_namespace_name() {
//  CTree* node = getParent(syntax().provider().node());
  CTree* node = getParent((CTree*) syntax().provider().current_context());
  if (is<CT_SimpleName>(node)) {
    CObjectInfo* obj = node->SemObject()->Object();
    if (obj && !obj->NamespaceInfo())
      return false;
  }
  return true;
}

bool InstantiationSemantic::is_this_instance() {
  if (tpl_inst) {
//    CTree* node = getParent(syntax().provider().node());
    CTree* node = getParent((CTree*) syntax().provider().current_context());
    if (is<CT_SimpleName>(node)) {
      CObjectInfo* obj = node->SemObject()->Object();
      if (obj && obj == tpl_inst->ObjectInfo())
        return true;
    }
  }
  return false;
}

void InstantiationSemantic::set_this_instance() {
  CTree* node = builder().Top();
  if (is<CT_SimpleName>(node)) {
    CT_SimpleName* sn = (CT_SimpleName*)node;
    sn->Object(tpl_inst->Instance());
    sn->setTypeRef(tpl_inst->Instance()->TypeInfo());
  }
}

bool InstantiationSemantic::is_this_specialization() {
  if (tpl_inst) {
//    CTree* node = getParent(syntax().provider().node(), 3);
    CTree* node = getParent((CTree*) syntax().provider().current_context(), 3);
    if (node) {
      if (is<CT_QualName>(node) || is<CT_RootQualName>(node))
        node = getParent(node);
      if (is<CT_FctDeclarator>(node))
        node = getParent(node);
      if (is<CT_ClassDef>(node) || is<CT_FctDef>(node)) {
        if (is<CT_ClassDef>(node))
          node = getParent(node, 2);
        if (node == syntax().source().root())
          return true;
      }
    }
  }
  return false;
}

CTree *InstantiationSemantic::introduce_function() {
  if (! parsing_fct_inst || ! tpl_inst || tpl_inst->Instance() != current_fct)
    return CCSemantic::introduce_function();

  // determine function return type
  CT_DeclSpecSeq *dss = (CT_DeclSpecSeq*)(builder().nodes() == 2 ? builder().get_node(0) : 0);
  CT_Declarator *d = (CT_Declarator*)(dss ? builder().get_node(1) : builder().get_node(0));
  if (dss && ! in_decl())
    return (CTree*)0;

  // determine function type
  CSemDeclSpecs *dsi = sem_decl_specs();
  CCSemDeclarator csd(_err, dsi->make_type(), d);
  if (! csd.Name() || ! csd.Type() || ! csd.Type()->VirtualType()->TypeFunction())
    return (CTree*)0;

  CFunctionInfo *info = (CFunctionInfo*)current_fct;
  CTypeInfo *type = csd.Type();
  type->VirtualType()->TypeFunction()->FunctionInfo(info);
  info->FctInstance()->RealTypeInfo(type);

  // create implicit `this' parameter
  addImplicitThisObject(info, type, d);

  CTree* name = csd.Name()->Name();
  if (info->isConversion() && name->NodeName() == CT_ConversionName::NodeId())
    info->ConversionType(((CT_ConversionName*)name)->TypeName()->Object()->TypeInfo());

  // enter function scope (re-used function prototype scope)
  reenter_scope(current_fct);
  current_fct = 0;

  csd.Name()->Object(info);
  csd.Name()->setTypeRef(type);
  if (! csd.Name()->Name()->Object()) {
    csd.Name()->Name()->Object(info);
    csd.Name()->Name()->setTypeRef(type);
  }
  csd.Type((CTypeInfo*)0);

  if (dss) decl_end();
  return d;
}

CTree *InstantiationSemantic::introduce_object (CTree*) {
  if (! parsing_fct_inst || ! tpl_inst || tpl_inst->Instance() != current_fct)
    return CCSemantic::introduce_object();

  if (! in_decl())
    return (CTree*)0;

  // determine type of declaration
  CT_InitDeclarator *id = (CT_InitDeclarator*)builder().init_declarator1();
  CSemDeclSpecs *dsi = sem_decl_specs();
  CCSemDeclarator csd(_err, dsi->make_type(), id);
  if (! csd.Name()) {
    delete id;
    return (CTree*)0;
  }

  // check if this is a function declaration
  CTypeInfo *type = csd.Type();
  if (dsi->declared(CT_PrimDeclSpec::PDS_TYPEDEF) || !type->isFunction() || type->isAddress())
    return CCSemantic::introduce_object(id);

  CFunctionInfo *info = (CFunctionInfo*)current_fct;
  type->VirtualType()->TypeFunction()->FunctionInfo(info);
  common_settings(info, id, &csd);

  // create implicit `this' parameter
  addImplicitThisObject(info, type, id);

  CTree* name = csd.Name()->Name();
  if (info->isConversion() && name->NodeName() == CT_ConversionName::NodeId())
    info->ConversionType(((CT_ConversionName*)name)->TypeName()->Object()->TypeInfo());

  id->Object(info);
  current_fct = 0;

  return id;
}

CTree *InstantiationSemantic::introduce_class() {
  if (! parsing_class_inst || ! tpl_inst || ! tpl_inst->Instance() || ! current_scope->isNamespace() ||
      tpl_inst->Instance()->TemplateInstance()->isInstantiated())
    return CCSemantic::introduce_class();

  CObjectInfo *info = tpl_inst->Instance();
  tpl_inst->insertInstance(info);

  // determine member access
  CT_ClassDef *cs = (CT_ClassDef*) builder().class_head1();
  if (cs->NodeName() == CT_UnionDef::NodeId()) {
    _protection.push(CProtection::PROT_PUBLIC);
  } else {
    if (cs->token()->type() == TOK_STRUCT)
      _protection.push(CProtection::PROT_PUBLIC);
    else
      _protection.push(CProtection::PROT_PRIVATE);
  }

  reenter_scope(info->Structure());

  common_settings(info, cs);
  cs->Name()->Object(info);
  cs->Name()->setTypeRef(info->TypeInfo());
  cs->Object(info);

  // semantic state to let parse function bodies and several initializers
  // later due to potential use of names of members that could not be
  // resolved untill the end of the class definition
  enter_class_def();

  return cs;
}


} // namespace Puma
