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
#include "Puma/CUnit.h"
#include "Puma/UnitIterator.h"
#include "Puma/Array.h"
#include "Puma/PtrStack.h"
#include "Puma/ErrorSink.h"
#include "Puma/CSyntax.h"
#include "Puma/CSemantic.h"
#include "Puma/ErrorSeverity.h"
#include "Puma/CSemDeclarator.h"
#include "Puma/CSemDeclSpecs.h"
#include "Puma/CFileInfo.h"
#include "Puma/CEnumInfo.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CLabelInfo.h"
#include "Puma/CLocalScope.h"
#include "Puma/CTypedefInfo.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CArgumentInfo.h"
#include "Puma/CAttributeInfo.h"
#include "Puma/CEnumeratorInfo.h"
#include "Puma/CMemberAliasInfo.h"
#include "Puma/CSemDatabase.h"
#include "Puma/CSemExpr.h"
#include "Puma/CConstant.h"

#include <stdio.h>
#include <string.h>
#include <iostream>

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

#define SEM_WARNING__implicit_fct(loc__,name__) \
  SEM_MSG (loc__, sev_warning << "implicit declaration of function ‘" \
    << name__ << "'")

#define SEM_MSG__prev_loc(info__) \
  *_err << info__->SourceInfo ()->StartToken ()->token ()->location () \
        << "previously defined here" << endMessage

#define SEM_WARNING__redefinition(token__,type__,info__) \
  SEM_WARNING (token__, "redefinition of " << type__ \
    << "`" << info__->Name () << "'"); \
  SEM_MSG__prev_loc(info__)

#define SEM_ERROR__redefinition(token__,type__,info__) \
  SEM_ERROR (token__, "redefinition of " << type__ \
    << "`" << info__->Name () << "'"); \
  SEM_MSG__prev_loc(info__)

#define SEM_ERROR__already_defined(token__,info__) \
  SEM_ERROR (token__, "`" << info__->Name () \
    << "' already defined"); \
  SEM_MSG__prev_loc(info__)

#define SEM_ERROR__conflicting_types(token__,info__) \
  SEM_ERROR (token__, "conflicting types for `" \
    << info__->Name () << "'"); \
  SEM_MSG__prev_loc(info__)

#define SEM_ERROR__duplicate(token__,type__,name__) \
  SEM_ERROR (token__, "duplicate " << type__ \
    << " `" << name__->Text () << "'")

#define SEM_ERROR__wrong_storage_class(token__,type__,name__) \
  SEM_ERROR (token__, "wrong storage class specified for " \
    << type__ << " `" << name__->Text () << "'")

#define SEM_ERROR__static_follows_non_static(token__,info__) \
  SEM_ERROR (token__, "static declaration of `" << info__->Name () \
    << "' follows non-static declaration"); \
  SEM_MSG__prev_loc(info__)


void CSemantic::init (CSemDatabase &db, Unit &u) {
  Semantic::init (db, u);
  if (! _file->haveCBuiltins ()) {
    _file->haveCBuiltins (true);
    declareImplicitFcts ();
  }
}


void CSemantic::configure (Config &c) {
  Semantic::configure (c);
}


void CSemantic::enter_scope (CStructure *scp) {
  Semantic::enter_scope(scp);
  if (current_scope && ! current_scope->Record())
    non_record_scopes.Push(current_scope);
}


void CSemantic::reenter_scope (CStructure *scp) {
  Semantic::reenter_scope(scp);
  if (current_scope && ! current_scope->Record())
    non_record_scopes.Push(current_scope);
}


void CSemantic::leave_scopes () {
  CStructure* scope = current_scope;
  Semantic::leave_scopes();
  if (scope != current_scope && scope == non_record_scopes.Top())
    non_record_scopes.Pop();
}


CAttributeInfo *CSemantic::createAttribute (const char *name, 
 CStructure *scope, CTypeInfo *type, bool is_static) {
  CAttributeInfo *info;
  info = scope->newAttribute ();
  info->Name (name);
  info->ObjectInfo ()->TypeInfo (type);
  info->SourceInfo ()->FileInfo (_file);
  info->isStatic (is_static);
  Push (info);
  return info;
}


CTypedefInfo *CSemantic::createTypedef (const char *name, 
 CStructure *scope, CTypeInfo *type) {
  CTypedefInfo *info;
  info = scope->newTypedef ();
  type = type->TypedefInfo (info);
  info->Name (name);
  info->ObjectInfo ()->TypeInfo (type);
  info->SourceInfo ()->FileInfo (_file);
  Push (info);
  return info;
}


CFunctionInfo *CSemantic::createFunction (const char *name, 
 CStructure *scope, CTypeInfo *type) {
  CFunctionInfo *info;
  info = scope->newFunction ();
  info->Linkage (CLinkage::LINK_EXTERNAL);
  info->Name (name);
  info->ObjectInfo ()->TypeInfo (type);
  info->SourceInfo ()->FileInfo (_file);
  type->VirtualType ()->TypeFunction ()->FunctionInfo (info);
  Push (info);
  return info;
}


void CSemantic::createParameter (CFunctionInfo *finfo, 
 CTypeInfo *argtype) {
  CArgumentInfo *info;
  info = finfo->newArgument ();
  std::ostringstream name; name << "%anon" << _Anonymous; _Anonymous++;
  info->Name (name.str ().c_str ());
  info->Storage (CStorage::CLASS_AUTOMATIC);
  info->SourceInfo ()->FileInfo (_file);
  info->TypeInfo (argtype->Duplicate ());
  Push (info);
}


