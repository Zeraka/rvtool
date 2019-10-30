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

#include "ClangResultBuffer.h"
#include "clang/AST/ASTContext.h"
#include <sstream>
using std::stringstream;

bool ClangResultBuffer::has_constructor_problem () const {
  return _type->isRecordType() && !_type->isReferenceType();
}

string ClangResultBuffer::tjp_result_type () const {
  stringstream out;
  clang::QualType type = _type;
  if (_type->isVoidType()/* || _type->isUndefined ()*/)
    out << "void";
  else if (type->isReferenceType ()) {
    if (_use_typedef) {
      const clang::ReferenceType *ref_type = clang::dyn_cast<clang::ReferenceType>(type);
      if (ref_type->getPointeeType ().isConstQualified ())
        out << "const ";
      out << "TResult";
    }
    else {
      out << TI_Type::get_type_code_text(type.getNonReferenceType(), &_ast_ctx, 0);
    }
  }
  else
    if (_use_typedef) {
      if (type.isConstQualified ())
        out << "const ";
      out << "TResult";
    }
    else
      out << TI_Type::get_type_code_text(type, &_ast_ctx, 0);
  return out.str ();
}

string ClangResultBuffer::result_type (const string &name, bool unqual) const {
  stringstream out;
  clang::QualType type = _type;
  if (unqual)
    type = type.getLocalUnqualifiedType ();
  if (_type->isVoidType()/* || _type->isUndefined ()*/)
    out << "void";
  else if (type->isReferenceType ()) {
    string ptrname = string ("*") + name;
    if (_use_typedef) {
      const clang::ReferenceType *ref_type = clang::dyn_cast<clang::ReferenceType>(type);
      if (ref_type->getPointeeType ().isConstQualified ())
        out << "const ";
      out << "TResult " << ptrname;
    }
    else {
      out << TI_Type::get_type_code_text(_ast_ctx.getPointerType(type.getNonReferenceType()),
                                    &_ast_ctx,
                                   name.c_str());
    }
  }
  else {
    if (_use_typedef) {
      if (type.isConstQualified())
        out << "const ";
      out << "TResult " << name;
    }
    else {
      out << TI_Type::get_type_code_text(type.getNonReferenceType(), &_ast_ctx, name.c_str());
    }
  }
  return out.str ();
}

string ClangResultBuffer::result_declaration() const {
  stringstream out;
  if (!(_type->isVoidType()/* || _type->isUndefined ()*/)) {
    if (_problem)
      out << "AC::ResultBuffer< " << result_type ("", false) << " > " << result_name();
    else
      out << result_type (result_name(), true);
    out << ";" << endl;
  }
  return out.str ();
}

string ClangResultBuffer::result_assignment(const string &result) const {
  stringstream out;
  if (!(_type->isVoidType()/* || _type->isUndefined ()*/)) {
    if (_problem) {
      out << "::new (&" << result_name() << ") ";
      out << result_type ("", true);
      out << " (";
    }
    else
      out << result_name() << " = ";
    if (_type->isReferenceType ())
      out << "&";
  }
  out << result;
  if (!(_type->isVoidType()/* || _type->isUndefined ()*/))
    if (_problem)
      out << ")";
  return out.str ();
}

string ClangResultBuffer::action_result_assignment(const string &result) const {
  stringstream out;
  if (!(_type->isVoidType()/* || _type->isUndefined ()*/)) {
    if (_problem) {
      out << "::new ((AC::ResultBuffer< ";
      out << result_type ("", false);
      out << " >*)__TJP::result ()) ";
      out << result_type ("", false);
      out << " (";
    }
    else {
      if( workaround_const_result() )
        out << "const_cast<" << result_type( "", false ) <<  " & >( ";
      out << "*__TJP::result()" << ( workaround_const_result() ? " )" : "" ) << " = (" << result_type( "", false ) << ")";
    }
    if (_type->isReferenceType ())
      out << "&";
  }
  out << result;
  if (!(_type->isVoidType()/* || _type->isUndefined ()*/))
    if (_problem)
      out << ")";
  return out.str ();
}

string ClangResultBuffer::result_return() const {
  stringstream out;
  if (!(_type->isVoidType ()/* || _type->isUndefined ()*/)) {
    out << "return ";
    if (_type->isReferenceType ())
      out << "*";
    out << "(";
    out << result_type ("&");
    out << ")" << result_name() << ";" << endl;
  }
  return out.str ();
}
