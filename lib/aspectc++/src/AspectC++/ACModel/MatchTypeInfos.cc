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

#include <string>
using std::string;
#include <sstream>
using std::ostringstream;

#include "MatchTypeInfos.h"

// comparison functions
  
// is it the same kind of type?
bool MatchType::operator ==(const MatchType &other) const {
  // by default two types are equal if they have the same ID
  return type () == other.type ();
}

MTUndefined *MTUndefined::clone () const {
  return new MTUndefined(*this);
}

void MTUndefined::print (ostream &out, const char *inner, bool prefix) const {
  out << "<undefined>";
  if (inner)
    out << " " << inner;
}

bool MTUndefined::matches (const MatchType &type) const {
  // conversion operator might have an undefined result type
  return type == *this; // true if both object types are equal
}

// print the string representation of this qualified type
void MTQual::print_qual (ostream &out) const {
  if (_const) out << "const";
  if (_const && _volatile) out << " ";
  if (_volatile) out << "volatile";
}

// perform a type match
// if a type is qualified as const/volatile in a match expression, it does
// only match types which are qualified like this. However, according to
// the AspectC++ semantics a type is also matched if it is const/volatile,
// but the match expression does not require this qualifier.
bool MTQual::qual_matches (const MTQual *qual) const {
  if (!qual && is_qualified ())
    return false;
  if (qual) {
    if (_const && !qual->is_const ())
        return false;
    if (_volatile && !qual->is_volatile ())
        return false;
  }
  return true;
}

// print the 'mangled' string representation of this type
void MTQual::mangle (ostream &os) const {
  if (_const)
    os << "K";
  if (_volatile)
    os << "V";
}

// print the string representation of this primitive type
// inner might be a name, prefix == true means that the name has prefix
void MTPrim::print (ostream &out, const char *inner, bool prefix) const {
  if (is_qualified ()) {
    print_qual (out);
    out << " ";
  }
  out << type ();
  if (inner)
    out << " " << inner;
}

// print the 'mangled' string representation of this type
void MTPrim::mangle (ostream &os) const {
  MTQual::mangle (os);
  // default mangling:
  string name (type ());
  os << name.length() << name;
}

MTNamed *MTNamed::clone () const {
  return new MTNamed (*this);
}

// print the string representation of this names type
// inner might be a name, prefix == true means that the name has prefix
void MTNamed::print (ostream &out, const char *inner, bool prefix) const {
  if (is_qualified ()) {
    print_qual (out);
    out << " ";
  }
  out << _name;
  if (inner)
    out << " " << inner;
}

// print the 'mangled' string representation of this type
void MTNamed::mangle (ostream &os) const {
  MTQual::mangle (os);
  _name.mangle (os);
}

bool MTNamed::matches (const MatchType &type) const {
  if (type != *this)
    return false;
  MTNamed &matched_type = (MTNamed&)type;
  return _name.matches (matched_type._name) &&
    qual_matches (matched_type.qualified ());
}


MTPointer *MTPointer::clone () const {
  return new MTPointer (*this);
}

// print the string represenation of this pointer type
// inner might be a name, prefix == true means that the name has prefix
void MTPointer::print (ostream &out, const char *inner, bool prefix) const {
  ostringstream declarator;
  declarator << "*";
  if (is_qualified ()) {
    declarator << " ";
    print_qual (declarator);
    declarator << " ";
  }
  if (inner)
    declarator << inner;
  if (base ())
    base ()->print (out, declarator.str ().c_str (), true);
  else
    out << "<any> " << declarator.str ();
}

// print the 'mangled' string representation of this type
void MTPointer::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "P";
  if (base ())
    base()->mangle(os);
}

bool MTPointer::matches (const MatchType &type) const {
  if (type != *this)
    return false;
  MTPointer &matched_type = (MTPointer&)type;
  return base ()->matches (*matched_type.base ()) &&
    qual_matches (matched_type.qualified ());
}


MTMembPointer *MTMembPointer::clone () const {
  return new MTMembPointer (*this);
}

// print the string represenation of this pointer type
// inner might be a name, prefix == true means that the name has prefix
void MTMembPointer::print (ostream &out, const char *inner, bool prefix) const {
  ostringstream declarator;
  declarator << _memb_ptr_scope << "*";
  if (is_qualified ()) {
    declarator << " ";
    print_qual (declarator);
    declarator << " ";
  }
  if (inner)
    declarator << inner;
  if (base ())
    base ()->print (out, declarator.str ().c_str (), true);
  else
    out << "<any> " << declarator.str ();
}