void CSemantic::Delete () {
  CObjectInfo *info = Top (); // get item to delete
  if (! info)
    return;

  if (info->Id () == CObjectInfo::ATTRIBUTE_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (Attribute "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::MEMBERALIAS_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (Member alias "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::ARGUMENT_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (Argument "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::ENUMERATOR_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (Enumerator "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::LABEL_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (Label "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::TYPEDEF_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (Typedef "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::ENUM_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (Enum "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::LOCAL_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (LocalScope "<<info->Name()<<")"<<std::endl;
    /*DEBUG*/if (TRACE_SCOPES) std::cerr<<(void*)this<<": -SCOPE: "<<(void*)info<<" (Local)"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::UNION_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (Union "<<info->Name()<<")"<<std::endl;
    /*DEBUG*/if (TRACE_SCOPES) std::cerr<<(void*)this<<": -SCOPE: "<<(void*)info<<" (Union "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::CLASS_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (Class "<<info->Name()<<")"<<std::endl;
    /*DEBUG*/if (TRACE_SCOPES) std::cerr<<(void*)this<<": -SCOPE: "<<(void*)info<<" (Class "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
  } else if (info->Id () == CObjectInfo::FUNCTION_INFO) {
#ifndef NDEBUG
    /*DEBUG*/if (TRACE_OBJS) std::cerr<<"DELETE: "<<(void*)info<<" (Function "<<info->Name()<<")"<<std::endl;
    /*DEBUG*/if (TRACE_SCOPES) std::cerr<<(void*)this<<": -SCOPE: "<<(void*)info<<" (Function "<<info->Name()<<")"<<std::endl;
#endif // not NDEBUG
    if (info == current_fct) {
#ifndef NDEBUG
      /*DEBUG*/if (TRACE_FCT) std::cout<<"FCT NIL(3): "<<(void*)current_fct<<std::endl;
#endif // not NDEBUG
      current_fct = 0;
    }
  }

  if (info->ScopeInfo () && info->ScopeInfo () == current_scope) {
    if (current_scope == non_record_scopes.Top())
      non_record_scopes.Pop();
    CScopeInfo* parent = info->ScopeInfo ()->Parent ();
    assert (parent && parent->Structure ());
    current_scope = parent->Structure ();
  }

  Semantic::Delete ();
}



/*****************************************************************************/
/*                                                                           */
/*                           Name lookup                                     */
/*                                                                           */
/*****************************************************************************/



CObjectInfo *CSemantic::lookup (const char *id, CStructure *scope, SemObjType type, bool nested) const {
  CObjectInfo *result = (CObjectInfo*)0;

  if (id && strncmp(id, "%anon", 5) == 0)
    return result;

  switch (type) {
    case CLASS:
      result = scope->Type (id);
      if (result && ! result->ClassInfo ())
        result = (CObjectInfo*)0;
      break;
    case UNION:
      result = scope->Type (id);
      if (result && ! result->UnionInfo ())
        result = (CObjectInfo*)0;
      break;
    case ENUM: 
      result = scope->Type (id);
      if (result && ! result->EnumInfo ())
        result = (CObjectInfo*)0;
      break;
    case FCT: 
      result = scope->Function (id);
      break;
    case TYPEDEF:
      result = scope->Typedef (id);
      break;
    case ATTRIB: 
      result = scope->Attribute (id);
      break;
    case ARG: 
      if (scope->FunctionInfo ())
        result = scope->FunctionInfo ()->Argument (id);
      break;
    case TAG:
      result = scope->Type (id);
      if (result && ! result->Record () && ! result->EnumInfo ()) {
        // if searching for a struct or enum type of a tag (struct X)
        // consider also typedefs to find struct or enum types hidden 
        // by redefining typedefs (typedef struct X X;)
        CTypedefInfo* tdef = result->TypedefInfo ();
        if (tdef && (tdef->TypeInfo ()->Record () || tdef->TypeInfo ()->EnumInfo ())) {
          result = tdef;
        } else {
          result = (CObjectInfo*)0;
        }
      }
      break;
    case NON_TAG:
      {
        CStructure::ObjectsByName::iterator objs = scope->ObjectInfoMap().find(id);
        if (objs != scope->ObjectInfoMap().end()) {
          CStructure::ObjectInfoList::reverse_iterator obj = objs->second.rbegin();
          for (; obj != objs->second.rend(); ++obj) {
            result = *obj;
            if (! result->Record () && ! result->EnumInfo ())
              break;
            result = (CObjectInfo*)0;
          }
        }
      }
      break;
    case ANY:
      result = scope->Object (id);
      break;
  }

  if ((nested || (! scope->FunctionInfo () && 
       scope->Parent ()->FunctionInfo ())) && 
      ! result && scope != scope->Parent ())
    return lookup (id, scope->Parent ()->Structure (), type, nested);
  else
    return result;
}



CObjectInfo* CSemantic::lookupBuiltin (const char* name) const {
  CObjectInfo* info = 0;
  if (strncmp (name, "__builtin_", 10) == 0) {
    info = lookup (name+10, NON_TAG, true);
  }
  return info;
}



/*****************************************************************************/
/*                                                                           */
/*                  Linkage and storage class determination                  */
/*                                                                           */
/*****************************************************************************/



CLinkage::Type CSemantic::determine_linkage (CSemDeclSpecs *dss, 
 SemObjType type, CObjectInfo *prev_decl) const {
  bool extern_fct = false;
  if (type == ATTRIB || type == FCT) {
    if (type == ATTRIB) {
      if (current_scope->FileInfo ()) {
        if (dss->declared (CT_PrimDeclSpec::PDS_STATIC))
          return CLinkage::LINK_INTERNAL;
        if (! dss->declared (CT_PrimDeclSpec::PDS_STATIC) &&
            ! dss->declared (CT_PrimDeclSpec::PDS_EXTERN) &&
            ! dss->declared (CT_PrimDeclSpec::PDS_AUTO) &&
            ! dss->declared (CT_PrimDeclSpec::PDS_REGISTER))
          return CLinkage::LINK_EXTERNAL;
      } else if (current_scope->LocalScope ()) {
        if (! dss->declared (CT_PrimDeclSpec::PDS_EXTERN))
          return CLinkage::LINK_NONE;
      }
    } else if (type == FCT) {
      if (current_scope->FileInfo ())
        if (dss->declared (CT_PrimDeclSpec::PDS_STATIC))
          return CLinkage::LINK_INTERNAL;
      if (! dss->declared (CT_PrimDeclSpec::PDS_STATIC) &&
          ! dss->declared (CT_PrimDeclSpec::PDS_EXTERN) &&
          ! dss->declared (CT_PrimDeclSpec::PDS_AUTO) &&
          ! dss->declared (CT_PrimDeclSpec::PDS_REGISTER))
        extern_fct = true;
    }
    if (extern_fct || dss->declared (CT_PrimDeclSpec::PDS_EXTERN)) {
      if (prev_decl && (prev_decl->Linkage () == CLinkage::LINK_INTERNAL ||
                        prev_decl->Linkage () == CLinkage::LINK_EXTERNAL))
        return prev_decl->Linkage ();
      else
        return CLinkage::LINK_EXTERNAL;
    }
  }
  return CLinkage::LINK_NONE;
}
  


CStorage::Type CSemantic::determine_storage_class (CSemDeclSpecs *dss, 
 SemObjType type, CLinkage::Type linkage) const {
  if (type == ATTRIB) {
    bool def_static = dss->declared (CT_PrimDeclSpec::PDS_STATIC);
    bool def_thread = dss->declared (CT_PrimDeclSpec::PDS_THREAD);
    if (linkage == CLinkage::LINK_EXTERNAL ||
        linkage == CLinkage::LINK_INTERNAL || def_static) {
      return def_thread ? CStorage::CLASS_THREAD : CStorage::CLASS_STATIC;
    } else if (linkage == CLinkage::LINK_NONE) {
      if (! def_static) 
        return CStorage::CLASS_AUTOMATIC;
    }
  }
  return CStorage::CLASS_NONE;
}



/*****************************************************************************/
/*                                                                           */
/*                           Node analysis                                   */
/*                                                                           */
/*****************************************************************************/



CTree *CSemantic::typedef_name () {
  CObjectInfo *info;
  info = lookup (builder ().get_node ()->token ()->text (), NON_TAG, true);
  if (info && info->TypedefInfo ()) {
    CT_SimpleName *result = (CT_SimpleName*)builder ().simple_name ();
    result->Object (info);
    result->setTypeRef (info->TypeInfo ());
    return result;
  } else 
    return (CTree*)0;
}



CTree *CSemantic::id_expr () {
  CT_SimpleName *result = 0;
  const char* name = builder ().get_node ()->token ()->text ();
  CObjectInfo *info = lookup (name, NON_TAG, true);
  if (! info)
    info = lookupBuiltin (name);
  if (info && ! info->TypedefInfo ()) {
    result = (CT_SimpleName*)builder ().simple_name ();
    result->Object (info);
    result->setTypeRef (info->TypeInfo ());
  } else if (syntax ().look_ahead () == TOK_OPEN_ROUND) {
    // call to undeclared function
    result = (CT_SimpleName*)builder ().simple_name ();
    SEM_WARNING__implicit_fct (result, name);
  }
  return result;
}



CTree *CSemantic::introduce_object (CTree* tree) {
  bool is_typedef, is_fct, is_attr;
  CObjectInfo *info, *prev;
  CT_InitDeclarator *id;
  CSemDeclSpecs *dsi;
  CTypeInfo *type;

  if (! in_decl ()) 
    return (CTree*)0;

  id = (CT_InitDeclarator*) (tree ? tree : builder ().init_declarator1 ());
  dsi = sem_decl_specs ();
  CSemDeclarator csd (_err, dsi->make_type (), id);
  if (! csd.Name ()) {
    builder ().destroyNode (id);
    return (CTree*)0;
  } 

  type = csd.Type ();
  is_typedef = dsi->declared (CT_PrimDeclSpec::PDS_TYPEDEF);
  is_fct = (! is_typedef && type->isFunction () && ! type->isAddress ());
  is_attr = ! (is_typedef || is_fct); 

  prev = lookup (csd.Name ()->Text (), NON_TAG, false);
  if (prev) {
    bool error = true;
    if ((is_typedef && ! prev->TypedefInfo ()) ||
        (is_attr && ! prev->AttributeInfo ()) ||
        (is_fct && ! prev->FunctionInfo ())) {
      SEM_ERROR__already_defined (id, prev);
    } else if (! typeMatch (type, prev->TypeInfo ())) {
      SEM_ERROR__conflicting_types (id, prev);
    } else if (is_typedef) {
      // for compatibility only issue a warning
      SEM_WARNING__redefinition (id, "typedef ", prev);
      id->Object (prev);
      return id;
    } else if (is_attr && (prev->Linkage () == CLinkage::LINK_NONE ||
      determine_linkage (dsi, ATTRIB, prev) == CLinkage::LINK_NONE)) {
      SEM_ERROR__redefinition (id, "", prev);
    } else if (is_fct && dsi->declared (CT_PrimDeclSpec::PDS_STATIC) &&
               !prev->isStatic () &&
               !(prev->isExtern () && prev->isInline ())) {
      SEM_ERROR__static_follows_non_static (id, prev);
      error = false; // it is sufficient to mark the error stream
    } else
      error = false;
    if (error) {
      builder ().destroyNode (id);
      return (CTree*)0;
    }
  }

  if (is_typedef) {
    info = current_scope->newTypedef ();
    Push (info);
  } else if (is_fct) {
    if (! current_fct)
      current_fct = current_scope->newFunction ();
/*DEBUG*/if (TRACE_FCT) std::cout<<"FCT DEC(2): "<<(void*)current_fct<<std::endl;
    info = current_fct;
    info->Linkage (determine_linkage (dsi, FCT, prev));
    info->ScopeInfo ()->Parent ()->Structure ()->
      addFunction (info->FunctionInfo ());
  } else {
    info = current_scope->newAttribute ();
    info->Linkage (determine_linkage (dsi, ATTRIB, prev));
    info->Storage (determine_storage_class (dsi, ATTRIB, info->Linkage ()));
    Push (info);
  }

  // common settings
  setSpecifiers (info, dsi);
  common_settings (info, id, &csd);
  if (is_typedef) {
    type = info->TypeInfo ()->TypedefInfo (info);
    info->TypeInfo (type);
  } else if (is_fct) {
    info->TypeInfo ()->VirtualType ()->TypeFunction ()->FunctionInfo (info->FunctionInfo ());
  }
  if (prev)
    prev->NextObject (info);
  id->Object (info);
  if (is_fct)
    current_fct = 0;
  return id;
}



void CSemantic::introduce_label (CTree *&tree) {
  CFunctionInfo *fct;
  CT_LabelStmt *ls;
  CLabelInfo *info;
  const char *name;
  
  // build label statement
  ls = (CT_LabelStmt*) tree;
  if (ls->NodeName () != CT_LabelStmt::NodeId ())
    return;
    
  // check syntactical position of label statement
  name = ls->Label ()->Text ();
  if (! current_scope->LocalScope ()) {
    SEM_ERROR (ls->Label (), "label `" << name << "' outside of function");
    builder ().destroyNode (ls);
    tree = (CTree*)0;
    return;
  }
  
  // check if label name already used as a label in this function
  fct = current_scope->LocalScope ()->Function ();
  if (fct->Label (name)) {
    SEM_ERROR__duplicate (ls, "label", ls->Label ());
    builder ().destroyNode (ls);
    tree = (CTree*)0;
    return;
  }
  
  // complete semantic information about the label
  info = fct->newLabel ();
  info->Name (name);
  info->TypeInfo (&CTYPE_UNDEFINED);
  common_settings (info, ls);
  ls->Label ()->Object (info);
  Push (info);
  return;
}



CTree *CSemantic::enumerator_def () {
  CEnumeratorInfo *einfo;
  CEnumInfo *enuminfo;
  CT_Enumerator *e;
  CTypeInfo *type;
  CConstant *v;
  CTree *init;
  LONG_LONG value;
  unsigned numenums;

  e = (CT_Enumerator*)builder ().enumerator_def ();

  if (e && e->Object ()) {
    einfo = e->Object ()->EnumeratorInfo ();
    enuminfo = einfo->Enum ();
    numenums = enuminfo->Enumerators ();
    value = 0;

    // if the enumerator has an initializer, get the value
    // from this constant expression
    init = e->Initializer ();
    if (init) {
      type = resolveExpr (init->Son (1), init);
      init = init->Son (1);

      if (! CSemExpr::isConstIntExpr (init)) {
        SEM_ERROR (e, "value for enumerator `" << e->Name ()->Text ()
          << "' isn't integer constant expression");
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
    bool long_gt_int = CTYPE_LONG.Size () > CTYPE_INT.Size ();
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



CTree *CSemantic::introduce_enum () {
  CObjectInfo *info, *oi;
  CT_SimpleName *name;
  CStructure *scope;
  CT_EnumDef *ed;

  // OS: I don't understand what this is/was good for!
//  if (builder ().nodes () == 2)
//    builder ().Push (PrivateName ());
    
  ed = (CT_EnumDef*)builder ().enum_spec1 ();
  name = (CT_SimpleName*)ed->Name ();
  scope = findParent ();
  oi = lookup (name->Text (), scope, TAG, false);

  if (oi) {
    if (isRedefiningTypedef (oi, CTypeInfo::TYPE_ENUM)) {
      oi = oi->TypeInfo ()->TypeEnum ()->EnumInfo ();
    } else if (oi->TypedefInfo ()) {
      oi = 0;
    }
  }
  if (oi) {
    if (oi->EnumInfo () && oi->EnumInfo ()->isDefined ()) {
      SEM_ERROR__redefinition (ed, "enum ", oi);
      builder ().destroyNode (ed);
      return (CTree*)0;
    } else if (! oi->EnumInfo ()) {
      SEM_ERROR__already_defined (ed, oi);
      builder ().destroyNode (ed);
      return (CTree*)0;
    }
  } 
  
  info = scope->newEnum ();
  info->TypeInfo (new CTypeEnum (info->EnumInfo ()));
  info->Name (name->Text ());
  if (scope != current_scope)
    info->AssignedScope (scope);
  Push (info);

  current_enum = info->EnumInfo ();
  common_settings (info, ed);
  name->Object (info);
  name->setTypeRef (info->TypeInfo ());
  if (oi)
    oi->NextObject (info);
  ed->Object (info);

  return ed;
}



CTree *CSemantic::introduce_enumerator () {
  CEnumeratorInfo *info;
  CStructure *scope;
  CT_Enumerator *e;
  CObjectInfo *oi;  
  Token *name;
  
  name = builder ().get_node ()->token ();
  scope = findParent ();
  oi = lookup (name->text (), scope, NON_TAG, false);
  if (oi) {
    SEM_ERROR__already_defined (builder ().get_node (), oi);
    return (CTree*)0;
  }

  e = (CT_Enumerator*) builder ().enumerator ();
  info = current_enum->newEnumerator ();
  info->Scope (scope);
  info->Name (name->dtext ());
  info->Storage (CStorage::CLASS_STATIC);
  Push (info);
  
  common_settings (info, e);
  scope->addAttribute (info);
  e->Object (info);
  e->Name ()->Object (info);
  e->Name ()->setTypeRef (&CTYPE_LONG);
  info->ObjectInfo ()->TypeInfo (&CTYPE_LONG);
  return e;
}



CTree *CSemantic::introduce_named_type () {
  CSemDeclSpecs *dsi;
  CObjectInfo *info;
  CT_NamedType *nt;
  
  if (! in_decl ())
    return (CTree*)0;
  
  nt = (CT_NamedType*)builder ().type_id ();
  dsi = sem_decl_specs ();
  CSemDeclarator csd (_err, dsi->make_type (), nt->Declarator ());
  if (! csd.Name ()) {
    builder ().destroyNode (nt);
    return (CTree*)0;
  }

  info = current_scope->newAttribute ();
  info->Linkage (determine_linkage (dsi, ATTRIB));
  info->Storage (determine_storage_class (dsi, ATTRIB, info->Linkage ()));
  Push (info);
  
  setSpecifiers (info, dsi);
  common_settings (info, nt, &csd);
  nt->Object (info);
  decl_end ();
  return nt;
}



CTree *CSemantic::introduce_parameter () {
  CArgumentInfo *info;
  CSemDeclSpecs *dsi;
  CT_ArgDecl *ad;
  bool error;
  
  if (! in_decl ())
    return (CTree*)0;    
  if (builder ().nodes () == 1)
    builder ().Push (PrivateName ());  
    
  ad = (CT_ArgDecl*)builder ().param_decl1 ();
  dsi = sem_decl_specs ();
  CSemDeclarator csd (_err, dsi->make_type (), ad->Declarator ());

  error = true;
  if (! csd.Name ()) {
    /* internal error */
  } else if (lookup (csd.Name ()->Text (), ARG, false)) {
    SEM_ERROR__duplicate (ad, "function parameter", csd.Name ());
  } else if (dsi->declared (CT_PrimDeclSpec::PDS_TYPEDEF) ||
             dsi->declared (CT_PrimDeclSpec::PDS_AUTO) ||
             dsi->declared (CT_PrimDeclSpec::PDS_STATIC) ||
             dsi->declared (CT_PrimDeclSpec::PDS_THREAD) ||
             dsi->declared (CT_PrimDeclSpec::PDS_EXTERN)) {
    SEM_ERROR__wrong_storage_class (ad, "function parameter", csd.Name ());
  } else
    error = false;

  if (error) {
    builder ().destroyNode (ad);
    return (CTree*)0;
  }

  info = current_scope->FunctionInfo ()->newArgument ();
  info->Storage (CStorage::CLASS_AUTOMATIC);
  Push (info);
  
  // adjust parameter type
  CTypeInfo* type = csd.Type ();

  // any cv-qualifier modifying a parameter is deleted
  if (type->TypeQualified ()) {
    CTypeInfo* tmp = type;
    type = type->UnqualType ()->Duplicate ();
    CTypeInfo::Destroy (tmp);
  } 
  // type `function returning T' is adjusted to be 
  // `pointer to function returning T'
  if (type->TypeFunction ()) {
    type = new CTypePointer (type);
    // type `array of T' is adjusted to be `pointer to T'
  } else if (type->TypeArray ()) {
    CTypeInfo* quals = type->TypeArray ()->Qualifiers ();
    CTypeInfo* tmp = type;
    type = new CTypePointer (type->BaseType ()->Duplicate ());
    if (quals) {
      type = new CTypeQualified (type, 
        quals->isConst (), quals->isVolatile (), quals->isRestrict ());
    }
    CTypeInfo::Destroy (tmp);
  }
  
  setSpecifiers (info, dsi);
  common_settings (info, ad);
  info->Name (csd.Name ()->Text ());
  info->TypeInfo (type);
  csd.Name ()->Object (info);
  csd.Name ()->setTypeRef (info->TypeInfo ());
  csd.Type ((CTypeInfo*)0);
  ad->Object (info);
  return ad;
}



CTree *CSemantic::introduce_function () {
  CFunctionInfo *info;
  CT_DeclSpecSeq *dss;
  CSemDeclSpecs *dsi;
  CStructure *scope;
  CT_Declarator *d;
  CObjectInfo *oi;
  CTypeInfo *type;
  const char *name;
  
  if (! current_fct)
    return (CTree*)0; // Fatal internal error
    
  dss = (CT_DeclSpecSeq*) (builder ().nodes () == 2 ? builder ().get_node (0) : 0);
  d = (CT_Declarator*) (dss ? builder ().get_node (1) : builder ().get_node (0));
  if (dss && ! in_decl ())
    return (CTree*)0;

  dsi = dss ? sem_decl_specs () : (CSemDeclSpecs*)0;
  CSemDeclarator csd (_err, dsi ? dsi->make_type () : &CTYPE_UNDEFINED, d);
  if (! csd.Name ())
    return (CTree*)0;
  
  type = csd.Type ();
  name = csd.Name ()->Text ();
  oi = lookup (name, NON_TAG, false);
  if (oi && (! oi->FunctionInfo () || ! typeMatch (type, oi->TypeInfo ())))
    return (CTree*)0;

  // re-use function prototype scope
/*DEBUG*/if (TRACE_FCT) std::cout<<"FCT ENT(2): "<<(void*)current_fct<<std::endl;
  reenter_scope (current_fct);

/*DEBUG*/if (TRACE_FCT) std::cout<<"FCT NIL(2): "<<(void*)current_fct<<std::endl;
  current_fct = 0;
  info = (CFunctionInfo*)current_scope;
  scope = (CStructure*)info->Parent ();

  if (! type->isFunction ())
    return (CTree*)0; // ill-formed
  type->VirtualType ()->TypeFunction ()->FunctionInfo (info);

  if (! scope->GlobalScope ()) {
    if (scope->insideFunction ())
      SEM_ERROR (d, "nested function `" << name << "'");
    return (CTree*)0;
  }
  
  if (dsi->declared (CT_PrimDeclSpec::PDS_TYPEDEF) ||
      dsi->declared (CT_PrimDeclSpec::PDS_REGISTER) ||
      dsi->declared (CT_PrimDeclSpec::PDS_AUTO)) {
    SEM_ERROR__wrong_storage_class (d, "function", csd.Name ());
    return (CTree*)0;
  }

  if (oi) {
    if (dsi->declared (CT_PrimDeclSpec::PDS_STATIC) && !oi->isStatic () &&
        !(oi->isExtern () && oi->isInline ())) {
      SEM_ERROR__static_follows_non_static (d, oi);
      return (CTree*)0;
    }
    else if (isRedefinedFunction (oi->FunctionInfo ())) {
      SEM_ERROR__redefinition (d, "function ", oi);
      return (CTree*)0;
    } else
      oi->NextObject (info);
  }
  
  if (dsi) 
    setSpecifiers (info, dsi);
  info->Name (csd.Name ()->Text ());
  info->SourceInfo ()->FileInfo (_file);
  info->ObjectInfo ()->TypeInfo (type);
  csd.Name ()->Object (info);
  csd.Name ()->setTypeRef (type);
  csd.Type ((CTypeInfo*)0);
  scope->addFunction (info);
  if (dss) decl_end ();
  return d;
}


bool CSemantic::isRedefinedFunction (CFunctionInfo* fct) const {
  return fct->isDefined ();
}


CTree *CSemantic::introduce_class () {
  CObjectInfo *info, *oi;
  CStructure *scope;
  CT_ClassDef *cs;
  
  cs = (CT_ClassDef*) builder ().class_head ();
  scope = findParent ();
  oi = lookup (cs->Name ()->Text (), scope, TAG, false);
  
  if (oi) {
    if (isRedefiningTypedef (oi, CTypeInfo::TYPE_CLASS)) {
      oi = oi->TypeInfo ()->TypeClass ()->ClassInfo ();
    } else if (oi->TypedefInfo ()) {
      oi = 0;
    }
  }
  if (oi) {
    if ((oi->ClassInfo () && oi->ClassInfo ()->isDefined ()) ||
        (oi->UnionInfo () && oi->UnionInfo ()->isDefined ())) {
      SEM_ERROR__redefinition (cs, 
        (oi->ClassInfo () ? "struct " : "union "), oi);
      builder ().destroyNode (cs);
      return (CTree*)0;
    } else if (! oi->Record () ||
      (cs->NodeName () == CT_ClassDef::NodeId () && ! oi->ClassInfo ()) ||
      (cs->NodeName () == CT_UnionDef::NodeId () && ! oi->UnionInfo ())) {
      SEM_ERROR__already_defined (cs, oi);
      builder ().destroyNode (cs);
      return (CTree*)0;
    }
  } 
  
  if (cs->NodeName () == CT_ClassDef::NodeId ()) {
    info = scope->newClass ();
    info->TypeInfo (new CTypeClass (info->ClassInfo ()));
  } else {
    info = scope->newUnion ();
    info->TypeInfo (new CTypeUnion (info->UnionInfo ()));
  }
  info->Name (cs->Name ()->Text ());
  if (scope != current_scope)
    info->AssignedScope (scope);
    
  // common settings
  enter_scope (info->Structure ()); 
  common_settings (info, cs);
  cs->Name ()->Object (info);
  cs->Name ()->setTypeRef (info->TypeInfo ());
  cs->Object (info);
  if (oi)
    oi->NextObject (info);

  return cs;
}



CTree *CSemantic::introduce_member () {
  CAttributeInfo *info;
  CSemDeclSpecs *dsi;
  CTree *d;

  if (! in_decl ()) 
    return (CTree*)0;
    
  d = builder ().member_declarator ();
  dsi = sem_decl_specs ();
  CSemDeclarator csd (_err, dsi->make_type (), d);
  
  if (! csd.Name ()) {
    builder ().destroyNode (d);
    return (CTree*)0;
  } else if (lookup (csd.Name ()->Text (), ATTRIB, false)) {
    SEM_ERROR__duplicate (d, "member", csd.Name ());
    builder ().destroyNode (d);
    return (CTree*)0;
  }

  info = current_scope->newAttribute ();
  info->Linkage (determine_linkage (dsi, ATTRIB));
  info->Storage (determine_storage_class (dsi, ATTRIB, info->Linkage ()));
  Push (info);
  
  setSpecifiers (info, dsi);
  common_settings (info, d, &csd);
  if (d->NodeName () == CT_InitDeclarator::NodeId ())
    ((CT_InitDeclarator*)d)->Object (info);
  else
    ((CT_BitFieldDeclarator*)d)->Object (info);
  return d;
}



CTree *CSemantic::introduce_tag () {
  CObjectInfo *info, *oi;
  CT_ClassSpec *tag;
  CStructure *scope;

  tag = (CT_ClassSpec*) builder ().elaborated_type_spec ();
  oi = lookup (tag->Name ()->Text (), TAG, true);
  
  // check for redefining typedef (typedef struct X X;)
  if (oi && oi->TypedefInfo ()) {
    CTypeInfo* t = oi->TypedefInfo ()->TypeInfo ();
    if (t->TypeRecord ())
      oi = t->TypeRecord ()->Record ();
    else if (t->TypeEnum ())
      oi = t->TypeEnum ()->EnumInfo ();
    if (oi && oi->Name () != DString(tag->Name ()->Text ()))
      oi = (CObjectInfo*)0; 
  }
  if (oi) {
    if ((tag->NodeName () == CT_ClassSpec::NodeId () && ! oi->ClassInfo ()) ||
        (tag->NodeName () == CT_UnionSpec::NodeId () && ! oi->UnionInfo ()) ||
        (tag->NodeName () == CT_EnumSpec::NodeId ()  && ! oi->EnumInfo ())) {
      SEM_ERROR (tag, "wrong use of `" << *tag->Name () << "'");
      return (CTree*)0;
    }
  }

  scope = findParent ();
  if (tag->NodeName () == CT_ClassSpec::NodeId ()) {
    info = scope->newClass ();
    info->TypeInfo (new CTypeClass (info->ClassInfo ()));
  } else if (tag->NodeName () == CT_UnionSpec::NodeId ()) {
    info = scope->newUnion ();
    info->TypeInfo (new CTypeUnion (info->UnionInfo ()));
  } else {
    info = scope->newEnum ();
    info->TypeInfo (new CTypeEnum (info->EnumInfo ()));
  }
  if (scope != current_scope)
    info->AssignedScope (scope);
  
  // common settings
  common_settings (info, tag);
  info->Name (tag->Name ()->end_token ()->dtext ());
  tag->Name ()->Object (info);
  tag->Name ()->setTypeRef (info->TypeInfo ());
  Push (info);
  if (oi)
    oi->NextObject (info);

  return tag;
}



CTree *CSemantic::identifier_list () {
  CT_ArgNameList *result;
  CT_SimpleName *arg;
  CFunctionInfo *fct;
  CObjectInfo *info;
  int num;

  fct = (CFunctionInfo*) current_scope;
  result = (CT_ArgNameList*) builder ().identifier_list ();
  num = result->Sons (); 
  
  for (int i = 0; i < num; i += 2) {
    arg = (CT_SimpleName*) result->Son (i);
    info = lookup (arg->Text (), NON_TAG, true);
    if (info && info->TypedefInfo ()) { 
      // typedef names shall not be redeclared as parameters
      // in an identifier list
      builder ().destroyNode (result);
      return (CTree*)0;
    } else if (fct->Argument (arg->Text ())) {
      SEM_ERROR__duplicate (result, "function parameter", arg);
      builder ().destroyNode (result);
      return (CTree*)0;
    } 
    info = fct->newArgument ();
    info->TypeInfo (&CTYPE_INT); // implicit int
    info->Name (arg->Text ());
    arg->Object (info);
    common_settings (info, arg);
    Push (info);
  }

  result->Scope (current_scope);
  return result;
}



CTree *CSemantic::declare_parameter () {
  CT_InitDeclarator *id;
  CSemDeclSpecs *dsi;
  CObjectInfo *info;
  CTypeInfo *type;
  bool error;
  
  if (! in_decl ()) 
    return (CTree*)0;

  id = (CT_InitDeclarator*) builder ().init_declarator1 ();
  dsi = sem_decl_specs ();
  CSemDeclarator csd (_err, dsi->make_type (), id);
  if (! csd.Type () || ! csd.Name () || ! current_scope->FunctionInfo ()) {
    builder ().destroyNode (id);
    return (CTree*)0;
  } 

  info = current_scope->FunctionInfo ()->Argument (csd.Name ()->Text ());
  error = true;
  if (! info) {
    SEM_ERROR (id, "declaration for unknown parameter `" 
      << csd.Name ()->Text () << "'");
  } else if (! info->Tree ()->IsSimpleName ()) {
    SEM_ERROR__redefinition (id, "function parameter ", info);
  } else if (id->Sons () == 2) {
    SEM_ERROR (id, "parameter `" << csd.Name ()->Text ()
      << "' is initialized");
  } else if (dsi->declared (CT_PrimDeclSpec::PDS_TYPEDEF) ||
             dsi->declared (CT_PrimDeclSpec::PDS_EXTERN) ||
             dsi->declared (CT_PrimDeclSpec::PDS_STATIC) ||
             dsi->declared (CT_PrimDeclSpec::PDS_THREAD) ||
             dsi->declared (CT_PrimDeclSpec::PDS_AUTO)) {
    SEM_ERROR__wrong_storage_class (id, "function parameter", csd.Name ());
  } else 
    error = false;
    
  if (error) {
    builder ().destroyNode (id);
    return (CTree*)0;
  }

  // adjust parameter type
  type = csd.Type ();
  if (type->isFunction ()) {
    type = new CTypePointer (type->Duplicate ());
  } else if (type->isArray ()) {
    CTypeQualified *quals = type->VirtualType ()->TypeArray ()->Qualifiers ();
    type = type->VirtualType ()->BaseType ();
    type = new CTypePointer (type->Duplicate ());
    if (quals) {
      type = new CTypeQualified (type, quals->isConst (), quals->isVolatile (), quals->isRestrict ());
    }
  }
  csd.Type (type);

  setSpecifiers (info, dsi);
  ((CT_SimpleName*)info->Tree ())->setTypeRef (type);
  common_settings (info, id, &csd);
  id->Object (info);
  return id;
}



bool CSemantic::empty_decl_spec_seq () { 
  CT_DeclSpecSeq *dss = new CT_DeclSpecSeq;
  decl_specs_begin (dss); 
  begin_decl ();
  builder ().Push (dss);
  return true;
}



bool CSemantic::decl_spec_seq () { 
  decl_specs_begin (new CT_DeclSpecSeq); 
  return true;
}



CTree *CSemantic::decl_spec_seq_err () { 
  builder ().destroyNode (decl_specs ());
  decl_specs_end (); 
  return (CTree*)0;
}



CTree *CSemantic::decl_spec_seq1 () {
  CT_PrimDeclSpec::Type type;
  CTree *cds, *ds;

  // check whether to parse an identifier in a declaration 
  // as type specifier or declarator identifier
  
  cds = builder ().decl_spec_seq1 ();
  if (decl_specs ()->Sons () &&                         // not first specifier?
      cds->NodeName () == CT_SimpleName::NodeId () &&   // identifier?
      lookup (cds->token ()->text (), TYPEDEF, true)) { // typedef name?

    for (int i = decl_specs ()->Sons () - 1; i >= 0; i--) {
      ds = decl_specs ()->Son (i);
      if (ds->NodeName () == CT_SimpleName::NodeId () ||
          ds->NodeName () == CT_ClassSpec::NodeId () ||
          ds->NodeName () == CT_ClassDef::NodeId () ||
          ds->NodeName () == CT_UnionSpec::NodeId () ||
          ds->NodeName () == CT_UnionDef::NodeId () ||
          ds->NodeName () == CT_EnumSpec::NodeId () ||
          ds->NodeName () == CT_EnumDef::NodeId ()) {
        return (CTree*)0;
      } 
      if (ds->NodeName () == CT_PrimDeclSpec::NodeId ()) {
        type = ((CT_PrimDeclSpec*)ds)->SpecType ();
        if (type == CT_PrimDeclSpec::PDS_CHAR || 
            type == CT_PrimDeclSpec::PDS_WCHAR_T || 
            type == CT_PrimDeclSpec::PDS_BOOL ||
            type == CT_PrimDeclSpec::PDS_C_BOOL ||
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
      } 
    }
  }

  decl_specs ()->AddSon (cds);
  return cds;
}



CTree *CSemantic::begin_decl () {
  CT_DeclSpecSeq *dss = decl_specs (); 
  decl_specs_end (); 
  decl_begin (new CSemDeclSpecs (_err, dss, support_implicit_int));
  return dss;
}



CTree *CSemantic::abst_declarator () {
  if (builder ().get_node (builder ().nodes () - 1)->NodeName () == 
      Builder::Container::NodeId ())
    builder ().Push (PrivateName ());
  return builder ().abst_declarator ();
}



CTree *CSemantic::direct_abst_declarator () {
  if (builder ().get_node (0)->NodeName () == Builder::Container::NodeId ())
    builder ().Push (PrivateName ());
  CTree *declarator = builder ().direct_abst_declarator ();

  // calculate delimiter of array declarator
  if (declarator && declarator->NodeName () == CT_ArrayDeclarator::NodeId ()) {
    CT_ArrayDelimiter *delim = ((CT_ArrayDeclarator*)declarator)->Delimiter ();
    if (delim && delim->Expr ()) {
      resolveExpr (delim->Expr (), delim);
    }
  }
  
  return declarator;
}



bool CSemantic::typeMatch (CTypeInfo *t1, CTypeInfo *t2) const {
  CTypeFunction *f1, *f2;
  if (t1->isFunction () && t2->isFunction ()) {
    f1 = t1->VirtualType ()->TypeFunction ();
    f2 = t2->VirtualType ()->TypeFunction ();
    // do not compare parameter types
    if (! (f1->ArgTypes ()->Entries () && f2->ArgTypes ()->Entries ()))
      return (*f1->ReturnType () == *f2->ReturnType ());
  }
  return *t1 == *t2;
}



CTree *CSemantic::array_delim () {
  CT_ArrayDelimiter *delim = (CT_ArrayDelimiter*) builder ().array_delim ();
  if (delim && delim->Expr ()) {
    resolveExpr (delim->Expr (), delim);
  }
  return delim;
}



CTypeInfo *CSemantic::resolveExpr (CTree *expr, CTree *base) const {
  if (expr) {
    CSemExpr cse (*_err, current_scope);
    return cse.resolveExpr (expr, base);
  }
  return (CTypeInfo*)0;
}



CTree *CSemantic::param_decl_clause () {
  CT_ArgDeclList *result;
  int nodes = builder ().nodes ();
  if (nodes) {
    CTree *pl = builder ().get_node (0);
    if (pl->NodeName () == Builder::Container::NodeId () &&
        (pl->Sons () % 2) == 0 && nodes != 2) 
      return (CTree*)0;  // trailing separator => ill-formed
  }
  result = (CT_ArgDeclList*) builder ().param_decl_clause ();
  result->Scope (current_scope);
  return result;
}



void CSemantic::begin_param_check () {
  in_arg_decl = true;
  is_type_name = false; 
}



bool CSemantic::finish_param_check (bool result) {
  // reset the state
  in_arg_decl = false; 
  is_type_name = false;
  
  // return if the rule wasn't parsed successfully anyway
  if (!result)
    return false;

  // if no ambiguity is possible here, return as well
  bool ambiguity = in_arg_decl && is_type_name; 
  if (! ambiguity)
    return true;

  // Handle the ambiguity between the declaration of a parameter 
  // of type pointer to function and the declaration of a parameter 
  // with redundant parentheses around the declarator-id. 
  // The resolution is to consider the type-name as a simple-type-
  // specifier rather than a declarator-id. [Example:
  // 
  //   class C { };
  //   void f(int(C));        // void f(int (*__fp)(C __parm));
  //                          // not: void f(int C);
  //   void g(int *(C[10]));  // void g(int *(*__fp)(C __parm[10]));
  //                          // not: void g(int *C[10]);
  // -end example]
  
  CTree *d = builder ().Top ();
  // if (! d) return d;
  
  while (d->NodeName () == CT_PtrDeclarator::NodeId () ||
         d->NodeName () == CT_RefDeclarator::NodeId () ||
         d->NodeName () == CT_MembPtrDeclarator::NodeId ())
    d = ((CT_Declarator*)d)->Declarator ();

  // check declarator => ( type-name ... )
  if (d->NodeName () == CT_BracedDeclarator::NodeId ()) {
    d = ((CT_Declarator*)d)->Declarator ();
    if (! (d->NodeName () == CT_SimpleName::NodeId () ||
           d->NodeName () == CT_QualName::NodeId () ||
           d->NodeName () == CT_RootQualName::NodeId ()))
      d = d->Son (0);
    if (d->NodeName () == CT_SimpleName::NodeId () ||
        d->NodeName () == CT_QualName::NodeId () ||
        d->NodeName () == CT_RootQualName::NodeId ()) {
      // delete the node and all allocated semantic objects
      undo (builder ().Top ());
      Builder::destroy (builder ().Top ());
      return false; // don't accept it => pointer to function
    }
  }
  return true;
}



CTree *CSemantic::finish_fct_def () {
  CT_FctDef *fd = (CT_FctDef*) builder ().fct_def ();
  common_settings (current_scope, fd);
  fd->Object (current_scope);
  return fd;
}



CTree *CSemantic::arg_decl_seq () {
  CT_ArgDeclSeq *result;
  CTypeList *argtypes;
  CTypeInfo *argtype;
  CFunctionInfo *fct;
  CObjectInfo *arg;
  unsigned num;
  
  fct = (CFunctionInfo*) current_scope;
  argtypes = fct->TypeInfo ()->ArgTypes ();

  num = fct->Arguments ();
  for (unsigned i = 0; i < num; i++) {
    arg = fct->Argument (i);
    argtype = arg->TypeInfo ();
    if (! argtype->is_undefined ()) {
      argtypes->AddEntry (argtype->Duplicate ());
    } else {
      argtypes->AddEntry (&CTYPE_UNDEFINED);
      SEM_ERROR (arg->Tree (), "missing declaration for parameter `" 
        << arg->Name () << "'");
    }
  }

  result = (CT_ArgDeclSeq*) builder ().arg_decl_seq ();
  result->Scope (current_scope);
  return result;
}



CTree *CSemantic::class_spec () {
  CT_ClassDef *result;

  result = (CT_ClassDef*) builder ().class_spec ();
  result->Members ()->Scope (current_scope);

  // anonymous struct or union declared within another struct or union
  // and no object is declared with it
  if (current_scope->isAnonymous () &&
      current_scope->Parent ()->isRecord () &&
      syntax ().look_ahead (TOK_SEMI_COLON)) {

    // introduce the members of the struct or union into the nearest
    // named parent struct or union, if it exists, otherwise into the
    // outermost parent struct or union

    // find the parent struct or union
    CStructure *targetScope = current_scope;
    while (targetScope->isAnonymous () && targetScope->Parent ()->isRecord ())
      targetScope = (CStructure*) targetScope->Parent ();

    // introduce the members into the target scope, check for each member
    // if there is already a member with that name in the target scope and
    // issue an error if so
    CMemberAliasInfo *alias;
    CObjectInfo *member;
    for (unsigned i = 0; i < current_scope->Attributes (); i++) {
      // next member, ignore anonymous members
      member = current_scope->Attribute (i);
      if (member->isAnonymous ())
        continue;

      // check for duplicates in target scope
      if (targetScope->Attribute (member->Name ())) {
        SEM_ERROR (member->Tree (), "duplicate member '" << member->Name () << "'");
        continue;
      }

      // create an alias for this member in the target scope
      alias = targetScope->newMemberAlias (member, true);
      alias->Name (member->Name ());
      alias->Member (member);
      alias->TypeInfo (member->TypeInfo ()->Duplicate ());
      alias->Linkage (member->Linkage ());
      alias->Storage (member->Storage ());
      alias->Specifiers (member->Specifiers ());
      common_settings (alias, member->Tree ());
      Push (alias);
    }
  }

  return result;
}


void CSemantic::reenter_class_scope () {
  CObjectInfo *info;
  CT_ClassDef *cd;

  cd = (CT_ClassDef*)builder ().get_node (0);
  info = cd->Object ();

  if (info && info->Record ())
    reenter_scope (info->Record ());
}


CTree *CSemantic::trans_unit () {
  CT_Program *result;
  result = (CT_Program*) builder ().trans_unit ();
  if (! result) 
    result = new CT_Program (1);
  result->Scope (_file);
  _file->Tree (result);
  return result;
}



CTree *CSemantic::cmpd_stmt () {
  CT_CmpdStmt *result;
  result = (CT_CmpdStmt*) builder ().cmpd_stmt ();
  result->Scope (current_scope);
  current_scope->Tree (result);
  return result;
}



CTree *CSemantic::select_stmt () {
  CTree *result;
  result = builder ().select_stmt ();
  if (result->NodeName () == CT_SwitchStmt::NodeId ())
    ((CT_SwitchStmt*) result)->Scope (current_scope);
  else if (result->NodeName () == CT_IfStmt::NodeId ())
    ((CT_IfStmt*) result)->Scope (current_scope);
  else 
    ((CT_IfElseStmt*) result)->Scope (current_scope);
  current_scope->Tree (result);
  return result;
}



CTree *CSemantic::iter_stmt () {
  CTree *result;
  result = builder ().iter_stmt ();
  if (result->NodeName () == CT_DoStmt::NodeId ())
    return result;
  if (result->NodeName () == CT_WhileStmt::NodeId ())
    ((CT_WhileStmt*) result)->Scope (current_scope);
  else 
    ((CT_ForStmt*) result)->Scope (current_scope);
  current_scope->Tree (result);
  return result;
}



CStructure *CSemantic::findParent () const {
  return (CStructure*) non_record_scopes.Top();
}



void CSemantic::setSpecifiers (CObjectInfo *info, CSemDeclSpecs *dsi) const {
  if (! info || ! dsi)
    return;
    
  if (dsi->declared (CT_PrimDeclSpec::PDS_VIRTUAL)) 
    info->isVirtual (true);
  if (dsi->declared (CT_PrimDeclSpec::PDS_STATIC)) 
    info->isStatic (true);
  if (dsi->declared (CT_PrimDeclSpec::PDS_EXTERN)) 
    info->isExtern (true);
  if (dsi->declared (CT_PrimDeclSpec::PDS_MUTABLE)) 
    info->isMutable (true);
  if (dsi->declared (CT_PrimDeclSpec::PDS_REGISTER)) 
    info->isRegister (true);
  if (dsi->declared (CT_PrimDeclSpec::PDS_EXPLICIT)) 
    info->isExplicit (true);
  if (dsi->declared (CT_PrimDeclSpec::PDS_INLINE)) 
    info->isInline (true);
  if (dsi->declared (CT_PrimDeclSpec::PDS_AUTO)) 
    info->isAuto (true);
  if (dsi->declared (CT_PrimDeclSpec::PDS_THREAD)) 
    info->isThreadLocal (true);
}



bool CSemantic::implicit_int () {
  if (! support_implicit_int || ! current_scope->isNamespace ())
    return false;
    
  CT_DeclSpecSeq *dss = new CT_DeclSpecSeq;
  CT_PrimDeclSpec *pds = new CT_PrimDeclSpec (CT_PrimDeclSpec::PDS_INT);
  dss->AddSon (pds);
  decl_specs_begin (dss); 
  begin_decl ();
  //builder ().Push (pds);
  builder ().Push (dss);
  return true;
}



CTree *CSemantic::init_declarator () {
  return builder ().init_declarator ();
}


bool CSemantic::isRedefiningTypedef (CObjectInfo* info, int obj_type) const {
  CTypedefInfo* tdef = info->TypedefInfo ();
  if (! tdef)
    return false;
  CTypeInfo* type = tdef->TypeInfo ();
  if (! type)
    return false;
  CObjectInfo* oinfo = 0;
  switch (obj_type) {
    case CTypeInfo::TYPE_CLASS: 
      oinfo = type->TypeClass () ? type->TypeClass ()->ClassInfo () : 0; 
      break;
    case CTypeInfo::TYPE_UNION: 
      oinfo = type->TypeUnion () ? type->TypeUnion ()->UnionInfo () : 0; 
      break;
    case CTypeInfo::TYPE_ENUM: 
      oinfo = type->TypeEnum () ? type->TypeEnum ()->EnumInfo () : 0; 
      break;
    default: 
      break;
  }
  if (oinfo && oinfo->Name () == tdef->Name ())
    return true;
  return false;
}


} // namespace Puma
