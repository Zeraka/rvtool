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

#include "MatchName.h"
#include "MatchType.h"
#include "MatchTemplateArg.h"

#include <string.h>
#include <iostream>
using namespace std;

#include "lexertl/rules.hpp"
#include "lexertl/generator.hpp"
#include "lexertl/lookup.hpp"

void MatchName::MRegComp::setup (const string &str) {
  if (_compiled_used) _compiled.clear ();

  const char *sig = str.c_str();
  char *buffer = new char[3 + strlen (sig) * 2];
  char *p = buffer;
  char ch;

  *p++ = '^';
  while (*sig) {
    ch = *sig++;
    switch (ch) {
      case '%':
        *p++ = '.';
        *p++ = '*';
        break;
      default:
        *p++ = ch;
    }
  }
  *p++ = '$';
  *p++ = 0;

  lexertl::rules rules;
  rules.push (buffer, 1); // id is '1', because it is not needed; but != 0
  try {
    lexertl::generator::build(rules, _compiled); // gen. state machine representation
    _compiled_used = true;
  }
  catch (const std::exception &e) {
    _error = true;
  }
  delete[] buffer;
}

bool MatchName::MRegComp::matches (const string &name) const {
  if (!_compiled_used || _error) return false;
  
  string input (name);
  string::const_iterator iter = input.begin ();
  string::const_iterator end = input.end ();
  lexertl::smatch results (iter, end);
  try {
    lexertl::lookup (_compiled, results);
  }
  catch (const std::exception &e) {
    return false; // hm, how to handle an error here?
  }

  return (results.id == 1);
}

// this is used if a match name component is '%' => always true
bool MatchName::MTrue::matches (const string &name) const {
  return true;
}

// efficient string comparison
bool MatchName::MStrComp::matches (const string &name) const {
  return _str == name;
}


void MatchName::Name::make_matcher () {
  // TODO: make more different Matcher variants (prefix/suffix)
  bool only = true; // only '%' characters in the match string
  bool none = true; // no '%' character
  const char *str = _str.c_str ();
  while (*str != '\0') {
    if (*str == '%')
      none = false;
    else
      only = false;
    str++;
  }
  if (only)
    _matcher = new MTrue;
  else if (none)
    _matcher = new MStrComp (_str);
  else
    _matcher = new MRegComp (_str);
}

MatchName::~MatchName () {
  for (int s = 0; s < scopes (); s++)
    if (_scope_template_args[s])
      delete _scope_template_args[s];
  if (_name_template_args)
    delete _name_template_args;
}

MatchName &MatchName::operator = (const MatchName &copy) {
  _scopes = copy._scopes;
  for (int i = 0; i < copy.scopes (); i++)
    if (copy._scope_template_args[i])
      _scope_template_args.push_back (
        new MatchTemplateArgList (*copy._scope_template_args[i]));
    else
      _scope_template_args.push_back (0);
  _name = copy._name;
  if (copy._name_template_args)
    _name_template_args = new MatchTemplateArgList (*copy._name_template_args);
  else
    _name_template_args = 0;
  _oper = copy._oper;
  _conv_type = copy._conv_type;
  return *this;
}

void MatchName::print (ostream &out) const {
  for (int s = 0; s < scopes (); s++) {
    out << scope (s);
    if (_scope_template_args[s])
      out << *_scope_template_args[s];
    out << "::";
  }
  
  if (!_conv_type.is_undefined ())
    out << "operator " << _conv_type;
  else
    out << name ();
    
  if (_name_template_args)
    out << *_name_template_args;
}

// print the 'mangled' string representation of this type
void MatchName::mangle (ostream &os) const {
  // TODO: still incomplete! Template args, ...
  if (scopes ()) {
    os << "N";
    for (int s = 0; s < scopes (); s++) {
      string sn = scope (s);
      if (sn == "<unnamed>")
        sn = "_GLOBAL__N_1";
      os << sn.length () << sn;
      if (_scope_template_args[s])
        _scope_template_args[s]->mangle (os);
    }
    os << "E";
  }
  os << name ().length() << name();
  if (_name_template_args)
    _name_template_args->mangle (os);
}