// print the 'mangled' string representation of this type
void MTMembPointer::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "M";
  _memb_ptr_scope.mangle (os);
  if (base ())
    base()->mangle(os);
}

bool MTMembPointer::matches (const MatchType &type) const {
  if (type != *this)
    return false;
  MTMembPointer &matched_type = (MTMembPointer&)type;
  return base ()->matches (*matched_type.base ()) &&
    qual_matches (matched_type.qualified ()) &&
    _memb_ptr_scope.matches (matched_type._memb_ptr_scope);
}

MTReference *MTReference::clone () const {
  return new MTReference (*this);
}

// print the string represenation of this reference type
// inner might be a name, prefix == true means that the name has prefix
void MTReference::print (ostream &out, const char *inner, bool prefix) const {
  ostringstream declarator;
  declarator << "&";
  if (inner)
    declarator << inner;
  if (base ())
    base ()->print (out, declarator.str ().c_str (), true);
  else
    out << "<any> " << declarator.str ();
}

// print the 'mangled' string representation of this type
void MTReference::mangle (ostream &os) const {
  os << "R";
  if (base ())
    base()->mangle(os);
}

bool MTReference::matches (const MatchType &type) const {
  return type == *this && base ()->matches (*type.base ());
}

//MTFunction::~MTFunction () {
//  for (int a = 0; a < args (); a++)
//    delete arg (a);
//}

MTFunction *MTFunction::clone () const {
  return new MTFunction (*this);
}

// print the string represenation of this function type
// inner might be a name, prefix == true means that the name has prefix
void MTFunction::print (ostream &out, const char *inner, bool prefix) const {
  ostringstream declarator;
  if (inner)
    declarator << (prefix ? "(" : "") << inner << (prefix ? ")" : "");
  declarator << "(";
  int a = 0;
  while (a < args ()) {
    if (a > 0) declarator << ",";
    arg (a).type_info ()->print (declarator);
    a++;    
  }
  if (_varargs) declarator << (a > 0 ? "," : "") << "...";
  declarator << ")";
  if (is_qualified ()) {
    declarator << " ";
    print_qual (declarator);
    declarator << " ";
  }
  if (base ())
    base ()->print (out, declarator.str ().c_str (), false);
  else
    out << "<any> " << declarator.str ();
}

// print the 'mangled' string representation of this type
void MTFunction::mangle (ostream &os) const {
  os << "F";
  // TODO: what about the scope here? Puma prints it.
  if (base ())
    base ()->mangle (os);
  if (!_varargs && args () > 0) {
    for (int a = 0; a < args (); ++a)
      arg(a).type_info()->mangle (os);
  }
  else
    os << "v";
  os << "E";
}

// perform a type match
bool MTFunction::matches (const MatchType &type) const {
  // check if it's a function type
  if (type != *this)
    return false;
  const MTFunction &matched_type = (MTFunction&)type;
  // check the type qualifier
  if (!qual_matches (matched_type.qualified ()))
    return false;
  // check the argument types
  int matched_args = matched_type.args ();
  // TODO: what about matched functions with varargs?
  if ((!_varargs && args () != matched_args) || (args () > matched_args))
    return false;
  for (int a = 0; a < args (); a++)
    if (!arg (a).matches (matched_type.arg (a)))
      return false;
  // finally check the result type
  return base ()->matches (*matched_type.base ());  
}

// adjust the argument types according to �8.3.5.2 and �8.3.5.3 of ISO C++
void MTFunction::adjust_args (bool &f, bool &a, bool &q, bool &v) {
  // adjust all argument types according to ISO C++
  for (int i = 0; i < args (); i++) {
    MatchType *curr = _arg_types[i].type_info ();
    
    // recursively adjust the argument
    curr->adjust_args (f, a , q, v);
    
    // type `function returning T' is adjusted to be
    // `pointer to function returning T'
    if (_arg_types[i].is_function ()) {
      f = true;
      _arg_types[i].to_pointer ();
      continue;
    }
    
    // type `array of T' is adjusted to be `pointer to T'
    if (_arg_types[i].is_array ()) {
      a = true;
      MatchTypeRef converted_type = _arg_types[i].base ();
      converted_type.to_pointer();
      _arg_types[i] = converted_type;
      continue;
    }
    
    // any cv-qualifier modifying a parameter is deleted
    if (_arg_types[i].is_qualifiable () &&
        _arg_types[i].qualifiers ().is_qualified ()) {
      q = true;
      _arg_types[i].qualifiers ().unqualify ();
    }
  }
  
  // �8.3.5.2 the parameter list `(void)' is equivalent to
  // the empty parameter list
  if (args () == 1 && _arg_types[0].is_void ()) {
    v = true;
    _arg_types.clear ();
  }
}

