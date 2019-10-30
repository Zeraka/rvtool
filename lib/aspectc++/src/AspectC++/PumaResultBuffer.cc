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

#include "PumaResultBuffer.h"
#include "Puma/CTypeInfo.h"
#include <sstream>
using std::stringstream;
using namespace Puma;

bool PumaResultBuffer::has_constructor_problem () const {
  return _type->isRecord () && !_type->isAddress ();
}

string PumaResultBuffer::tjp_result_type () const {
  stringstream out;
  CTypeInfo *type = _type;
  if (type->isVoid () || type->isUndefined ())
    out << "void";
  else if (type->isAddress ()) {
    if (_use_typedef) {
      if (type->BaseType()->isConst())
        out << "const ";
      out << "TResult";
    }
    else
      type->BaseType ()->TypeText (out, "", true, true);
  }
  else
    if (_use_typedef) {
      if (type->isConst())
        out << "const ";
      out << "TResult";
    }
    else
      type->TypeText (out, "", true, true);
  return out.str ();
}

string PumaResultBuffer::result_type (const string &name, bool unqual) const {
  stringstream out;
  CTypeInfo *type = _type;
  if (unqual)
    type = type->UnqualType ();
  if (type->isVoid () || type->isUndefined ())
    out << "void";
  else if (type->isAddress ()) {
    string ptrname = string ("*") + name;
    if (_use_typedef) {
      if (type->BaseType()->isConst())
        out << "const ";
      out << "TResult " << ptrname;
    }
    else {
      if (type->BaseType ()->TypeFunction () || type->BaseType ()->TypeArray ())
        ptrname = string("(") + ptrname + ")";
      type->BaseType ()->TypeText (out, ptrname.c_str (), true, true);
    }
  }
  else
    if (_use_typedef) {
      if (type->isConst())
        out << "const ";
      out << "TResult " << name;
    }
    else
      type->TypeText (out, name.c_str (), true, true);
  return out.str ();
}

string PumaResultBuffer::result_declaration() const {
  stringstream out;
  if (!(_type->isVoid () || _type->isUndefined ())) {
    if (_problem)
      out << "AC::ResultBuffer< " << result_type ("", false) << " > " << result_name();
    else
      out << result_type (result_name(), true);
    out << ";" << endl;
  }
  return out.str ();
}

string PumaResultBuffer::result_assignment(const string &result) const {
  stringstream out;
  if (!(_type->isVoid() || _type->isUndefined ()))  {
    if (_problem) {
      out << "::new (&" << result_name() << ") ";
      out << result_type ("", true);
      out << " (";
    }
    else
      out << result_name() << " = ";
    if (_type->isAddress ())
      out << "&";
  }
  out << result;
  if (!(_type->isVoid() || _type->isUndefined ()))
    if (_problem)
      out << ")";
  return out.str ();
}

string PumaResultBuffer::action_result_assignment(const string &result) const {
  stringstream out;
  if (!(_type->isVoid() || _type->isUndefined ()))  {
    if (_problem) {
      out << "::new ((AC::ResultBuffer< ";
      out << result_type ("", false);
      out << " >*)__TJP::result ()) ";
      out << result_type ("", false);
      out << " (";
    }
    else {
      out << "*__TJP::result () = (" << result_type ("", false) << ")";
    }
    if (_type->isAddress ())
      out << "&";
  }
  out << result;
  if (!(_type->isVoid() || _type->isUndefined ()))
    if (_problem)
      out << ")";
  return out.str ();
}

string PumaResultBuffer::result_return() const {
  stringstream out;
  if (!(_type->isVoid () || _type->isUndefined ())) {
    out << "return ";
    if (_type->isAddress ())
      out << "*";
    out << "(";
    out << result_type ("&");
    out << ")" << result_name() << ";" << endl;
  }
  return out.str ();
}