bool MatchName::scope_matches (vector<Name> &scopes,
  vector<MatchTemplateArgList*> &scope_template_args,
  int match_pos, int sig_pos) {

  Name &name = _scopes[match_pos];
  MatchTemplateArgList *mtal = _scope_template_args[match_pos];
  match_pos--;
  
  static string dots ("...");
  if (name.str () == dots) {
    // if '...' is the start of the qualified name match expression => match
    if (match_pos == -1)
      return true;
    // handle ellipses (brute force :-( )
    while (sig_pos >= 0) {
      if (scope_matches (scopes, scope_template_args, match_pos, sig_pos))
        return true;
      sig_pos--;
    }
    return false;
  }

  // if the argument scope is the global scope it is a mismatch
  if (sig_pos == -1)
    return false;

  // check template arguments of the scope
  if (mtal) {
    MatchTemplateArgList *sig_mtal = scope_template_args[sig_pos];
    if (!sig_mtal || !mtal->matches(*sig_mtal))
      return false;
  }
         
  // compare the name pattern
  if (!name.matches (scopes[sig_pos].str ()))
    return false;

  sig_pos--;
  // check the next scope
  if (match_pos >= 0)
    return scope_matches (scopes, scope_template_args, match_pos, sig_pos);
    
  // no scope to check left, 'scope' must be defined in the global scope
  if (sig_pos != -1)
    return false;
    
  // everything ok
  return true;
}

bool MatchName::oper_matches (Operator oper) {
  static string any ("%");
  
  // '%' as a function name also matches operators
  if (!_name.str ().empty ())
    return _name.str () == any;
      
  // only operator signatures are considered
  if (_oper == OP_UNDEFINED)
    return false;
    
  // any operator
  if (_oper == OP_ANY)
    return true;

  // return true, iff the operator is the same
  return _oper == oper;
}

bool MatchName::conv_matches (MatchTypeRef type) {
  static string any ("%");
  
  // '%' as a function name also matches operators
  if (!_name.str ().empty ())
    return _name.str () == any;
      
  // any operator is ok
  if (_oper == OP_ANY)
    return true;

  // check the conversion type of the function (if it is a conversion function)
  return !type.is_undefined () && !_conv_type.is_undefined () &&
    _conv_type.matches (type);
}

bool MatchName::name_matches (Name &name,
  MatchTemplateArgList *name_template_args) {
  if (_name_template_args) {
    if (!name_template_args ||
        !_name_template_args->matches (*name_template_args))
      return false;
  }
  return _name.matches (name.str ());
}

bool MatchName::matches (MatchName &matched_name) {
  // if the matched name is not defined in the global scope and the match
  // name has no scopes, it is no match
  if (scopes () == 0 && matched_name.scopes () > 0)
    return false;

  // if the match name has a scope, compare it
  if (scopes () > 0 && !scope_matches (matched_name._scopes,
    matched_name._scope_template_args,
    scopes () - 1, matched_name.scopes () - 1))
    return false;
    
  if (matched_name.oper () != OP_UNDEFINED) {
    // this an operator name match expression
    return oper_matches (matched_name.oper ());
  }
  else if (!matched_name.conv_type ().is_undefined ()) {
    // this is a conversion operator match expression
    return conv_matches (matched_name.conv_type ());
  }
  else if (_oper == OP_UNDEFINED && _conv_type.is_undefined ()) {
    // normal name
    return name_matches (matched_name._name, matched_name._name_template_args);
  }
  else
    return false;
}

// check whether the name will match only a single entity
bool MatchName::is_trivial () const {
  // the name should not contain any wildcards
  if (strstr (_name.str ().c_str(), "%") != 0 ||
      strstr (_name.str ().c_str(), "...") != 0)
    return false;
  // the name should not have template parameters, because
  // this could be type pattern
  if (_name_template_args)
    return false;
  // now check all scopes for the same conditions
  for (int i = 0; i < scopes (); i++) {
    const char *scope_name = scope (i).c_str();
    if (strstr (scope_name, "%") != 0 ||
        strstr (scope_name, "...") != 0)
      return false;
    if (_scope_template_args[i])
      return false;
  }
  return true;
}