MTArray *MTArray::clone () const {
  return new MTArray (*this);
}

// print the string represenation of this array type
// inner might be a name, prefix == true means that the name has prefix
void MTArray::print (ostream &out, const char *inner, bool prefix) const {
  ostringstream declarator;
  if (inner)
    declarator << (prefix ? "(" : "") << inner << (prefix ? ")" : "");
  declarator << "[";
  if (_any_size)
    declarator << "%";
  else
    declarator << _dim;
  declarator << "]";
  if (base ())
    base ()->print (out, declarator.str ().c_str (), false);
  else
    out << "<any> " << declarator.str ();
}

// print the 'mangled' string representation of this type
void MTArray::mangle (ostream &os) const {
  os << "A";
  if (!_any_size)
    os << dimension();
  os << "_";
  if (base ())
    base ()->mangle (os);
}

// perform a type match
bool MTArray::matches (const MatchType &type) const {
  if (type != *this)
    return false;
  MTArray &matched_type = (MTArray&)type;
  return base ()->matches (*matched_type.base ()) &&
    (_any_size || _dim == matched_type._dim);
}

MTAny *MTAny::clone () const {
  return new MTAny (*this);
}
// perform a type match
bool MTAny::matches (const MatchType &type) const {
  return qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTAny::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "3Any";
}

MTBool *MTBool::clone () const {
  return new MTBool (*this);
}

// perform a type match
bool MTBool::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTBool::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "b";
}

MTSignedChar *MTSignedChar::clone () const {
  return new MTSignedChar (*this);
}
// perform a type match
bool MTSignedChar::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTSignedChar::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "a";
}

MTUnsignedChar *MTUnsignedChar::clone () const {
  return new MTUnsignedChar (*this);
}
// perform a type match
bool MTUnsignedChar::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTUnsignedChar::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "h";
}

MTChar *MTChar::clone () const {
  return new MTChar (*this);
}
// perform a type match
bool MTChar::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTChar::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "c";
}

MTUnsignedShort *MTUnsignedShort::clone () const {
  return new MTUnsignedShort (*this);
}
// perform a type match
bool MTUnsignedShort::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTUnsignedShort::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "t";
}

MTShort *MTShort::clone () const {
  return new MTShort (*this);
}
// perform a type match
bool MTShort::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTShort::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "s";
}

MTUnsignedInt *MTUnsignedInt::clone () const {
  return new MTUnsignedInt (*this);
}
// perform a type match
bool MTUnsignedInt::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTUnsignedInt::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "j";
}

MTInt *MTInt::clone () const {
  return new MTInt (*this);
}
// perform a type match
bool MTInt::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTInt::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "i";
}

MTWCharT *MTWCharT::clone () const {
  return new MTWCharT (*this);
}
// perform a type match
bool MTWCharT::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTWCharT::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "w";
}

MTUnsignedLong *MTUnsignedLong::clone () const {
  return new MTUnsignedLong (*this);
}
// perform a type match
bool MTUnsignedLong::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTUnsignedLong::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "m";
}

MTLong *MTLong::clone () const {
  return new MTLong (*this);
}
// perform a type match
bool MTLong::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTLong::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "l";
}

MTUnsignedLongLong *MTUnsignedLongLong::clone () const {
  return new MTUnsignedLongLong (*this);
}
// perform a type match
bool MTUnsignedLongLong::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTUnsignedLongLong::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "y";
}

MTLongLong *MTLongLong::clone () const {
  return new MTLongLong (*this);
}
// perform a type match
bool MTLongLong::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTLongLong::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "x";
}

MTUnsignedInt128 *MTUnsignedInt128::clone () const {
  return new MTUnsignedInt128 (*this);
}
// perform a type match
bool MTUnsignedInt128::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTUnsignedInt128::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "o";
}

