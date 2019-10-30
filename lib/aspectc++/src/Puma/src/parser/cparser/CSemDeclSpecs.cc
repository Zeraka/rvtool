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

#include "Puma/CSemDeclSpecs.h"
#include "Puma/CObjectInfo.h"
#include "Puma/CEnumInfo.h"
#include "Puma/ErrorSink.h"
#include "Puma/CRecord.h"
#include "Puma/CTree.h"

namespace Puma {


CSemDeclSpecs::CSemDeclSpecs (ErrorSink *err, CT_DeclSpecSeq *dss, bool support_implicit_int) :
  _err (err), _dss (dss) {
   
  bool no_error = true, have_type = false;
  
  // first gather information about all nodes in the sequence
  no_error = (analyze_seq () && no_error);
  
  // check for errors
  no_error = (check_prim_decl_specs (have_type) && no_error);
  no_error = (check_complex_decl_specs (have_type) && no_error);  
  no_error = (check_storage_class () && no_error);
  no_error = (check_signed_unsigned () && no_error);
  no_error = (check_long_short () && no_error); 

  // implicit int
  if (! have_type && support_implicit_int) {
    _prim_map[CT_PrimDeclSpec::PDS_INT]++;
    have_type = true;
  }

  // now create the type
  _type = &CTYPE_UNDEFINED;
  if (have_type && no_error) {
    CTypeInfo *type = create_qualifiers (create_type ());
    // assign the type if the final check is also ok
    if (check_finally ())
      _type = type;
    else
      CTypeInfo::Destroy (type);
  }
}


// analyses the syntax tree nodes of the decl-spec sequence. Return false
// if an error was detected. 
bool CSemDeclSpecs::analyze_seq () {
  _names = 0, _class_specs = 0, _union_specs = 0, _enum_specs = 0;
  _def_class = _def_union = _def_enum = false;
  _name = 0;
  for (int i = 0; i < CT_PrimDeclSpec::NumTypes; i++)
    _prim_map[i] = 0; // clear the array

  // Gather information about the declaration specifiers
  for (int s = 0; s < _dss->Sons (); s++) {
    CTree *spec = _dss->Son (s);
    if (!analyze (spec)) {
      Token *token = spec->token ();
      if (token) *_err << token->location ();
      *_err << sev_fatal << "unknown declaration specifier `" 
           << spec->NodeName () << "'" << endMessage;
      return false;
    }
  }
  return true;
}


// analyzes the current declaration specifier in the sequence and set some
// attributes according to the result. Returns false if the specifier type
// is unknown.
bool CSemDeclSpecs::analyze (CTree *spec) {
  const char *id = spec->NodeName ();
  if (id == CT_PrimDeclSpec::NodeId ()) {
    CT_PrimDeclSpec *pds = (CT_PrimDeclSpec*)spec;
    _prim_map[pds->SpecType ()]++;
    _prim_token[pds->SpecType ()] = pds->token ();
  } else if (id == CT_SimpleName::NodeId () ||
             id == CT_TemplateName::NodeId () ||
             id == CT_QualName::NodeId () ||
      id == CT_RootQualName::NodeId ()) {
    _names++;
    _name = (CT_SimpleName*)spec;
  } else if (id == CT_ClassSpec::NodeId ()) {
    _class_specs++;
    _name = (CT_SimpleName*)spec->Son (1);
    _def_class = (_name && _name->Object () && 
                  _name->Object ()->Record () &&
                  _name->Object ()->Record ()->isDefined ());
  } else if (id == CT_ClassDef::NodeId ()) {
    _class_specs++;
    _name = (CT_SimpleName*)spec->Son (1);
    _def_class = true;
  } else if (id == CT_UnionSpec::NodeId ()) {
    _union_specs++;
    _name = (CT_SimpleName*)spec->Son (1);
    _def_union = (_name && _name->Object () && 
                  _name->Object ()->Record ()->isDefined ());
  } else if (id == CT_UnionDef::NodeId ()) {
    _union_specs++;
    _name = (CT_SimpleName*)spec->Son (1);
    _def_union = true;
  } else if (id == CT_EnumSpec::NodeId ()) {
    _enum_specs++;
    _name = (CT_SimpleName*)spec->Son (1);
    _def_enum = (_name && _name->Object () && 
                 _name->Object ()->EnumInfo ()->isDefined ());
  } else if (id == CT_EnumDef::NodeId ()) {
    _enum_specs++;
    _name = (CT_SimpleName*)spec->Son (1);
    _def_enum = true;
  } else
    return false;
  return true;
}


bool CSemDeclSpecs::check_prim_decl_specs (bool &have_type) {
  bool have_error = false;
  for (int s = 0; s < CT_PrimDeclSpec::NumTypes; s++) {
    int count = _prim_map[s];
    switch (s) { // be careful, some break statement are missing!
      case CT_PrimDeclSpec::PDS_LONG:
        if (count > 0) have_type = true;
        if (count > 2) {
          *_err << sev_error << _prim_token[s]->location () << count 
                << " times `long' is too long" << endMessage;
          have_error = true;
        }
        break;
      case CT_PrimDeclSpec::PDS_CHAR:
      case CT_PrimDeclSpec::PDS_WCHAR_T:
      case CT_PrimDeclSpec::PDS_BOOL:
      case CT_PrimDeclSpec::PDS_C_BOOL:
      case CT_PrimDeclSpec::PDS_INT:
      case CT_PrimDeclSpec::PDS_INT64:
      case CT_PrimDeclSpec::PDS_INT128:
      case CT_PrimDeclSpec::PDS_FLOAT:
      case CT_PrimDeclSpec::PDS_DOUBLE:
      case CT_PrimDeclSpec::PDS_VOID:
      case CT_PrimDeclSpec::PDS_UNKNOWN_T:
        if (count > 0) have_type = true;
        if (count > 1) {
          *_err << sev_error << _prim_token[s]->location ()
                << "two or more `" << _prim_token[s]->text ()
                << "' in declaration" << endMessage;
          have_error = true;
        }
        break;
      case CT_PrimDeclSpec::PDS_SIGNED:
        if (count > 0 && _prim_map[CT_PrimDeclSpec::PDS_UNSIGNED] > 0) {
          *_err << sev_error << _prim_token[s]->location () 
                << "signed and unsigned given together" << endMessage;
          have_error = true;
        }
      case CT_PrimDeclSpec::PDS_UNSIGNED:
      case CT_PrimDeclSpec::PDS_SHORT:
        if (count > 0) have_type = true;
      default:
        if (count > 1) {
          *_err << sev_warning << _prim_token[s]->location () 
                << "duplicate `" << _prim_token[s]->text () << "'"
                << endMessage;
          have_error = true;
        }
    }
  }
  return !have_error;
}


bool CSemDeclSpecs::check_complex_decl_specs (bool &have_type) {
  if (_names || _class_specs || _union_specs || _enum_specs) {
    have_type = true;
  }
  return true;
}


bool CSemDeclSpecs::check_storage_class () {
  if (_prim_map[CT_PrimDeclSpec::PDS_TYPEDEF] + 
      _prim_map[CT_PrimDeclSpec::PDS_AUTO] + 
      _prim_map[CT_PrimDeclSpec::PDS_REGISTER] + 
      _prim_map[CT_PrimDeclSpec::PDS_STATIC] + 
      _prim_map[CT_PrimDeclSpec::PDS_EXTERN] + 
      _prim_map[CT_PrimDeclSpec::PDS_MUTABLE]  > 1) {
    *_err << sev_error << _dss->token ()->location () 
         << "multiple storage classes in declaration" << endMessage;
    return false;
  }
  return true;
}


bool CSemDeclSpecs::check_signed_unsigned () {
  if ((_prim_map[CT_PrimDeclSpec::PDS_SIGNED] || 
       _prim_map[CT_PrimDeclSpec::PDS_UNSIGNED]) &&
      (_prim_map[CT_PrimDeclSpec::PDS_BOOL] || 
       _prim_map[CT_PrimDeclSpec::PDS_C_BOOL] || 
       _prim_map[CT_PrimDeclSpec::PDS_FLOAT] || 
       _prim_map[CT_PrimDeclSpec::PDS_DOUBLE] || 
       _prim_map[CT_PrimDeclSpec::PDS_VOID] ||
       _prim_map[CT_PrimDeclSpec::PDS_UNKNOWN_T] ||
       _prim_map[CT_PrimDeclSpec::PDS_WCHAR_T] ||
       _class_specs || _union_specs || _enum_specs)) {
    *_err << sev_error << _dss->token ()->location () 
         << "`signed' or `unsigned' invalid for given type"
         << endMessage;
    return false;
  }
  return true;
}


bool CSemDeclSpecs::check_long_short () {
  if (_prim_map[CT_PrimDeclSpec::PDS_LONG] &&
      _prim_map[CT_PrimDeclSpec::PDS_SHORT]) {
    *_err << sev_error << _dss->token ()->location () 
         << "`long' and `short' specified together"
         << endMessage;
    return false;
  }
  return true;
}


// makes a type object according to the state of this object after analysis
CTypeInfo *CSemDeclSpecs::create_type () {
  // --------------------------------------------- //
  //  type specifier(s)    | specified type        //       
  // --------------------------------------------- //
  //  void                 | "void"                //
  //  bool                 | "bool"                //       
  //  _Bool                | "_Bool"               //       
  //  wchar_t              | "wchar_t"             //       
  //                       |                       //
  //  char                 | "char"                //       
  //  unsigned char        | "unsigned char"       //       
  //  signed char          | "signed char"         //       
  //                       |                       //
  //  unsigned             | "unsigned int"        //       
  //  unsigned int         | "unsigned int"        //       
  //  signed               | "int"                 //       
  //  signed int           | "int"                 //       
  //  int                  | "int"                 //       
  //  unsigned short int   | "unsigned short int"  //
  //  unsigned short       | "unsigned short int"  //
  //  unsigned long int    | "unsigned long int"   //
  //  unsigned long        | "unsigned long int"   //
  //  signed long int      | "long int"            //       
  //  signed long          | "long int"            //       
  //  long int             | "long int"            //       
  //  long                 | "long int"            //       
  //  signed short int     | "short int"           //       
  //  signed short         | "short int"           //       
  //  short int            | "short int"           //       
  //  short                | "short int"           //       
  //                       |                       //
  //  float                | "float"               //       
  //  double               | "double"              //       
  //  long double          | "long double"         //       
  // --------------------------------------------- //
  CObjectInfo *info;
  CTypeInfo *type = &CTYPE_UNDEFINED;
  // handle 'bool'
  if (_prim_map[CT_PrimDeclSpec::PDS_BOOL]) {
    _prim_map[CT_PrimDeclSpec::PDS_BOOL] = 0;
    type = &CTYPE_BOOL;
  }
  // handle '_Bool'
  else if (_prim_map[CT_PrimDeclSpec::PDS_C_BOOL]) {
    _prim_map[CT_PrimDeclSpec::PDS_C_BOOL] = 0;
    type = &CTYPE_C_BOOL;
  }
  // handle 'float'
  else if (_prim_map[CT_PrimDeclSpec::PDS_FLOAT]) {
    _prim_map[CT_PrimDeclSpec::PDS_FLOAT] = 0;
    type = &CTYPE_FLOAT;
  }
  // handle 'double' <= must be handled before 'long'
  else if (_prim_map[CT_PrimDeclSpec::PDS_DOUBLE]) {
    _prim_map[CT_PrimDeclSpec::PDS_DOUBLE] = 0;
    if (_prim_map[CT_PrimDeclSpec::PDS_LONG]) {
      type = &CTYPE_LONG_DOUBLE;
      _prim_map[CT_PrimDeclSpec::PDS_LONG] = 0;
    } else
      type = &CTYPE_DOUBLE;
  }
  // handle 'char'
  else if (_prim_map[CT_PrimDeclSpec::PDS_CHAR]) {
    _prim_map[CT_PrimDeclSpec::PDS_CHAR] = 0;
    if (_prim_map[CT_PrimDeclSpec::PDS_UNSIGNED]) {
      type = &CTYPE_UNSIGNED_CHAR;
      _prim_map[CT_PrimDeclSpec::PDS_UNSIGNED] = 0;
    } else if (_prim_map[CT_PrimDeclSpec::PDS_SIGNED]) {
      type = &CTYPE_SIGNED_CHAR;
      _prim_map[CT_PrimDeclSpec::PDS_SIGNED] = 0;
    } else
      type = &CTYPE_CHAR;
  }
  // handle 'wchar_t'
  else if (_prim_map[CT_PrimDeclSpec::PDS_WCHAR_T]) {
    _prim_map[CT_PrimDeclSpec::PDS_WCHAR_T] = 0;
    type = &CTYPE_WCHAR_T;
  }
  // handle 'short'
  else if (_prim_map[CT_PrimDeclSpec::PDS_SHORT]) {
    _prim_map[CT_PrimDeclSpec::PDS_SHORT] = 0;
    _prim_map[CT_PrimDeclSpec::PDS_INT] = 0;
    if (_prim_map[CT_PrimDeclSpec::PDS_UNSIGNED]) {
      type = &CTYPE_UNSIGNED_SHORT;
      _prim_map[CT_PrimDeclSpec::PDS_UNSIGNED] = 0;
    } else if (_prim_map[CT_PrimDeclSpec::PDS_SIGNED]) {
      type = &CTYPE_SHORT;
      _prim_map[CT_PrimDeclSpec::PDS_SIGNED] = 0;
    } else
      type = &CTYPE_SHORT;
  }
  // handle 'long long'  <= be careful, this must be before 'int'!
  else if (_prim_map[CT_PrimDeclSpec::PDS_LONG] == 2) {
    _prim_map[CT_PrimDeclSpec::PDS_LONG] = 0;
    _prim_map[CT_PrimDeclSpec::PDS_INT] = 0;
    if (_prim_map[CT_PrimDeclSpec::PDS_UNSIGNED]) {
      type = &CTYPE_UNSIGNED_LONG_LONG;
      _prim_map[CT_PrimDeclSpec::PDS_UNSIGNED] = 0;
    } else if (_prim_map[CT_PrimDeclSpec::PDS_SIGNED]) {
      type = &CTYPE_LONG_LONG;
      _prim_map[CT_PrimDeclSpec::PDS_SIGNED] = 0;
    } else
      type = &CTYPE_LONG_LONG;
  }
  // handle 'long'  <= be careful, this must be before 'int'!
  else if (_prim_map[CT_PrimDeclSpec::PDS_LONG] == 1) {
    _prim_map[CT_PrimDeclSpec::PDS_LONG] = 0;
    _prim_map[CT_PrimDeclSpec::PDS_INT] = 0;
    if (_prim_map[CT_PrimDeclSpec::PDS_UNSIGNED]) {
      type = &CTYPE_UNSIGNED_LONG;
      _prim_map[CT_PrimDeclSpec::PDS_UNSIGNED] = 0;
    } else if (_prim_map[CT_PrimDeclSpec::PDS_SIGNED]) {
      type = &CTYPE_LONG;
      _prim_map[CT_PrimDeclSpec::PDS_SIGNED] = 0;
    } else
      type = &CTYPE_LONG;
  }
  // handle 'int'
  else if (_prim_map[CT_PrimDeclSpec::PDS_INT]) {
    _prim_map[CT_PrimDeclSpec::PDS_INT] = 0;
    if (_prim_map[CT_PrimDeclSpec::PDS_UNSIGNED]) {
      type = &CTYPE_UNSIGNED_INT;
      _prim_map[CT_PrimDeclSpec::PDS_UNSIGNED] = 0;
    } else if (_prim_map[CT_PrimDeclSpec::PDS_SIGNED]) {
      type = &CTYPE_INT;
      _prim_map[CT_PrimDeclSpec::PDS_SIGNED] = 0;
    } else
      type = &CTYPE_INT;
  }
  // handle '__int64'
  else if (_prim_map[CT_PrimDeclSpec::PDS_INT64]) {
    _prim_map[CT_PrimDeclSpec::PDS_INT64] = 0;
    if (_prim_map[CT_PrimDeclSpec::PDS_UNSIGNED]) {
      type = &CTYPE_UNSIGNED_LONG_LONG;
      _prim_map[CT_PrimDeclSpec::PDS_UNSIGNED] = 0;
    } else if (_prim_map[CT_PrimDeclSpec::PDS_SIGNED]) {
      type = &CTYPE_LONG_LONG;
      _prim_map[CT_PrimDeclSpec::PDS_SIGNED] = 0;
    } else
      type = &CTYPE_LONG_LONG;
  }
  // handle '__int128'
  else if (_prim_map[CT_PrimDeclSpec::PDS_INT128]) {
    _prim_map[CT_PrimDeclSpec::PDS_INT128] = 0;
    if (_prim_map[CT_PrimDeclSpec::PDS_UNSIGNED]) {
      type = &CTYPE_UNSIGNED_INT128;
      _prim_map[CT_PrimDeclSpec::PDS_UNSIGNED] = 0;
    } else if (_prim_map[CT_PrimDeclSpec::PDS_SIGNED]) {
      type = &CTYPE_INT128;
      _prim_map[CT_PrimDeclSpec::PDS_SIGNED] = 0;
    } else
      type = &CTYPE_INT128;
  }
  // handle 'signed'
  else if (_prim_map[CT_PrimDeclSpec::PDS_SIGNED]) {
    _prim_map[CT_PrimDeclSpec::PDS_SIGNED] = 0;
    type = &CTYPE_INT;
  }
  // handle 'unsigned'
  else if (_prim_map[CT_PrimDeclSpec::PDS_UNSIGNED]) {
    _prim_map[CT_PrimDeclSpec::PDS_UNSIGNED] = 0;
    type = &CTYPE_UNSIGNED_INT;
  }
  // handle 'void'
  else if (_prim_map[CT_PrimDeclSpec::PDS_VOID]) {
    _prim_map[CT_PrimDeclSpec::PDS_VOID] = 0;
    type = &CTYPE_VOID;
  }
  // handle '__unknown_t'
  else if (_prim_map[CT_PrimDeclSpec::PDS_UNKNOWN_T]) {
    _prim_map[CT_PrimDeclSpec::PDS_UNKNOWN_T] = 0;
    type = &CTYPE_UNKNOWN_T;
  }
  else if (_names) {
    _names--;
    if (_name) {
      info = 0;
      if (_name->NodeName () == CT_TemplateName::NodeId ())
        info = _name->Object ();
      else if (_name->NodeName () == CT_QualName::NodeId () ||
               _name->NodeName () == CT_RootQualName::NodeId ()) {
        for (unsigned i = _name->Entries (); i > 0; i--)
          if ((info = ((CT_SimpleName*)_name->Entry (i-1))->Object ()))
            break;
      } else
        info = _name->Name ()->Object ();
      if (info && (info->Record () || info->EnumInfo () || 
                   info->TypedefInfo () || info->TemplateParamInfo ()))
        type = info->TypeInfo ()->Duplicate ();
    }
  }
  else if (_class_specs) {
    _class_specs--;
    if (_name) {
      info = _name->Object ();
      if (info)
        type = CTypeInfo::Duplicate (info->TypeInfo ());
    }
  }
  else if (_union_specs) {
    _union_specs--;
    if (_name) {
      info = _name->Object ();
      if (info)
        type = CTypeInfo::Duplicate (info->TypeInfo ());
    }
  }
  else if (_enum_specs) {
    _enum_specs--;
    if (_name) {
      info = _name->Object ();
      if (info)
        type = CTypeInfo::Duplicate (info->TypeInfo ());
    }
  }
  return type;
}


CTypeInfo *CSemDeclSpecs::create_qualifiers (CTypeInfo *type) {
  // handle 'const', 'volatile' and 'restrict'
  if (_prim_map[CT_PrimDeclSpec::PDS_CONST] ||
      _prim_map[CT_PrimDeclSpec::PDS_VOLATILE] ||
      _prim_map[CT_PrimDeclSpec::PDS_RESTRICT]) {
    if (type->TypeQualified ()) {
      if (_prim_map[CT_PrimDeclSpec::PDS_CONST]) 
        ((CTypeQualified*)type)->isConst (true);
      if (_prim_map[CT_PrimDeclSpec::PDS_VOLATILE]) 
        ((CTypeQualified*)type)->isVolatile (true);
      if (_prim_map[CT_PrimDeclSpec::PDS_RESTRICT]) 
        ((CTypeQualified*)type)->isRestrict (true);
    } else if (type->TypeArray ()) {
      CTypeInfo* base = type->BaseType ();
      if (base->TypeQualified ()) {
        if (_prim_map[CT_PrimDeclSpec::PDS_CONST]) 
          ((CTypeQualified*)base)->isConst (true);
        if (_prim_map[CT_PrimDeclSpec::PDS_VOLATILE]) 
          ((CTypeQualified*)base)->isVolatile (true);
        if (_prim_map[CT_PrimDeclSpec::PDS_RESTRICT]) 
          ((CTypeQualified*)base)->isRestrict (true);
      } else {
        base = new CTypeQualified (base, 
          _prim_map[CT_PrimDeclSpec::PDS_CONST], 
          _prim_map[CT_PrimDeclSpec::PDS_VOLATILE],
          _prim_map[CT_PrimDeclSpec::PDS_RESTRICT]);
        type->BaseType (base);
      }
    } else {
      type = new CTypeQualified (type, 
        _prim_map[CT_PrimDeclSpec::PDS_CONST], 
        _prim_map[CT_PrimDeclSpec::PDS_VOLATILE],
        _prim_map[CT_PrimDeclSpec::PDS_RESTRICT]);
    }
  }
  return type;
}


bool CSemDeclSpecs::check_finally () {
  if (_prim_map[CT_PrimDeclSpec::PDS_CHAR] + 
      _prim_map[CT_PrimDeclSpec::PDS_WCHAR_T] + 
      _prim_map[CT_PrimDeclSpec::PDS_BOOL] + 
      _prim_map[CT_PrimDeclSpec::PDS_C_BOOL] + 
      _prim_map[CT_PrimDeclSpec::PDS_SHORT] + 
      _prim_map[CT_PrimDeclSpec::PDS_INT] + 
      _prim_map[CT_PrimDeclSpec::PDS_LONG] + 
      _prim_map[CT_PrimDeclSpec::PDS_SIGNED] + 
      _prim_map[CT_PrimDeclSpec::PDS_UNSIGNED] + 
      _prim_map[CT_PrimDeclSpec::PDS_FLOAT] + 
      _prim_map[CT_PrimDeclSpec::PDS_DOUBLE] + 
      _prim_map[CT_PrimDeclSpec::PDS_INT64] + 
      _prim_map[CT_PrimDeclSpec::PDS_INT128] + 
      _prim_map[CT_PrimDeclSpec::PDS_UNKNOWN_T] + 
      _prim_map[CT_PrimDeclSpec::PDS_VOID] +
      _names + _class_specs + _union_specs + _enum_specs > 0) {
    *_err << sev_error << _dss->token ()->location () 
          << "invalid type specified in declaration" << endMessage;
    return false;
  }
  return true;
}

} // namespace Puma
