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

#include "Puma/CEnumeratorInfo.h"
#include "Puma/CTemplateParamInfo.h"
#include "Puma/CSemDatabase.h"
#include "Puma/CAttributeInfo.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CArgumentInfo.h"
#include "Puma/CAttributeInfo.h"
#include "Puma/CBaseClassInfo.h"
#include "Puma/CTypedefInfo.h"
#include "Puma/CTemplateInstance.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CObjectInfo.h"
#include "Puma/CScopeInfo.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CFileInfo.h"
#include "Puma/CEnumInfo.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CUsingInfo.h"
#include "Puma/CNamespaceInfo.h"
#include "Puma/CClassInstance.h"
#include "Puma/CFctInstance.h"
#include "Puma/CLabelInfo.h"
#include "Puma/CLocalScope.h"
#include "Puma/CMemberAliasInfo.h"
#include "Puma/CTemplateInstance.h"
#include "Puma/CUnionInstance.h"
#include "Puma/ErrorStream.h"
#include "Puma/CProject.h"
#include "Puma/Token.h"
#include "Puma/Unit.h"
#include <iostream>
#include <sstream>
#include <string.h>
#include <stdio.h>

namespace Puma {


CSemDatabase::~CSemDatabase () {
  ObjectSet::iterator i = _Objects.begin();
  for (; i != _Objects.end(); ++i) {
    CObjectInfo* o = *i;
    switch (o->Id()) {
      case CObjectInfo::FILE_INFO: delete (CFileInfo*)o; break;
      case CObjectInfo::UNION_INFO: delete (CUnionInfo*)o; break;
      case CObjectInfo::CLASS_INFO: delete (CClassInfo*)o; break;
      case CObjectInfo::BASECLASS_INFO: delete (CBaseClassInfo*)o; break;
      case CObjectInfo::MEMBERALIAS_INFO: delete (CMemberAliasInfo*)o; break;
      case CObjectInfo::ENUM_INFO: delete (CEnumInfo*)o; break;
      case CObjectInfo::TYPEDEF_INFO: delete (CTypedefInfo*)o; break;
      case CObjectInfo::FUNCTION_INFO: delete (CFunctionInfo*)o; break;
      case CObjectInfo::LABEL_INFO: delete (CLabelInfo*)o; break;
      case CObjectInfo::ENUMERATOR_INFO: delete (CEnumeratorInfo*)o; break;
      case CObjectInfo::ATTRIBUTE_INFO: delete (CAttributeInfo*)o; break;
      case CObjectInfo::TEMPLATE_PARAM_INFO: delete (CTemplateParamInfo*)o; break;
      case CObjectInfo::TEMPLATE_INFO: delete (CTemplateInfo*)o; break;
      case CObjectInfo::CLASS_INSTANCE_INFO: delete (CClassInstance*)o; break;
      case CObjectInfo::UNION_INSTANCE_INFO: delete (CUnionInstance*)o; break;
      case CObjectInfo::FCT_INSTANCE_INFO: delete (CFctInstance*)o; break;
      case CObjectInfo::ARGUMENT_INFO: delete (CArgumentInfo*)o; break;
      case CObjectInfo::LOCAL_INFO: delete (CLocalScope*)o; break;
      case CObjectInfo::NAMESPACE_INFO: delete (CNamespaceInfo*)o; break;
      case CObjectInfo::USING_INFO: delete (CUsingInfo*)o; break;
      default: break;
    }
  }
}

void CSemDatabase::Insert (CObjectInfo *info) {
  if (! info)
    return;

  _Objects.insert (info);
  info->SemDB (this);

  if (info->Name ()) {
    if (info->ClassInfo ())
      _Classes.append (info);
    else if (info->UnionInfo ())
      _Unions.append (info);
    else if (info->EnumInfo ())
      _Enums.append (info);
    else if (info->TypedefInfo ())
      _Typedefs.append (info);
    else if (info->FunctionInfo ())
      _Functions.append (info);
    else if (info->FileInfo ())
      _Files.append (info);
  }
}

void CSemDatabase::Remove (CObjectInfo *info) {
  if (! info)
    return;

  _Objects.erase (info);

  if (info->ClassInfo ()) {
    for (long i = ClassInfos (); i > 0; i--)
      if (_Classes.fetch (i-1) == info)
        _Classes.remove (i-1);
  } else if (info->UnionInfo ()) {
    for (long i = UnionInfos (); i > 0; i--)
      if (_Unions.fetch (i-1) == info)
        _Unions.remove (i-1);
  } else if (info->EnumInfo ()) {
    for (long i = EnumInfos (); i > 0; i--)
      if (_Enums.fetch (i-1) == info)
        _Enums.remove (i-1);
  } else if (info->TypedefInfo ()) {
    for (long i = TypedefInfos (); i > 0; i--)
      if (_Typedefs.fetch (i-1) == info)
        _Typedefs.remove (i-1);
  } else if (info->FunctionInfo ()) {
    for (long i = FunctionInfos (); i > 0; i--)
      if (_Functions.fetch (i-1) == info)
        _Functions.remove (i-1);
  } else if (info->FileInfo ()) {
    for (long i = FileInfos (); i > 0; i--)
      if (_Files.fetch (i-1) == info)
        _Files.remove (i-1);
  }

  return; // Unknown!
}

CFunctionInfo *CSemDatabase::BuiltinOperator (const char *name, int tok,
  CTypeInfo *rtype, CTypeInfo *t0, CTypeInfo *t1) {
  CFunctionInfo *info;
  CTypeList *args;
  CTypeInfo *type;

  // first check if this built-in operator already exists ...
  std::multimap<int,CFunctionInfo*>::iterator iter, lower, upper;
  lower = _builtin_ops.lower_bound (tok);
  upper = _builtin_ops.upper_bound (tok);
  // check all exisiting operators with the right token type
  for (iter = lower; iter != upper; ++iter) {
    info = iter->second;
    args = info->TypeInfo ()->ArgTypes ();
    // same number of arguments?
    if (args->Entries () == (unsigned)((t0?1:0) + (t1?1:0))) {
      // same parameter type list
      if ((t0 ? *t0 == *args->Entry (0) : true) &&
          (t1 ? *t1 == *args->Entry (1) : true)) {
        if (t0) CTypeInfo::Destroy (t0);
        if (t1) CTypeInfo::Destroy (t1);
        if (rtype) CTypeInfo::Destroy (rtype);
        return info; // operator already exists
      }
    }
  }

  // the operator object has to be created ...
    // return type
  if (! rtype)
    rtype = &CTYPE_UNDEFINED;

  // parameter type list
  args = new CTypeList ((t0?1:0)+(t1?1:0));
  if (t0) args->AddEntry (t0);
  if (t1) args->AddEntry (t1);

  // operator function type
  type = new CTypeFunction (rtype, args, true);

  // operator function
  info = newFunction();
  info->Name (name);
  info->isOperator (true);
  info->ObjectInfo ()->TypeInfo (type);
  type->VirtualType ()->TypeFunction ()->FunctionInfo (info);

  // create function parameters
  if (t0) CreateParameter (info, t0);
  if (t1) CreateParameter (info, t1);

  // insert the object into the map
  _builtin_ops.insert (std::multimap<int,CFunctionInfo*>::value_type (tok, info));

  return info;
}

void CSemDatabase::CreateParameter (CFunctionInfo *fi, CTypeInfo *type) const {
  CArgumentInfo *info;
  info = fi->newArgument ();
  info->Name ("<noname>");
  info->Storage (CStorage::CLASS_AUTOMATIC);
  info->TypeInfo (type->Duplicate ());
}

CObjectInfo *CSemDatabase::ObjectInfo (Token *token) const {
  ObjectSet::const_iterator i = _Objects.begin();
  for (; i != _Objects.end(); ++i) {
    CObjectInfo *info = (CObjectInfo*)*i;
    if (! info->FileInfo ())
      if (*info->SourceInfo () == token)
        return info;
  }
  return (CObjectInfo*)0;
}

CObjectInfo *CSemDatabase::ObjectInfo (CT_Token *token) const {
  ObjectSet::const_iterator i = _Objects.begin();
  for (; i != _Objects.end(); ++i) {
    CObjectInfo *info = (CObjectInfo*)*i;
    if (! info->FileInfo ())
      if (*info->SourceInfo () == token)
        return info;
  }
  return (CObjectInfo*)0;
}

CObjectInfo *CSemDatabase::ObjectInfo (unsigned i) const {
  ObjectSet::const_iterator o = _Objects.begin();
  for (unsigned j = 0; j != i && o != _Objects.end(); ++j, ++o);
  return (o != _Objects.end()) ? (CObjectInfo*)*o : (CObjectInfo*)0;
}


void CSemDatabase::Dump (std::ostream &out, int level, bool dump_builtins) const {
  unsigned files = FileInfos ();
  CStructure *file;
  int count = 0;

  for (unsigned i = 0; i < files; i++) {
    file = FileInfo (i);
    out << "file " << count++ << " : " << file->Name () << std::endl;
    Dump (out, file, 0, level, dump_builtins);
  }

  if (dump_builtins) {
    out << "built-in operators" << std::endl;
    std::multimap<int,CFunctionInfo*>::const_iterator iter;
    for (iter = _builtin_ops.begin(); iter != _builtin_ops.end(); ++iter) {
      DumpFunction (out, (CFunctionInfo*) iter->second, 0, level);
    }
  }
}

void CSemDatabase::Dump (std::ostream &out, CStructure *scope, int ind, int level, bool dump_builtins) const {
  unsigned num;

  if (! scope || (ind / 4) > level)
    return;

  // Template parameter (print on template scope only)
  num = scope->TemplateParams ();
  if (num && scope->TemplateInfo ()) {
    indent (out, ind+2); out << num << " template parameters:" << std::endl;
    for (unsigned j = 0; j < num; j++) {
      CTemplateParamInfo* param = scope->TemplateParam (j);
      if (param && (dump_builtins || ! param->isBuiltin ()))
        DumpTemplateParam (out, param, ind);
    }
  }

  // Template instances (print on template scope only)
  if (scope->TemplateInfo ()) {
    num = scope->TemplateInfo ()->Instances ();
    indent (out, ind+2); out << num << " template instances:" << std::endl;
    for (unsigned j = 0; j < num; j++) {
      CObjectInfo* inst = scope->TemplateInfo ()->Instance (j);
      if (inst)
        DumpTemplateInstance (out, inst, ind);
    }
  }

  // Types
  num = scope->Types ();
  if (num) {
    indent (out, ind+2); out << num << " types:" << std::endl;
    for (unsigned j = 0; j < num; j++) {
      CObjectInfo* type = scope->Type (j);
      if (type && (dump_builtins || ! type->isBuiltin ()))
        DumpType (out, type, ind);
    }
  }

  // Namespaces
  num = scope->Namespaces ();
  if (num) {
    indent (out, ind+2); out << num << " namespaces:" << std::endl;
    for (unsigned j = 0; j < num; j++)  {
      CNamespaceInfo* ns = scope->Namespace (j);
      if (ns && (dump_builtins || ! ns->isBuiltin () || (ns->Name () && *ns->Name ().c_str() == '%')))
        DumpNamespace (out, ns, ind);
    }
  }

  // Usings
  num = scope->Usings ();
  if (num) {
    indent (out, ind+2); out << num << " usings:" << std::endl;
    for (unsigned j = 0; j < num; j++) {
      CUsingInfo* ui = scope->Using (j);
      if (ui && (dump_builtins || ! ui->isBuiltin ()))
        DumpUsing (out, ui, ind);
    }
  }

  // Attributes
  num = scope->Attributes ();
  if (num) {
    indent (out, ind+2); out << num << " attributes: " << std::endl;
    for (unsigned j = 0; j < num; j++) {
      CAttributeInfo* attr = scope->Attribute (j);
      if (attr && (dump_builtins || ! attr->isBuiltin ()))
        DumpAttribute (out, attr, ind);
    }
  }

  // Functions
  num = scope->Functions ();
  if (num) {
    indent (out, ind+2); out << num << " functions:" << std::endl;
    for (unsigned j = 0; j < num; j++) {
      CFunctionInfo* fct = scope->Function (j);
      if (fct && (dump_builtins || ! fct->isBuiltin ()))
        DumpFunction (out, fct, ind, level);
    }
  }

  // Local scopes
  num = scope->Children ();
  if (num) {
    indent (out, ind+2); out << num << " scopes:" << std::endl;
    for (unsigned j = 0; j < num; j++) {
      CStructure *ls = scope->Child (j)->Structure ();
      if (ls && (dump_builtins || ! ls->isBuiltin () ||
          (ls->NamespaceInfo () && ls->Name () && *ls->Name ().c_str() == '%'))) {
        DumpLocalScope (out, ls, ind);
        Dump (out, ls, ind+4, level, dump_builtins);
      }
    }
  }
}

void CSemDatabase::DumpLocalScope (std::ostream &out, CObjectInfo *info, int ind) const {
  indent (out, ind+4);
  if (info->Record () || info->NamespaceInfo () ||
      info->FunctionInfo () || info->TemplateInfo ()) {
    if (info->UnionInfo ())
      out << "union";
    else if (info->ClassInfo ())
      out << (info->ClassInfo ()->isStruct () ? "struct" : "class");
    else if (info->NamespaceInfo ())
      out << "namespace";
    else if (info->FunctionInfo ())
      out << "function";
    else if (info->TemplateInfo ())
      out << "template";
    out << " : " << info->Name ();
  } else
    out << "block";
  out << "\t";
  if (info->ClassInfo ()) {
    unsigned base_classes = info->ClassInfo ()->BaseClasses ();
    if (base_classes) {
      out << "[base classes: ";
      for (unsigned i = 0; i < base_classes; i++) {
        CClassInfo *cinfo = info->ClassInfo ()->BaseClass (i)->Class ();
        if (cinfo && cinfo->TypeInfo ()) {
          cinfo->TypeInfo ()->TypeText (out);
          if (i+1 < base_classes)
            out << ",";
        }
      }
      out << "] ";
    }
  }
  DumpQualities (out, info);
  out << std::endl;
}

void CSemDatabase::DumpFunction (std::ostream &out, CFunctionInfo *info, int ind, int level) const {
  indent (out, ind+4);
  out << info->Name () << "\t (type: ";
  info->TypeInfo ()->TypeText (out);
  out << ") ";

  if (info->ObjectInfo()->QualifiedScope () && info->ObjectInfo()->QualifiedScope () != info->Scope ()) {
    out << " [member of ";
    DumpScopeName (out, info->ObjectInfo()->QualifiedScope ());
    out << "]";
  }
  DumpFriends (out, info);
  if (info->isDestructor ())
    out << " [destructor]";
  else if (info->isConstructor ())
    out << " [constructor]";
  if (! info->isDefined () && ! info->isBuiltin ())
    out << " [undefined]";
  if (info->isOperator () || info->isConversion ())
    out << " [operator]";

  DumpQualities (out, info);
  out << std::endl;

  if (level == 0)
    return;
}

void CSemDatabase::DumpFriends (std::ostream &out, CStructure *info) const {
  CStructure* sinfo = info;
  do {
    unsigned friends = sinfo->Friends ();
    if (friends) {
      out << " [friend of ";
      for (unsigned i = 0; i < friends; i++) {
        CRecord *friend_class = sinfo->Friend (i);
        if (friend_class && friend_class->TypeInfo ()) {
          DumpScopeName (out, friend_class);
          if (i+1 < friends)
            out << ",";
        }
      }
      out << "]";
    }
    sinfo = sinfo->NextObject () ? sinfo->NextObject ()->Structure () : 0;
  } while (sinfo && sinfo != info);
}

void CSemDatabase::DumpAttribute (std::ostream &out, CAttributeInfo *info, int ind) const {
  indent (out, ind+4);
  out << info->Name () << "\t (type: ";
  info->TypeInfo ()->TypeText (out);
  out << ") ";

  if (info->ObjectInfo()->QualifiedScope () && info->ObjectInfo()->QualifiedScope () != info->Scope ()) {
    out << " [member of ";
    DumpScopeName (out, info->ObjectInfo()->QualifiedScope ());
    out << "]";
  }
  if (info->TypeInfo ()->isVarArray ())
    out << " [dim: variable]";
  else if ((info->TypeInfo ()->isArray () || info->TypeInfo ()->isBitField ()) &&
           info->TypeInfo ()->Dimension () != -1)
    out << " [dim: " << info->TypeInfo ()->Dimension () << "]";
  else if (info->EnumeratorInfo ())
    out << " [value: " << info->EnumeratorInfo ()->Value () << "]";

  DumpQualities (out, info);
  out << std::endl;
}

void CSemDatabase::DumpType (std::ostream &out, CObjectInfo *info, int ind) const {
  indent (out, ind+4);
  if (info->TypedefInfo ())
    out << "typedef ";
  else if (info->UnionInfo ())
    out << "union ";
  else if (info->EnumInfo ())
    out << "enum ";
  else if (info->ClassInfo ())
    out << (info->ClassInfo ()->isStruct () ? "struct " : "class ");
  out << ": " << info->Name () << "\t";
  if (info->EnumInfo ()) {
    out << " (underlying type: ";
    info->EnumInfo ()->TypeInfo ()->UnderlyingType ()->TypeText (out);
    out << ") ";
  } else if (info->TypedefInfo ()) {
    out << " (type: ";
    info->TypeInfo ()->TypeText (out);
    out << ") ";
  } else if (info->ClassInfo ()) {
    unsigned base_classes = info->ClassInfo ()->BaseClasses ();
    if (base_classes) {
      out << "[base classes: ";
      for (unsigned i = 0; i < base_classes; i++) {
        CClassInfo *cinfo = info->ClassInfo ()->BaseClass (i)->Class ();
        if (cinfo && cinfo->TypeInfo ()) {
          cinfo->TypeInfo ()->TypeText (out);
          if (i+1 < base_classes)
            out << ",";
        }
      }
      out << "]";
    }
    DumpFriends (out, info->ClassInfo ());
  }
  DumpQualities (out, info);
  out << std::endl;
}

void CSemDatabase::DumpTemplateParam (std::ostream &out, CTemplateParamInfo *info, int ind) const {
  indent (out, ind+4);
  out << info->Name () << "\t";
  if (info->isTemplate ())
    out << " [template] ";
  else if (info->isTypeParam ())
    out << " [type] ";
  else
    out << " [non-type] ";
  out << std::endl;
}

void CSemDatabase::DumpTemplateInstance (std::ostream &out, CObjectInfo *info, int ind) const {
  indent (out, ind+4);
  out << info->Name ();
  CTemplateInstance *instance = info->TemplateInstance ();
  out << "<";
  std::ostringstream os;
  for (unsigned i = 0; i < instance->InstantiationArgs (); i++) {
    DeducedArgument *arg = instance->InstantiationArg (i);
    if (arg->isDefaultArg ())
      break;
    os << (i ? "," : "") << *arg;
  }
  std::string args = os.str ();
  out << args;
  if (args.size () && args.at (args.size () - 1) == '>')
    out << " ";
  out << ">\t";
  if (instance->isPseudoInstance ())
    out << " [pseudo] ";
  if (instance->isInstantiated ())
    out << " [instantiated] ";
  else if (instance->canInstantiate ())
    out << " [instantiable] ";
  out << " [" << (void*)info << "]";
  out << std::endl;
}

void CSemDatabase::DumpNamespace (std::ostream &out, CNamespaceInfo *info, int ind) const {
  CNamespaceInfo *nsinfo;
  indent (out, ind+4);
  out << "namespace : " << info->Name () << "\t";
  if (info->isAlias ()) {
    nsinfo = info;
    while (nsinfo->isAlias ()) {
      nsinfo = (CNamespaceInfo*)nsinfo->NextObject ();
      if (nsinfo == info) // should never be true
        break;
    }
    out << " [alias of ";
    DumpScopeName (out, nsinfo);
    out << "]";
  }
  DumpQualities (out, info);
  out << std::endl;
}

void CSemDatabase::DumpUsing (std::ostream &out, CUsingInfo *info, int ind) const {
  CNamespaceInfo *nsinfo = info->Namespace ();
  indent (out, ind+4);
  out << "using namespace " << ": ";
  DumpScopeName (out, nsinfo);
  out << std::endl;
}

void CSemDatabase::DumpQualities (std::ostream &out, CObjectInfo *info) const {
  if (info->isTemplate ())
    out << " [template]";
  if (info->TemplateInfo () && ! info->TemplateInfo ()->isBaseTemplate ())
    out << " [specialization of " << (void*)info->TemplateInfo ()->BaseTemplate () << "]";
  if (info->NamespaceInfo () && info->Name () && *info->Name ().c_str() == '%')
    out << " [instance scope]";
  if (info->isTemplateInstance ()) {
    if (info->TemplateInstance ()->isPseudoInstance ())
      out << " [pseudo instance]";
    else
      out << " [instance of "
          << (void*)info->TemplateInstance ()->Template () << "]";
  }

  // dump member access level
  if (info->Protection () == CProtection::PROT_PROTECTED)
    out << " [protected]";
  else if (info->Protection () == CProtection::PROT_PRIVATE)
    out << " [private]";
//  else if (info->Protection () == CProtection::PROT_PUBLIC)
//    out << " [public]";

  // dump linkage
  if (! (info->Language () == CLanguage::LANG_UNDEFINED))
    out << " [language: " << info->Language ().Text () << "]";

  // dump type qualities
  if (info->TypeInfo () && info->TypeInfo ()->isVolatile ())
    out << " [volatile]";
  if (info->TypeInfo () && info->TypeInfo ()->isConst ())
    out << " [const]";

  if (info->isBuiltin ())
    out << " [built-in]";
  if (info->isInline ())
    out << " [inline]";
  if (info->isAnonymous ())
    out << " [anonymous]";

  if (info->isVirtual ())
    out << " [virtual]";
  if (info->isStatic ())
    out << " [static]";
  if (info->isExtern () || info->Linkage () == CLinkage::LINK_EXTERNAL)
    out << " [extern]";
  if (info->isMutable ())
    out << " [mutable]";
  if (info->isRegister ())
    out << " [register]";

  // dump storage class
  if (info->Storage () == CStorage::CLASS_DYNAMIC)
    out << " [dynamic]";
  else if (info->Storage () == CStorage::CLASS_THREAD)
    out << " [thread-local]";
//  else if (info->Storage () == CStorage::CLASS_STATIC)
//    out << " [static]";
//  else if (info->Storage () == CStorage::CLASS_AUTOMATIC)
//    out << " [automatic]";

  // dump address
  out << " [" << (void*)info << "]";
  // dump next linked object
  if (info->NextObject () != info)
    out << " [next: " << (void*)info->NextObject () << "]";
}

void CSemDatabase::DumpScopeName (std::ostream &out, CStructure *scope) const {
  if (scope->FileInfo ())
    return;

  CStructure *parent = scope->Parent ()->Structure ();
  if (parent->isRecord () || parent->isNamespace ()) {
    DumpScopeName (out, parent);
    out << "::" << scope->Name ();
  } else
    out << scope->Name ();
}

void CSemDatabase::indent (std::ostream &out, int ind) const {
  for (int i = 0; i < ind; i++)
    out << " ";
}

CArgumentInfo *CSemDatabase::newArgument () {
  CArgumentInfo *info = new CArgumentInfo();
  Insert (info);
  return info;
}

CAttributeInfo *CSemDatabase::newAttribute () {
  CAttributeInfo *info = new CAttributeInfo();
  Insert (info);
  return info;
}

CBaseClassInfo *CSemDatabase::newBaseClass () {
  CBaseClassInfo *info = new CBaseClassInfo();
  Insert (info);
  return info;
}

CClassInfo *CSemDatabase::newClass () {
  CClassInfo *info = new CClassInfo();
  Insert (info);
  return info;
}

CClassInstance *CSemDatabase::newClassInstance () {
  CClassInstance *info = new CClassInstance();
  Insert (info);
  return info;
}

CEnumInfo *CSemDatabase::newEnum () {
  CEnumInfo *info = new CEnumInfo();
  Insert (info);
  return info;
}

CEnumeratorInfo *CSemDatabase::newEnumerator () {
  CEnumeratorInfo *info = new CEnumeratorInfo();
  Insert (info);
  return info;
}

CFunctionInfo *CSemDatabase::newFunction () {
  CFunctionInfo *info = new CFunctionInfo();
  Insert (info);
  return info;
}

CFctInstance *CSemDatabase::newFctInstance () {
  CFctInstance *info = new CFctInstance();
  Insert (info);
  return info;
}

CLabelInfo *CSemDatabase::newLabel () {
  CLabelInfo *info = new CLabelInfo();
  Insert (info);
  return info;
}

CLocalScope *CSemDatabase::newLocalScope () {
  CLocalScope *info = new CLocalScope();
  Insert (info);
  return info;
}

CMemberAliasInfo *CSemDatabase::newMemberAlias () {
  CMemberAliasInfo *info = new CMemberAliasInfo();
  Insert (info);
  return info;
}

CNamespaceInfo *CSemDatabase::newNamespace () {
  CNamespaceInfo *info = new CNamespaceInfo();
  Insert (info);
  return info;
}

CTemplateInfo *CSemDatabase::newTemplate () {
  CTemplateInfo *info = new CTemplateInfo();
  Insert (info);
  return info;
}

CTemplateParamInfo *CSemDatabase::newTemplateParam () {
  CTemplateParamInfo *info = new CTemplateParamInfo();
  Insert (info);
  return info;
}

CTypedefInfo *CSemDatabase::newTypedef () {
  CTypedefInfo *info = new CTypedefInfo();
  Insert (info);
  return info;
}

CUnionInfo *CSemDatabase::newUnion () {
  CUnionInfo *info = new CUnionInfo();
  Insert (info);
  return info;
}

CUnionInstance *CSemDatabase::newUnionInstance () {
  CUnionInstance *info = new CUnionInstance();
  Insert (info);
  return info;
}

CUsingInfo *CSemDatabase::newUsing () {
  CUsingInfo *info = new CUsingInfo();
  Insert (info);
  return info;
}

CFileInfo *CSemDatabase::newFile () {
  CFileInfo *info = new CFileInfo();
  Insert (info);
  return info;
}

} // namespace Puma