MTInt128 *MTInt128::clone () const {
  return new MTInt128 (*this);
}
// perform a type match
bool MTInt128::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTInt128::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "n";
}

MTFloat *MTFloat::clone () const {
  return new MTFloat (*this);
}
// perform a type match
bool MTFloat::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTFloat::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "f";
}

MTDouble *MTDouble::clone () const {
  return new MTDouble (*this);
}
// perform a type match
bool MTDouble::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTDouble::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "d";
}

MTLongDouble *MTLongDouble::clone () const {
  return new MTLongDouble (*this);
}
// perform a type match
bool MTLongDouble::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTLongDouble::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "e";
}

MTVoid *MTVoid::clone () const {
  return new MTVoid (*this);
}
// perform a type match
bool MTVoid::matches (const MatchType &type) const {
  return type == *this && qual_matches (type.qualified ());
}
// print the 'mangled' string representation of this type
void MTVoid::mangle (ostream &os) const {
  MTQual::mangle (os);
  os << "v";
}

// type ids
MatchType::TID MTUndefined::_tid = "<undefined>";
MatchType::TID MTUndefined::type () const { return _tid; }
MatchType::TID MTDeclarator::_tid = "<declarator>";
MatchType::TID MTDeclarator::type () const { return _tid; }
MatchType::TID MTReference::_tid = "&";
MatchType::TID MTReference::type () const { return _tid; }
MatchType::TID MTPointer::_tid = "*";
MatchType::TID MTPointer::type () const { return _tid; }
MatchType::TID MTMembPointer::_tid = "::*";
MatchType::TID MTMembPointer::type () const { return _tid; }
MatchType::TID MTFunction::_tid = "()";
MatchType::TID MTFunction::type () const { return _tid; }
MatchType::TID MTArray::_tid = "[]";
MatchType::TID MTArray::type () const { return _tid; }
MatchType::TID MTAny::_tid = "%";
MatchType::TID MTAny::type () const { return _tid; }
MatchType::TID MTNamed::_tid = "<named>";
MatchType::TID MTNamed::type () const { return _tid; }
MatchType::TID MTBool::_tid = "bool";
MatchType::TID MTBool::type () const { return _tid; }
MatchType::TID MTSignedChar::_tid = "signed char";
MatchType::TID MTSignedChar::type () const { return _tid; }
MatchType::TID MTUnsignedChar::_tid = "unsigned char";
MatchType::TID MTUnsignedChar::type () const { return _tid; }
MatchType::TID MTChar::_tid = "char";
MatchType::TID MTChar::type () const { return _tid; }
MatchType::TID MTUnsignedShort::_tid = "unsigned short";
MatchType::TID MTUnsignedShort::type () const { return _tid; }
MatchType::TID MTShort::_tid = "short";
MatchType::TID MTShort::type () const { return _tid; }
MatchType::TID MTUnsignedInt::_tid = "unsigned int";
MatchType::TID MTUnsignedInt::type () const { return _tid; }
MatchType::TID MTInt::_tid = "int";
MatchType::TID MTInt::type () const { return _tid; }
MatchType::TID MTWCharT::_tid = "wchar_t";
MatchType::TID MTWCharT::type () const { return _tid; }
MatchType::TID MTUnsignedLong::_tid = "unsigned long";
MatchType::TID MTUnsignedLong::type () const { return _tid; }
MatchType::TID MTLong::_tid = "long";
MatchType::TID MTLong::type () const { return _tid; }
MatchType::TID MTUnsignedLongLong::_tid = "unsigned long long";
MatchType::TID MTUnsignedLongLong::type () const { return _tid; }
MatchType::TID MTLongLong::_tid = "long long";
MatchType::TID MTLongLong::type () const { return _tid; }
MatchType::TID MTUnsignedInt128::_tid = "unsigned __int128";
MatchType::TID MTUnsignedInt128::type () const { return _tid; }
MatchType::TID MTInt128::_tid = "__int128";
MatchType::TID MTInt128::type () const { return _tid; }
MatchType::TID MTFloat::_tid = "float";
MatchType::TID MTFloat::type () const { return _tid; }
MatchType::TID MTDouble::_tid = "double";
MatchType::TID MTDouble::type () const { return _tid; }
MatchType::TID MTLongDouble::_tid = "long double";
MatchType::TID MTLongDouble::type () const { return _tid; }
MatchType::TID MTVoid::_tid = "void";
MatchType::TID MTVoid::type () const { return _tid; }
