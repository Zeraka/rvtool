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

#include "Puma/CTypeInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CEnumInfo.h"
#include "Puma/CConstant.h"
#include "Puma/CFileInfo.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CSemDatabase.h"
#include "Puma/CClassInfo.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CClassInstance.h"
#include "Puma/CUnionInstance.h"
#include "Puma/CFctInstance.h"
#include "Puma/CObjectInfo.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CArgumentInfo.h"
#include "Puma/CAttributeInfo.h"
#include "Puma/DeducedArgument.h"
#include "Puma/CEnumeratorInfo.h"
#include "Puma/CTemplateParamInfo.h"
#include "Puma/Limits.h"
#include "Puma/CTree.h"
#include <iostream>
#include <sstream>
#include <string.h>

namespace Puma {


// The empty type ///////////////////////////////////////////////

CTypeInfo CTYPE_EMPTY ((CTypeInfo*)0, CTypeInfo::TYPE_EMPTY);

// Basic types //////////////////////////////////////////////////

CTypePrimitive CTYPE_BOOL ("bool", CTypeInfo::TYPE_BOOL, BOOL_SIZE);
CTypePrimitive CTYPE_C_BOOL ("_Bool", CTypeInfo::TYPE_C_BOOL, BOOL_SIZE);
CTypePrimitive CTYPE_CHAR ("char", CTypeInfo::TYPE_CHAR, CHAR_SIZE);
CTypePrimitive CTYPE_SIGNED_CHAR ("signed char", CTypeInfo::TYPE_SIGNED_CHAR, SCHAR_SIZE);
CTypePrimitive CTYPE_UNSIGNED_CHAR ("unsigned char", CTypeInfo::TYPE_UNSIGNED_CHAR, UCHAR_SIZE);
CTypePrimitive CTYPE_WCHAR_T ("wchar_t", CTypeInfo::TYPE_WCHAR_T, WCHAR_T_SIZE);
CTypePrimitive CTYPE_SHORT ("short int", CTypeInfo::TYPE_SHORT, SHORT_SIZE);
CTypePrimitive CTYPE_UNSIGNED_SHORT ("unsigned short int", CTypeInfo::TYPE_UNSIGNED_SHORT, USHORT_SIZE);
CTypePrimitive CTYPE_INT ("int", CTypeInfo::TYPE_INT, INT_SIZE);
CTypePrimitive CTYPE_UNSIGNED_INT ("unsigned int", CTypeInfo::TYPE_UNSIGNED_INT, UINT_SIZE);
CTypePrimitive CTYPE_LONG ("long int", CTypeInfo::TYPE_LONG, LONG_SIZE);
CTypePrimitive CTYPE_UNSIGNED_LONG ("unsigned long int", CTypeInfo::TYPE_UNSIGNED_LONG, ULONG_SIZE);
CTypePrimitive CTYPE_LONG_LONG ("long long int", CTypeInfo::TYPE_LONG_LONG, LLONG_SIZE);
CTypePrimitive CTYPE_UNSIGNED_LONG_LONG ("unsigned long long int", CTypeInfo::TYPE_UNSIGNED_LONG_LONG, ULLONG_SIZE);
CTypePrimitive CTYPE_INT128 ("__int128", CTypeInfo::TYPE_INT128, INT128_SIZE);
CTypePrimitive CTYPE_UNSIGNED_INT128 ("unsigned __int128", CTypeInfo::TYPE_UNSIGNED_INT128, UINT128_SIZE);
CTypePrimitive CTYPE_FLOAT ("float", CTypeInfo::TYPE_FLOAT, FLOAT_SIZE);
CTypePrimitive CTYPE_DOUBLE ("double", CTypeInfo::TYPE_DOUBLE, DOUBLE_SIZE);
CTypePrimitive CTYPE_LONG_DOUBLE ("long double", CTypeInfo::TYPE_LONG_DOUBLE, LDOUBLE_SIZE);
CTypePrimitive CTYPE_VOID ("void", CTypeInfo::TYPE_VOID, 0);
CTypePrimitive CTYPE_UNKNOWN_T ("__unknown_t", CTypeInfo::TYPE_UNKNOWN_T, 0);
CTypePrimitive CTYPE_UNDEFINED ("<undefined>", CTypeInfo::TYPE_UNDEFINED, 0);
CTypePrimitive CTYPE_ELLIPSIS ("...", CTypeInfo::TYPE_ELLIPSIS, 0);

// Internal representation of size_t and ptrdiff_t types ////////

CTypeInfo *CTypeInfo::CTYPE_SIZE_T = &CTYPE_UNSIGNED_INT;  // default type
CTypeInfo *CTypeInfo::CTYPE_PTRDIFF_T = &CTYPE_INT;        // default type

bool CTypeInfo::equals(const CTypeInfo &type, bool matchTemplateParams, bool noDependentFctParams) const {
  if (! (_Id == type._Id)) {
    return false;
  } else if (TypeEmpty () || TypePrimitive ()) {
    return true;
  } else if (TypeTemplateParam ()) {
    CTemplateParamInfo *p1 = TypeTemplateParam ()->TemplateParamInfo ();
    CTemplateParamInfo *p2 = type.TypeTemplateParam ()->TemplateParamInfo ();
    return p1 && p2 && (matchTemplateParams ? p1->match(*p2) : *p1 == *p2);
  } else if (TypeClass ()) {
    CClassInfo *c1 = TypeClass ()->ClassInfo ();
    CClassInfo *c2 = type.TypeClass ()->ClassInfo ();
    if (! c1 || ! c2) {
      return false;
    } else if (c1->isTemplate () && c2->isTemplate ()) {
      CTemplateInfo* t1 = c1->TemplateInfo ();
      CTemplateInfo* t2 = c2->TemplateInfo ();
      if (t1 && t2 && ! CTemplateInfo::equalParameters (t1, t2))
        return false;
    } else if (isTemplateInstance () && type.isTemplateInstance ()) {
      CTemplateInstance *t1 = c1->ClassInstance ()->TemplateInstance ();
      CTemplateInstance *t2 = c2->ClassInstance ()->TemplateInstance ();
      if (t1 && t2 && t1->isPseudoInstance () && t2->isPseudoInstance ()) {
        // compare template arguments
        if (! CTemplateInfo::equalArguments (t1, t2, matchTemplateParams))
          return false;
        c1 = t1->Template ()->ObjectInfo ()->ClassInfo ();
        c2 = t2->Template ()->ObjectInfo ()->ClassInfo ();
      }
    }
    return (*c1 == *c2);
  } else if (TypeUnion ()) {
    CUnionInfo *u1 = TypeUnion ()->UnionInfo ();
    CUnionInfo *u2 = type.TypeUnion ()->UnionInfo ();
    return (u1 && u2 && *u1 == *u2);
  } else if (TypeEnum ()) {
    CEnumInfo *e1 = TypeEnum ()->EnumInfo ();
    CEnumInfo *e2 = type.TypeEnum ()->EnumInfo ();
    return (e1 && e2 && *e1 == *e2);
  } else if (TypeArray ()) {
    if (! TypeVarArray () && ! TypeArray ()->DepDim () && ! type.TypeArray ()->DepDim ())
      if (TypeArray ()->hasDimension () && type.TypeArray ()->hasDimension () && Dimension () != type.Dimension ())
        return false;
  } else if (TypeQualified ()) {
    if (TypeQualified ()->isConst () != type.TypeQualified ()->isConst ())
      return false;
    if (TypeQualified ()->isVolatile () != type.TypeQualified ()->isVolatile ())
      return false;
  } else if (TypeMemberPointer ()) {
    if (TypeMemberPointer ()->Record () &&
        *TypeMemberPointer ()->Record () != *type.TypeMemberPointer ()->Record ())
      return false;
  } else if (TypeFunction ()) {
    CTypeList *atl = TypeFunction ()->ArgTypes ();
    CTypeList *tatl = type.TypeFunction ()->ArgTypes ();
    if (atl && tatl && atl->Entries () != tatl->Entries ())
      return false;
    for (unsigned i = 0; i < atl->Entries (); i++)
      if (! atl->Entry (i)->equals(*tatl->Entry (i), matchTemplateParams, noDependentFctParams) ||
          (noDependentFctParams && (atl->Entry (i)->isDependent () || tatl->Entry (i)->isDependent ())))
        return false;
    if ((TypeFunction ()->isConst () != type.TypeFunction ()->isConst ()) ||
        (TypeFunction ()->isVolatile () != type.TypeFunction ()->isVolatile ()))
      return false;
    if (TypeFunction ()->Record () && type.TypeFunction ()->Record ())
      if (*TypeFunction ()->Record () != *type.TypeFunction ()->Record ())
        return false;
    CFunctionInfo* f1 = TypeFunction ()->FunctionInfo ();
    CFunctionInfo* f2 = type.TypeFunction ()->FunctionInfo ();
    if (f1 && f2 && f1->isTemplate () && f2->isTemplate ()) {
      CTemplateInfo* t1 = f1->TemplateInfo ();
      CTemplateInfo* t2 = f2->TemplateInfo ();
      if (t1 && t2 && ! CTemplateInfo::equalParameters (t1, t2))
        return false;
    }
  }
  return BaseType ()->equals(*type.BaseType (), matchTemplateParams, noDependentFctParams);
}

bool CTypeInfo::equalsUnqualified (const CTypeInfo &type) const {
  CTypeInfo* t1 = UnqualType ();
  CTypeInfo* t2 = type.UnqualType ();
  CTypeArray* a1 = t1->TypeArray ();
  CTypeArray* a2 = t2->TypeArray ();
  while (a1 && a2) {
    if (! a1->DepDim () && ! a2->DepDim () && a1->hasDimension () && a2->hasDimension () && a1->Dimension () != a2->Dimension ())
      return false;
    t1 = a1->BaseType ()->VirtualType ();
    t2 = a2->BaseType ()->VirtualType ();
    a1 = t1->TypeArray ();
    a2 = t2->TypeArray ();
  }
  return (*t1 == *t2);
}

CTypeInfo *CTypeInfo::TypedefInfo (CObjectInfo *info) { 
  // duplicate primitive types only
  if (TypePrimitive () && ! isTypedef ()) {
    CTypeInfo *type = new CTypePrimitive (TypePrimitive ()->TypeName (), Id (), TypePrimitive ()->Size ());
    type->_TypedefInfo = info;
    return type;
  }
  _TypedefInfo = info; 
  return (CTypeInfo*)this;
}

CTypeFunction *CTypeInfo::PtrToFct () const {
  if (TypeFunction ()) return TypeFunction (); 
  if (TypeEmpty ()) return (CTypeFunction*)0;
  return BaseType ()->PtrToFct ();
}

CTypeArray *CTypeInfo::PtrToArray () const {
  if (TypeArray ()) return TypeArray (); 
  if (TypeEmpty ()) return (CTypeArray*)0;
  return BaseType ()->PtrToArray ();
}

bool CTypeInfo::isArithmetic () const {
  CTypeInfo *base = VirtualType ();
  if (base->_Id < TYPE_VOID || base->TypeEnum ())
    return true;
  if (base->TypeBitField ())
    return BaseType ()->isArithmetic ();
  return false;
}

bool CTypeInfo::isTemplateInstanceArg () const {
  bool result = false;
  if (_TypedefInfo) {
    CScopeInfo* scope = _TypedefInfo->Scope ();
    if (strcmp (_TypedefInfo->Name (), "__puma_redirect") == 0)
      scope = scope->Parent ();
    if (scope->isNamespace () &&
        ((scope->isAnonymous () && strstr (scope->Name (), "<")) ||
         strcmp (scope->Name (), "__puma_dargs") == 0))
      result = true;
  }
  return result;
}

void CTypeInfo::TypeText (char state, std::ostream &out, const char *t) const {
  // check for prefix, save and clear the flag
  bool prefix = state & PRINT_PRE;
  state = state & ~PRINT_PRE;

  if (isTypedef () && ! isTemplateInstanceArg () &&
      ((state & PRINT_TDN) ||
       (TypeEnum () && EnumInfo ()->isAnonymous ()) ||
       (TypeRecord () && Record ()->isAnonymous ()))) {
    // print TYPEDEFS
    printScope (state, out, TypedefInfo ());
    out << TypedefInfo ()->Name ();
  } else if (TypeQualified ()) {
    // print QUALIFIED TYPES
    std::ostringstream qualifiers;
    if (TypeQualified ()->isConst ()) {
      qualifiers << "const";
      if (TypeQualified ()->isVolatile ()) 
        qualifiers << " ";
    }
    if (TypeQualified ()->isVolatile ()) 
      qualifiers << "volatile";
    if (BaseType ()->TypePointer () || BaseType ()->TypeAddress ()) {
      if (t) qualifiers << " " << t;
      BaseType ()->TypeText (state, out, qualifiers.str ().c_str ());
    }
    else {
      out << qualifiers.str ().c_str () << " ";
      BaseType ()->TypeText (state, out, t);
    }
    return;
  } else if (TypePointer () || TypeAddress ()) {
    // print POINTERS or REFERENCES
    std::ostringstream inner;
    if (TypeMemberPointer ()) {
      if (TypeMemberPointer ()->Record () || TypeMemberPointer ()->TemplateParam ()) {
        CTypeFunction *fctptr = BaseType ()->TypeFunction ();
        if (!fctptr || !fctptr->Record () || !fctptr->FunctionInfo () ||
             fctptr->FunctionInfo ()->isStaticMethod ()) {
          if (TypeMemberPointer ()->Record ()) {
            printScope (state, inner, TypeMemberPointer ()->Record ());
            printName (state, inner, TypeMemberPointer ()->Record ());
          } else
            printName (state, inner, TypeMemberPointer ()->TemplateParam ());
          inner << "::*";
        } else
          inner << "*";
      } else
        inner << " <null>::*";
    } else if (TypePointer ()) inner << "*";
    else if (TypeAddress ()) inner << "&";
    if (t) inner << t;
    BaseType ()->TypeText (state | PRINT_PRE, out, inner.str ().c_str ());
    return;
  } else if (TypeFunction ()) {
    // print FUNCTION TYPES
    // first put "[scope::][name]([arg-types])[ const][ volatile]" in a buffer
    CTypeFunction *this_type = TypeFunction ();
    std::ostringstream name_and_args;
    if (prefix) name_and_args << "(";
    if (this_type->Record () && this_type->FunctionInfo () &&
        !this_type->FunctionInfo ()->isStaticMethod ()) {
      printScope (state, name_and_args, this_type->Record ());
      printName (state, name_and_args, this_type->Record ());
      name_and_args << "::";
    }
    if (t) name_and_args << t;
    if (prefix) name_and_args << ")";
    name_and_args << "(";
    CTypeList *argtypes = this_type->ArgTypes ();
    for (unsigned i = 0; i < argtypes->Entries (); i++) {
      if (i > 0) name_and_args << ",";
      CTypeInfo *argtype = argtypes->Entry (i);
      argtype->TypeText ((state & PRINT_ELA) ? (state & ~PRINT_INE) : state,
                         name_and_args);
    }
    name_and_args << ")";
    if (this_type->isConst ())
      name_and_args << " const";
    if (this_type->isVolatile ())
      name_and_args << " volatile";

    // now write the generated embedded into its base type    
    if (BaseType ()->is_undefined ())
      out << name_and_args.str ().c_str ();
    else
      BaseType ()->TypeText (state, out, name_and_args.str ().c_str ());
    return;
  } else if (TypeBitField ()) {
    BaseType ()->TypeText (state, out);
    //out << " : " << TypeBitField ()->Dimension (); // ???
  } else if (TypeArray ()) {
    // print ARRAY TYPES
    std::ostringstream name_and_dim;
    if (prefix) name_and_dim << "(";  
    if (t) name_and_dim << t;
    if (prefix) name_and_dim << ")";  
    name_and_dim << "[";
    if (TypeVarArray ())
      name_and_dim << "*";
    else if (Dimension ())
      name_and_dim << Dimension ();
    name_and_dim << "]";
    BaseType ()->TypeText (state, out, name_and_dim.str ().c_str ());
    return;
  } else if (TypeClass ()) {
    // print CLASSES
    if ((state & PRINT_ELA) && !(state & PRINT_INE)) {
      CRecord* record = TypeClass ()->Record ();
      if (record && record->ClassInfo ()->isStruct ()) {
        out << "struct ";
      } else {
        out << "class ";
      }
    }
    printScope (state, out, TypeClass ()->Record ());
    printName (state, out, TypeClass ()->Record ());
  } else if (TypeUnion ()) {
    // print UNIONS
    if ((state & PRINT_ELA) && !(state & PRINT_INE)) {
      out << "union ";
    }
    printScope (state, out, TypeUnion ()->Record ());
    printName (state, out, TypeUnion ()->Record ());
  } else if (TypeEnum ()) {
    // print ENUMS
    if ((state & PRINT_ELA) && !(state & PRINT_INE)) {
      out << "enum ";
    }
    printScope (state, out, TypeEnum ()->EnumInfo ());
    printName (state, out, TypeEnum ()->EnumInfo ());
  } else if (TypeTemplateParam ()) {
    // print TEMPLATE PARAMS
    printName (state, out, TypeTemplateParam ()->TemplateParamInfo ());
  } else if (TypePrimitive ()) {
    // print PRIMITIVE TYPES
    out << TypePrimitive ()->TypeName ();
  } else
    return;

  if (t) out << " " << t;
}

void CTypeInfo::printScope (char state, std::ostream &out, CObjectInfo *info) const {
  if (info->TemplateInstance ()) {
    CTemplateInfo *tpl = info->TemplateInstance ()->Template ();
    if (tpl && tpl->ObjectInfo ()) {
      info = tpl->ObjectInfo ();
    }
  }
  CScopeInfo *scope = info->QualifiedScope ();
  if (! scope) scope = info->AssignedScope ();
  if (! scope) scope = info->Scope ();
  while (scope && scope->TemplateInfo ()) {
    scope = scope->Parent ();
  }
  if (scope) {
    if ((state & PRINT_ABS) && scope->GlobalScope ()) {
      out << "::";
    } else if (! scope->isAnonymous () ||
        ((state & PRINT_UNN) && strcmp(scope->Name(), "<unnamed>") == 0)) {
      if (scope->Record ()) {
        // print the scope name, but don't use elaborated type specifier
        scope->TypeInfo ()->TypeText (state | PRINT_INE, out);
        out << "::";
      } else if (scope->NamespaceInfo () && ! scope->GlobalScope ()) {
        if (state & PRINT_ABS)
          out << "::";
        out << scope->NamespaceInfo ()->QualName (false, false,
                                                  state & PRINT_UNN) << "::";
      }
    }
  }
}

void CTypeInfo::printName (char state, std::ostream &out, CObjectInfo *info) const {
  CTemplateInstance *instance;
  DeducedArgument *arg;

  if (! info->Name ()/* || info->isAnonymous ()*/)
    out << "<null>";
  else
    out << info->Name ();

  instance = info->TemplateInstance ();
  if (! instance)
    return;

  out << "<";
  std::ostringstream os;
  for (unsigned i = 0; i < instance->InstantiationArgs (); i++) {
    arg = instance->InstantiationArg (i);
    // do not list default arguments
    if (arg->isDefaultArg ())
      break;
    if (i > 0) os << ",";
    arg->print (os, state & PRINT_ABS);
  }
  std::string args = os.str();
  // avoid misinterpretation of "<::" as digraph "<:" by adding whitespace
  if (args.size () && args.at (0) == ':')
    out << " ";
  out << args;
  if (args.size() && args.at(args.size()-1) == '>')
    out << " ";
  out << ">";
}

CTypeInfo *CTypeInfo::Duplicate (const CTypeInfo *type) {
  if (! type) return &CTYPE_EMPTY;

  CTypeInfo *copy = (CTypeInfo*)0;
  if (type->TypeEmpty ()) {
    copy = &CTYPE_EMPTY;
  } else if (type->TypePrimitive ()) {
    // copy primitive type only if it contains typedef information
    if (type->isTypedef ())
      copy = new CTypePrimitive (type->TypePrimitive ()->TypeName (), type->Id (), type->TypePrimitive ()->Size ());
    else
      copy = (CTypeInfo*)type;
  } else if (type->TypeTemplateParam ()) {
    copy = new CTypeTemplateParam (
      type->TypeTemplateParam ()->TemplateParamInfo ());
  } else if (type->TypeEnum ()) {
    copy = new CTypeEnum (type->EnumInfo ());
  } else if (type->TypeRecord ()) {
    if (type->TypeUnion ())
      copy = new CTypeUnion (type->UnionInfo ());
    else // TypeClass
      copy = new CTypeClass (type->ClassInfo ());
  }

  if (copy) {
    if (type->isTypedef ())
      copy->_TypedefInfo = type->_TypedefInfo;
    return copy;
  }

  CTypeInfo *base = Duplicate (type->BaseType ());
  if (type->TypeMemberPointer ()) {
    if (type->TypeMemberPointer ()->TemplateParam ())
      copy = new CTypeMemberPointer (base, 
        type->TypeMemberPointer ()->TemplateParam ());
    else 
      copy = new CTypeMemberPointer (base, 
        type->TypeMemberPointer ()->Record ());
  } else if (type->TypeBitField ()) {
    copy = new CTypeBitField (base);
    copy->TypeBitField ()->Dimension (type->Dimension ());
  } else if (type->TypeAddress ()) {
    copy = new CTypeAddress (base);
  } else if (type->TypeFunction ()) {
    CTypeList *args = new CTypeList;
    args->ArgumentList (type->ArgTypes ()->ArgumentList ());
    for (unsigned i = 0; i < type->ArgTypes ()->Entries (); i++)
      args->AddEntry (Duplicate (type->ArgTypes ()->Entry (i)));
    copy = new CTypeFunction (base, args, 
      type->TypeFunction ()->hasPrototype ());
    copy->TypeFunction ()->FunctionInfo (type->FunctionInfo ());
    copy->TypeFunction ()->isConst (type->TypeFunction ()->isConst ());
    copy->TypeFunction ()->isVolatile (type->TypeFunction ()->isVolatile ());
    copy->TypeFunction ()->isRestrict (type->TypeFunction ()->isRestrict ());
  } else if (type->TypePointer ()) {
    copy = new CTypePointer (base);
  } else if (type->TypeVarArray ()) {
    if (type->TypeVarArray ()->Qualifiers ())
      copy = Duplicate (type->TypeVarArray ()->Qualifiers ());
    else
      copy = 0;
    copy = new CTypeVarArray (base, (CTypeQualified*)copy);
  } else if (type->TypeArray ()) {
    if (type->TypeArray ()->Qualifiers ())
      copy = Duplicate (type->TypeArray ()->Qualifiers ());
    else
      copy = 0;
    copy = new CTypeArray (base, (CTypeQualified*)copy);
    copy->TypeArray ()->Dimension (type->Dimension ());
    copy->TypeArray ()->DepDim (type->TypeArray ()->DepDim ());
    copy->TypeArray ()->hasDimension (type->TypeArray ()->hasDimension ());
  } else if (type->TypeQualified ()) {
    copy = new CTypeQualified (base, type->isConst (), type->isVolatile (),
                                     type->isRestrict ());
  } else
    Destroy (base);

  if (type->isTypedef ())
    copy->_TypedefInfo = type->_TypedefInfo;
  return copy;
}

void CTypeInfo::Destroy (CTypeInfo *type) {
  if (! type || type->TypeEmpty ()) return;
  if (type->TypePrimitive ()) {
    if (type->isTypedef ())
      delete type->TypePrimitive ();
    return;
  }
  if (type->TypeClass ()) delete type->TypeClass ();
  else if (type->TypeTemplateParam ()) delete type->TypeTemplateParam ();
  else if (type->TypeEnum ()) delete type->TypeEnum ();
  else if (type->TypeUnion ()) delete type->TypeUnion ();
  else {
    if (! type->BaseType ()->TypeEmpty ())
      Destroy (type->BaseType ());
    if (type->TypeQualified ()) delete type->TypeQualified ();
    else if (type->TypePointer ()) delete type->TypePointer ();
    else if (type->TypeMemberPointer ()) delete type->TypeMemberPointer ();
    else if (type->TypeAddress ()) delete type->TypeAddress ();
    else if (type->TypeBitField ()) delete type->TypeBitField ();
    else if (type->TypeArray ()) {
      if (type->TypeArray ()->Qualifiers ())
        Destroy (type->TypeArray ()->Qualifiers ());
      if (type->TypeVarArray ()) delete type->TypeVarArray ();
      else delete type->TypeArray ();
    } else if (type->TypeFunction ()) {
      CTypeList *args = type->TypeFunction ()->ArgTypes ();
      if (args) {
        for (unsigned i = 0; i < args->Entries (); i++)
          Destroy (args->Entry (i));
        delete args;
      }
      delete type->TypeFunction ();
    }
  }
}

// rank C++ arithmetic types
unsigned CTypeInfo::rank () const {
  switch (Id ()) {
    case TYPE_BOOL               : return 1;
    case TYPE_C_BOOL             : return 1;
    case TYPE_CHAR               : return 2;
    case TYPE_SIGNED_CHAR        : return 2;
    case TYPE_UNSIGNED_CHAR      : return 3;
    case TYPE_SHORT              : return 4;
    case TYPE_UNSIGNED_SHORT     : return 5;
    case TYPE_WCHAR_T            : return 6;
    case TYPE_ENUM               : return 7;
    case TYPE_INT                : return 8;
    case TYPE_UNSIGNED_INT       : return 9;
    case TYPE_LONG               : return 10;
    case TYPE_UNSIGNED_LONG      : return 11;
    case TYPE_LONG_LONG          : return 12;
    case TYPE_UNSIGNED_LONG_LONG : return 13;
    case TYPE_INT128             : return 14;
    case TYPE_UNSIGNED_INT128    : return 15;
    case TYPE_FLOAT              : return 16;
    case TYPE_DOUBLE             : return 17; 
    case TYPE_LONG_DOUBLE        : return 18;
    default: break;
  }
  return 0;
}

// arithmetic conversion rank (C only)
unsigned CTypeInfo::conv_rank () const {
  if (is_bool ()) return 1;
  else if (is_char ()) return 2;
  else if (is_signed_char ()) return 2;
  else if (is_unsigned_char ()) return 2;
  else if (is_unsigned_short ()) return 3;
  else if (is_short ()) return 4;
  else if (is_unsigned_int ()) return 5;
  else if (is_wchar_t ()) return 6;
  else if (is_int ()) return 6; 
  else if (is_unsigned_long ()) return 7;
  else if (is_long ()) return 8;
  else if (is_unsigned_long_long ()) return 9;
  else if (is_long_long ()) return 10;
  else if (is_unsigned_int128 ()) return 11;
  else if (is_int128 ()) return 12;
  else if (is_float ()) return 13;
  else if (is_double ()) return 14;
  else if (is_long_double ()) return 15;
  return 0;
}

bool CTypeInfo::operator >(const CTypeInfo &type) const {
  CTypeInfo *t1 = VirtualType ();
  CTypeInfo *t2 = type.VirtualType ();
  if (t1 == t2) return false;

  if (t1->is_long_double ()) return true;
  else if (t2->is_long_double ()) return false;
  else if (t1->is_double ()) return true;
  else if (t2->is_double ()) return false;
  else if (t1->is_float ()) return true;
  else if (t2->is_float ()) return false;
  
  bool t1_is_signed = t1->is_signed ();
  bool t2_is_signed = t2->is_signed ();
  int t1_rank = t1->conv_rank ();
  int t2_rank = t2->conv_rank ();
  if (t1_is_signed == t2_is_signed)
    return t1_rank >= t2_rank;
  else if (t1_is_signed && t1_rank < t2_rank)
    return false;
  else if (t2_is_signed && t2_rank < t1_rank)
    return true;
  else if (t1_is_signed && t1_rank >= (t2_rank+2))
    return true;
  else if (t2_is_signed && t2_rank >= (t1_rank+2))
    return false;
  else 
    return ! t1_is_signed;
}

bool CTypeInfo::operator <(const CTypeInfo &type) const {
  CTypeInfo *t1 = VirtualType ();
  CTypeInfo *t2 = type.VirtualType ();
  return t1 != t2 && *t1 <= *t2;
}

// signed integer type?
bool CTypeInfo::is_signed () const {
  if (is_char () || 
      is_signed_char () ||
      is_short () || 
      is_int () || 
      is_wchar_t () ||
      is_long () || 
      is_long_long () ||
      is_int128 ())
    return true;
  return false;
}

// unsigned integer type?
bool CTypeInfo::is_unsigned () const {
  if (is_bool () ||           // ???
      is_unsigned_char () ||
      is_unsigned_short () ||
      is_unsigned_int () || 
      is_unsigned_long () ||
      is_unsigned_long_long () ||
      is_unsigned_int128 ())
    return true;
  return false;
}

// complete type (at this source position)?
bool CTypeInfo::isComplete (unsigned long pos) const {
  if (isUndefined () || isVoid () ||
      (isArray () && ! isVarArray () && ! VirtualType ()->TypeArray ()->hasDimension ()) || 
      (isRecord () && ! VirtualType ()->TypeRecord ()->isComplete (pos)) ||
      (isEnum () && ! VirtualType ()->TypeEnum ()->isComplete (pos))) 
    return false;
  return true;
}

bool CTypeEnum::isComplete (unsigned long pos) const { 
  return _Enum && _Enum->DefObject ()->EnumInfo ()->isComplete (pos); 
}

bool CTypeRecord::isComplete (unsigned long pos) const { 
  return _Record && _Record->DefObject ()->Record ()->isComplete (pos); 
}

long int CTypeInfo::Size () const {
  if (TypeQualified () || TypeAddress () || TypeBitField ())
    return BaseType ()->Size ();
  else if (TypeArray ())
    return BaseType ()->Size () * Dimension ();
  else if (TypeMemberPointer ())
    return MEMB_PTR_SIZE;
  else if (TypePointer ())
    return PTR_SIZE;
  else if (TypeEnum ())
    return ENUM_SIZE;
  else if (TypeClass () || TypeUnion ())
    return TypeRecord ()->Size ();
  else if (TypePrimitive ())
    return TypePrimitive ()->Size ();
  return 0;
}

long int CTypeRecord::Size () {
  CObjectInfo *info;
  CStructure *cinfo;
  long int align, s;
  CTypeInfo *type;

  if (_Size) 
    return _Size;

  _Size = 0;
  _Align = 32;

  cinfo = Record ()->DefObject ()->Record ();
  if (cinfo) {
    for (unsigned i = cinfo->Attributes (); i > 0; i--) {
      info = cinfo->Attribute (i-1);
      if (info->EnumeratorInfo () || 
          info->Storage () == CStorage::CLASS_STATIC || 
          info->Storage () == CStorage::CLASS_THREAD)
        continue;
      type = info->TypeInfo ();
      align = type->Align ();
      if (! TypeUnion () && align != 0 && (_Size % align) != 0) 
        _Size = _Size + align - (_Size % align);
      if (align > _Align) 
        _Align = align;
      s = type->Size ();
      if (TypeUnion ())
        _Size = s > _Size ? s : _Size;
      else
        _Size += s;
    }
    if (cinfo->ClassInfo ())
      for (unsigned i = 0; i < cinfo->ClassInfo ()->BaseClasses (); i++)
        _Size += (cinfo->ClassInfo ()->BaseClass (i)->
                  Class ()->TypeInfo ()->Size ());
  }

  if (! _Size) 
    _Size = 8;

  return _Size;
}

bool CTypeInfo::isTemplate () const { 
  return TypeRecord () && TypeRecord ()->Record () ? 
          TypeRecord ()->Record ()->isTemplate () :
         TypeFunction () && TypeFunction ()->FunctionInfo () ?
          TypeFunction ()->FunctionInfo ()->isTemplate () : false; 
}

bool CTypeInfo::isTemplateInstance () const { 
  return TypeRecord () && TypeRecord ()->Record () ? 
          TypeRecord ()->Record ()->isTemplateInstance () :
         TypeFunction () && TypeFunction ()->FunctionInfo () ?
          TypeFunction ()->FunctionInfo ()->isTemplateInstance () : false; 
}

CRecord *CTypeFunction::Record () const { 
  return _Function ? _Function->Record () : (CRecord*)0; 
}

CClassInfo *CTypeInfo::ClassInfo () const { 
  return Record () ? Record ()->DefObject ()->ClassInfo () : (CClassInfo*)0; 
}

CUnionInfo *CTypeInfo::UnionInfo () const { 
  return Record () ? Record ()->DefObject ()->UnionInfo () : (CUnionInfo*)0; 
}

CClassInfo *CTypeClass::ClassInfo () const { 
  return Record () ? (CClassInfo*)Record ()->DefObject () : (CClassInfo*)0; 
}

CUnionInfo *CTypeUnion::UnionInfo () const { 
  return Record () ? (CUnionInfo*)Record ()->DefObject () : (CUnionInfo*)0; 
}


bool CTypeTemplateParam::isType () const {
  return TemplateParamInfo () && TemplateParamInfo ()->isTypeParam ();
}

bool CTypeInfo::isDependent (bool consider_unknown_t, bool is_named_type) const {
  if (consider_unknown_t && is_unknown_t ())
    return true;
  if (TypeTemplateParam ())
    return true;
  if (TypeFunction () && TypeFunction ()->ArgTypes ()->isDependent ())
    return true;
  if (TypeMemberPointer () && TypeMemberPointer ()->TemplateParam ())
    return true;
  if (isTemplateInstance ()) {
    if ((TypeRecord () && 
         ! TypeRecord ()->Record ()->TemplateInstance ()->canInstantiate ()) ||
        (TypeFunction () && 
         ! TypeFunction ()->FunctionInfo ()->TemplateInstance ()->canInstantiate ()))
      return true;
  }
  if (TypeClass () && TypeClass ()->ClassInfo ()) {
    CClassInfo* ci = TypeClass ()->ClassInfo ();
    if (! ci->isTemplate () && ci->hasDepBaseClass ())
      return true;
    if (! is_named_type && ci->isTemplate () && ci->hasDepBaseClass ())
      return true;
    if (ci->isTemplate ())
      return true;
  }
  return BaseType () && this != BaseType () && 
         BaseType ()->isDependent (consider_unknown_t, is_named_type);
}

bool CTypeList::isDependent () const {
  for (unsigned i = Entries (); i > 0; i--)
    if (Entry (i-1) && Entry (i-1)->isDependent ())
      return true;
  return false;
}

static void MangleTemplateArguments (CTemplateInstance *instance, 
                                     std::ostream &out) {
  DeducedArgument *arg;

  out << "I";
  for (unsigned i = 0; i < instance->InstantiationArgs (); i++) {
    arg = instance->InstantiationArg (i);
    if (arg->isDefaultArg ())
      continue;
      
    if (arg->Type ()) {
      arg->Type ()->Mangled (out);
    } else if (arg->Value ()) { // some types are missing here, but CConstant..
      if (arg->Value ()->isSigned ()) {
        CTYPE_INT.Mangled (out);
        if (arg->Value ()->convert_to_int () < 0)
          out << "n"; // a negative value is prececed by 'n'
        out << arg->Value ()->convert_to_int ();
      }
      else if (arg->Value ()->isUnsigned ()) {
        CTYPE_UNSIGNED_INT.Mangled (out);
        out << arg->Value ()->convert_to_uint ();
      }
      else if (arg->Value ()->isFloat ()) {
        CTYPE_FLOAT.Mangled (out);
        long double val = arg->Value ()->convert_to_float ();
        // does not 100% conform with the standard (byte order)
        out.width (2);
        out.fill ('0');
        out << std::hex;
        for (int p = 0; p < (int)sizeof(long double); p++)
          out << (unsigned int) *(((unsigned char*)&val) + p);
        out.width (0);
        out << std::dec;
      }
    }
  }
  out << "E";
}

static void MangleScopedName (const CObjectInfo *info, std::ostream &out, bool first = true) {
  CScopeInfo *scope;
  if (! info || ! info->Name ())
    return;

  if (! info->Record () && ! info->NamespaceInfo () &&
      ! info->FunctionInfo () && ! info->EnumInfo () &&
      ! info->TypedefInfo ()) 
    out << "9<UNKNOWN>";
  else {
    scope = info->Scope ();
    CTemplateInstance *instance = info->TemplateInstance ();
    // if this is a template instance, there is always an exclosing scope
    if (instance)
      scope = scope->Scope ();

    if (scope && ! scope->GlobalScope ()) {
      if (first) 
        out << "N"; 

      if (scope->Record () || scope->NamespaceInfo ())
        MangleScopedName (scope, out, false);


      if (first) 
        out << "E";
    }

    if (strcmp (info->Name (), "<unnamed>") == 0)
      out << "12_GLOBAL__N_1";
    else
      out << strlen (info->Name ()) << info->Name ();

    // if this is a template instance, mangle the argument types
    if (instance)
      MangleTemplateArguments (instance, out);
  } 
}

// C++ V3 ABI mangling: see http://www.codesourcery.com/cxx-abi/abi.html#mangling for details 
void CTypeInfo::Mangled (std::ostream &out) const {

  // g++ uses a typedef name for anonymous structs, enums, etc. if one exists
  if (isTypedef ()) {
    CObjectInfo *obj = Record ();
    if (! obj) obj = EnumInfo ();
    if (obj && obj->isAnonymous ()) {
      MangleScopedName (TypedefInfo (), out);
      return;
    }
  }
 
  if (TypeAddress ()) {
    out << "R";
    BaseType ()->Mangled (out);
  } else if (TypeQualified ()) {
    if (TypeQualified ()->isConst ())    out << "K";
    if (TypeQualified ()->isVolatile ()) out << "V";
    if (TypeQualified ()->isRestrict ()) out << "r";
    BaseType ()->Mangled (out);
  } else if (TypeRecord ()) {
    MangleScopedName (TypeRecord ()->Record (), out);
  } else if (TypeEnum ()) {
    MangleScopedName (TypeEnum ()->EnumInfo (), out);
  } else if (TypeArray ()) {
    out << "A";
    if (! TypeVarArray () && TypeArray ()->Dimension () > 0)
      out << TypeArray ()->Dimension ();
    out << "_";
    BaseType ()->Mangled (out);
  } else if (TypePointer ()) {
    if (TypeMemberPointer () && TypeMemberPointer ()->Record ()) {
      out << "M";
      MangleScopedName (TypeMemberPointer ()->Record (), out);
    } else 
      out << "P";
    BaseType ()->Mangled (out);
  } else if (TypeFunction ()) {
    out << "F";
    MangleScopedName (TypeFunction ()->Record (), out);
    TypeFunction ()->BaseType ()->Mangled (out);
    if (TypeFunction ()->ArgTypes () && TypeFunction ()->ArgTypes ()->Entries ()) {
      for (unsigned i = 0; i < TypeFunction ()->ArgTypes ()->Entries (); i++)
        TypeFunction ()->ArgTypes ()->Entry (i)->Mangled (out);
    } else 
      out << "v";
    out << "E";
  } else if (TypeBitField ()) {
    // ???
    BaseType ()->Mangled (out);
  } else if (TypePrimitive ()) {
    switch (_Id) {
      case TYPE_BOOL:                out << "b"; break;
      case TYPE_C_BOOL:              out << "b"; break;
      case TYPE_CHAR:                out << "c"; break;
      case TYPE_WCHAR_T:             out << "w"; break;
      case TYPE_SHORT:               out << "s"; break;
      case TYPE_INT:                 out << "i"; break;
      case TYPE_LONG:                out << "l"; break;
      case TYPE_LONG_LONG:           out << "x"; break; // __int64
      case TYPE_INT128:              out << "n"; break; // __int128
      case TYPE_SIGNED_CHAR:         out << "a"; break;
      case TYPE_UNSIGNED_CHAR:       out << "h"; break;
      case TYPE_UNSIGNED_SHORT:      out << "t"; break;
      case TYPE_UNSIGNED_INT:        out << "j"; break;
      case TYPE_UNSIGNED_LONG:       out << "m"; break;
      case TYPE_UNSIGNED_LONG_LONG:  out << "y"; break; // __int64
      case TYPE_UNSIGNED_INT128:     out << "o"; break; // __int128
      case TYPE_FLOAT:               out << "f"; break;
      case TYPE_DOUBLE:              out << "d"; break;
      //case TYPE_FLOAT128:            out << "g"; break;
      case TYPE_LONG_DOUBLE:         out << "e"; break;
      case TYPE_VOID:                out << "v"; break;
      case TYPE_ELLIPSIS:            out << "z"; break;
      case TYPE_UNDEFINED:
        break;
      default: // vendor extended type := u<name-length><name>
        out << "u" << strlen (TypePrimitive ()->TypeName ()) 
            << TypePrimitive ()->TypeName (); 
    }
  }
}

CRecord *CTypeMemberPointer::Record () const { 
  return _Class ? _Class->Record () : (CRecord*)0; 
}

CTemplateParamInfo *CTypeMemberPointer::TemplateParam () const { 
  return _Class ? _Class->TemplateParamInfo () : (CTemplateParamInfo*)0; 
}

CTypeInfo *CTypeEnum::UnderlyingType () const { 
  return _Enum ? _Enum->UnderlyingType() : &CTYPE_INT; 
}

bool CTypeClass::instantiate (CStructure *scope) {
  // If this is the type of a pseudo class instance, 
  // create a real instance of the corresponding template. 
  // Return true if instantiation succeeded.
  CClassInfo* ci = ClassInfo ();
  if (ci) {
    CTemplateInstance* ti = ci->TemplateInstance ();
    if (ti && ! ti->isInstantiated () && ti->canInstantiate ()) {
      // instantiate
      return ti->instantiate (scope);
    }
  }
  return true;
}

/** Check if this is a plain old data (POD) type. */
bool CTypeInfo::isPOD () const {
  // ��3.9p10
  // Arithmetic types (3.9.1), enumeration types, pointer types, and pointer
  // to member types (3.9.2), and cv-qualified versions of these types (3.9.3) are
  // collectively called scalar types. Scalar types, POD-struct types, POD-union
  // types (clause 9), arrays of such types and cv-qualified versions of these
  // types (3.9.3) are collectively called POD types.
  if (isScalar ())
    return true;

  // ��9p4
  // A POD-struct is an aggregate class that has no non-static data members of
  // type pointer to member, non-POD-struct, non-POD-union, or array of such types,
  // or reference, and has no user-defined copy assignment operator and no user-defined
  // destructor. Similarly, a POD-union is an aggregate union that has no non-static
  // data members of type pointer to member, non-POD-struct, non-POD-union, or array
  // of such types, or reference, and has no user-defined copy assignment operator
  // and no user-defined destructor. A POD class is a class that is either a POD-struct
  // or a POD-union.
  //
  // $8.5.1p1
  // An aggregate is an array or a class (clause 9) with no user-declared constructors
  // (12.1), no private or protected non-static data members (clause 11), no base
  // classes (clause 10), and no virtual functions (10.3).
  CRecord* c = isClassOrUnion () ? VirtualType ()->TypeRecord ()->Record () : 0;

  // class or union without base classes
  if (c && (! c->ClassInfo () || ! c->ClassInfo ()->BaseClasses ())) {

    // check data members
    unsigned i, num = c->Attributes ();
    for (i = 0; i < num; i++) {
      CAttributeInfo* member = c->Attribute (i);
      // private or protected data member
      if (member->Protection () == CProtection::PROT_PRIVATE ||
          member->Protection () == CProtection::PROT_PROTECTED)
        break;
      // static members don't matter
      if (! member->isStatic ()) {
        CTypeInfo* type = member->TypeInfo ();
        // reference type
        if (type->isAddress ())
          break;
        // get underlying type of array
        while (type->isArray ())
          type = type->VirtualType ()->BaseType ();
        // pointer to member
        if (type->isMemberPointer ())
          break;
        // non-POD class
        if (type->isClassOrUnion () && ! type->isPOD ())
          break;
      }
    }
    // non-POD member found
    if (i != num)
      return false;

    // check member functions
    num = c->Functions ();
    for (i = 0; i < num; i++) {
      CFunctionInfo* member = c->Function (i);
      // virtual function
      if (member->isVirtual ())
        break;
      // user-defined
      if (! member->isBuiltin ()) {
        // user-defined constructor or destructor
        if (member->isConstructor () || member->isDestructor ())
          break;
        // copy assignment operator
        if (member->isCopyAssignOperator ())
          break;
      }
    }

    // POD class
    if (i == num)
      return true;
  }

  return false;
}

/** Check if this is a trivial type:
 * (1) a scalar type
 * (2) a trivial class type
 * (3) an array of (1), (2), or (3)
 */
bool CTypeInfo::isTrivial () const {
  if (isScalar ())
    return true;
  if (isArray ())
    return VirtualType ()->BaseType ()->isTrivial ();
  if (isClassOrUnion ())
    return TypeRecord ()->isTrivial ();
  return false;
}

long int CTypeInfo::Align () const {
  // TODO: Calculation of alignment not yet implemented
  //       except for class and union types.
  return 1;
}

/** Check if this is a local type. A type is local if it was 
 *  declared in a local scope. */
bool CTypeInfo::isLocal () const {
  if (TypeEmpty () || TypePrimitive ()) {
    return false;
  }
  CObjectInfo* info = 0;
  if (TypeTemplateParam ()) {
    info = TypeTemplateParam ()->TemplateParamInfo ();
  } else if (TypeEnum ()) {
    info = TypeEnum ()->EnumInfo ();
  } else if (TypeRecord ()) {
    info = TypeRecord ()->Record ();
  } else if (TypeMemberPointer ()) {
    if (TypeMemberPointer ()->TemplateParam ())
      info = TypeMemberPointer ()->TemplateParam ();
    else 
      info = TypeMemberPointer ()->Record ();
  } else if (TypeFunction ()) {
    info = TypeFunction ()->FunctionInfo ();
  }
  if (info) {
    return info->isLocal ();
  }
  return BaseType () && this != BaseType () && BaseType ()->isLocal ();
}


} // namespace Puma
