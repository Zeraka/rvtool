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
#include "Puma/CCSemantic.h"
#include "Puma/CSemDatabase.h"
#include "Puma/CCNameLookup.h"
#include "Puma/CMemberAliasInfo.h"
#include "Puma/CNamespaceInfo.h"
#include "Puma/CCSemExpr.h"
#include "Puma/CConstant.h"
#include "Puma/CCInstantiation.h"
#include "Puma/FileUnit.h"
#include "Puma/MacroUnit.h"
#include "Puma/InstantiationParser.h"
#include "Puma/InstantiationBuilder.h"
#include "Puma/InstantiationSyntax.h"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>

namespace Puma {


/*DEBUG*/extern int TRACE_FCT;
/*DEBUG*/extern int TRACE_OBJS;
/*DEBUG*/extern int TRACE_SCOPES;

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

#define STACK_INIT 5
#define STACK_INCR 5


CCSemantic::CCSemantic (CCSyntax &s, CCBuilder &b) :
 CSemantic (s, b),
 _syntax (s),
 _in_base_spec (STACK_INIT, STACK_INCR),
 _in_nested_name (STACK_INIT, STACK_INCR),
 _in_conv_type_id (STACK_INIT, STACK_INCR),
 _in_extern_decl (STACK_INIT, STACK_INCR),
 _protection (STACK_INIT, STACK_INCR),
 shared_nl (*_err),
 _in_template_arg_list (STACK_INIT, STACK_INCR),
 _in_template_param_list (STACK_INIT, STACK_INCR),
 parsing_entity_name (STACK_INIT, STACK_INCR),
 _in_expr (STACK_INIT, STACK_INCR),
 _postfix_expr (STACK_INIT, STACK_INCR),
 _in_memb_access_expr (STACK_INIT, STACK_INCR) {
  last_scope = 0;
  search_scope = 0;
  last_search_scope = 0;
  last_token_nl.tok = 0;
  last_token_nl.scope = 0;
  last_token_nl.nested = false;
  last_token_nl.nested_name = false;
  last_token_nl.base_spec = false;
  expect_template = false;
  dep_nested_name = false;
  _outermost_class_def = 0;
  _postfix_expr.push (0);
  _in_expr.push (false);
  _in_base_spec.push (false);
  _in_extern_decl.push (0);
  _in_nested_name.push (false);
  _in_conv_type_id.push (false);
  _in_memb_access_expr.push (MAO_NONE);
  _in_template_arg_list.push (0);
  _in_template_param_list.push (0);
  _protection.push (CProtection::PROT_NONE);
  parsing_class_inst = false;
  parsing_fct_inst = false;
  parsing_entity_name.push (false);
  tpl_inst = 0;
  _last_unit = 0;
  _last_body_skipped = false;
  real_inst = false;
  support_implicit_int = false;
}


void CCSemantic::init (CSemDatabase &db, Unit &unit) {
  CSemantic::init (db, unit);
  if (! _file->haveCCBuiltins ()) {
    _file->haveCCBuiltins (true);
    declareImplicitFcts ();
    declareNamespaceStd ();
  }
  if (!db.Project ()->config ().Option ("--pseudo-instances"))
    real_inst = true;
}


void CCSemantic::init (CSemDatabase &db, Unit &unit,
 CStructure *scope, bool fi, bool ci, CCInstantiation *cci) {
  CSemantic::init (db, unit);
  if (scope)
    current_scope = scope;
  parsing_fct_inst = fi;
  parsing_class_inst = ci;
  tpl_inst = cci;
  if (!db.Project ()->config ().Option ("--pseudo-instances"))
    real_inst = true;
}


/*****************************************************************************/
/*                                                                           */
/*                  Linkage and storage class determination                  */
/*                                                                           */
/*****************************************************************************/


void CCSemantic::enter_extern_decl (bool block_decl) {
  // language string is at the end of the parsed tokens
  CTree *lang = builder ().get_node (builder ().nodes () - (block_decl ? 2 : 1));
  if (lang && lang->token ()) {
    const char *str = lang->token ()->text ();
    unsigned len = strlen (++str);
    if (len) {
      char *literal = new char[len];
      strncpy (literal, str, len-1);
      literal[len-1] = '\000';
      _in_extern_decl.push (literal);
      return;
    }
  }
  _in_extern_decl.push (0);
}


// ���3.5 name linkage
CLinkage::Type CCSemantic::determine_linkage (CSemDeclSpecs *dss,
 SemObjType type, CObjectInfo *prev) const {
  // special: global: enumerator of external enum                  => external
  // special: global: named class/enum                              => external
  // special: global: unnamed class/enum in typedef                 => external
  // special: global: data member of anon union                     => internal
  // special: class: method|static data member|
  //          nested class/enum & external class                    => external
  // special: unnamed union on namespace scope: data member         => internal
  if (in_extern_decl ())
    return CLinkage::LINK_EXTERNAL;

  bool def_extern = dss->declared (CT_PrimDeclSpec::PDS_EXTERN);
  bool def_const = dss->declared (CT_PrimDeclSpec::PDS_CONST);
  if (type == ATTRIB || type == FCT) {
    if (current_scope->isNamespace ()) {
      if (dss->declared (CT_PrimDeclSpec::PDS_STATIC) ||
          (type == ATTRIB && def_const && ! def_extern &&
           (prev ? prev->Linkage () != CLinkage::LINK_EXTERNAL : true)))
        return CLinkage::LINK_INTERNAL;
      return CLinkage::LINK_EXTERNAL;
    } else if (current_scope->LocalScope () &&
               ! (type == ATTRIB && ! def_extern)) {
      if (prev && prev->Linkage () != CLinkage::LINK_NONE)
        return prev->Linkage ();
      else
        return CLinkage::LINK_EXTERNAL;
    } else if (current_scope->Record ()) {
      return CLinkage::LINK_EXTERNAL;
    }
  }
  return CLinkage::LINK_NONE;
}


// ���3.7 storage duration
CStorage::Type CCSemantic::determine_storage_class (CSemDeclSpecs *dss) const {
  // ���3.7.2 automatic storage duration
  if (current_scope->LocalScope ())
    if (! dss->declared (CT_PrimDeclSpec::PDS_STATIC) &&
        ! dss->declared (CT_PrimDeclSpec::PDS_EXTERN))
      return CStorage::CLASS_AUTOMATIC;
  // ���3.7.1 static storage duration
  return dss->declared (CT_PrimDeclSpec::PDS_THREAD) ?
         CStorage::CLASS_THREAD : CStorage::CLASS_STATIC;
}


// set the entity encoding
void CCSemantic::setLanguage (CObjectInfo *info) const {
  if (in_extern_decl ()) {
    const char *l = _in_extern_decl.top ();
    if (strcmp (l, "C") == 0) {
      info->Language ().Type (CLanguage::LANG_C);
    } else if (strcmp (l, "C++") == 0) {
      info->Language ().Type (CLanguage::LANG_CPLUSPLUS);
    } else {
      info->Language ().Type (CLanguage::LANG_OTHER, l);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*                                Helper                                     */
/*                                                                           */
/*****************************************************************************/


bool CCSemantic::isClassOrEnum (CObjectInfo *info) const {
  return info && (info->Record () || info->EnumInfo ());
}


void CCSemantic::deleteContainer (int p) {
  CTree *node = builder ().get_node (builder ().nodes () - p);
  if (node->NodeName () == Builder::Container::NodeId ())
    delete (Builder::Container*)node;
}


CStructure *CCSemantic::getNameScope (CT_SimpleName *name, bool ps) const {
  CStructure *scope = current_scope;
  int entries = name->Entries ();
  if (isQualName (name) && entries > 1)
    scope = getScope (((CT_SimpleName*)name->Entry (entries-2))->Object ());
  else if (name->NodeName () == CT_RootQualName::NodeId ())
    scope = _file;
  if (ps && scope && scope->TemplateInstance () &&
      ! scope->TemplateInstance ()->canInstantiate ())
    scope = scope->TemplateInstance ()->Template ()->ObjectInfo ()->Structure ();
  return scope ? scope : current_scope;
}


CStructure *CCSemantic::getFriendScope (bool skip_tpl_scope) const {
  CStructure *scope = current_scope;
  // nearest non-class, non-template-instance scope
  // Note: if skip_tpl_scope is false, then do not skip template
  // scopes because friends declared in a template scope are not
  // introduced into the enclosing  non-template scope until the
  // template is instantiated
  while (scope->Record () ||
         (skip_tpl_scope && scope->TemplateInfo ()) ||
         (scope->NamespaceInfo () && scope->NamespaceInfo ()->aroundInstantiation ()))
    scope = scope->Parent ()->Structure ();
  return scope;
}


CStructure *CCSemantic::getScope (CObjectInfo *info) const {
  if (! info)
    return 0;
  if (info->isTemplateInstance () && ! info->TemplateInstance ()->canInstantiate ())
    info = info->TemplateInstance ()->Template ()->ObjectInfo ();
  if (info->Record () || info->NamespaceInfo ())
    return (CStructure*)info->DefObject ();
  if (info->TypedefInfo () && ! realTypedef (info))
    return (CStructure*)info->TypeInfo ()->UnqualType ()->
           TypeRecord ()->Record ()->DefObject ();
  return 0;
}


CStructure *CCSemantic::getNonTplScope (bool consider_instance_scope) const {
  CStructure *scope = current_scope;
  while (scope && (scope->TemplateInfo () ||
         (consider_instance_scope &&
          scope->NamespaceInfo () &&
          scope->NamespaceInfo ()->aroundInstantiation ())))
    scope = scope->Parent ()->Structure ();
  return scope;
}


CStructure *CCSemantic::getQualifiedScope (CStructure *scope) const {
  CObjectInfo *oinfo;
  if (scope && scope->isTemplateInstance () && ! scope->TemplateInstance ()->canInstantiate ()) {
    oinfo = scope->TemplateInstance ()->Template ()->ObjectInfo ();
    if (oinfo && oinfo->Structure ())
      return oinfo->Structure ();
  }
  return scope;
}


bool CCSemantic::isQualName (CT_SimpleName *name) const {
  return name->NodeName () == CT_QualName::NodeId () ||
         name->NodeName () == CT_RootQualName::NodeId ();
}


void CCSemantic::qualifiedScopeName (CT_SimpleName *name, std::ostringstream &qn) const {
  if (name->NodeName () == CT_RootQualName::NodeId ())
    qn << "::";
  if (isQualName (name)) {
    int entries = name->Entries () - 1;
    for (int i = 0; i < entries; i++) {
      qn << ((CT_SimpleName*)name->Entry (i))->Text ();
      if (i+1 < entries)
        qn << "::";
    }
  } else
    qn << name->Text ();
}


bool CCSemantic::isBaseClass (CClassInfo *info, CClassInfo *base) const {
  if (info == base)
    return true;
  for (unsigned i = 0; i < info->BaseClasses (); i++)
    if (isBaseClass (info->BaseClass (i)->Class (), base))
      return true;
  return false;
}


// ���3.4.3 typedefs to classes are classes
bool CCSemantic::realTypedef (CObjectInfo *info) const {
  return info && ! info->TypeInfo ()->UnqualType ()->TypeRecord ();
}


void CCSemantic::enter_scope (CStructure *scp) {
  CStructure *scope;
  bool propagate = true;

  // propagate template parameters to nested scope (if any)
  // (to simplify identification of template parameter names)
  // ���14.6.1.5-7 do not propagate if scope belongs to definition
  // of a class template member outside of the template definition
  if (current_scope->TemplateInfo () && ! scp->TemplateInfo ()) {
    scope = current_scope;
    while (scope->TemplateInfo () && scope->Parent () && scope != scope->Parent ())
      scope = scope->Parent ()->Structure ();
    if (! scope->Record ())
      propagate = false;
  }

  if (propagate)
    for (unsigned i = 0; i < current_scope->TemplateParams (); i++)
      scp->addTemplateParam (current_scope->TemplateParam (i));

  // enter nested scope
  Semantic::enter_scope (scp);
}


bool CCSemantic::non_project_loc () {
  FileUnit *funit;
  Token *token;
  Unit *unit;

  token = syntax ().token_provider->current ();
  if (token) {
    unit = (Unit*)token->belonging_to ();
    if (unit) {
      // same unit, so return last decision
      if (unit == _last_unit)
        return _last_body_skipped;
      _last_unit = unit;

      // if macro expansion, get the file containing the macro call
      while (unit && unit->isMacroExp ())
        unit = ((MacroUnit*)unit)->CallingUnit ();

      // must be a file unit
      if (unit && unit->isFile ()) {
        funit = (FileUnit*)unit;

        // file doesn't belong to project?
        if (! funit->belongsTo (*_db->Project ())) {
          _last_body_skipped = true;
          return true;
        }
      }
    }
  }

  _last_body_skipped = false;
  return false;
}


bool CCSemantic::non_primary_loc () {
  Token* token = syntax ().token_provider->current ();
  if (token) {
    Unit* unit = (Unit*)token->belonging_to ();
    if (unit) {
      // if macro expansion, get the file containing the macro call
      while (unit && unit->isMacroExp ())
        unit = ((MacroUnit*)unit)->CallingUnit ();

      // not the primary file?
      if (unit != _file->Primary ()) {
        return true;
      }
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*                                Names                                      */
/*                                                                           */
/*****************************************************************************/


// lookup type-names and class-or-original-names more efficient
void CCSemantic::lookupName (const Token *tok, CStructure *scope,
 bool nested) {
  bool nested_name = in_nested_name ();
  bool base_spec = in_base_spec ();

  // if same lookup as before, don't lookup again
  if (//(shared_nl.Objects ()) &&
      (tok == last_token_nl.tok) &&             // => same token!!!
      (scope == last_token_nl.scope) &&
      (nested == last_token_nl.nested) &&
      (nested_name == last_token_nl.nested_name) &&
      (base_spec == last_token_nl.base_spec))
    return;

  // not the same lookup as before, start new lookup
  shared_nl.reset (current_scope, !is_entity_name());
  if (nested_name || base_spec) {
    shared_nl.lookupType (tok->dtext (), scope, nested, true);
    if (! shared_nl.Objects ()) {
      shared_nl.reset (current_scope, !is_entity_name());
      shared_nl.lookupNamespace (tok->dtext (), scope);
    }
    if (! shared_nl.Objects ()) {
      shared_nl.reset (current_scope, !is_entity_name());
      shared_nl.lookup (tok->dtext (), scope, nested, true);
    }
  } else
    shared_nl.lookup (tok->dtext (), scope, nested, true);

  // remember the parameters of the last lookup
  last_token_nl.tok = tok;
  last_token_nl.scope = scope;
  last_token_nl.nested = nested;
  last_token_nl.nested_name = nested_name;
  last_token_nl.base_spec = base_spec;
}


CTree *CCSemantic::typedef_name () {
  CT_SimpleName *result = 0;
  CObjectInfo *info;
//  const char *name;
  const Token *tok;

  if (! search_scope)
    last_search_scope = 0;

//  name = builder ().get_node ()->token ()->text ();
  tok = builder ().get_node ()->token ();

  if (search_scope) // qualified
    lookupName (tok, search_scope, false);
  else              // unqualified
    lookupName (tok, current_scope, true);

  if (shared_nl.Objects ()) {
    info = shared_nl.Object ();
    if (realTypedef (info->TypedefInfo ())) {
      result = (CT_SimpleName*)builder ().simple_name ();
      result->Object (info);
      result->setTypeRef (info->TypeInfo ());
    }
  }

  expect_template = false;
  return result;
}


CTree *CCSemantic::class_name () {
  CT_SimpleName *result = 0;
  bool is_dep, tpl_param;
  CObjectInfo *info;
  CStructure *scope;
  Token* tok;
  CTypeInfo *type;

  // implicit `typename' hack
  is_dep = dep_nested_name;
  if (! in_nested_name ())
    dep_nested_name = false;
  if (! search_scope)
    last_search_scope = 0;

  // handle template-id
  if (builder ().get_node ()->NodeName () == CT_TemplateName::NodeId ()) {
    expect_template = false;
    result = (CT_SimpleName*)builder ().get_node ();
    info = result->Object ();
    if (info && info->FunctionInfo())
      return (CTree*)0;
    if (! search_scope && is_dep)
      return result;
    if (info && (info->Record() || info->TemplateParamInfo()))
      return result;
    if (result->Type() == &CTYPE_UNDEFINED) // dependent
      return result;
    return (CTree*)0;
  }

  // part of a nested name that depends on a template parameter
  if (is_dep && ! search_scope) {
    result = (CT_SimpleName*)builder ().simple_name ();
    result->setTypeRef (&CTYPE_UNDEFINED);
  // dirty hack to support implicit `typename' :(
  } else if (! in_expr () && search_scope && search_scope->TemplateInstance () &&
             ! search_scope->TemplateInstance ()->canInstantiate ()) {
    result = (CT_SimpleName*)builder ().simple_name ();
    info = search_scope;
    result->Object (info);
    result->setTypeRef (info->TypeInfo ());
  } else {
    tok = builder ().get_node ()->token ();
    scope = search_scope;

    // in case of "expr.id" or "expr->id"
    if (! scope && _in_memb_access_expr.top ()) {
      if (_postfix_expr.top ()->Type () &&
          ! _postfix_expr.top ()->Type ()->is_undefined ()) {
        type = _postfix_expr.top ()->Type ();
        is_dep = CCSemExpr::isDependent (_postfix_expr.top ());
      } else {
        CCSemExpr cse (*_err, current_scope);
        type = cse.resolveExpr (_postfix_expr.top ());
        is_dep = cse.isDependent ();
      }
      if (is_dep && expect_template) {
        result = (CT_SimpleName*)builder ().simple_name ();
        result->setTypeRef (&CTYPE_UNDEFINED);
        expect_template = false;
        return result;
      }

      // get the member's class
      if (type->isPointer ()) // "expr->id"
        type = type->VirtualType ()->BaseType ();
      if (type->isRecord ()) {
        type = type->VirtualType ();
        if (type->TypeRecord ()->Record ())
          scope = type->TypeRecord ()->Record ()->DefObject ()->Record ();
      }
    }

    if (scope) { // qualified
      lookupName (tok, scope, false);
      // in case of a qualified member name in a member access expression
      // the member name is first looked up in the member's class and then,
      // if name lookup fails, in the scope of the expression (current_scope)
      if (scope != search_scope && ! shared_nl.Objects ())
        lookupName (tok, current_scope, true);
    } else // unqualified
      lookupName (tok, current_scope, true);

    if (shared_nl.Objects ()) {
      info = shared_nl.Object ();
      tpl_param = ((info->Record () || info->TemplateInfo () ||
                    info->TypedefInfo () || info->TemplateParamInfo ()) &&
                   info->TypeInfo () &&                     // type template parameter
                   ((info->TypeInfo ()->is_unknown_t ()) ||
                    (info->TypeInfo ()->TypeTemplateParam () &&
                     info->TypeInfo ()->TypeTemplateParam ()->isType ())));
      if (info->Record () || tpl_param ||
          (info->TypedefInfo () && ! realTypedef (info))) { // typedef naming class
        result = (CT_SimpleName*)builder ().simple_name ();
        result->Object (info);
        result->setTypeRef (info->TypeInfo ());
      }
    } else if (is_dep && search_scope && search_scope->isTemplate ()) {
      result = (CT_SimpleName*)builder ().simple_name ();
      result->setTypeRef (&CTYPE_UNDEFINED);
    }
  }

  expect_template = false;
  return result;
}


CTree *CCSemantic::template_name (bool class_only) {
  CT_SimpleName *result = 0;
  CTemplateInfo *tpinfo = 0;
  CObjectInfo *info = 0;
  CStructure *scope;
  Token* tok;
  CTypeInfo *type;
  bool is_dep;

  if (syntax ().look_ahead (TOK_LESS)) {
    // implicit `typename' hack
    is_dep = dep_nested_name;
    if (! in_nested_name ())
      dep_nested_name = false;
    if (! search_scope)
      last_search_scope = 0;

    // part of a nested name that depends on a template parameter
    if (is_dep && ! search_scope) {
      // name is prefixed by keyword `template'
      if (expect_template || ! in_expr ()) {
        result = (CT_SimpleName*)builder ().simple_name ();
        result->setTypeRef (&CTYPE_UNDEFINED);
      }
    } else {
      tok = builder ().get_node ()->token ();
      scope = search_scope;

      // in case of "expr.id <" or "expr->id <"
      MemberAccessOperator mao = _in_memb_access_expr.top ();
      if (! scope && mao) {
        if (_postfix_expr.top ()->Type () &&
            ! _postfix_expr.top ()->Type ()->is_undefined ()) {
          type = _postfix_expr.top ()->Type ();
          is_dep = CCSemExpr::isDependent (_postfix_expr.top ());
        } else {
          CCSemExpr cse (*_err, current_scope);
          type = cse.resolveExpr (_postfix_expr.top ());
          is_dep = cse.isDependent ();
        }
        if (is_dep && (expect_template || type == &CTYPE_UNDEFINED)) {
          result = (CT_SimpleName*)builder ().simple_name ();
          result->setTypeRef (&CTYPE_UNDEFINED);
          expect_template = false;
          return result;
        }

        // get the member's class
        bool pointer_type = type->isPointer ();
        if (pointer_type) // "expr->id <"
          type = type->VirtualType ()->BaseType ();
        if (type->isRecord ()) {
          type = type->VirtualType ();
          if (type->TypeRecord ()->Record ()) {
            scope = type->TypeRecord ()->Record ()->DefObject ()->Record ();
            // check for and handle operator ->()
            if (!pointer_type && mao == MAO_PTS) {
              shared_nl.reset (current_scope, !is_entity_name());
              shared_nl.lookup ("operator ->", scope, false, true);
              if (shared_nl.Objects () == 0) {
                SEM_ERROR (_postfix_expr.top (),
                    "base operand of '->' has non-pointer type '" << *type << "'");
                return (CTree*)0;
              }
              else if (shared_nl.Objects () > 1) {
                CObjectInfo *err_obj = shared_nl.Object (1);
                SEM_ERROR (err_obj->Tree (),
                    "'" << err_obj->QualName () << "()' cannot be overloaded");
                return (CTree*)0;
              }
              else {
                // search the template name in the class reference by the result
                // type of the operator ->()
                if (shared_nl.Object (0)->FunctionInfo ()) {
                  CTypeInfo *res_type = shared_nl.Object (0)->TypeInfo ()->BaseType ();
                  if (res_type && res_type->isPointer ()) {
                    type = res_type->VirtualType ()->BaseType ()->VirtualType ();
                    scope = type->TypeRecord ()->Record ()->DefObject ()->Record ();
                  }
                }
              }
            }
          }
        }
      }

      if (scope) // qualified
        lookupName (tok, scope, false);
      else // unqualified
        lookupName (tok, current_scope, true);

      if (shared_nl.Objects ()) {
        for (unsigned i = shared_nl.Objects (); i > 0; i--) {
          info = shared_nl.Object (i-1);
          tpinfo = 0;

          // template instance
          if (info->isTemplateInstance ()) {
            tpinfo = info->TemplateInstance ()->Template ();
            if (tpinfo)
              info = tpinfo->ObjectInfo ();
          // base template or template template parameter
          } else if (info->isTemplate () &&
                         (info->TemplateParamInfo () ? ! search_scope : true)) {
            if (info->TemplateParamInfo ())
              tpinfo = info->TemplateParamInfo ()->TemplateTemplate ();
            else
              tpinfo = info->Template ();
          } else
            continue;
          break;
        }

        if (tpinfo) {
          // get the object of the base template
          if (tpinfo->BaseTemplate ())
            info = tpinfo->BaseTemplate ()->ObjectInfo ();

          // reject function templates if not allowed at this point
          if (! class_only || ! info->FunctionInfo ()) {
            result = (CT_SimpleName*)builder ().simple_name ();
            result->Object (info);
            result->setTypeRef (info->TypeInfo ());
          }
        }
      } else if (is_dep && search_scope && search_scope->isTemplate ()) {
        result = (CT_SimpleName*)builder ().simple_name ();
        result->setTypeRef (&CTYPE_UNDEFINED);
      }
    }
  }

  expect_template = false;
  return result;
}


CTree *CCSemantic::enum_name () {
  CT_SimpleName *result = 0;
  CObjectInfo *info;
//  const char *name;
  Token *tok;

  if (! search_scope)
    last_search_scope = 0;

  tok = builder ().get_node ()->token ();

  if (search_scope) // qualified
    lookupName (tok, search_scope, false);
  else // unqualified
    lookupName (tok, current_scope, true);

  if (shared_nl.Objects ()) {
    info = shared_nl.Object ();
    if (info->EnumInfo ()) {
      result = (CT_SimpleName*)builder ().simple_name ();
      result->Object (info);
      result->setTypeRef (info->TypeInfo ());
    }
  }

  expect_template = false;
  return result;
}


CTree *CCSemantic::original_ns_name () {
  CT_SimpleName *result = 0;
  CObjectInfo *info;
//  const char *name;
  Token *tok;

  if (! search_scope)
    last_search_scope = 0;

  tok = builder ().get_node ()->token ();

  if (search_scope) // qualified
    lookupName (tok, search_scope, false);
  else // unqualified
    lookupName (tok, current_scope, true);

  if (shared_nl.Objects ()) {
    info = shared_nl.Object ();
    if (info->NamespaceInfo () && ! info->NamespaceInfo ()->isAlias ()) {
      result = (CT_SimpleName*)builder ().simple_name ();
      result->Object (info);
      result->setTypeRef (info->TypeInfo ());
    }
  }

  expect_template = false;
  return result;
}


CTree *CCSemantic::namespace_alias () {
  CT_SimpleName *result = 0;
  CObjectInfo *info;
//  const char *name;
  Token* tok;

  if (! search_scope)
    last_search_scope = 0;

  tok = builder ().get_node ()->token ();

  if (search_scope) // qualified
    lookupName (tok, search_scope, false);
  else // unqualified
    lookupName (tok, current_scope, true);

  if (shared_nl.Objects ()) {
    info = shared_nl.Object ();
    if (info->NamespaceInfo () && info->NamespaceInfo ()->isAlias ()) {
      result = (CT_SimpleName*)builder ().simple_name ();
      result->Object (info);
      result->setTypeRef (info->TypeInfo ());
    }
  }

  expect_template = false;
  return result;
}


CTree *CCSemantic::nested_name_spec () {
  if (is_entity_name ())
    dep_nested_name = false;

  // shall not be prefixed by `template'
  if (builder ().get_node ()->token ()->type () == TOK_TEMPLATE)
    return (CTree*)0;

  return builder ().nested_name_spec ();
}


CTree *CCSemantic::nested_name_spec1 () {
  CT_SimpleName *name;

  name = (CT_SimpleName*)builder ().get_node (builder ().nodes ()-2);
  // dependent nested name specifier of the form `T::...'
  // suppress further semantic analysis
//  if (! dep_nested_name) {
//    if (! search_scope &&
//        name->Object () &&
//        name->Object ()->TypeInfo () &&
//        name->Object ()->TypeInfo ()->isDependent ())
//      dep_nested_name = true;
//    else
//      last_search_scope = search_scope = getScope (name->Object ());
//  }
  if (name->Object ()) {
    if (name->Object ()->TypeInfo () &&
        name->Object ()->TypeInfo ()->isDependent ())
      dep_nested_name = true;
    last_search_scope = search_scope = getScope (name->Object ());
  }
  return builder ().nested_name_spec1 ();
}


CTree *CCSemantic::colon_colon () {
  search_scope = _file;
  return builder ().get_node ();
}


bool CCSemantic::reset_search_scope (bool also_last) {
  search_scope = 0;
  if (also_last)
    last_search_scope = 0;
  dep_nested_name = false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*                              Functions                                    */
/*                                                                           */
/*****************************************************************************/


CTree *CCSemantic::introduce_function () {
  CStructure *scope, *lookup_scope;
  bool is_friend, is_qual_name, is_special;
  CTypeInfo *type;
  CTemplateInfo *tinfo;
  CT_DeclSpecSeq *dss;
  CFunctionInfo *info;
  CSemDeclSpecs *dsi;
  CT_Declarator *d;

  if (! current_fct)
    return (CTree*)0; // Fatal internal error

  // determine function return type
  dss = (CT_DeclSpecSeq*)(builder ().nodes () == 2 ? builder ().get_node (0) : 0);
  d = (CT_Declarator*)(dss ? builder ().get_node (1) : builder ().get_node (0));
  if (dss && ! in_decl ())
    return (CTree*)0;

  // determine function type
  dsi = sem_decl_specs ();
  CCSemDeclarator csd (_err, dsi->make_type (), d);
  if (! csd.Name () || ! csd.Type () || ! csd.Type ()->VirtualType ()->TypeFunction ())
    return (CTree*)0;

  // determine function attributes
  type = csd.Type ();
  is_qual_name = isQualName (csd.Name ());
  is_friend = (dsi->declared (CT_PrimDeclSpec::PDS_FRIEND));
  is_special = isSpecialization (csd.Name ());
  info = (CFunctionInfo*)current_fct;
  lookup_scope = is_special ? current_scope : getNonTplScope (true);

  // lookup name
  CCNameLookup nl (*_err, current_scope, false);
  if (! is_friend || is_qual_name) {
    scope = is_qual_name ? getNameScope (csd.Name ()) : lookup_scope;
    if (is_qual_name && scope == lookup_scope) {
      // ignore extra qualification if it refers to current class scope
      // lookup as if no qualification is given
      is_qual_name = false;
      nl.lookupFunction (csd.Name ()->Name (), lookup_scope);
    } else {
      if (is_qual_name) {
        // definition of member function inside class def of another class
        if (! is_friend && lookup_scope->Record ()) {
          SEM_ERROR__cannot_declare (d, csd.Name (), lookup_scope);
          return (CTree*)0;
        }
        lookup_scope = scope;
      }
      nl.lookupFunction (csd.Name (), lookup_scope);
    }
  } else { // friend && unqualified
    scope = lookup_scope = getFriendScope (false);
    nl.lookupFunction (csd.Name (), lookup_scope);
    if (! nl.Objects ()) {
      // lookup in non-template scope
      scope = lookup_scope = getFriendScope (true);
      nl.lookupFunction (csd.Name (), lookup_scope);
    }
  }

  // handle function templates
  tinfo = current_scope->TemplateInfo ();
  if (tinfo) {
    setSpecializationName (csd.Name (), tinfo);
    setTemplateScope (nl, info, csd.Type (), tinfo, is_qual_name);
  }

  // definition of class or namespace member
  if ((! is_friend || is_qual_name) &&
      (lookup_scope->Record () || (is_qual_name && lookup_scope->NamespaceInfo ()))) {
    info->QualifiedScope (getQualifiedScope (lookup_scope));
    // class member (method)
    if (lookup_scope->Record ()) {
      // definition of class constructor?
      if (lookup_scope->Name () &&
          ! strcmp (lookup_scope->Name (), csd.Name ()->Text ()))
        info->isConstructor (true);

      // determine member protection
      setProtection (info);

      // create implicit `this' parameter
      addImplicitThisObject (info, type, d);
    }
  }

  // determine member function category
  if (csd.Name ()->Name ()->NodeName () == CT_DestructorName::NodeId ())
    info->isDestructor (true);
  else if (csd.Name ()->Name ()->NodeName () == CT_OperatorName::NodeId ())
    info->isOperator (true);
  else if (csd.Name ()->Name ()->NodeName () == CT_ConversionName::NodeId ()) {
    info->isConversion (true);
    info->ConversionType (((CT_ConversionName*)csd.Name ()->Name ())->
                          TypeName ()->Object ()->TypeInfo ());
  }

  // determine member function specifiers such as `virtual'
  if (! is_friend) {
    setLanguage (info);
    setSpecifiers (info, dsi);
    if (current_scope->Record ())
      info->isInline (true);
  // add friend to class
  } else {
    info->AssignedScope (lookup_scope);
    if (current_scope->Record ())
      info->addFriend (current_scope->Record ());
  }

  // check name lookup results
  if (! check_function_lookup (nl, lookup_scope, dsi, csd, d, is_qual_name))
    return (CTree*)0;

  // enter function scope (re-used function prototype scope)
/*DEBUG*/if (TRACE_FCT) std::cout<<"FCT ENT(1): "<<(void*)current_fct<<std::endl;
  reenter_scope (current_fct);
/*DEBUG*/if (TRACE_FCT) std::cout<<"FCT NIL(1): "<<(void*)current_fct<<std::endl;
  current_fct = 0;

  // complete semantic information about the function
  type->VirtualType ()->TypeFunction ()->FunctionInfo (info);
  info->Name (csd.Name ()->Text ());
  info->SourceInfo ()->FileInfo (_file);
  info->ObjectInfo ()->TypeInfo (type);

  // add function to surrounding scope
  if (! is_friend)
    info->Parent ()->Structure ()->addFunction (info);
  else if (! is_qual_name)
    lookup_scope->addFunction (info);

  // handle template declarations
  if (current_scope->Parent ()->TemplateInfo ())
    introduceTemplate (info, current_scope->Parent ()->TemplateInfo (), is_friend);

  // link function with previous declarations
  if (! is_special)
    linkFctDecls (nl, info);
  if (info->isConstructor ())
    linkCtorDecls (info);

  // clean up
  csd.Type ((CTypeInfo*)0);
  csd.Name ()->Object (info);
  csd.Name ()->setTypeRef (type);
  if (! csd.Name ()->Name ()->Object ()) {
    csd.Name ()->Name ()->Object (info);
    csd.Name ()->Name ()->setTypeRef (type);
  }

  // if return type or a parameter type is class object type,
  // ensure that the class is really instantiated
  /*
  CTypeFunction* ftype = type->VirtualType()->TypeFunction();
  if (ftype->ReturnType()->UnqualType()->TypeClass()) {
    ftype->ReturnType()->UnqualType()->TypeClass()->instantiate(current_scope);
  }
  CTypeList *ptypes = ftype->ArgTypes();
  if (ptypes) {
    for (unsigned i = 0; i < ptypes->Entries(); i++) {
      CTypeInfo* ptype = ptypes->Entry(i);
      if (ptype && ptype->UnqualType()->TypeClass()) {
        ptype->UnqualType()->TypeClass()->instantiate(current_scope);
      }
    }
  }*/

  if (dss) decl_end ();
  return d;
}


bool CCSemantic::check_function_lookup (CCNameLookup &nl, CStructure *lookup_scope,
 CSemDeclSpecs *dsi, CCSemDeclarator &csd, CTree *d, bool is_qual_name) {
  CFunctionInfo *info;
  CT_SimpleName *name;
  CObjectInfo *oinfo;
  CStructure *scope;
  CTypeInfo *type;
  unsigned i = 0;
  bool is_friend, is_special;

  info = (CFunctionInfo*)current_fct;
  name = csd.Name ();
  type = csd.Type ();
  is_friend = (dsi->declared (CT_PrimDeclSpec::PDS_FRIEND));
  is_special = isSpecialization (csd.Name ());
  scope = getNonTplScope ();

  // check all functions looked up
  if (nl.Objects ()) {
    oinfo = nl.Object ();
    if (oinfo->FunctionInfo ()) {
      do {
        if (oinfo && (oinfo->isTemplate () == info->isTemplate ()) && ! is_special &&
            (oinfo->Template () ? ! oinfo->Template ()->isSpecialization () : true)) {
          if (equalType (type, oinfo->TypeInfo ()) && (!oinfo->Template () ||
              CTemplateInfo::equalParameters(info->TemplateInfo (), oinfo->TemplateInfo ()))) {
            if (oinfo->FunctionInfo ()->isDefined ()) {
              SEM_ERROR__redefinition (d, name, oinfo);
              return false;
            } else if (is_qual_name && ! is_friend && scope->Record ()) {
                SEM_ERROR__already_defined (d, name, oinfo);
              return false;
            }
          }

          // definition of member function outside of class body
          if (equalType (type, oinfo->TypeInfo (), false)) {
            info->Protection (oinfo->Protection ());
            info->isVirtual (oinfo->isVirtual ());
            info->isStatic (oinfo->isStatic ());
            oinfo->isInline (info->isInline ());
          }
        }
      } while (++i < nl.Objects () && (oinfo = nl.Object (i)));
    // constructors are never found by name lookup
    } else if (! (info->isConstructor () || isClassOrEnum (oinfo)))
      return false;
  // qualified function name not found
  } else if (is_qual_name) {
    std::ostringstream qname; qualifiedScopeName (name, qname);
    SEM_ERROR__not_member (d, name->Name (), qname.str ().c_str ());
    return false;
  }

  // check function properties
  if (scope->isLocalScope () && scope->insideFunction ()) {
    SEM_ERROR (d, "nested function `" << *name << "'");
    return false;
  } else if (dsi->declared (CT_PrimDeclSpec::PDS_TYPEDEF)) {
    SEM_ERROR (d, "typedef declaration invalid in definition of function `"
      << *name << "'");
    return false;
  } else if (dsi->declared (CT_PrimDeclSpec::PDS_AUTO) ||
             dsi->declared (CT_PrimDeclSpec::PDS_REGISTER)) {
    SEM_ERROR (d, "wrong storage class specifier in definition of function `"
      << *name << "'");
    return false;
  } else if (! type->isFunction ())
    return false;
  return true;
}


void CCSemantic::addImplicitThisObject(CFunctionInfo* info, CTypeInfo* type, CTree* tree) {
  // create implicit `this' object
  CRecord *record = info->Record();
  if (record && ! info->isStatic()) {
    CTypeInfo *thistype = record->TypeInfo()->Duplicate();
    if (type->VirtualType()->TypeFunction()->isConst() ||
        type->VirtualType()->TypeFunction()->isVolatile())
      thistype = new CTypeQualified(thistype,
        type->VirtualType()->TypeFunction()->isConst(),
        type->VirtualType()->TypeFunction()->isVolatile(), false);
    thistype = new CTypePointer(thistype);

    CObjectInfo *oinfo = info->newAttribute();
    oinfo->Name("this");
    oinfo->Storage(CStorage::CLASS_AUTOMATIC);
    oinfo->SourceInfo()->FileInfo(_file);
    oinfo->SourceInfo()->StartToken(tree->token_node());
    oinfo->TypeInfo(thistype);
    Push(oinfo);
  }
}


CTree *CCSemantic::introduce_parameter () {
  CArgumentInfo *info;
  CSemDeclSpecs *dsi;
  CT_ArgDecl *ad;

  if (! in_decl ())
    return (CTree*)0;

  // determine parameter type
  ad = (CT_ArgDecl*)builder ().param_decl1 ();
  dsi = sem_decl_specs ();
  CCSemDeclarator csd (_err, dsi->make_type (), ad->Declarator ());
  if (! csd.Name ()) {
    delete ad;
    return (CTree*)0;
  }

  // check function parameter properties
  bool error = true;
  if (current_scope->FunctionInfo ()->Argument (csd.Name ()->Text ())) {
    SEM_ERROR__duplicate (ad, "function parameter", csd.Name ());
  } else if (dsi->declared (CT_PrimDeclSpec::PDS_STATIC) ||
             dsi->declared (CT_PrimDeclSpec::PDS_EXTERN)) {
    SEM_ERROR__invalid_in_param (ad, "storage class specifier", csd.Name ());
  } else if (dsi->declared (CT_PrimDeclSpec::PDS_TYPEDEF)) {
    SEM_ERROR__invalid_in_param (ad, "typedef declaration", csd.Name ());
  } else if (csd.Name ()->NodeName () != CT_QualName::NodeId () &&
             csd.Name ()->NodeName () != CT_RootQualName::NodeId ())
    error = false;
  if (error) {
    delete ad;
    return (CTree*)0;
  }

  // complete semantic information about the function parameter
  info = current_scope->FunctionInfo ()->newArgument ();
  common_settings (info, ad, &csd);
  info->Storage (CStorage::CLASS_AUTOMATIC);
  setLanguage (info);
  setSpecifiers (info, dsi);
  Push (info);
  ad->Object (info);
  ad->setTypeRef (info->TypeInfo ());

  return ad;
}


void CCSemantic::enter_param_decl_clause () {
  // create and enter new function prototype scope
  CObjectInfo* instance = tpl_inst ? tpl_inst->Instance () : 0;
  if (parsing_fct_inst && instance &&
      ! instance->TemplateInstance ()->isInstantiated () &&
      current_scope->isNamespace ()) {
    tpl_inst->insertInstance (instance);
    // to be left if param decl clause is done
    reenter_scope (instance->Structure ());
  } else {
    CFunctionInfo *info = _db->newFunction();
    enter_scope (info);
  }
  _in_param_decl_clause.push (true);
  // qualified function name
  if (last_search_scope)
    current_scope->FunctionInfo ()->QualifiedScope (getQualifiedScope (last_search_scope));
}


void CCSemantic::leave_param_decl_clause () {
  last_search_scope = (CStructure*)0;
  CSemantic::leave_param_decl_clause ();

  CObjectInfo* instance = tpl_inst ? tpl_inst->Instance () : 0;
  if (parsing_fct_inst && instance && current_scope == instance) {
    // reentered the scope of function instance, now reenter
    // the parent scope
    reenter_scope (current_scope->Parent ()->Structure ());
  }
}


CTree *CCSemantic::condition () {
  CSemDeclSpecs *dsi;
  CT_Condition *cond;
  CObjectInfo *info;
  CTypeInfo *type;

  if (! in_decl ())
    return (CTree*)0;

  // determine type of condition declaration
  cond = (CT_Condition*)builder ().condition ();
  dsi = sem_decl_specs ();
  CCSemDeclarator csd (_err, dsi->make_type (), cond->Declarator ());
  if (! csd.Name ()) {
    delete cond;
    return (CTree*)0;
  }

  // check properties of the condition declaration
  bool error = true;
  type = csd.Type ();
  if (type->isFunction () || type->isArray ()) {
    SEM_ERROR (cond, "function or array type in condition");
  } else if (dsi->declared (CT_PrimDeclSpec::PDS_TYPEDEF)) {
    SEM_ERROR (cond, "typedef in condition");
  } else if (dsi->defClass () || dsi->defEnum ())  {
    SEM_ERROR (cond, "class or enumeration defined in condition");
  } else if (isQualName (csd.Name ())) { // qualified name
    CCNameLookup nl (*_err, current_scope, false);
    nl.lookup (csd.Name (), current_scope);
    if (! nl.Objects ()) {
      if (current_scope != getNameScope (csd.Name ())) {
        std::ostringstream name; qualifiedScopeName (csd.Name (), name);
        SEM_ERROR__not_member (cond, csd.Name ()->Name (), name.str ().c_str ());
      }
    } else {
      SEM_ERROR__redefinition (cond, csd.Name (), nl.Object ());
    }
  } else
    error = false;
  if (error) {
    delete cond;
    return (CTree*)0;
  }

  // complete semantic information about the condition
  info = current_scope->newAttribute ();
  common_settings (info, cond, &csd);
  setLanguage (info);
  setSpecifiers (info, dsi);
  info->Linkage (determine_linkage (dsi, ATTRIB));
  info->Storage (determine_storage_class (dsi));
  Push (info);
  cond->Object (info);
  decl_end ();
  return cond;
}


bool CCSemantic::equalType (CTypeInfo *t1, CTypeInfo *t2, bool consider_dependent) const {
  if (t1->isFunction () && t2->isFunction ())
    return t1->equals (*t2, true, consider_dependent);
  else
    return *t1 == *t2;
}


void CCSemantic::linkFctDecls (CCNameLookup &nl, CObjectInfo *info) const {
  CObjectInfo *oinfo;

  for (unsigned i = 0; i < nl.Objects (); i++) {
    oinfo = nl.Object (i);
    if (oinfo && oinfo->FunctionInfo () &&
        oinfo->isTemplate () == info->isTemplate () &&
        (oinfo->Template () ? ! oinfo->Template ()->isSpecialization () : true) &&
        equalType (info->TypeInfo (), oinfo->TypeInfo (), false)) {
      oinfo->NextObject (info);
      info->Language () = oinfo->Language ();
      break;
    }
  }
}


void CCSemantic::linkCtorDecls (CObjectInfo *info) const {
  CObjectInfo *oinfo;

  if (info && info->QualifiedScope ()) {
    CStructure *scope = info->QualifiedScope ();
    for (unsigned i = 0; i < scope->Functions (); i++) {
      oinfo = scope->Function (i);
      if (oinfo && oinfo->FunctionInfo ()->isConstructor () &&
          oinfo->isTemplate () == info->isTemplate () &&
          (oinfo->Template () ? ! oinfo->Template ()->isSpecialization () : true) &&
          equalType (info->TypeInfo (), oinfo->TypeInfo (), false)) {
        oinfo->NextObject (info);
        info->Language () = oinfo->Language ();
        break;
      }
    }
  }
}


bool CCSemantic::implicit_int () {
  if (! current_scope->isNamespace () && ! current_scope->TemplateInfo ())
    return false;

  CT_DeclSpecSeq *dss = new CT_DeclSpecSeq;
  decl_specs_begin (dss);
  begin_decl ();
  //CT_PrimDeclSpec *pds = new CT_PrimDeclSpec (CT_PrimDeclSpec::PDS_INT);
  //dss->AddSon (pds);
  //builder ().Push (pds);
  builder ().Push (dss);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*                            Enumerations                                   */
/*                                                                           */
/*****************************************************************************/


CTree *CCSemantic::introduce_enum () {
  CObjectInfo *info;
  CT_EnumDef *ed;

  ed = (CT_EnumDef*)builder ().enum_spec1 ();

  // lookup name in current scope
  CCNameLookup nl (*_err, current_scope, false);
  nl.lookupType (ed->Name (), current_scope);
  if (nl.Objects ()) {
    info = nl.Object ();
    // enum already defined in current scope
    if (info->EnumInfo () && info->EnumInfo ()->isDefined ()) {
      SEM_ERROR__redefinition (ed, ed->Name (), info);
      delete ed;
      return (CTree*)0;
    // conflicting declaration in current scope
    } else if (! info->EnumInfo ()) {
      SEM_ERROR__already_defined (ed, ed->Name (), info);
      delete ed;
      return (CTree*)0;
    }
  }

  // complete semantic information about the enum
  info = current_scope->newEnum ();
  setProtection (info);
  common_settings (info, ed);
  info->TypeInfo (new CTypeEnum (info->EnumInfo ()));
  info->Name (ed->Name ()->Text ());
  current_enum = info->EnumInfo ();
  Push (info);

  // link forward declarations with this definition
  if (nl.Objects ())
    nl.Object ()->NextObject (info);

  // attribute syntax tree
  ed->Name ()->Object (info);
  ed->Name ()->setTypeRef (info->TypeInfo ());
  ed->Object (info);
  return ed;
}


CTree *CCSemantic::introduce_enumerator () {
  CT_Enumerator *result;
  CT_SimpleName *name;
  CObjectInfo *info;

  name = (CT_SimpleName*)builder ().get_node ();

  // lookup name in current scope
  CCNameLookup nl (*_err, current_scope, false);
  nl.lookup (name, current_scope);
  if (nl.Objects ()) {
    info = nl.Object ();
    // conflicting declaration in current scope
    if (! (info->Record () || info->EnumInfo ())) {
      SEM_ERROR__already_defined (name, name, info);
      return (CTree*)0;
    }
  }

  // complete semantic information about enumerator
  result = (CT_Enumerator*)builder ().enumerator ();
  info = current_enum->newEnumerator ();
  common_settings (info, result);
  info->EnumeratorInfo ()->Scope (current_scope);
  info->Name (name->Text ());
  info->Storage (CStorage::CLASS_AUTOMATIC);
  current_scope->addAttribute (info->EnumeratorInfo ());
  info->TypeInfo (current_enum->TypeInfo ()->Duplicate ());
  setProtection (info);
  Push (info);

  // attribute syntax tree
  result->Object (info);
  result->Name ()->Object (info);
  result->Name ()->setTypeRef (current_enum->TypeInfo ());
  return result;
}


CTree *CCSemantic::enumerator_def () {
  CEnumeratorInfo *einfo;
  CEnumInfo *enuminfo;
  CT_Enumerator *e;
  CTypeInfo *type;
  CConstant *v;
  CTree *init;
  LONG_LONG value = 0;
  unsigned numenums;

  e = (CT_Enumerator*)builder ().enumerator_def ();

  if (e && e->Object ()) {
    einfo = e->Object ()->EnumeratorInfo ();
    enuminfo = einfo->Enum ();
    numenums = enuminfo->Enumerators ();

    // if the enumerator has an initializer, get the value
    // from this constant expression
    init = e->Initializer ();
    if (init) {
      type = init->Son (1)->Type ();
      init = init->Son (1);

      if (! CCSemExpr::isConstIntExpr (init)) {
        if (! CCSemExpr::isDependent (init)) {
          SEM_ERROR (e, "value for enumerator `" << e->Name ()->Text ()
            << "' isn't integer constant expression");
        }
        return e;
      } else {
        v = init->Value ()->Constant ();
        if (type->VirtualType ()->is_signed ()) {
          value = v->convert_to_int ();
        } else {
          value = v->convert_to_uint ();
        }
      }
    // else get the value from the predecessor enumerator incremented by 1
    } else if (numenums > 1) {
      if (! enuminfo->Enumerator (numenums - 2)->hasValue ())
        return e;
      value = enuminfo->Enumerator (numenums - 2)->Value ();
      value++;
    // else this is the first enumerator, so the value is 0
    } else {
      value = 0;
    }

    // set the value
    einfo->Value (value);
    einfo->hasValue (true);

    // determine the underlying type of the enumeration,
    // must be big enough to hold all enumerator values
    type = enuminfo->UnderlyingType ();
    bool long_gt_int = CTYPE_LONG.Size() > CTYPE_INT.Size();
    long int size = CTypePrimitive::SizeOfNumber (value);
    if (value < 0) {
      // signed type
      if (type->is_unsigned ()) {
        type = long_gt_int && *type == CTYPE_UNSIGNED_INT ? &CTYPE_LONG : &CTYPE_LONG_LONG;
      } else if (size > CTYPE_LONG_LONG.Size()) {
        type = &CTYPE_INT128;
      } else if (size > CTYPE_LONG.Size()) {
        type = &CTYPE_LONG_LONG;
      } else if (size > CTYPE_INT.Size()) {
        type = long_gt_int && *type == CTYPE_INT ? &CTYPE_LONG : &CTYPE_LONG_LONG;
      }
    } else {
      // unsigned, if value cannot be represented by a
      // signed type and no other value is signed
      bool is_unsigned = numenums == 1 || type->is_unsigned ();
      if (size <= CTYPE_INT.Size()) {
        type = &CTYPE_INT;
      } else if (size <= CTYPE_UNSIGNED_INT.Size()) {
        type = is_unsigned ? &CTYPE_UNSIGNED_INT : long_gt_int ? &CTYPE_LONG : &CTYPE_LONG_LONG;
      } else if (size <= CTYPE_LONG.Size()) {
        type = &CTYPE_LONG;
      } else if (size <= CTYPE_UNSIGNED_LONG.Size()) {
        type = is_unsigned ? &CTYPE_UNSIGNED_LONG : &CTYPE_LONG_LONG;
      } else if (size <= CTYPE_LONG_LONG.Size()) {
        type = &CTYPE_LONG_LONG;
      } else if (size <= CTYPE_UNSIGNED_LONG_LONG.Size()) {
        type = &CTYPE_UNSIGNED_LONG_LONG;
      } else {
        type = is_unsigned ? &CTYPE_UNSIGNED_INT128 : &CTYPE_INT128;
      }
    }
    if (enuminfo->UnderlyingType ()->rank () < type->rank ()) {
      enuminfo->UnderlyingType (type);
    }
  }

  return e;
}


/*****************************************************************************/
/*                                                                           */
/*                                Classes                                    */
/*                                                                           */
/*****************************************************************************/


void CCSemantic::enter_class_def () {
  if (! _outermost_class_def) {
    // find the outermost not complete parsed class or union
    CScopeInfo* scope = current_scope;
    while (scope && !scope->isFile()) {
      if (scope->Record() && !scope->Record()->isCompleteParsed())
        _outermost_class_def = (CRecord*)scope;
      scope = scope->Parent();
    }
  }
  // save list position to be able to reject delayed function
  // bodies and initializers of this class if parsing the
  // class definition fails
  _outermost_class_def->saveParseDelayedListLength();
}


void CCSemantic::leave_class_def (bool reject) {
  // mark as complete parsed and clean up
  current_scope->Record()->isCompleteParsed(true);

  // if parsing the class failed, reject the function bodies and
  // initializers of this class noted to be parsed at the end of
  // the outermost class definition
  if (reject)
    _outermost_class_def->rejectParseDelayed();
  else
    _outermost_class_def->forgetParseDelayedListLength();

  // leaving outermost class def?
  if (_outermost_class_def == current_scope) {
    // now parse function bodies and several initializers not parsed due
    // to potential use of names of members that could not be resolved
    // untill the end of the class definition
    parseDelayed();
    _outermost_class_def = 0;
  }

  // clean up
  if (current_scope->Record()->ParseDelayed())
    current_scope->Record()->resetDelayed();
}


CTree *CCSemantic::introduce_class () {
  bool error, is_special;
  CStructure *lookup_scope;
  CTemplateInfo *tinfo;
  CObjectInfo *info, *prev;
  CT_ClassDef *cs;

  // determine class properties
  cs = (CT_ClassDef*) builder ().class_head1 ();
  is_special = isSpecialization (cs->Name ());

  // implicit template scope around class template explicit specialization
  if (is_special && ! current_scope->TemplateInfo ()) {
    info = cs->Name ()->Object ();
    if (info && info->TemplateInfo ())
      reenter_scope (info->Structure ());
  }

  prev = 0;
  info = 0;

  // lookup name
  CCNameLookup nl (*_err, current_scope, false);
  lookup_scope = is_special ? current_scope : getNonTplScope ();
  nl.lookupType (cs->Name (), lookup_scope);
  error = true;

  if (nl.Objects ()) {
    prev = nl.Object ();
    // class already defined in lookup scope
    if (prev->Record () && prev->Record ()->isDefined () && ! is_special) {
      SEM_ERROR__redefinition (cs, cs->Name (), prev);
    // conflicting declaration in lookup scope
    } else if (! prev->Record () ||
               (cs->NodeName () == CT_ClassDef::NodeId () && ! prev->ClassInfo ()) ||
               (cs->NodeName () == CT_UnionDef::NodeId () && ! prev->UnionInfo ())) {
      SEM_ERROR__already_defined (cs, cs->Name (), prev);
    } else
      error = false;
  // class defined using qualified name but class does not exist
  } else if (isQualName (cs->Name ())) {
    std::ostringstream name; qualifiedScopeName (cs->Name (), name);
    SEM_ERROR__not_member (cs, cs->Name ()->Name (), name.str ().c_str ());
  } else
    error = false;

  if (error) {
    delete cs;
    return (CTree*)0;
  }

  // union definition
  if (cs->NodeName () == CT_UnionDef::NodeId ()) {
    info = current_scope->newUnion ();
    info->TypeInfo (new CTypeUnion (info->UnionInfo ()));
    setProtection (info);
    // set member access
    _protection.push (CProtection::PROT_PUBLIC);
  // class definition
  } else {
    info = current_scope->newClass ();
    info->TypeInfo (new CTypeClass (info->ClassInfo ()));
    setProtection (info);
    // set member access
    if (cs->token ()->type () == TOK_STRUCT)
      _protection.push (CProtection::PROT_PUBLIC);
    else
      _protection.push (CProtection::PROT_PRIVATE);
  }

  // complete semantic information about class or union
  info->Name (cs->Name ()->Text ());
  enter_scope (info->Structure ());
  setLanguage (info);
  common_settings (info, cs);
  cs->Name ()->Object (info);
  cs->Name ()->setTypeRef (info->TypeInfo ());
  cs->Object (info);

  // link forward declarations with this definition
  if (prev)
    prev->NextObject (info);

  // handle class templates
  tinfo = current_scope->Parent ()->TemplateInfo ();
  if (tinfo) {
    setSpecializationName (cs->Name (), tinfo);
    setTemplateScope (nl, info, info->TypeInfo (), tinfo, isQualName (cs->Name ()));
    introduceTemplate (info, tinfo);
  }

  // set qualified scope if this is the definition of a nested class
  // outside of the class definition
  if (isQualName (cs->Name ()) && prev && prev->Structure ()) {
    info->Record ()->QualifiedScope (getQualifiedScope (
      prev->Structure ()->Parent ()->Structure ()));
  }

  // semantic state to let parse function bodies and several initializers
  // later due to potential use of names of members that could not be
  // resolved untill the end of the class definition
  enter_class_def ();

  return cs;
}


CTree *CCSemantic::add_base_classes () {
  CT_ClassDef *cd = (CT_ClassDef*)builder ().class_head ();
  addBaseClasses (current_scope->ClassInfo (), cd);
  return cd;
}


void CCSemantic::addBaseClasses (CClassInfo *cinfo, CT_ClassDef *cd) {
  CT_BaseSpecList *bl = cd->BaseClasses ();
  if (cinfo && bl) {
    int num = bl->Entries ();
    for (int i = 0; i < num; i++)
      addBaseClass (cinfo, (CT_BaseSpec*)bl->Entry (i));
  }
}


void CCSemantic::addBaseClass (CClassInfo *cinfo, CT_BaseSpec *bs) {
  CBaseClassInfo *bcinfo;
  CObjectInfo *info;
  CTypeClass *type;
  CTypeInfo *otype;
  int access_level;
  CTree *access;

  info = bs->Name ()->Name ()->Object ();
  if (! info) {
    // must depend on a template parameter
    cinfo->hasDepBaseClass (true);
    return;
  }

  // handle typedefs
  if (info->TypedefInfo ())
    otype = info->TypedefInfo ()->TypeInfo ();
  else
    otype = info->TypeInfo ();

  type = otype->TypeClass ();
  if (! type) {
    // if not class type this must be a template type param type
    if (! otype->TypeTemplateParam () ||
        ! otype->TypeTemplateParam ()->isType ()) {
      SEM_ERROR__not_class (bs, bs->Name ());
    }
    cinfo->hasDepBaseClass (true);
    return;
  }

  if (type->ClassInfo ()->TemplateInstance () &&
      ! type->ClassInfo ()->TemplateInstance ()->canInstantiate ()) {
    cinfo->hasDepBaseClass (true);
  }
  else if (type->ClassInfo ()->hasDepBaseClass ()) {
    cinfo->hasDepBaseClass (true);
  }

  // create new base class semantic info object
  bcinfo = cinfo->newBaseClass (type->ClassInfo ());
  common_settings (bcinfo, bs);
  bcinfo->Scope (cinfo);
  bcinfo->TypeInfo (&CTYPE_UNDEFINED);
  Push (bcinfo);
  bs->Name ()->Object (type->ClassInfo ());
  bs->Name ()->setTypeRef (type);

  // determine properties of base class
  if (bs->Virtual ())
    bcinfo->isVirtual (true);
  access = bs->AccessSpec ();
  if (access) {
    access_level = access->token ()->type ();
    if (access_level == TOK_PUBLIC)
      bcinfo->Protection (CProtection::PROT_PUBLIC);
    else if (access_level == TOK_PROTECTED)
      bcinfo->Protection (CProtection::PROT_PROTECTED);
    else // if (access_level == TOK_PRIVATE)
      bcinfo->Protection (CProtection::PROT_PRIVATE);
  // ���11.2.2 no access specifier given: the base class is assumed
  // to be PUBLIC when derived class is declared >struct<
  // otherwise base class is PRIVATE
  } else {
    if (cinfo->isStruct ())
      bcinfo->Protection (CProtection::PROT_PUBLIC);
    else
      bcinfo->Protection (CProtection::PROT_PRIVATE);
  }
}


CTree *CCSemantic::access_spec () {
  CT_AccessSpec *as = (CT_AccessSpec*)builder ().member_decl ();
  if (as->Access () == TOK_PUBLIC)
    _protection.top () = CProtection::PROT_PUBLIC;
  else if (as->Access () == TOK_PROTECTED)
    _protection.top () = CProtection::PROT_PROTECTED;
  else if (as->Access () == TOK_PRIVATE)
    _protection.top () = CProtection::PROT_PRIVATE;
  return as;
}


CTree *CCSemantic::class_spec () {
  CObjectInfo *info, *oinfo;
  CMemberAliasInfo *minfo;
  CT_ClassDef *result;
  CStructure *scope;

  // switch to enclosing member access level
  _protection.pop ();

  result = (CT_ClassDef*)builder ().class_spec ();
  result->Members ()->Scope (current_scope);

  // ���12.1 implicitly declare default constructor, copy constructor,
  // copy assignment operator, and destructor if not explicitly
  // declared by the program
  declareSpecialMembers ();

  // finished parsing a class or union definition; now parse
  // function bodies and several initializers not parsed due to
  // potential use of names of members that could not be resolved
  // untill the end of the class definition
  leave_class_def ();

  // ���9.5.2 if this is an anonymous union and no instance of
  // this union is created, then the data members of that union are
  // introduced in the enclosing scope
  // as an extension to the C++ language standard anonymous structs
  // are supported too
  if (! (current_scope->isAnonymous () && syntax ().look_ahead (TOK_SEMI_COLON) &&
         (current_scope->isUnion () || (current_scope->isClass () &&
          current_scope->ClassInfo ()->isStruct ()))))
    return result;

  scope = current_scope->Parent ()->Structure ();
  for (unsigned i = 0; i < current_scope->/*Objects*/Attributes (); i++) {
    info = current_scope->/*Object*/Attribute (i);

    //// skip implicitly declared member functions
    //if (info->isBuiltin ())
    //  continue;
    //
    //// ���9.5.2 only non-static data members shall be defined in
    //// an anonymous union
    //if (! info->AttributeInfo ()) {
    //  SEM_ERROR (result,
    //    "anonymous unions can only have non-static data members");
    //  continue;
    //}

    // introduce member in enclosing scope; there shall be
    // no other entity with the same name
    CCNameLookup nl (*_err, current_scope, !is_entity_name());
    nl.lookup (info->Name (), scope);
    if (nl.Objects ()) {
      oinfo = nl.Object ();
      if (! (oinfo->Record () || oinfo->EnumInfo ())) {
        SEM_ERROR (info->Tree (), "redefinition of `" << info->Name () << "'");
        SEM_MSG__prev_loc (oinfo);
        continue;
      }
    }

    // create new member alias in enclosing scope
    minfo = scope->newMemberAlias (info, true);
    minfo->Name (info->Name ());
    minfo->Member (info);
    minfo->TypeInfo (info->TypeInfo ()->Duplicate ());
    setProtection (minfo); // adjust member access
    Push (minfo);
    common_settings (minfo, info->Tree ());
  }
  return result;
}


CTree *CCSemantic::class_spec_err () {
  // finished parsing a class or union definition; reject parsing
  // function bodies and several initializers not parsed due to
  // potential use of names of members that could not be resolved
  // untill the end of the class definition
  leave_class_def (true);

  // switch to enclosing member access level
  _protection.pop ();

  return 0;
}


void CCSemantic::declareImplicitFcts () {
  CFunctionInfo *info;
  CTypeInfo *type;
  CTypeList *args;

  // ���3.7.3 declare global allocation and deallocation
  // functions new and delete

  // allocation functions
  // void *operator new(std::size_t) throw(std::bad_alloc);
  args = new CTypeList (1);
  args->AddEntry (CTypeInfo::CTYPE_SIZE_T);
  type = new CTypePointer (&CTYPE_VOID);
  type = new CTypeFunction (type, args, true);
  info = createFunction ("operator new", current_scope, type);
  info->isOperator (true);
  createParameter (info, args->Entry (0));
  // void *operator new[](std::size_t) throw(std::bad_alloc);
  args = new CTypeList (1);
  args->AddEntry (CTypeInfo::CTYPE_SIZE_T);
  type = new CTypePointer (&CTYPE_VOID);
  type = new CTypeFunction (type, args, true);
  info = createFunction ("operator new[]", current_scope, type);
  info->isOperator (true);
  createParameter (info, args->Entry (0));

  // deallocation functions
  // void operator delete(void *) throw();
  type = new CTypePointer (&CTYPE_VOID);
  args = new CTypeList (1);
  args->AddEntry (type);
  type = new CTypeFunction (&CTYPE_VOID, args, true);
  info = createFunction ("operator delete", current_scope, type);
  info->isOperator (true);
  createParameter (info, args->Entry (0));
  // void operator delete[](void *) throw();
  type = new CTypePointer (&CTYPE_VOID);
  args = new CTypeList (1);
  args->AddEntry (type);
  type = new CTypeFunction (&CTYPE_VOID, args, true);
  info = createFunction ("operator delete[]", current_scope, type);
  info->isOperator (true);
  createParameter (info, args->Entry (0));
}


void CCSemantic::declareSpecialMembers () {
  bool has_ctor, has_copy, has_assign, has_dtor;
  CFunctionInfo *info;
  CTypeInfo *type;
  CTypeList *args;
  CRecord *scope;

  scope = current_scope->Record ();
  if (! scope)
    return;

  // check explicit declarations
  has_ctor = has_copy = has_assign = has_dtor = false;
  for (unsigned i = 0; i < scope->Functions (); i++) {
    info = scope->Function (i);

    if (info->isConstructor ()) {
      has_ctor = true;
      if (info->Arguments () == 1 ||
          (info->Arguments () > 1 && info->Argument (1)->hasDefaultArg ())) {
        type = info->Argument ((unsigned)0)->TypeInfo ();
        if (isRefToClass (type, scope))
          has_copy = true;
      }
    } else if (info->isOperator ()) {
      if (strcmp (info->Name (), "operator =") == 0 &&
          info->Arguments () == 1) {
        type = info->Argument ((unsigned)0)->TypeInfo ();
        if (isRefToClass (type, scope))
          has_assign = true;
      }
    } else if (info->isDestructor ())
      has_dtor = true;
  }

  // create implicit members

  // implicit destructor
  if (! has_dtor) {
    std::ostringstream name;
    name << "~" << scope->Name ();
    type = new CTypeFunction (&CTYPE_UNDEFINED, new CTypeList (1), true);
    info = createMethod (name.str ().c_str (), scope, type);
    info->isDestructor (true);
  }
  // implicit default constructor
  if (! has_ctor) {
    type = new CTypeFunction (&CTYPE_UNDEFINED, new CTypeList (1), true);
    info = createMethod (scope->Name (), scope, type);
    info->isConstructor (true);
  }
  // implicit copy constructor
  if (! has_copy) {
    type = scope->TypeInfo ()->Duplicate ();
    type = new CTypeAddress (new CTypeQualified (type, true, false, false));
    args = new CTypeList (1);
    args->AddEntry (type);
    type = new CTypeFunction (&CTYPE_UNDEFINED, args, true);
    info = createMethod (scope->Name (), scope, type);
    info->isConstructor (true);
    createParameter (info, args->Entry (0));
  }
  // implicit assignment operator
  if (! has_assign) {
    type = scope->TypeInfo ()->Duplicate ();
    type = new CTypeAddress (new CTypeQualified (type, true, false, false));
    args = new CTypeList (1);
    args->AddEntry (type);
    type = scope->TypeInfo ()->Duplicate ();
    type = new CTypeAddress (type);
    type = new CTypeFunction (type, args, true);
    info = createMethod ("operator =", scope, type);
    info->isOperator (true);
    createParameter (info, args->Entry (0));
  }
}


CFunctionInfo *CCSemantic::createMethod (const char *name, CRecord *scope,
 CTypeInfo *type) {
  CFunctionInfo *info;
  info = createFunction (name, scope, type);
  info->Protection (CProtection::PROT_PUBLIC);
  info->isInline (true);
  info->QualifiedScope (getQualifiedScope (scope));
  return info;
}


bool CCSemantic::isRefToClass (CTypeInfo *type, CRecord *record) const {
  // ���12.8.9 type must be X, X&, const X&, volatile X&, or const volatile X&
  if (type) {
    type = type->UnqualType ();
    if (type->TypeAddress ()) {
      type = type->BaseType ()->UnqualType ();
    }
    if (type->TypeRecord () && type->TypeRecord ()->Record () == record)
      return true;
  }
  return false;
}


CTree *CCSemantic::ctor_init () {
  CT_MembInitList *result;
  result = (CT_MembInitList*)builder ().ctor_init ();
  result->Scope (current_scope);
  return result;
}


CTree *CCSemantic::introduce_member () {
  bool is_fct, is_friend, is_typedef, is_qual_name, redef_typedef, is_special, new_fct;
  CStructure *scope, *lookup_scope;
  CObjectInfo *info, *oinfo = 0;
  CTemplateInfo *tinfo;
  CSemDeclSpecs *dsi;
  CTree *d;

  // check declaration parse state
  info = current_fct;
  finish_declarator ();
  if (! in_decl ())
    return (CTree*)0;

  // determine member type
  d = builder ().member_declarator ();
  dsi = sem_decl_specs ();
  CCSemDeclarator csd (_err, dsi->make_type (), d);
  if (! csd.Name ()) {
    delete d;
    return (CTree*)0;
  }

  // determine member properties
  is_friend = (dsi->declared (CT_PrimDeclSpec::PDS_FRIEND));
  is_typedef = (dsi->declared (CT_PrimDeclSpec::PDS_TYPEDEF));
  is_fct = (! is_typedef && csd.Type ()->isFunction () && ! csd.Type ()->isAddress ());
  is_qual_name = isQualName (csd.Name ());
  is_special = isSpecialization (csd.Name ());
  lookup_scope = is_special ? current_scope : getNonTplScope ();
  new_fct = false;

  // lookup name
  CCNameLookup nl (*_err, current_scope, false);
  if (! is_friend || ! is_fct || is_qual_name) {
    scope = getNameScope (csd.Name ());
    if (is_qual_name && scope == lookup_scope) {
      // ignore extra qualification if it refers to current class scope
      // lookup as if no qualification is given
      is_qual_name = false;
      nl.lookupFunction (csd.Name ()->Name (), lookup_scope);
    } else {
      if (is_qual_name) {
        // method of another class declared in enclosing class
        if (! is_friend) {
          SEM_ERROR__cannot_declare (d, csd.Name (), lookup_scope);
          delete d;
          return (CTree*)0;
        }
        lookup_scope = scope;
      }
      nl.lookupFunction (csd.Name (), lookup_scope);
    }
  } else { // friend && fct && unqualified
    scope = lookup_scope = getFriendScope (false);
    nl.lookupFunction (csd.Name (), lookup_scope);
    if (! nl.Objects ()) {
      // lookup in non-template scope
      scope = lookup_scope = getFriendScope (true);
      nl.lookupFunction (csd.Name (), lookup_scope);
    }
  }

  // check member name lookup
  if (! check_member_lookup (nl, lookup_scope, dsi, csd, d, redef_typedef, is_qual_name)) {
    delete d;
    return (CTree*)0;
  }

  // friend declaration for member of another class, find the
  // matching member
  if (is_friend && is_qual_name) {
    info = 0;

    for (unsigned i = 0; ! info && i < nl.Objects (); i++) {
      CObjectInfo* oinfo = nl.Object (i);
      // TODO: handle templates and specializations correctly
      if (oinfo && (current_scope->TemplateInfo () || oinfo->isTemplate () ||
          equalType (csd.Type (), oinfo->TypeInfo (), false))) {
        info = oinfo;
      }
    }

    if (! info) {
      std::ostringstream qname; qualifiedScopeName (csd.Name (), qname);
      SEM_ERROR__no_matching_decl (d, csd.Name ()->Name (), qname.str ().c_str ());
      delete d;
      return (CTree*)0;
    }
  }

  // member typedef declaration
  if (is_typedef) {
    info = current_scope->newTypedef ();
    Push (info);
  // member function declaration
  } else if (is_fct) {
    if (! info) {
      if (is_friend)
        current_fct = lookup_scope->newFunction ();
      else
        current_fct = current_scope->newFunction ();
      info = current_fct;
      new_fct = true;
    } else if (! is_friend) {
      current_scope->addFunction (info->FunctionInfo ());
      new_fct = true;
    }

    // handle template functions
    if (new_fct && current_scope->TemplateInfo ())
      setTemplateScope (nl, info, csd.Type (), current_scope->TemplateInfo (), is_qual_name);

    if (! is_friend || ! is_qual_name) {
      // determine member function category
      if (lookup_scope->Name () &&
          strcmp (lookup_scope->Name (), csd.Name ()->Text ()) == 0)
        info->FunctionInfo ()->isConstructor (true);
      else if (csd.Name ()->Name ()->NodeName () == CT_DestructorName::NodeId ())
        info->FunctionInfo ()->isDestructor (true);
      else if (csd.Name ()->Name ()->NodeName () == CT_OperatorName::NodeId ())
        info->FunctionInfo ()->isOperator (true);
      else if (csd.Name ()->Name ()->NodeName () == CT_ConversionName::NodeId ()) {
        info->FunctionInfo ()->isConversion (true);
        info->FunctionInfo ()->ConversionType (
          ((CT_ConversionName*)csd.Name ()->Name ())->TypeName ()->Object ()->TypeInfo ());
      }

      // determine further function properties
      info->Linkage (determine_linkage (dsi, FCT));
    }

    // add friend to enclosing class
    if (is_friend) {
      info->FunctionInfo ()->addFriend (current_scope->Record ());
      if (! is_qual_name && new_fct)
        info->FunctionInfo ()->AssignedScope (lookup_scope);
    } else {
      info->FunctionInfo ()->QualifiedScope (getQualifiedScope (lookup_scope));

      // create implicit `this' parameter
      addImplicitThisObject (info->FunctionInfo (), csd.Type (), d);
    }

    csd.Type ()->VirtualType ()->TypeFunction ()->FunctionInfo (info->FunctionInfo ());
  // data member declaration
  } else {
    info = current_scope->newAttribute ();
    info->Linkage (determine_linkage (dsi, ATTRIB));
    if (dsi->declared (CT_PrimDeclSpec::PDS_STATIC))
      info->Storage (dsi->declared (CT_PrimDeclSpec::PDS_THREAD) ?
        CStorage::CLASS_THREAD : CStorage::CLASS_STATIC);
    else
      info->Storage (determine_storage_class (dsi));
    info->AttributeInfo ()->QualifiedScope (getQualifiedScope (lookup_scope));
    Push (info);
  }

  // determine common properties of the member
  if (! is_friend)
    setProtection (info);

  if (! is_friend || ! is_fct || new_fct) {
    setLanguage (info);
    setSpecifiers (info, dsi);
    common_settings (info, d, &csd);

    // handle "redefining" typedefs such as `typedef class X X;'
    if (redef_typedef) {
      char aname[100];
      sprintf (aname, "%%anon%ld", _Anonymous);
      _Anonymous++;
      info->Name (aname);

      // link previous declarations with this one
      if (oinfo && oinfo->TypedefInfo ())
        oinfo->NextObject (info);
    }
    else if (info->TypedefInfo ()) {
      CTypeInfo *type = info->TypeInfo ()->TypedefInfo (info);
      info->TypeInfo (type);
    }

    if (is_fct && ! is_special)
      linkFctDecls (nl, info);
  }

  // handle template declarations
  tinfo = current_scope->TemplateInfo ();
  if (tinfo) {
    setSpecializationName (csd.Name (), tinfo);
    introduceTemplate (info, tinfo, is_friend);
  }

  // attribute syntax tree
  if (d->NodeName () == CT_InitDeclarator::NodeId ())
    ((CT_InitDeclarator*)d)->Object (info);
  else
    ((CT_BitFieldDeclarator*)d)->Object (info);

  if (is_fct)
    current_fct = 0;

  // if an attribute with class object type, ensure
  // that the class is really instantiated
  CTypeInfo* type = info->TypeInfo();
  if (info->AttributeInfo() && type->UnqualType()->TypeClass()) {
    type->UnqualType()->TypeClass()->instantiate(current_scope);
  }

  return d;
}


bool CCSemantic::check_member_lookup (CCNameLookup &nl, CStructure *lookup_scope,
 CSemDeclSpecs *dsi, CCSemDeclarator &csd, CTree *d, bool &redef_typedef,
 bool is_qual_name) {
  bool is_friend, is_typedef, is_fct;
  CObjectInfo *oinfo;

  is_friend = (dsi->declared (CT_PrimDeclSpec::PDS_FRIEND));
  is_typedef = (dsi->declared (CT_PrimDeclSpec::PDS_TYPEDEF));
  is_fct = (! is_typedef && csd.Type ()->isFunction () && ! csd.Type ()->isAddress ());
  redef_typedef = false;

  // member declared using qualified name but member not exists
  if (! nl.Objects () && is_qual_name) {
    std::ostringstream name; qualifiedScopeName (csd.Name (), name);
    SEM_ERROR__not_member (d, csd.Name ()->Name (), name.str ().c_str ());
  // check all members looked up
  } else if (nl.Objects ()) {
    oinfo = nl.Object ();
    // ���7.1.3.2 typedef redefining same type
    if (is_typedef &&
        ((oinfo->TypedefInfo () &&
          (equalType (csd.Type (), oinfo->TypeInfo ()) ||
           (! real_inst && csd.Type ()->isTemplateInstance () &&
            oinfo->TypeInfo ()->isTemplateInstance ()))) ||
         (oinfo->TypeInfo ()->TypeRecord () &&
          csd.Type ()->TypeRecord () &&
          *csd.Type ()->TypeRecord ()->Record () ==
          *oinfo->TypeInfo ()->TypeRecord ()->Record ()) ||
         (oinfo->TypeInfo ()->TypeEnum () &&
          csd.Type ()->TypeEnum () &&
          *csd.Type ()->TypeEnum ()->EnumInfo () ==
          *oinfo->TypeInfo ()->TypeEnum ()->EnumInfo ())))
      redef_typedef = true;

    // conflicting declaration in lookup scope
    if (is_typedef && ! redef_typedef) {
      SEM_ERROR__duplicate (d, "nested type", csd.Name ());
    // check all functions looked up
    } else if (is_fct && oinfo->FunctionInfo ()) {
      unsigned i = 0;
      do {
        // duplicate member function
        if (! is_friend &&
            (current_fct ? oinfo->isTemplate () ==
             current_fct->isTemplate () :
             current_scope ? oinfo->isTemplate () ==
             (current_scope->TemplateInfo ()?true:false) : true) &&
            equalType (csd.Type (), oinfo->TypeInfo ())) {
          SEM_ERROR__already_defined (d, csd.Name (), oinfo);
          return false;
        }
      } while (++i < nl.Objects () && (oinfo = nl.Object (i)));
      return true;
    // conflicting member declarations
    } else if (is_fct && ! oinfo->FunctionInfo () &&
               ! (oinfo == lookup_scope || isClassOrEnum (oinfo))) {
      SEM_ERROR__duplicate (d, "member", csd.Name ());
    } else
      return true;
  } else
    return true;
  return false;
}


CTree *CCSemantic::member_decl1 () {
  CT_ObjDecl *od;

  od = (CT_ObjDecl*)builder ().member_decl1 ();
  obj_decl (od); // link object decl and init declarators

  if (od && ! od->Declarators ()->Entries ()) {
    // ���9.2.7 the member-declarator-list can be ommited only after a
    // class- or enum-specifier, or a decl-specifier-seq of the form
    // `friend elaborated-type-specifier'
    CT_DeclSpecSeq *dss = od->DeclSpecs ();
    int entries = dss->Entries ();
    if (! entries) // nothing declared
      return od;
    const char *entry = dss->Entry (0)->NodeName ();
    if (entries == 1 &&
        (entry == CT_ClassSpec::NodeId () || entry == CT_ClassDef::NodeId () ||
         entry == CT_UnionSpec::NodeId () || entry == CT_UnionDef::NodeId () ||
         entry == CT_EnumSpec::NodeId () || entry == CT_EnumDef::NodeId ()))
      return od;
    entry = dss->Entry (1)->NodeName ();
    if (entries == 2 && dss->Entry (0)->token ()->type () == TOK_FRIEND &&
        (entry == CT_ClassSpec::NodeId () ||
         entry == CT_UnionSpec::NodeId () ||
         entry == CT_EnumSpec::NodeId ()))
      return od;
    return (CTree*)0;
  }
  return od;
}


CTree *CCSemantic::member_declarator_err () {
  finish_declarator ();
  return (CTree*)0;
}


bool CCSemantic::inTemplateDecl (CScopeInfo* scope) const {
  if (! scope)
    return false;
  if (scope->TemplateInfo ())
    return true;
//  if (scope->NamespaceInfo () && scope->NamespaceInfo ()->aroundInstantiation ())
//    return true;
  return scope->Parent () != scope && inTemplateDecl (scope->Parent ());
}


CTree *CCSemantic::introduce_tag () {
  bool is_friend, is_typename, is_special, do_lookup, is_decl, is_qual_name, in_tpl_decl;
  CStructure *lookup_scope, *scope;
  CObjectInfo *info, *dinfo, *prev;
  CTemplateInfo *tinfo;
  CT_SimpleName *name;
  CT_DeclSpecSeq *dss;
  CT_ClassSpec *tag;

  // determine class properties
  tag = (CT_ClassSpec*) builder ().elaborated_type_spec ();
  info = prev = 0;
  name = tag->Name ();
  is_qual_name = isQualName(name);
  is_typename = (tag->token ()->type () == TOK_TYPENAME);
  in_tpl_decl = inTemplateDecl (current_scope);
  is_friend = false;
  is_special = isSpecialization (name);
  do_lookup = true;
  dss = decl_specs ();
  lookup_scope = getNonTplScope ();

  // get correct name object
  //while (name->NodeName () == CT_RootQualName::NodeId ())
  //  name = (CT_SimpleName*)name->Son (1);

  // declarations of the form `class-key identifier ;'
  is_decl = syntax().look_ahead(TOK_SEMI_COLON, syntax().look_ahead(TOK_FRIEND) ? 2 : 1);

  // get friend status
  // declarations of the form `friend class-key identifier ;'
  if (is_decl && lookup_scope->isRecord ()) {
    if (tag->NodeName () != CT_EnumSpec::NodeId ()) {
      if (dss->Sons () == 1 && dss->token ()->type () == TOK_FRIEND) {
        is_friend = true;
      }
      else if (dss->Sons () == 0 && syntax ().look_ahead (TOK_FRIEND)) {
        is_friend = true;
      }
    }
  }

  // handle forward declaration of template instance
  if (is_special && ! current_scope->TemplateInfo ()) {
    info = name->Name ()->Object ();
    if (info) {
      if (is_friend && info->Structure () && current_scope->Record ())
        info->Structure ()->addFriend (current_scope->Record ());
      deleteContainer (2);
      name->Object (info);
      name->setTypeRef (info->TypeInfo ());
      tag->Object (info);
      return tag;
    }
  }

  // check whether this is a dependent nested name and
  // therefore name lookup need not to be performed
  if (is_typename && in_tpl_decl && is_qual_name && name->Entries () > 1) {
    dinfo = ((CT_SimpleName*)name->Entry (name->Entries ()-2))->Object ();
    if (! dinfo || (dinfo->TypeInfo () &&
        (dinfo->TypeInfo ()->TypeTemplateParam () ||
         dinfo->TypeInfo ()->is_unknown_t ())))
      do_lookup = false;
  }

  // lookup name
  CCNameLookup nl (*_err, current_scope);
  if (do_lookup) {
    if (! is_friend) {
      nl.lookupType (name, lookup_scope, !is_decl, !is_decl);
    } else {
      nl.lookupFriendClass (name, lookup_scope);
    }
  }

  // check name lookup results
  if (nl.Objects ()) {
    prev = info = nl.Object ();
    // class key used with wrong type name
    if (! is_typename &&
        ((tag->NodeName () == CT_ClassSpec::NodeId () && ! info->ClassInfo ()) ||
         (tag->NodeName () == CT_UnionSpec::NodeId () && ! info->UnionInfo ()) ||
         (tag->NodeName () == CT_EnumSpec::NodeId ()  && ! info->EnumInfo ()))) {
      SEM_ERROR (tag, "wrong use of `" << *name << "'");
      return (CTree*)0;
    }
  // handle `typename' keyword
  } else if (is_typename && in_tpl_decl) {
    info = getLastResolvedName (name);
    if (! info)
      return 0;
  // qualified name must name a type
  } else if (is_qual_name) {
    std::ostringstream qname; qualifiedScopeName (name, qname);
    SEM_ERROR__not_member (tag, name->Name (), qname.str ().c_str ());
    return (CTree*)0;
  }

  // delete possibly created container object because it would
  // be deleted by the builder only when the rule fails
  deleteContainer (2);

  // create tag
  if (! is_qual_name && ! is_typename && (! prev || (! is_friend && prev->isTemplate ()))) {
    bool is_instance = parsing_class_inst && tpl_inst->Instance () &&
                       ! tpl_inst->Instance ()->TemplateInstance ()->isInstantiated () &&
                       current_scope->isNamespace ();
    // forward class declaration
    if (tag->NodeName () == CT_ClassSpec::NodeId ()) {
      if (is_instance) {
        info = tpl_inst->Instance ();
        tpl_inst->insertInstance (info);
      } else {
        info = _db->newClass();
        info->TypeInfo (new CTypeClass (info->ClassInfo ()));
      }
    // forward union declaration
    } else if (tag->NodeName () == CT_UnionSpec::NodeId ()) {
      if (is_instance) {
        info = tpl_inst->Instance ();
        tpl_inst->insertInstance (info);
      } else {
        info = _db->newUnion();
        info->TypeInfo (new CTypeUnion (info->UnionInfo ()));
      }
    // forward enum declaration
    } else {
      info = current_scope->newEnum ();
      info->TypeInfo (new CTypeEnum (info->EnumInfo ()));
    }

    // ���3.3.1 introduce name into enclosing non-class, non-function
    // prototype scope if declaration has the form `.. class-key identifier ..'
    scope = current_scope;
    if (is_friend || ! syntax ().look_ahead (TOK_SEMI_COLON)) {
      if (prev && info->Record ()) {
        info->Record ()->isHiddenForwardDecl (true);
      }
      while (scope->isRecord () || scope->isFunction () || scope->TemplateInfo () ||
             (scope->NamespaceInfo () && scope->NamespaceInfo ()->aroundInstantiation ()))
        scope = scope->Parent ()->Structure ();
    } else {
      setProtection (info);
    }

    if (! info->EnumInfo ())
      info->ScopeInfo ()->Parent (scope);

    scope->addType (info);
    common_settings (info, tag);

    if (! info->isTemplateInstance ()) {
      // complete semantic information about the forward declaration
      info->Name (name->Text ());
      Push (info);

      // link with previous declarations
      if (prev)
        prev->NextObject (info);
    }

    // handle template declarations
    tinfo = current_scope->TemplateInfo ();
    if (tinfo && is_decl) {
      setSpecializationName (name, tinfo);
      setTemplateScope (nl, info, info->TypeInfo (), tinfo, is_qual_name);
      introduceTemplate (info, tinfo, is_friend);
    }
  }

  if (is_friend) {
    scope = getNonTplScope ();
    if (scope && scope->Record ())
      info->Structure ()->addFriend (scope->Record ());
  }

  // attribute syntax tree
  name->Object (info);
  name->setTypeRef (info->TypeInfo ());
  tag->Object (info);
  return tag;
}


CObjectInfo *CCSemantic::getLastResolvedName (CT_SimpleName *name) const {
  CObjectInfo *info = 0;
  int entries;

  if (name->NodeName () == CT_TemplateName::NodeId ())
    info = name->Object ();
  else if (name->NodeName () == CT_QualName::NodeId () ||
           name->NodeName () == CT_RootQualName::NodeId ()) {
    entries = name->Entries ();
    for (unsigned i = entries; i > 0; i--) {
      info = ((CT_SimpleName*)name->Entry (i-1))->Object ();
      if (info)
        return info;
    }
  } else
    info = name->Name ()->Object ();
  return info;
}


// parsing class definitions


void CCSemantic::Delete () {
  CObjectInfo *info = Top (); // get item to delete
  if (! info)
    return;

  // parsing template instance failed
  if (tpl_inst && info == tpl_inst->Instance ())
    tpl_inst->removeInstance ();

  if (info->Id () == CObjectInfo::BASECLASS_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (BaseClass "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::TEMPLATE_PARAM_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (TemplateParam "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::USING_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (Using "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::NAMESPACE_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (Namespace "<<info->Name()<<")"<<std::endl;
    /*DEBUG*/if (TRACE_SCOPES) std::cerr<<(void*)this<<": -SCOPE: "<<(void*)info<<" (Namespace "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::TEMPLATE_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (Template "<<info->Name()<<")"<<std::endl;
    /*DEBUG*/if (TRACE_SCOPES) std::cerr<<(void*)this<<": -SCOPE: "<<(void*)info<<" (Template "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::UNION_INSTANCE_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (Union instance "<<info->Name()<<")"<<std::endl;
    /*DEBUG*/if (TRACE_SCOPES) std::cerr<<(void*)this<<": -SCOPE: "<<(void*)info<<" (Union instance "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::CLASS_INSTANCE_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (Class instance "<<info->Name()<<")"<<std::endl;
    /*DEBUG*/if (TRACE_SCOPES) std::cerr<<(void*)this<<": -SCOPE: "<<(void*)info<<" (Class instance "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::FCT_INSTANCE_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (Function instance "<<info->Name()<<")"<<std::endl;
    /*DEBUG*/if (TRACE_SCOPES) std::cerr<<(void*)this<<": -SCOPE: "<<(void*)info<<" (Function instance "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
    if (info == current_fct) {
#ifndef NDEBUG
      /*DEBUG*/if (TRACE_FCT) std::cout<<"FCT NIL(3): "<<(void*)current_fct<<std::endl;
#endif // not NDEBUG
      current_fct = 0;
    }
  }

  CSemantic::Delete ();
}


Token* CCSemantic::getCloseCurly () {
  TokenProvider::State state = syntax ().provider ()->get_state ();
  syntax ().skip_block (TOK_OPEN_CURLY, TOK_CLOSE_CURLY, false);
  Token* closeCurly = syntax ().provider ()->current ();
  syntax ().provider ()->set_state (state);
  return closeCurly;
}


CTree* CCSemantic::createDelayedParse (bool (CCSyntax::*rule)(), CTree *tree) {
  Token *token = syntax ().provider ()->current ();
  CT_DelayedParse* dp = 0;

  if (rule == &CCSyntax::param_init) {
    dp = new CT_DelayedParseParamInit (token);
    CT_ArgDecl *ad = (CT_ArgDecl*)tree;
    ad->Initializer (dp);
    // if we skipped a default argument of a function this fact should be
    // stored in the function info, because then the overload resolution already
    // works, even without knowing the actual default value
    if (ad && ad->Object ())
      ad->Object ()->ArgumentInfo ()->hasDefaultArg (true);
  } else if (rule == &CCSyntax::fct_try_block) {
    dp = new CT_DelayedParseFctTryBlock (token);
    CT_FctDef *fd = (CT_FctDef*)tree;
    fd->Handlers (dp);
  } else if (rule == &CCSyntax::ctor_init) {
    dp = new CT_DelayedParseCtorInit (token);
    CT_FctDef *fd = (CT_FctDef*)tree;
    fd->CtorInit (dp);
  } else if (rule == &CCSyntax::pure_spec) {
    dp = new CT_DelayedParsePureSpec (token);
    CT_FctDef *fd = (CT_FctDef*)tree;
    fd->CtorInit (dp);
  } else if (rule == &CCSyntax::fct_body) {
    dp = new CT_DelayedParseFctBody (token, getCloseCurly ());
    CT_FctDef *fd = (CT_FctDef*)tree;
    fd->Body (dp);
  } else if (rule == &CCSyntax::skipped_fct_body) {
    dp = new CT_DelayedParseSkippedFctBody (token, getCloseCurly ());
    CT_FctDef *fd = (CT_FctDef*)tree;
    fd->Body (dp);
  } else if (rule == &CCSyntax::const_init) {
    dp = new CT_DelayedParseConstInit (token);
    CT_InitDeclarator *id = (CT_InitDeclarator*)tree;
    id->Initializer (dp);
  }

  dp->Class (_outermost_class_def);
  dp->Scope (current_scope);
  dp->State (syntax ().provider ()->get_state ());

  return dp;
}


void CCSemantic::parse_delayed (bool (CCSyntax::*rule)(), CTree *tree) {
  CT_DelayedParse* dp = (CT_DelayedParse*) createDelayedParse (rule, tree);
  // add to the outermost class def
  _outermost_class_def->addParseDelayed (dp);
}


void CCSemantic::parseDelayed() {
  // parse function bodies and several initializers not parsed due
  // to potential use of names of members that could not be resolved
  // untill the end of the class definition

  CT_DelayedParse* dp;
  const char* id;
  bool accepted = false;
  CTree* parent;
  CTree* result;

  CCSyntax::State old_state = syntax().provider()->get_state();
  CStructure* old_scope = current_scope;

  for (unsigned i = 0; i < _outermost_class_def->ParseDelayed(); i++) {
    dp = (CT_DelayedParse*) _outermost_class_def->ParseDelayed(i);
    id = dp->NodeName();
    parent = dp->Parent();

    // setup parser for the current delayed code fragment
    current_scope = dp->Scope();
    syntax().provider()->set_state(dp->State());
    builder().save_state();

    // parse the code fragment now
    if (id == CT_DelayedParseParamInit::NodeId())
      accepted = syntax().param_init();
    else if (id == CT_DelayedParseFctTryBlock::NodeId())
      accepted = syntax().fct_try_block();
    else if (id == CT_DelayedParseCtorInit::NodeId())
      accepted = syntax().ctor_init();
    else if (id == CT_DelayedParsePureSpec::NodeId())
      accepted = syntax().pure_spec();
    else if (id == CT_DelayedParseFctBody::NodeId())
      accepted = syntax().fct_body();
    else if (id == CT_DelayedParseSkippedFctBody::NodeId())
      accepted = syntax().skipped_fct_body();
    else if (id == CT_DelayedParseConstInit::NodeId())
      accepted = syntax().const_init();

    // get tree fragment
    if (accepted) {
      result = builder().get_node(builder().nodes()-1);
      builder().forget_state();

      // instantiate result tree if needed
      if (_outermost_class_def->haveInstantiateDelayed(dp->token()))
        instantiateDelayed(dp, result);

    } else {
      // failed, clean up
      builder().restore_state();
      result = builder().error();
    }

    // complete the syntax tree
    if (id == CT_DelayedParseFctBody::NodeId() || id == CT_DelayedParseSkippedFctBody::NodeId()) {
      CT_FctDef *fd = (CT_FctDef*)parent;
      fd->Body(result);
    } else if (id == CT_DelayedParseCtorInit::NodeId() || id == CT_DelayedParsePureSpec::NodeId()) {
      CT_FctDef *fd = (CT_FctDef*)parent;
      fd->CtorInit(result);
    } else if (id == CT_DelayedParseFctTryBlock::NodeId()) {
      CT_FctDef *fd = (CT_FctDef*)parent;
      if (accepted) {
        Builder::Container *c = (Builder::Container*)result;
        int offset = (c->Son(1)->NodeName() == CT_MembInitList::NodeId()) ? 1 : 0;
        fd->FctTryBlock(c->Son(0), offset ? c->Son(1) : (CTree*)0, c->Son(1+offset), c->Son(2+offset));
        delete c;
      } else {
        fd->Handlers(result);
      }
    } else if (id == CT_DelayedParseParamInit::NodeId()) {
      CT_ArgDecl *ad = (CT_ArgDecl*)parent;
      ad->Initializer(result);
    } else if (id == CT_DelayedParseConstInit::NodeId()) {
      CT_InitDeclarator *idl = (CT_InitDeclarator*)parent;
      idl->Initializer(result);
    }

    // delete the CT_DelayedParse tree
    dp->Class(0);
    builder().destroy(dp);
  }

  syntax().provider()->set_state(old_state);
  current_scope = old_scope;
}


void CCSemantic::instantiateDelayed(CT_DelayedParse* odp, CTree* tree) {
  // instantiate function bodies and several initializers not instantiated
  // due to potential use of names of members that could not be resolved
  // untill the end of the class definition

  CTemplateInstance* instance;
  CT_DelayedParse* dp;
  CTemplateInfo *tpl;
  CScopeInfo* scope;
  const char* id;
  CTree* parent;
  CTree* result = 0;

  InstantiationParser parser;

  const CRecord::TreeList& list = _outermost_class_def->InstantiateDelayed(odp->token());
  CRecord::TreeList::const_iterator i = list.begin();
  for (; i != list.end(); ++i) {
    dp = (CT_DelayedParse*) *i;
    id = dp->NodeName();
    parent = dp->Parent();

    // initialize the parser
    scope = dp->Scope();
    while (!scope->isTemplateInstance())
      scope = scope->Parent();
    instance = scope->TemplateInstance();
    tpl = instance->Template();
    parser.init(instance->Object(), tpl, dp->Scope());

    // parse the code fragment now
    bool delayed = false;
    if (id == CT_DelayedParseParamInit::NodeId())
      result = parser.parse(tpl, tree, &InstantiationSyntax::param_init, delayed, false);
    else if (id == CT_DelayedParseFctTryBlock::NodeId())
      result = parser.parse(tpl, tree, &InstantiationSyntax::fct_try_block, delayed, false);
    else if (id == CT_DelayedParseCtorInit::NodeId())
      result = parser.parse(tpl, tree, &InstantiationSyntax::ctor_init, delayed, false);
    else if (id == CT_DelayedParsePureSpec::NodeId())
      result = parser.parse(tpl, tree, &InstantiationSyntax::pure_spec, delayed, false);
    else if (id == CT_DelayedParseFctBody::NodeId())
      result = parser.parse(tpl, tree, &InstantiationSyntax::fct_body, delayed, false);
    else if (id == CT_DelayedParseSkippedFctBody::NodeId())
      result = parser.parse(tpl, tree, &InstantiationSyntax::skipped_fct_body, delayed, false);
    else if (id == CT_DelayedParseConstInit::NodeId())
      result = parser.parse(tpl, tree, &InstantiationSyntax::const_init, delayed, false);

    // semantic expression analysis
    if (result)
      parser.resolve(tpl, dp->Scope(), result);

    // report errors
    if (parser.failed()) {
      //First().printInstantiationErrorHeader(getPoiToken());
      parser.builder().errors(*_err);
    }
    if (!result)
      result = builder().error();

    // complete the syntax tree
    if (id == CT_DelayedParseFctBody::NodeId() || id == CT_DelayedParseSkippedFctBody::NodeId()) {
      CT_FctDef *fd = (CT_FctDef*)parent;
      fd->Body(result);
    } else if (id == CT_DelayedParseCtorInit::NodeId() || id == CT_DelayedParsePureSpec::NodeId()) {
      CT_FctDef *fd = (CT_FctDef*)parent;
      fd->CtorInit(result);
    } else if (id == CT_DelayedParseFctTryBlock::NodeId()) {
      CT_FctDef *fd = (CT_FctDef*)parent;
      if (result->NodeName() == Builder::Container::NodeId()) {
        Builder::Container *c = (Builder::Container*)result;
        int offset = (c->Son(1)->NodeName() == CT_MembInitList::NodeId()) ? 1 : 0;
        fd->FctTryBlock(c->Son(0), offset ? c->Son(1) : (CTree*)0, c->Son(1+offset), c->Son(2+offset));
        delete c;
      } else {
        fd->Handlers(result);
      }
    } else if (id == CT_DelayedParseParamInit::NodeId()) {
      CT_ArgDecl *ad = (CT_ArgDecl*)parent;
      ad->Initializer(result);
    } else if (id == CT_DelayedParseConstInit::NodeId()) {
      CT_InitDeclarator *idl = (CT_InitDeclarator*)parent;
      idl->Initializer(result);
    }

    // delete the CT_DelayedParse tree
    dp->Class(0);
    builder().destroy(dp);
  }
}


/*****************************************************************************/
/*                                                                           */
/*                               Exceptions                                  */
/*                                                                           */
/*****************************************************************************/


CTree *CCSemantic::handler () {
  CT_Handler *result;
  result = (CT_Handler*)builder ().handler ();
  result->Scope (current_scope);
  current_scope->Tree (result);
  return result;
}


CTree *CCSemantic::exception_spec () {
  CT_ExceptionSpec *es;

  es = (CT_ExceptionSpec*)builder ().exception_spec ();
  es->Arguments ()->Scope (current_scope);

  leave_exception_spec ();
  return es;
}


CTree *CCSemantic::introduce_exception () {
  CAttributeInfo *info;
  CSemDeclSpecs *dsi;
  CT_ArgDecl *ad;

  if (! in_decl ())
    return (CTree*)0;

  // determine exception type
  ad = (CT_ArgDecl*)builder ().exception_decl ();
  dsi = sem_decl_specs ();
  CCSemDeclarator csd (_err, dsi->make_type (), ad->Declarator ());
  if (! csd.Name ()) {
    delete ad;
    return (CTree*)0;
  }

  // complete semantic information about the exception
  info = current_scope->newAttribute ();
  info->Storage (CStorage::CLASS_AUTOMATIC);
  setLanguage (info);
  setSpecifiers (info, dsi);
  common_settings (info, ad, &csd);
  Push (info);

  // attribute syntax tree
  ad->Object (info);
  return ad;
}


void CCSemantic::enter_exception_spec () {
  // prevent the parser from rejecting the current function;
  // this always happens when a declarator was parsed and we
  // are no longer in the scope of a function parameter clause
  _in_param_decl_clause.push (true);
  // search in scope of function first
  reenter_scope (current_fct);
}


void CCSemantic::leave_exception_spec () {
  _in_param_decl_clause.pop ();
  // leave scope of function
  reenter_scope (current_scope->Parent ()->Structure ());
}


/*****************************************************************************/
/*                                                                           */
/*                                Templates                                  */
/*                                                                           */
/*****************************************************************************/


bool CCSemantic::isSpecialization (CTree *name) const {
  if (name && name->IsSimpleName () &&
      name->IsSimpleName ()->Name ()->NodeName () == CT_TemplateName::NodeId ())
    return true;
  if (current_scope->TemplateInfo () &&
      current_scope->TemplateInfo ()->isSpecialization ())
    return true;
  return false;
}


void CCSemantic::setSpecializationName (CTree *tree, CTemplateInfo *tinfo) const {
  if (tree->IsSimpleName ()) {
    CT_SimpleName *name = tree->IsSimpleName ()->Name ();
    if (name && name->NodeName () == CT_TemplateName::NodeId ()) {
      tinfo->SpecializationName ((CT_TemplateName*)name);
    }
  }
}


bool CCSemantic::skip_tpl_fct_bodies () const {
  CStructure *scope = current_scope;
  while (! scope->FileInfo ()) {
    if (scope->TemplateInfo ())
      return true;
    scope = scope->Parent ()->Structure ();
  }
  return false;
}


CTree *CCSemantic::template_key () {
  // next identifier is expected to be a template name
  expect_template = true;
  return builder ().get_node ();
}


CTree *CCSemantic::introduce_template_instance () {
  bool instantiate, pseudo, is_special;
  CScopeInfo *scope, *class_or_fct;
  CTemplateInfo *tpinfo, *tpl;
  CT_TemplateArgList *args;
  CT_TemplateName *tn;
  CObjectInfo *info;
  unsigned numargs;
  CTypeInfo *type;
  CTree *arg;

  tn = (CT_TemplateName*)builder ().template_id ();
  info = tn->TemplateName ()->Object ();
  if (! info) // ???
    return tn;

  info = info->DefObject ();
  pseudo = is_entity_name ();
  instantiate = ! info->FunctionInfo ();
  is_special = (is_entity_name () ? (! syntax ().look_ahead (TOK_OPEN_ROUND)) :
                (current_scope->TemplateInfo () && syntax ().look_ahead (TOK_COLON_COLON))) &&
                (! in_conv_type_id () && ! in_nested_name ());
  args = tn->Arguments ();
  numargs = args->Entries ();

  // compare number of parameters and arguments of
  // non-function templates
  if (info->TemplateParamInfo ()) {
    tpinfo = info->TemplateParamInfo ()->TemplateTemplate ();
  }
  else
    tpinfo = info->Template ();
  if (real_inst) {
    if (tpinfo && ! info->FunctionInfo () && ! is_special) {
      if (numargs > tpinfo->Parameters () ||
          (numargs < tpinfo->Parameters () &&
           ! tpinfo->DefaultArgument (numargs))) {
        SEM_ERROR (tn, "wrong number of template arguments ("
          << numargs << ", should be " << tpinfo->Parameters () << ")");
        args->RemoveSon (args->Sons ()-1);
        args->RemoveSon (0);
        delete tn;
        return 0;
      }
    }
  }

  // apply implicit conversions to the template non-type arguments
  for (unsigned i = 0; i < numargs; i++) {
    arg = args->Entry (i);

    // template type argument
    if (arg->NodeName () == CT_NamedType::NodeId ()) {
      if (arg->SemObject () && arg->SemObject ()->Object () &&
          arg->SemObject ()->Object ()->TypeInfo ()) {
        type = arg->SemObject ()->Object ()->TypeInfo ();
        if (type->isDependent (true, true) ||
            (type->isTemplate () && i < tpinfo->Parameters () &&
             ! tpinfo->Parameter (i)->isTemplate ()))
          pseudo = true;
      }
      continue;
    }

    // resolve template non-type argument (expression)
    CCSemExpr cse (*_err, current_scope);
    type = cse.resolveExpr (arg, args);
    if (cse.isDependent () || cse.isDependent (arg))
      pseudo = true;

    // ��14.3.2 address as non-type template argument
    if ((type->isArray () || type->isFunction ()) || CCSemExpr::isAddrExpr (arg))
      calculateAddress (arg);

    // ��14.3.2.5 implicit template argument conversions
    if (type->isAddress ())
      type = type->NonReferenceType ();

    // array-to-pointer and function-to-pointer conversions
    if (type->isArray () || type->isFunction ()) {
      type = CCSemExpr::convLvalueToRvalue (arg);
    } else {
      // qualification conversion
      if (type->TypeQualified ())
        type = CCSemExpr::castToType (type->UnqualType (), arg);
      // integral promotion and conversion
      if (type->isInteger ())
        type = CCSemExpr::intPromotion (arg);
    }
  }

  // handle recursive template instantiation
  if (instantiate) {
    scope = current_scope;
    class_or_fct = 0;
    do {
      // inside class or function
      if (scope->isRecord () || scope->isFunction ())
        class_or_fct = scope;

      // local scope?
      if (! scope->TemplateInfo () || ! class_or_fct)
        continue;

      // compare scope with template to instantiate
      if (*class_or_fct != *info) {
        bool matched = false;

        // if specialization scope we have to compare against
        // the registered specializations of the template
        if (class_or_fct->Template () &&
            class_or_fct->Template ()->isSpecialization ()) {
          for (unsigned i = 0; i < tpinfo->Specializations (); i++) {
            tpl = tpinfo->Specialization (i);
            if (*tpl->ObjectInfo () == *class_or_fct) {
              matched = matchArguments(tn, tpl->SpecializationName ());
              if (matched)
                break;
            }
          }
        }
        if (! matched)
          continue;
      }

      pseudo = true;
      break;
    } while (scope != scope->Parent () && (scope = scope->Parent ()));
  }

  // handle explicit specialization
  if (is_special && instantiate) {
    // enclose in template scope
    if (info->Record () && ! current_scope->TemplateInfo ()) {
      scope = _db->newTemplate();
      scope->Name (info->Name ());
      scope->SourceInfo ()->FileInfo (_file);
      enter_scope (scope->Structure ());
      //Push (scope);

      if (! tpinfo && info->FunctionInfo ())
        scope->TemplateInfo ()->BaseTemplate (info->FunctionInfo ()->TemplateInfo ());
      else if (tpinfo)
        scope->TemplateInfo ()->BaseTemplate (tpinfo);

      tn->Name (scope->Name ());
      tn->Object (scope);
      //tn->setTypeRef (info->TypeInfo ());
      tn->TemplateName ()->Object (info);
      tn->TemplateName ()->setTypeRef (info->TypeInfo ());
      return tn;
    } else if (current_scope->TemplateInfo ()) {
      if (! tpinfo && info->FunctionInfo ())
        current_scope->TemplateInfo ()->BaseTemplate (info->FunctionInfo ()->TemplateInfo ());
      else if (tpinfo)
        current_scope->TemplateInfo ()->BaseTemplate (tpinfo);
    }
    instantiate = false;
  }

  // function templates are instantiated later
  if (info->FunctionInfo ()) {
    tn->Object (info);
    tn->Name (info->Name ());
    return tn;
  }

  // don't instantiate entity names, and non-base templates
  if ((is_entity_name () && ! in_nested_name ()) ||
      (tpinfo && ! tpinfo->isBaseTemplate ())) {
    instantiate = false;
  // template template parameter
  } else if (info->TemplateParamInfo ()) {
    instantiate = true;
    pseudo = true;
  }

  // instantiate non-function templates
  if (instantiate) {
    // instantiate now
    CCInstantiation cci (*_err);
    info = cci.instantiate (tn, info, ! pseudo, current_scope);

    if (! info) {
      args->RemoveSon (args->Sons ()-1);
      args->RemoveSon (0);
      delete tn;
      return 0;
    }
  }

  // attribute syntax tree
  tn->Name (info->Name ());
  tn->Object (info);
  tn->setTypeRef (info->TypeInfo ());
  tn->TemplateName ()->Object (info);
  tn->TemplateName ()->setTypeRef (info->TypeInfo ());
  return tn;
}


bool CCSemantic::matchArguments (CT_TemplateName *n1, CT_TemplateName *n2) const {
  // template names
  if (! n1 || ! n2)
    return false;

  // template arguments
  CT_TemplateArgList *args1 = n1->Arguments ();
  CT_TemplateArgList *args2 = n2->Arguments ();
  if (! args1 || ! args2)
    return false;

  // number of arguments
  unsigned entries1 = args1->Entries ();
  unsigned entries2 = args2->Entries ();
  if (entries1 != entries2)
    return false;

  // compare arguments (not yet complete)
  for (unsigned i = 0; i < entries1; i++) {
    CTree *arg1 = args1->Entry (i);
    CTree *arg2 = args2->Entry (i);

    // match values
    CConstant *value;
    if (arg1->Value () && arg1->Value ()->Constant ()) {
      if (arg2->Value () && (value = arg2->Value ()->Constant ()) &&
          (*value == *arg1->Value ()->Constant ()))
        continue;
      return false;
    }

    // match types
    CTypeInfo *type1 = 0, *type2 = 0;
    if (arg1->NodeName () == CT_NamedType::NodeId () &&
        arg1->SemObject () && arg1->SemObject ()->Object ())
      type1 = arg1->SemObject ()->Object ()->TypeInfo ();
    if (arg2->NodeName () == CT_NamedType::NodeId () &&
        arg2->SemObject () && arg2->SemObject ()->Object ())
      type2 = arg2->SemObject ()->Object ()->TypeInfo ();
    if (! type1)
      type1 = arg1->Type ();
    if (! type2)
      type2 = arg2->Type ();
    if (! type1 || ! type2 || *type1 != *type2)
      return false;
  }

  return true;
}


void CCSemantic::calculateAddress (CTree *node) const {
  CObjectInfo *info;
  LONG_LONG value;

  if (node->Value () || ! node->SemValue ())
    return;

  info = CCSemExpr::findObject (node);
  if (info && info->Tree () && info->Tree ()->token_node ()) {
    value = info->Tree ()->token_node ()->Number ();
    node->SemValue ()->setValue (new CConstant (value, node->Type ()));
  }
}


void CCSemantic::enter_template_decl () {
  CT_TemplateParamList *tpl;

  // let the following declaration be in template scope
  tpl = (CT_TemplateParamList*)builder ().get_node (builder ().nodes ()-2);
  reenter_scope (tpl->Scope ()->Structure ());
}


CTree *CCSemantic::template_decl () {
  CT_TemplateDecl *td;
  CTemplateInfo *info;
  CObjectInfo *oinfo;
  CT_ObjDecl *od;
  CTree *decl;

  // parsing template declaration failed?
  if (builder ().get_node (builder ().nodes ()-3)->NodeName () !=
      CT_TemplateParamList::NodeId ())
    return (CTree*)0;

  td = (CT_TemplateDecl*)builder ().template_decl ();
  td->Scope (current_scope);
  current_scope->Tree (td);
  current_scope->SourceInfo ()->StartToken (td->token_node ());

  info = current_scope->TemplateInfo ();
  decl = td->Declaration ();

  // ���14.1
  oinfo = 0;
  if (decl->NodeName () == CT_TemplateDecl::NodeId ())
    oinfo = ((CT_TemplateDecl*)decl)->Scope ();
  else if (decl->NodeName () == CT_ObjDecl::NodeId ()) {
    od = (CT_ObjDecl*)decl;
    if (od->Declarators ()->Entries () > 1) {
      SEM_ERROR (td, "multiple declarators in template declaration");
    }
  }

  if (oinfo && ! info->ObjectInfo ())
    info->ObjectInfo (oinfo);

  return td;
}


void CCSemantic::introduceTemplate (CObjectInfo *oinfo, CTemplateInfo *info, bool is_friend) {
  CStructure *scope;

  info->ObjectInfo (oinfo);

  // introduce declared class/union/function/static member into
  // enclosing non-template scope for name lookup purposes only
  scope = info->Parent ()->Structure ();
  while (scope && scope != info && (scope->TemplateInfo () ||
         (is_friend && (scope->isRecord () || 
          (scope->NamespaceInfo () && scope->NamespaceInfo ()->aroundInstantiation ())))))
    scope = scope->Parent ()->Structure ();

  if (oinfo->Record ()) {
    if (scope) {
      // handle template specialization
      if (info->isBaseTemplate ())
        scope->addType (oinfo);
      setQualName (oinfo, info);
    } else
      info->Name (oinfo->Name ());
  } else if (oinfo->FunctionInfo ()) {
    if (scope) {
      // handle template specializations
      if (info->isBaseTemplate ())
        scope->addFunction (oinfo->FunctionInfo ());
      setQualName (oinfo, info);
    } else
      info->Name (oinfo->Name ());
  } else if (oinfo->AttributeInfo ()) {
    if (scope) {
      // handle template specializations
      if (info->isBaseTemplate ())
        scope->addAttribute (oinfo->AttributeInfo ());
      setQualName (oinfo, info);
    } else
      info->Name (oinfo->Name ());
  }
}


void CCSemantic::setTemplateScope (CCNameLookup &nl, CObjectInfo *oinfo,
 CTypeInfo *type, CTemplateInfo *info, bool is_qual_name) {
  CObjectInfo *prev;
  bool is_template = ! is_qual_name;

  // is this a template definition or the definition of
  // a non-template class template member?
  if (is_qual_name) {
    is_template = nl.Objects () ? false : true;
    for (unsigned i = 0; i < nl.Objects (); i++) {
      prev = nl.Object (i);
      if (prev && prev->isTemplate () && equalType (prev->TypeInfo (), type, false)) {
        is_template = true;
        break;
      }
    }
  }

  if (is_template) {
    if (oinfo->Record ()) {
      oinfo->Record ()->TemplateInfo (info);
      oinfo->Record ()->isTemplate (true);
    } else if (oinfo->FunctionInfo ()) {
      oinfo->FunctionInfo ()->TemplateInfo (info);
      oinfo->FunctionInfo ()->isTemplate (true);
    } else if (oinfo->AttributeInfo ()) {
      oinfo->AttributeInfo ()->TemplateInfo (info);
    }
  }
}


void CCSemantic::setQualName (CObjectInfo *oinfo, CObjectInfo *info) const {
  CStructure *scope;
  std::ostringstream name;

  if (! oinfo->Record ())
    scope = oinfo->QualifiedScope ();
  else
    scope = 0;

  if (scope && scope->Name ())
    name << scope->Name () << "::";
  name << oinfo->Name ();
  info->Name (name.str ().c_str ());
}


CTree *CCSemantic::explicit_instantiation () {
  CT_TemplateDecl *td;
  CTemplateInfo *info;
  CObjectInfo *oinfo;
  CTree *decl, *node;
  CT_ObjDecl *od;

  node = builder ().explicit_instantiation ();
  if (node->NodeName () == CT_LinkageSpec::NodeId ()) // => GCC EXTENSION!!!
    td = (CT_TemplateDecl*)((CT_LinkageSpec*)node)->Decls ();
  else
    td = (CT_TemplateDecl*)node;
  td->Scope (current_scope);
  current_scope->Tree (td);
  current_scope->SourceInfo ()->StartToken (td->token_node ());

  info = current_scope->TemplateInfo ();
  decl = td->Declaration ();

  // ���14.1
  oinfo = 0;
  if (decl->NodeName () == CT_TemplateDecl::NodeId ())
    oinfo = ((CT_TemplateDecl*)decl)->Scope ();
  else if (decl->NodeName () == CT_ObjDecl::NodeId ()) {
    od = (CT_ObjDecl*)decl;
    if (od->Declarators ()->Entries () > 1) {
      SEM_ERROR (td, "multiple declarators in template declaration");
    }
  }

  if (oinfo && ! info->ObjectInfo ())
    info->ObjectInfo (oinfo);

  return node;
}


CTree *CCSemantic::explicit_specialization () {
  CT_TemplateDecl *td;
  CTemplateInfo *info;
  CObjectInfo *oinfo;
  CT_ObjDecl *od;
  CTree *decl;

  td = (CT_TemplateDecl*)builder ().explicit_specialization ();
  td->Scope (current_scope);
  current_scope->Tree (td);
  current_scope->SourceInfo ()->StartToken (td->token_node ());

  info = current_scope->TemplateInfo ();
  decl = td->Declaration ();

  // ���14.1
  oinfo = 0;
  if (decl->NodeName () == CT_TemplateDecl::NodeId ())
    oinfo = ((CT_TemplateDecl*)decl)->Scope ();
  else if (decl->NodeName () == CT_ObjDecl::NodeId ()) {
    od = (CT_ObjDecl*)decl;
    if (od->Declarators ()->Entries () > 1) {
      SEM_ERROR (td, "multiple declarators in template declaration");
    }
  }

  if (oinfo && ! info->ObjectInfo ())
    info->ObjectInfo (oinfo);

  return td;
}


void CCSemantic::enter_template_param_list (Syntax::State state) {
  // create and enter new template scope
  CTemplateInfo *info = _db->newTemplate();
  info->SourceInfo ()->FileInfo (_file);
  enter_scope (info);
  if (state)
    _in_template_param_list.push (state);
  // state==0 means that this is an explicit specialization or
  // instantiation (no template parameter list given)
  else
    info->isSpecialization (true);
}


CTree *CCSemantic::template_param_list () {
  CT_TemplateParamList *tpl;
  tpl = (CT_TemplateParamList*)builder ().template_param_list ();
  tpl->Scope (current_scope);
  return tpl;
}


CTree *CCSemantic::template_param () {
  CT_TemplateParamDecl *tpd;
  CT_ExprList *da;
  CTree *arg;

  tpd = (CT_TemplateParamDecl*)builder ().template_param ();
  da = tpd->DefaultArgument ();

  // resolve default template argument
  if (da && da->Entries ()) {
    arg = da->Entry (0);
    if (arg->NodeName () != CT_NamedType::NodeId ()) {
      resolveExpr (arg, da);
    }
  }

  return tpd;
}


CTree *CCSemantic::introduce_type_param () {
  CTemplateParamInfo *info;
  CT_TypeParamDecl *td;

  td = (CT_TypeParamDecl*)builder ().type_param ();

  // analysis not yet complete!

  info = current_scope->TemplateInfo ()->newTemplateParam ();
  info->ObjectInfo ()->TypeInfo (new CTypeTemplateParam (info));
  info->isTypeParam (true);
  Push (info);

  // template template parameter
  if (td->Parameters ()) {
    info->TemplateTemplate (td->Parameters ()->Scope ()->TemplateInfo ());
    info->TemplateTemplate ()->ObjectInfo (info);
  }

  common_settings (info, td);
  info->Name (td->Name ()->Text ());
  td->Name ()->Object (info);
  td->Name ()->setTypeRef (info->TypeInfo ());
  td->Object (info);
  return td;
}


CTree *CCSemantic::introduce_non_type_param () {
  CTemplateParamInfo *info;
  CT_NonTypeParamDecl *td;
  CSemDeclSpecs *dsi;
  CTypeInfo *type;

  finish_declarator ();
  if (! in_decl ())
    return (CTree*)0;

  td = (CT_NonTypeParamDecl*)builder ().non_type_param ();
  dsi = sem_decl_specs ();
  CCSemDeclarator csd (_err, dsi->make_type (), td->Declarator ());
  if (! csd.Name ()) {
    delete td;
    return (CTree*)0;
  }

  type = csd.Type ();
  bool error = true;
  if (current_scope->TemplateParams (csd.Name ()->Text ())) {
    SEM_ERROR__duplicate (td, "template parameter", csd.Name ());
  } else if (dsi->declared (CT_PrimDeclSpec::PDS_STATIC) ||
             dsi->declared (CT_PrimDeclSpec::PDS_THREAD) ||
             dsi->declared (CT_PrimDeclSpec::PDS_EXTERN) ||
             dsi->declared (CT_PrimDeclSpec::PDS_AUTO) ||
             dsi->declared (CT_PrimDeclSpec::PDS_REGISTER) ||
             dsi->declared (CT_PrimDeclSpec::PDS_MUTABLE)) {
    SEM_ERROR__invalid_in_param (td, "storage class specifier", csd.Name ());
  } else if (dsi->declared (CT_PrimDeclSpec::PDS_TYPEDEF)) {
    SEM_ERROR__invalid_in_param (td, "typedef declaration", csd.Name ());
  } else if (type->isFunction () || type->isArray ()) {
    // template non-type parameter adjustment
    csd.Type (new CTypePointer (
      type->isArray () ? type->VirtualType ()->BaseType () : type));
    error = false;
  } else if (! (type->isInteger () || type->isPointer () ||
                type->isAddress () || type->isMemberPointer () ||
                type->isTemplateParam ())) {
    SEM_ERROR__invalid_param (td, "type", csd.Name ());
  } else
    error = false;
  if (error) {
    delete td;
    return (CTree*)0;
  }

  info = current_scope->TemplateInfo ()->newTemplateParam ();
  Push (info);

  setLanguage (info);
  setSpecifiers (info, dsi);
  common_settings (info, td);
  info->Name (csd.Name ()->Text ());
  info->ObjectInfo ()->TypeInfo (new CTypeTemplateParam (info));
  info->ValueType (csd.Type ());
  csd.Type ((CTypeInfo*)0);
  csd.Name ()->Object (info);
  csd.Name ()->setTypeRef (info->TypeInfo ());
  td->Object (info);
  if (dsi) decl_end ();
  return td;
}


/*****************************************************************************/
/*                                                                           */
/*                                 Objects                                   */
/*                                                                           */
/*****************************************************************************/


CTree *CCSemantic::obj_decl (CTree *node) {
  CT_DeclaratorList *dl;
  CT_DeclSpecSeq *dss;
  CTree *d;

  if (! node || node->NodeName () != CT_ObjDecl::NodeId ())
    return node;

  // attribute init declarators and class defs

  dl = ((CT_ObjDecl*)node)->Declarators ();
  if (dl) {
    for (unsigned i = dl->Entries (); i > 0; i--) {
      d = dl->Entry (i-1);
      if (d->NodeName () == CT_InitDeclarator::NodeId ())
        ((CT_InitDeclarator*)d)->ObjDecl (node);
    }
  }

  dss = ((CT_ObjDecl*)node)->DeclSpecs ();
  if (dss) {
    for (unsigned i = dss->Entries (); i > 0; i--) {
      d = dss->Entry (i-1);
      if (d->NodeName () == CT_ClassDef::NodeId () ||
          d->NodeName () == CT_UnionDef::NodeId ())
        ((CT_ClassDef*)d)->ObjDecl (node);
    }
  }

  return node;
}


CTree *CCSemantic::introduce_object (CTree* tree) {
  bool is_typedef, is_fct, is_attr, redef_typedef, is_special, is_qual_name;
  CStructure *scope, *lookup_scope;
  CObjectInfo *info, *prev = 0;
  CT_InitDeclarator *id;
  CTemplateInfo *tinfo;
  CSemDeclSpecs *dsi;
  CTypeInfo *type;

  if (! in_decl ())
    return (CTree*)0;

  // determine type of declaration
  id = (CT_InitDeclarator*)(tree ? tree : builder ().init_declarator1 ());
  dsi = sem_decl_specs ();
  CCSemDeclarator csd (_err, dsi->make_type (), id);
  if (! csd.Name ()) {
    delete id;
    return (CTree*)0;
  }

  // determine properties of declarations
  type = csd.Type ();
  is_typedef = dsi->declared (CT_PrimDeclSpec::PDS_TYPEDEF);
  is_fct = (! is_typedef && type->isFunction () && ! type->isAddress ());
  is_attr = ! (is_typedef || is_fct);
  is_special = isSpecialization (csd.Name ());
  is_qual_name = isQualName (csd.Name ());
  lookup_scope = is_special ? current_scope : getNonTplScope (true);
  scope = getNameScope (csd.Name ());
  if (is_qual_name && scope != lookup_scope)
    lookup_scope = scope;

  // lookup name
  CCNameLookup nl (*_err, current_scope, false);
  nl.lookup (csd.Name (), lookup_scope);

  // check name lookup results
  if (! check_object_lookup (nl, lookup_scope, dsi, csd, id, redef_typedef, prev)) {
    delete id;
    return (CTree*)0;
  }

  // typedef declaration
  if (is_typedef) {
    info = current_scope->newTypedef ();
    Push (info);
  // function prototype declaration
  } else if (is_fct) {
/*DEBUG*/if (TRACE_FCT) std::cout<<"FCT DEC(1): "<<(void*)current_fct<<std::endl;
    if (! current_fct)
      current_fct = current_scope->newFunction ();
    else
      current_fct->ScopeInfo ()->Parent ()->Structure ()->
        addFunction (current_fct->FunctionInfo ());
    info = current_fct;
    // determine function category
    if (lookup_scope->Record () && lookup_scope->Name () &&
        strcmp (lookup_scope->Name (), csd.Name ()->Name ()->Text ()) == 0)
      info->FunctionInfo ()->isConstructor (true);
    else if (csd.Name ()->Name ()->NodeName () == CT_DestructorName::NodeId ())
      info->FunctionInfo ()->isDestructor (true);
    else if (csd.Name ()->Name ()->NodeName () == CT_OperatorName::NodeId ())
      info->FunctionInfo ()->isOperator (true);
    else if (csd.Name ()->Name ()->NodeName () == CT_ConversionName::NodeId ()) {
      info->FunctionInfo ()->isConversion (true);
      info->FunctionInfo ()->ConversionType (
        ((CT_ConversionName*)csd.Name ()->Name ())->
        TypeName ()->Object ()->TypeInfo ());
    }
    info->Linkage (determine_linkage (dsi, FCT, prev));
    if (is_qual_name || lookup_scope->Record ())
      info->FunctionInfo ()->QualifiedScope (getQualifiedScope (lookup_scope));
    type->VirtualType ()->TypeFunction ()->FunctionInfo (info->FunctionInfo ());
    // handle template declaration
    if (current_scope->TemplateInfo ())
      setTemplateScope (nl, info, csd.Type (), current_scope->TemplateInfo (), is_qual_name);
  // non-fct non-type declaration
  } else {
    info = current_scope->newAttribute ();
    info->Linkage (determine_linkage (dsi, ATTRIB, prev));
    info->Storage (determine_storage_class (dsi));
    Push (info);
    // enter scope of class if definition of static class member
    if (is_qual_name) {
      last_scope = current_scope;
      reenter_scope (lookup_scope);
      info->AttributeInfo ()->QualifiedScope (getQualifiedScope (lookup_scope));
      if (prev)
        info->Protection (prev->Protection ());
    }
  }

  // complete semantic information about the declaration
  setLanguage (info);
  setSpecifiers (info, dsi);
  common_settings (info, id, &csd);
  id->Object (info);

  // handle "redefining" typedefs
  if (redef_typedef) {
    char aname[100];
    sprintf (aname, "%%anon%ld", _Anonymous);
    _Anonymous++;
    info->Name (aname);
  } else if (info->TypedefInfo ()) {
    type = info->TypeInfo ()->TypedefInfo (info);
    info->TypeInfo (type);
  }

  // link former declarations with this one
  if (prev) {
    if ((is_attr && prev->AttributeInfo ()) || (! is_fct && redef_typedef && prev->TypedefInfo ()))
      prev->NextObject (info);
  }
  if (is_fct) {
    if (! is_special)
      linkFctDecls (nl, info);
    if (info->FunctionInfo ()->isConstructor ())
      linkCtorDecls (info->FunctionInfo ());
  }

  // handle template declarations
  tinfo = current_scope->TemplateInfo ();
  if (tinfo) {
    setSpecializationName (csd.Name (), tinfo);
    introduceTemplate (info, tinfo);
  }

  if (is_fct)
    current_fct = 0;

  // if an attribute with class object type, ensure
  // that the class is really instantiated
  if (info->AttributeInfo() && type->UnqualType()->TypeClass()) {
    type->UnqualType()->TypeClass()->instantiate(current_scope);
  }

  return id;
}


bool CCSemantic::check_object_lookup (CCNameLookup &nl, CStructure *lookup_scope,
 CSemDeclSpecs *dsi, CCSemDeclarator &csd, CTree *id, bool &redef_typedef,
 CObjectInfo *& prev) {
  bool is_typedef, is_fct, is_attr, error;
  CTypeInfo *type;

  type = csd.Type ();
  is_typedef = dsi->declared (CT_PrimDeclSpec::PDS_TYPEDEF);
  is_fct = (! is_typedef && type->isFunction () && ! type->isAddress ());
  is_attr = ! (is_typedef || is_fct);
  error = true;
  redef_typedef = false;

  // check lookup results
  if (nl.Objects ()) {
    prev = nl.Object ();
    // ���7.1.3.2 typedef redefining same type
    if (is_typedef &&
        ((prev->TypedefInfo () && equalType (type, prev->TypeInfo ())) ||
         (prev->Record () && type->TypeRecord () &&
          *type->TypeRecord ()->Record () == *prev) ||
         (prev->EnumInfo () && type->TypeEnum () &&
          *type->TypeEnum ()->EnumInfo () == *prev)))
      redef_typedef = true;

    // conflicting declarations in lookup scope
    if (is_typedef && ! redef_typedef) {
      SEM_ERROR__conflicting_types (id, prev);
    // conflicting declarations in lookup scope
    } else if ((is_attr && ! (prev->AttributeInfo () || isClassOrEnum (prev))) ||
               (is_fct && ! (prev->FunctionInfo () || isClassOrEnum (prev)))) {
      SEM_ERROR__already_defined (id, csd.Name (), prev);
    // conflicting declarations in lookup scope
    } else if (is_attr && prev->AttributeInfo () &&
               prev->Linkage () != CLinkage::LINK_EXTERNAL &&
               ! ((prev->Storage () == CStorage::CLASS_STATIC ||
                   prev->Storage () == CStorage::CLASS_THREAD) &&
                  prev->Scope ()->Record ())) {
      SEM_ERROR__redefinition (id, csd.Name (), prev);
    } else
      error = false;
  // declaration using qualified name but entity not exists
  } else if (isQualName (csd.Name ())) {
    std::ostringstream name; qualifiedScopeName (csd.Name (), name);
    SEM_ERROR__not_member (id, csd.Name ()->Name (), name.str ().c_str ());
  } else
    error = false;
  return ! error;
}


CTree *CCSemantic::introduce_named_type () {
  CSemDeclSpecs *dsi;
  CObjectInfo *info;
  CT_NamedType *nt;

  if (! in_decl ())
    return (CTree*)0;

  nt = (CT_NamedType*)builder ().type_id ();
  dsi = sem_decl_specs ();
  CCSemDeclarator csd (_err, dsi->make_type (), nt->Declarator ());
  if (! csd.Name ()) {
    delete nt;
    return (CTree*)0;
  }

  info = current_scope->newAttribute ();
  info->Linkage (determine_linkage (dsi, ATTRIB));
  info->Storage (determine_storage_class (dsi));
  Push (info);

  setLanguage (info);
  setSpecifiers (info, dsi);
  common_settings (info, nt, &csd);
  nt->Object (info);
  decl_end ();
  return nt;
}


CTree *CCSemantic::decl_spec_seq1 () {
  CT_PrimDeclSpec::Type type;
  CObjectInfo* info, *scope;
  CT_DeclSpecSeq *dss;
  CT_SimpleName *name;
  Token *token;
  CTree *ds;
  bool ctor;

  // handle the ambiguity between parsing a name in a
  // declaration as a type-name or as a declarator-id

  dss = decl_specs ();
  ds = builder ().decl_spec_seq1 ();

  // here no ambiguity exist
  if (in_conv_type_id ()) {
    /* nothing to do :) */
  // type-name
  } else if (ds->NodeName () == CT_SimpleName::NodeId () ||
                 ds->NodeName () == CT_TemplateName::NodeId () ||
                 ds->NodeName () == CT_QualName::NodeId () ||
                 ds->NodeName () == CT_RootQualName::NodeId ()) {
    name = (CT_SimpleName*)ds;
    info = name->Name ()->Object ();
    ctor = info && info->Record () && syntax ().look_ahead (TOK_OPEN_ROUND);

    // ambiguous situation?
    if (ctor || (info && dss->Sons () &&
                 (info->Record () || info->EnumInfo () || info->TypedefInfo ()))) {
      // are there already type specifiers?
      for (int i = 0; i < dss->Sons (); i++) {
        if (dss->Son (i)->NodeName () == CT_PrimDeclSpec::NodeId ()) {
          type = ((CT_PrimDeclSpec*)dss->Son (i))->SpecType ();
          if (type == CT_PrimDeclSpec::PDS_CHAR ||
              type == CT_PrimDeclSpec::PDS_WCHAR_T ||
              type == CT_PrimDeclSpec::PDS_BOOL ||
              type == CT_PrimDeclSpec::PDS_SHORT ||
              type == CT_PrimDeclSpec::PDS_INT ||
              type == CT_PrimDeclSpec::PDS_LONG ||
              type == CT_PrimDeclSpec::PDS_SIGNED ||
              type == CT_PrimDeclSpec::PDS_UNSIGNED ||
              type == CT_PrimDeclSpec::PDS_FLOAT ||
              type == CT_PrimDeclSpec::PDS_DOUBLE ||
              type == CT_PrimDeclSpec::PDS_INT64 ||
              type == CT_PrimDeclSpec::PDS_INT128 ||
              type == CT_PrimDeclSpec::PDS_UNKNOWN_T ||
              type == CT_PrimDeclSpec::PDS_VOID)
            return (CTree*)0;
        } else
          return (CTree*)0;
      }

      // constructor name?
      if (ctor) {
        scope = getNameScope (name, false);
        if (! isQualName (name))
          while (scope->TemplateInfo () || (scope->NamespaceInfo () && scope->NamespaceInfo ()->aroundInstantiation ()))
            scope = scope->ScopeInfo ()->Parent ();

        CTemplateInstance *instance = info->TemplateInstance ();
        if (instance && (! scope->isTemplateInstance () || ! instance->canInstantiate ()))
          info = instance->Template ()->ObjectInfo ();
        instance = scope->TemplateInstance ();
        if (instance && (! info->isTemplateInstance () || ! instance->canInstantiate ()))
          scope = instance->Template ()->ObjectInfo ();

        if (*info == *scope)
          return (CTree*)0;
      }
    }
  } else if (ds->NodeName () == CT_PrimDeclSpec::NodeId () &&
             ((CT_PrimDeclSpec*)ds)->SpecType () == CT_PrimDeclSpec::PDS_BOOL &&
             dss->Sons () && ! current_scope->GlobalScope ()) {
    // is there already a complete type?
    for (int i = 0; i < dss->Sons (); i++) {
      if (dss->Son (i)->NodeName () == CT_PrimDeclSpec::NodeId ()) {
        type = ((CT_PrimDeclSpec*)dss->Son (i))->SpecType ();
        if (! (type == CT_PrimDeclSpec::PDS_CHAR ||
               type == CT_PrimDeclSpec::PDS_WCHAR_T ||
               type == CT_PrimDeclSpec::PDS_BOOL ||
               type == CT_PrimDeclSpec::PDS_SHORT ||
               type == CT_PrimDeclSpec::PDS_INT ||
               type == CT_PrimDeclSpec::PDS_LONG ||
               type == CT_PrimDeclSpec::PDS_SIGNED ||
               type == CT_PrimDeclSpec::PDS_UNSIGNED ||
               type == CT_PrimDeclSpec::PDS_FLOAT ||
               type == CT_PrimDeclSpec::PDS_DOUBLE ||
               type == CT_PrimDeclSpec::PDS_INT64 ||
               type == CT_PrimDeclSpec::PDS_INT128 ||
               type == CT_PrimDeclSpec::PDS_UNKNOWN_T ||
               type == CT_PrimDeclSpec::PDS_VOID))
          continue;
      }
      token = ds->token ();
      if (token)
        token->reset (TOK_ID, "bool", Token::identifier_id);
      return (CTree*)0;
    }
  }

  dss->AddSon (ds);
  return ds;
}


CTree *CCSemantic::valid_id_expr() {
  CTree* expr = _postfix_expr.top ();
  if (expr && expr->IsSimpleName() && !CCSemExpr::isDependent(expr)) {
    // if name denotes a type, then this is not a valid id-expression
    CT_SimpleName* name = (CT_SimpleName*)expr;
    Token *token = name->Name()->token();

    // get the name's scope
    CStructure *scope = current_scope;
    int entries = name->Entries ();
    if (isQualName(name) && entries > 1)
      scope = getScope(((CT_SimpleName*)name->Entry(entries-2))->Object());
    else if (name->NodeName() == CT_RootQualName::NodeId())
      scope = _file;

    if (scope) {
      // lookup the name
      lookupName(token, scope, true);
      if (shared_nl.Objects()) {
        CObjectInfo *info = shared_nl.Object();
        if (info->isType()) {
          return 0;
        }
      }
    }
  }
  return expr;
}


CTree *CCSemantic::postfix_expr () {
  if (!_postfix_expr.top ())
    _postfix_expr.top () = builder ().postfix_expr ();
  else
    _postfix_expr.top () = builder ().postfix_expr (_postfix_expr.top ());
  return _postfix_expr.top ();
}


CTree *CCSemantic::postfix_expr2 () {
  CT_ConstructExpr *ce;
  CTree *result, *name;
  CT_DeclSpecSeq *dss;
  CObjectInfo *info;
  CT_NamedType *nt;

  result = builder ().postfix_expr2 ();
  if (result && result->NodeName () == CT_ConstructExpr::NodeId ()) {
    name = PrivateName ();
    dss = new CT_DeclSpecSeq;
    dss->AddSon (result->Son (0));
    nt = new CT_NamedType (dss, name);
    CSemDeclSpecs dsi (_err, dss);
    CCSemDeclarator csd (_err, dsi.make_type (), name);

    info = current_scope->newAttribute ();
    info->Linkage (determine_linkage (&dsi, ATTRIB));
    info->Storage (determine_storage_class (&dsi));
    Push (info);

    setSpecifiers (info, &dsi);
    setLanguage (info);
    common_settings (info, nt, &csd);
    nt->Object (info);
    ce = (CT_ConstructExpr*)result;
    ce->ReplaceSon (result->Son (0), nt);
    ce->Object (info);
    ce->setTypeRef (info->TypeInfo ());

    // resolve initializer arguments
    CT_ExprList *init = ce->Initializer ();
    if (init) {
      unsigned entries = init->Entries ();
      for (unsigned i = 0; i < entries; i++)
        resolveExpr (init->Entry (i), init);
    }
  }

  return result;
}


CTree *CCSemantic::postfix_expr1 () {
  CT_SimpleName *sn;

  if (builder ().nodes () == 3) {
    // member name prefixed by `template'?
    switch (builder ().get_node (0)->token ()->type ()) {
      case TOK_DOT:
      case TOK_PTS:
        // in a postfix-expression only unqualified
        // member names may be prefixed by `template'
        sn = (CT_SimpleName*)builder ().get_node (2);
        if (sn->NodeName () == CT_QualName::NodeId () ||
            sn->NodeName () == CT_RootQualName::NodeId ())
          return (CTree*)0;
      default:
        break;
    }
  }

  return builder ().postfix_expr1 ();
}


CTree *CCSemantic::pseudo_dtor_name () {
  int num = builder ().nodes ();

  // `template' may only be used after a nested_name_spec
  if ((num == 5 &&
       builder ().get_node (0)->token ()->type () == TOK_TEMPLATE) ||
      (num == 6 &&
       builder ().get_node (1)->token ()->type () == TOK_TEMPLATE &&
       builder ().get_node (0)->token ()->type () == TOK_COLON_COLON))
    return (CTree*)0;

  return builder ().pseudo_dtor_name ();
}


CTree *CCSemantic::simple_type_spec () {
  int num = builder ().nodes ();

  // `template' may only be used after a nested_name_spec
  if ((num == 2 &&
       builder ().get_node (0)->token ()->type () == TOK_TEMPLATE) ||
      (num == 3 &&
       builder ().get_node (1)->token ()->type () == TOK_TEMPLATE &&
       builder ().get_node (0)->token ()->type () == TOK_COLON_COLON))
    return (CTree*)0;

  return builder ().simple_type_spec ();
}


CTree *CCSemantic::direct_new_declarator () {
  builder ().Push (PrivateName ());
  return builder ().direct_new_declarator ();
}


void CCSemantic::declarator_id (bool tname) {
  // handle parameter declaration ambiguity
  if (in_arg_decl)
    is_type_name = tname;
}


CTree *CCSemantic::finish_init_declarator () {
  if (last_scope)
    reenter_scope (last_scope);
  last_scope = 0;
  return (CTree*)0;
}


/*****************************************************************************/
/*                                                                           */
/*                              Namespaces                                   */
/*                                                                           */
/*****************************************************************************/


void CCSemantic::declareNamespaceStd () {
  CNamespaceInfo *info;

  info = _file->newNamespace ();
  info->TypeInfo (&CTYPE_UNDEFINED);
  info->Name ("std");
  info->SourceInfo ()->FileInfo (_file);
  Push (info);
}


CTree *CCSemantic::introduce_orig_ns () {
  CUsingInfo *uinfo;
  CNamespaceInfo *info;
  CT_NamespaceDef *nd;

  nd = (CT_NamespaceDef*)builder ().orig_namespace_def1 ();

  CCNameLookup nl (*_err, current_scope, false);
  nl.lookup (nd->Name (), current_scope);
  if (nl.Objects ()) {
    // ���7.3.1.2 namespace name shall be undefined in this scope
    // (for gcc3.2 compatibility there can also be a type with
    //  the same name in this scope)
    SEM_ERROR__already_defined (nd, nd->Name (), nl.Object ());
    delete nd;
    return (CTree*)0;
  }

  info = current_scope->newNamespace ();
  info->TypeInfo (&CTYPE_UNDEFINED);
  info->Name (nd->Name ()->Text ());

  if (nd->isInline ()) {
    // C++11 inline namespace
    info->isInline (true);
    // A using-directive that names the inline namespace is implicitly
    // inserted in the enclosing namespace (similar to the implicit
    // using-directive for the unnamed namespace)
    uinfo = current_scope->newUsing ();
    uinfo->TypeInfo (&CTYPE_UNDEFINED);
    uinfo->Name (info->Name ());
    uinfo->Namespace (info);
    Push (uinfo);
  }

  // common settings
  enter_scope (info);
  common_settings (info, nd);
  nd->Name ()->Object (info);
  nd->Object (info);

  return nd;
}


CTree *CCSemantic::introduce_ns_extension () {
  CNamespaceInfo *info, *prev;
  CT_NamespaceDef *nd;

  // this is only an extension if the scope of the namespace name is
  // equivalent with the current scope. However, as original_ns_name only
  // guarantees that the name *exists*, we have to check this here.
  int isinline = builder ().get_node (0)->token ()->type () == TOK_INLINE;
  CT_SimpleName *name = (CT_SimpleName*)builder ().get_node (isinline ? 2 : 1);
  if (*name->Object ()->Scope () != *current_scope)
    return 0; // will become a new namespace, not an extension!

  nd = (CT_NamespaceDef*)builder ().orig_namespace_def1 ();
  prev = (CNamespaceInfo*)nd->Name ()->Object ();

  info = current_scope->newNamespace ();
  info->setShared (prev);
  info->TypeInfo (&CTYPE_UNDEFINED);
  info->Name (prev->Name ());
  info->isInline (prev->isInline ());
//  Push (info); // pushed by enter_scope()!!!

  // link it together
  prev->NextObject (info);

  // common settings
  enter_scope (info);
  common_settings (info, nd);
  nd->Name ()->Object (info);
  nd->Object (info);
  return nd;
}


CTree *CCSemantic::introduce_unnamed_ns () {
  CUsingInfo *uinfo;
  CNamespaceInfo *info, *prev;
  CT_NamespaceDef *nd;

  nd = (CT_NamespaceDef*)builder ().orig_namespace_def1 ();

  // check whether there is already an unnamed namespace in the
  // current scope
  prev = (CNamespaceInfo*)0;
  CCNameLookup nl (*_err, current_scope, false);
  nl.lookup ("<unnamed>", current_scope);
  if (nl.Objects ()) {
    // yes, this is an extension.
    // connect the namespaces and share a symbol table
    prev = (CNamespaceInfo*)nl.Object (0);
  }

  info = current_scope->newNamespace ();
  info->TypeInfo (&CTYPE_UNDEFINED);
  info->Name ("<unnamed>"); // all unnamed namespaces get the same name
  if (nd->isInline ()) {
    // C++11 inline namespace
    info->isInline (true);
  }
//  info->Name (nd->Name ()->Text ());
//  Push (info);  // pushed by enter_scope()!!!

  // connect with original definition
  if (prev) {
    info->setShared (prev);
    prev->NextObject (info);
  }
  else {
    // ���7.3.1.1 an unnamed namespace definition behaves as if
    // it were replaced by:
    //   namespace %unique-name { /* empty body */ }
    //   using namespace %unique-name;
    //   namespace %unique-name { /* namespace-body */ }
    uinfo = current_scope->newUsing ();
    uinfo->TypeInfo (&CTYPE_UNDEFINED);
    uinfo->Name (info->Name ());
    uinfo->Namespace (info);
    Push (uinfo);
  }
  // common settings
  enter_scope (info);
  common_settings (info, nd);
  nd->Name ()->Object (info);
  nd->Object (info);
  return nd;
}


CTree *CCSemantic::namespace_def () {
  CT_NamespaceDef *result = (CT_NamespaceDef*)builder ().orig_namespace_def ();
  result->Members ()->Scope (current_scope);
  return result;
}


CTree *CCSemantic::introduce_ns_alias () {
  CNamespaceInfo *info, *orig;
  CT_NamespaceAliasDef *nd;
  CObjectInfo *prev = 0;

  nd = (CT_NamespaceAliasDef*)builder ().ns_alias_def ();

  // lookup original namespace name
  CCNameLookup nl (*_err, current_scope, false);
  nl.lookupNamespace (nd->Name (), current_scope);
  if (! nl.Objects ()) {
    SEM_ERROR__unknown_ns (nd, nd->Name ());
    delete nd;
    return (CTree*)0;
  } else
    orig = (CNamespaceInfo*)nl.Object ();

  // lookup namespace alias name
  nl.reset ();
  nl.lookupType (nd->Alias (), current_scope);
  if (nl.Objects ()) {
    prev = nl.Object ();
    // ���7.3.2.3 namespace aliases can be redefined
    if (! prev->NamespaceInfo () || ! prev->NamespaceInfo ()->isAlias ()) {
      SEM_ERROR__already_defined (nd, nd->Alias (), prev);
      delete nd;
      return (CTree*)0;
    }
    // the original alias has to be an alias of the same namespace
    // as specified in the current namespace alias definition
    info = prev->NamespaceInfo ();
    while (true) {
      info = (CNamespaceInfo*)info->NextObject ();
      if (info == orig) // ok, alias of the same namespace
        break;
      if (info == prev) {
        SEM_ERROR__redefinition (nd, nd->Alias (), prev);
        delete nd;
        return (CTree*)0;
      }
    }
  }

  info = current_scope->newNamespace ();
  info->setShared (orig);
  info->TypeInfo (&CTYPE_UNDEFINED);
  info->Name (nd->Alias ()->Text ());
  info->isAlias (true);
  Push (info);

  // link alias and original namespace
  orig->NextObject (info);

  // common settings
  common_settings (info, nd);
  nd->Alias ()->Object (info);
  nd->Object (info);
  return nd;
}


CTree *CCSemantic::using_directive () {
  CUsingInfo *info;
  CNamespaceInfo *nsinfo;
  CT_UsingDirective *ud;

  ud = (CT_UsingDirective*)builder ().using_directive ();
  if (isQualName (ud->Name ()))
    nsinfo = (CNamespaceInfo*)ud->Name ()->Name ()->Object ();
  else
    nsinfo = (CNamespaceInfo*)ud->Name ()->Object ();

  // ���7.3.4 a using directive shall only appear in namespace
  // or local scope
  if (! (current_scope->isNamespace () || current_scope->isLocalScope ())) {
    SEM_ERROR (ud, "using directive only allowed in block or namespace scope");
    delete ud;
    return (CTree*)0;
  }

  // delete possibly created container object because it would
  // be deleted by the builder only when the rule fails
  deleteContainer (3);

  info = current_scope->newUsing ();
  info->TypeInfo (&CTYPE_UNDEFINED);
  info->Name (nsinfo->Name ());
  info->Namespace (nsinfo);
  Push (info);

  // common settings
  common_settings (info, ud);
  return ud;
}


CTree *CCSemantic::using_decl () {
  CMemberAliasInfo *minfo;
  CObjectInfo *info;
  CT_UsingDecl *ud;

  ud = (CT_UsingDecl*)builder ().using_decl ();
  if (current_scope->ClassInfo ())
    return access_decl (ud);

  // check for dependent name
  info = getLastResolvedName (ud->Member ());
  if (info && info->TypeInfo () && info->TypeInfo ()->isDependent ()) {
    deleteContainer (3);
    return ud;
  }

  // using declaration in block or namespace scope

  CCNameLookup nl (*_err, current_scope, !is_entity_name());
  nl.lookup (ud->Member (), current_scope);
  if (! nl.Objects ()) {
    // report error
    std::ostringstream name; qualifiedScopeName (ud->Member (), name);
    SEM_ERROR__not_member (ud, ud->Member ()->Name (), name.str ().c_str ());
    delete ud;
    return (CTree*)0;
  }

  // delete possibly created container object because it would
  // be deleted by the builder only when the rule fails
  deleteContainer (3);

  // check for duplicates in current scope
//  CCNameLookup onl (*_err, current_scope, !is_entity_name());
//  if (nl.Objects ()) {
//    info = nl.Object ();
//    if (info->Record () || info->EnumInfo ())
//      onl.lookupType (info->Name (), current_scope);
//    else
//      onl.lookup (info->Name (), current_scope);
//  }

  // introduce aliases into the current scope
  for (unsigned i = 0; i < nl.Objects (); i++) {
    info = nl.Object (i);

    // handle redefined typedefs -> we might have found an anonymous object
    CObjectInfo *start = info;
    while (info->TypedefInfo () && info->isAnonymous ()) {
      info = info->NextObject ();
      if (info == start)
        break; // TODO: error message?
    }

// error checks: not yet implemented
//    if (info->Record () || info->EnumInfo () || info->TypedefIndo () ||
//         info->isNamespace ()) && onl.Objects ())
//    if (info->isFunction () && onl.Objects () &&
//        ((onl->areFunctions && equalTypes()) || !onl->isType))
//    if (info->isAttrib &&
//        ((onl->isAttrib && !onl->isExtern && !info->isExtern) || !onl->isType))

    minfo = current_scope->newMemberAlias (info);
    minfo->TypeInfo (info->TypeInfo ()->Duplicate ());
    minfo->Name (info->Name ());
    minfo->Member (info);
    setProtection (minfo);
    Push (minfo);
    common_settings (minfo, ud);
  }
  return ud;
}


CTree *CCSemantic::access_decl (CTree *d) {
  CMemberAliasInfo *minfo;
  CT_AccessDecl *ad;
  CObjectInfo *info;
  CRecord *cinfo;

  ad = (CT_AccessDecl*)(d ? d : builder ().access_decl ());

  // check for dependent name
  info = getLastResolvedName (ad->Member ());
  if (info && info->TypeInfo () && info->TypeInfo ()->isDependent ()) {
    return ad;
  }

  CCNameLookup nl (*_err, current_scope, !is_entity_name());
  nl.lookup (ad->Member (), current_scope);
  if (! nl.Objects ()) {
    std::ostringstream name; qualifiedScopeName (ad->Member (), name);
    SEM_ERROR__not_member (ad, ad->Member ()->Name (), name.str ().c_str ());
    delete ad;
    return (CTree*)0;
  } else {
    info = nl.Object ();
    cinfo = (info->FunctionInfo () ? info->FunctionInfo ()->Record () :
                                     info->Scope ()->Record ());
    if (! cinfo ||
        ! isBaseClass (current_scope->ClassInfo (), cinfo->ClassInfo ())) {
      if (! cinfo) {
        SEM_ERROR (ad, "`" << *ad->Member () << "' is not a class member");
      } else {
        SEM_ERROR__not_base_class (ad, cinfo);
      }
      delete ad;
      return (CTree*)0;
    }
  }

  // introduce aliases of the base class members looked up
  // into the current scope
  for (unsigned i = 0; i < nl.Objects (); i++) {
    info = nl.Object (i);
    minfo = current_scope->newMemberAlias (info);
    minfo->TypeInfo (info->TypeInfo ()->Duplicate ());
    minfo->Name (info->Name ());
    minfo->Member (info);
    setProtection (minfo);
    Push (minfo);
    common_settings (minfo, ad);
  }
  return ad;
}


/*****************************************************************************/
/*                                                                           */
/*                   constant expression evaluation                          */
/*                                                                           */
/*****************************************************************************/


CTree *CCSemantic::const_expr () {
  CTree *result = builder ().const_expr ();
  if (result)
    resolveExpr (result);
  return result;
}


CStructure *CCSemantic::array_delim (CStructure *scope) {
  // enter class scope to find class scope members
  if (scope)
    reenter_scope (scope);
  else {
    scope = current_scope;
    if (last_search_scope)
      reenter_scope (last_search_scope);
  }
  return scope;
}


CStructure *CCSemantic::conv_fct_id (CStructure *scope) {
  // enter class scope to find class scope members
  if (scope)
    reenter_scope (scope);
  else {
    scope = current_scope;
    if (last_search_scope) {
      CFunctionInfo *info = _db->newFunction();
      info->Name("%conv_type_id");
      info->QualifiedScope (getQualifiedScope (last_search_scope));
      enter_scope (info);
    }
  }
  return scope;
}


CTypeInfo *CCSemantic::resolveExpr (CTree *expr, CTree *base) const {
  if (expr) {
    CCSemExpr cse (*_err, current_scope);
    return cse.resolveExpr (expr, base);
  }
  return (CTypeInfo*)0;
}


CTree *CCSemantic::init_declarator () {
  CT_InitDeclarator *node;
  CObjectInfo *info;
  CTree *init;

  node = (CT_InitDeclarator*)builder ().init_declarator ();
  info = node->Object ();

  // evaluate the initializer of constant variables and
  // class members before its further use
  if (info && info->TypeInfo ()->isConst ()) {
    init = node->Initializer ();
    if (init) {
      CCSemExpr cse (*_err, current_scope);
      cse.resolveInit (init, node);
    }
  }

  return node;
}


CTree *CCSemantic::pure_spec () {
  CT_ExprList *el;
  CTree *expr;

  el = (CT_ExprList*)builder ().pure_spec ();
  if (el->Entries ()) {
    expr = el->Entry (0);
    resolveExpr (expr, el);
    expr = el->Entry (0);
    if (expr->Type ())
      el->setTypeRef (expr->Type ());
    else
      el->setTypeRef (&CTYPE_UNDEFINED);

    if (expr->Value ())
      el->setValueRef (expr->Value ());
  }
  return el;
}


CTree *CCSemantic::const_init () {
  CT_ExprList *el;
  CTree *expr;

  el = (CT_ExprList*)builder ().const_init ();
  if (el->Entries ()) {
    expr = el->Entry (0);
    if (expr->Type ())
      el->setTypeRef (expr->Type ());
    else
      el->setTypeRef (&CTYPE_UNDEFINED);

    if (expr->Value ())
      el->setValueRef (expr->Value ());
  }
  return el;
}


} // namespace Puma
