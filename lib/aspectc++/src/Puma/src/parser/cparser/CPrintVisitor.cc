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

#include "Puma/CPrintVisitor.h"
#include "Puma/CWStrLiteral.h"
#include "Puma/CStrLiteral.h"
#include "Puma/CConstant.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CObjectInfo.h"
#include "Puma/CStructure.h"
#include "Puma/CScopeInfo.h"
#include "Puma/CTree.h"
#include "Puma/WChar.h"

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>

namespace Puma {


CPrintVisitor::CPrintVisitor () : out (0), indent (0), linecol (0) {
  setPrintLoc(true);
}

CPrintVisitor::~CPrintVisitor () {
  if (linecol)
    delete[] linecol;
}

void CPrintVisitor::setPrintLoc (bool enable, int len) {
  print_loc = len > 0 ? enable : false;
  loc_len = len;
  if (linecol) {
    delete[] linecol;
    linecol = 0;
  }
  if (print_loc)
    linecol = new char[loc_len];
}

void CPrintVisitor::configure (Config& config) {
}

void CPrintVisitor::print (CTree *node, std::ostream &os) {
  out = &os;
  indent = 0;
  visit (node);
}

void CPrintVisitor::pre_visit (CTree *node) {
  if (! node)
    return;

  if (print_loc) {
    Token* t = node->token();
    int n = 0;
    if (t) {
      const Location& l = t->location();
      snprintf(linecol, loc_len, ":%i:%i", l.line(), l.column());
      const char* filename = l.filename().name();
      int avail = loc_len - strlen(linecol);
      int flen = strlen(filename);
      if (flen > avail)
        filename += flen - avail;
      n = flen + (loc_len - avail);
      *out << filename << linecol;
    }
    for (; n < loc_len; n++)
      *out << " ";
    *out << "  ";
  }

  /* warning: On Windows/64bit the cast loses bits! */
  *out << std::hex << std::setfill('0') << std::setw(8)
       << (unsigned long)(size_t)node << " " << std::dec;
  for (int i = 0; i < indent; i++)
    *out << "  ";

  print_node (node);

  *out << "\t### " << node->Sons ();
  if (node->token ())
    *out << " from:" << node->token ()->text ();
  if (node->end_token ())
    *out << " to:" << node->end_token ()->text ();

  CTypeInfo *type = node->Type ();
  CExprValue *value = node->Value ();

  if (type && ! type->is_undefined ()) {
    *out << " (";
    type->TypeText (*out);

    if (value && ! value->WStrLiteral ()) {
      *out << " ";
      if (value->StrLiteral ())
        *out << "\"" << value->StrLiteral ()->String () << "\"";
      else if (type->isInteger ()) {
        if (type->VirtualType ()->is_signed ())
          *out << value->Constant ()->convert_to_int ();
        else
          *out << value->Constant ()->convert_to_uint ();
      } else
        *out << value->Constant ()->convert_to_float ();
    }
    *out << ")";
  }
  *out << std::endl;

  indent++;
}

void CPrintVisitor::post_visit (CTree *node) {
  indent--;
}

void CPrintVisitor::print_node (CTree *node) const {
  if (! node)
    return;

  const char *id = node->NodeName ();
  *out << id;

  if (id == CT_Token::NodeId ()) {
    *out << " \"" << node->token ()->text () << "\"";
  } else if (node->IsCall ()) {
    CObjectInfo *obj = node->IsCall ()->Object ();
    if (obj)
      print_qual_name (obj);
  } else if (id == CT_ConstructExpr::NodeId ()) {
    CObjectInfo *obj = ((CT_ConstructExpr*)node)->Object ();
    if (obj)
      print_qual_name (obj);
  } else if (id == CT_SimpleName::NodeId () ||
             id == CT_PrivateName::NodeId () ||
             id == CT_QualName::NodeId () ||
             id == CT_RootQualName::NodeId () ||
             id == CT_DestructorName::NodeId () ||
             id == CT_OperatorName::NodeId () ||
             id == CT_ConversionName::NodeId ()) {
    *out << " " << *(CT_SimpleName*)node;
  } else if (id == CT_Integer::NodeId ()) {
    CT_Integer *val = (CT_Integer*) node;
    if (val->Value () && val->Value ()->Constant ()) {
      if (val->Value ()->Type ()->VirtualType ()->is_signed ())
        *out << " " << val->Value ()->Constant ()->convert_to_int ();
      else
        *out << " " << val->Value ()->Constant ()->convert_to_uint ();
    }
  } else if (id == CT_Float::NodeId ()) {
    CT_Float *val = (CT_Float*) node;
    if (val->Value () && val->Value ()->Constant ())
      *out << " " << val->Value ()->Constant ()->convert_to_float ();
  } else if (id == CT_Character::NodeId ()) {
    CT_Character *val = (CT_Character*) node;
    if (val->Value () && val->Value ()->Constant ()) {
      if (val->Type ()->VirtualType ()->is_int ())
        *out << " " << (int)val->Value ()->Constant ()->convert_to_int ();
      else
        *out << " " << (char)val->Value ()->Constant ()->convert_to_int ();
    }
  } else if (id == CT_WideCharacter::NodeId ()) {
    CT_Character *val = (CT_Character*) node;
    if (val->Value () && val->Value ()->Constant ()) {
      if (val->Type ()->VirtualType ()->is_int ())
        *out << " " << (int)val->Value ()->Constant ()->convert_to_int ();
      else
        *out << " " << (wchar_t)val->Value ()->Constant ()->convert_to_int ();
    }
  } else if (id == CT_Bool::NodeId ()) {
    CT_Bool *val = (CT_Bool*) node;
    if (val->Value () && val->Value ()->Constant ())
      *out << " " << (val->Value ()->Constant ()->
        convert_to_uint () ? "true" : "false");
  } else if (id == CT_String::NodeId ()) {
    CT_String *val = (CT_String*) node;
    *out << " \"";
    if (val->Value () && val->Value ()->StrLiteral ()) {
      const char *str = val->Value ()->StrLiteral ()->String ();
      unsigned len = strlen (str);
      len = (len > 10) ? 10 : len;
      for (unsigned i = 0; i < len; i++)
        *out << str[i];
      *out << "\"";
      if (strlen (str) > len)
        *out << "...";
      *out << " [" << val->Value ()->StrLiteral ()->Length () << "]";
    } else
      *out << "\"";
  } else if (id == CT_WideString::NodeId ()) {
    CT_WideString *val = (CT_WideString*) node;
    *out << " \"";
    if (val->Value () && val->Value ()->WStrLiteral ()) {
      const wchar_t *str = val->Value ()->WStrLiteral ()->String ();
      unsigned len = wcslen (str);
      len = (len > 10) ? 10 : len;
      for (unsigned i = 0; i < len; i++)
        *out << (char)str[i];
      *out << "\"";
      if (wcslen (str) > len)
        *out << "...";
      *out << " [" << val->Value ()->WStrLiteral ()->Length () << "]";
    } else
      *out << "\"";
  }
}

void CPrintVisitor::print_qual_name (CObjectInfo *oinfo) const {
  Array<const char*> scopes;
  CObjectInfo *info;

  *out << " ";

  info = oinfo->AssignedScope ();
  if (! info) info = oinfo->Scope ();
  while (info && ! info->isAnonymous () &&
         (info->Record () || (info->NamespaceInfo () && ! info->FileInfo ()))) {
    scopes.append (info->Name ());
    info = info->Scope ();
  }

  for (long i = scopes.length () - 1; i >= 0; i--)
    *out << scopes.lookup (i) << "::";
  *out << oinfo->Name ();
}


} // namespace Puma
