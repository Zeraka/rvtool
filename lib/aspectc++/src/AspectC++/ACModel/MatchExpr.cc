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

#include "MatchExpr.h"
#include <string.h>

// parse a match expression string
// complain about errors => result false
bool MatchSignature::parse (const string &str) {
  const char *strptr = str.c_str();
  if( str == "::" ) { // special case for root namespace
    _state = NORMAL;
    _expr_type = TYPE;
    MatchName n;
    n.name( "::" );
    _type = MatchTypeRef::make_named( n );
    return true;
  }
  else if (parse_match_expr (strptr)) {
    // perform argument conversions, warn if any conversion are applied
    bool f = false, a = false, q = false, v = false;
    _type.adjust_args (f, a, q, v);
    _state = NORMAL;
    return true;
  }
  else
    return false;
}

// internal parser functions that analyse a match expr string
bool MatchSignature::parse_match_expr (const char *&str) {
  // empty strings are no valid match expressions
  if (*str == '\0') {
    _state = ERR_EMPTY_MATCH_EXPR;
    return false;
  }

  // parse as a C++ declaration
  MatchTypeRef type;
  if (!parse_declaration (str, false, type))
    return false;

  // check for tokens after the end of the accepted input
  if (*str != '\0') {
    _state = ERR_TOKENS_AFTER_END;
    return false;
  }

  _expr_type = TYPE;
  if (!_name.undefined ())
    _expr_type = (type.is_function () ? FUNCTION : ATTRIBUTE);

  _type = type; // everything is ok, the parsed type is copied

  return true;
}

bool MatchSignature::parse_declaration (const char *&str, bool abstract,
                                   MatchTypeRef &type) {
  // decl_spec_seq_opt declarator_opt
  FctSpec fct_spec;
  Spec spec;
  parse_decl_spec_seq( str, type, fct_spec, spec );
  if (error ())
    return false;
    
  skip_blanks (str);
  if (*str == '\0')
    return true;

  parse_declarator (str, abstract, type);
  if (error ())
    return false;
    
  // function specifiers are only valid in conjunctions with function types
  if (!type.is_function() && fct_spec) {
    _state = ERR_INVALID_SPEC;
    return false;
  }
  
  // function specifier in type that appear in argument lists etc. are ignored.
  // Top-level function specifiers are saved for matching functions
  if (!abstract) {
    _fct_spec = fct_spec;
    _spec = spec;
  }
    
  return true;
}

bool MatchSignature::parse_decl_spec_seq (const char *&str, MatchTypeRef &type,
  FctSpec &fs, Spec &s ) {

  DeclSpecs ds; // semantic knowledge about the decl spec seq
  
  // parse a normal sequence of declaration specifiers
  bool empty = true;
  while (parse_decl_spec (str, ds))
    empty = false;

  // no problem if the sequence is empty -> yields UNDEFINED
  if (empty)
    return true;
    
  // now generate the match type for this decl spec sequence
  if (!ds._name.undefined ()) {
    // a named type ...

    // check for the any-type wildcard first
    if (ds._name.scopes () == 0 && ds._name.name () == string("%")) {
      if (_wildcards_allowed)
        type = MatchTypeRef::make_any ();
      else {
        _state = ERR_WILDCARD;
        return false;
      }
    }
    else
      type = MatchTypeRef::make_named (ds._name);
  }
  else {
    // a builtin type ...
    
    // if no type is given 'int' is the default
    if (ds._type == PRIM_NONE)
      ds._type = PRIM_INT;
      
    if (ds._sign < 0) {
      // a signed type ...
      if (ds._type == PRIM_CHAR && ds._size == 0)
        type = MatchTypeRef::make_signed_char ();
      else if (ds._type == PRIM_INT) {
        if (ds._size == -1)
          type = MatchTypeRef::make_short ();
        else if (ds._size == 0)
          type = MatchTypeRef::make_int ();
        else if (ds._size == 1)
          type = MatchTypeRef::make_long ();
        else if (ds._size == 2)
          type = MatchTypeRef::make_long_long ();
      }
    }
    else if (ds._sign > 0) {
      // an unsigned type
      if (ds._type == PRIM_CHAR && ds._size == 0)
        type = MatchTypeRef::make_unsigned_char ();
      else if (ds._type == PRIM_INT) {
        if (ds._size == -1)
          type = MatchTypeRef::make_unsigned_short ();
        else if (ds._size == 0)
          type = MatchTypeRef::make_unsigned_int ();
        else if (ds._size == 1)
          type = MatchTypeRef::make_unsigned_long ();
        else if (ds._size == 2)
          type = MatchTypeRef::make_unsigned_long_long ();
      }
      else if (ds._type == PRIM_INT128 && ds._size == 0)
        type = MatchTypeRef::make_unsigned_int128 ();
    }
    else {
      // neither defined as signed nor unsigned
      if (ds._type == PRIM_BOOL && ds._size == 0)
        type = MatchTypeRef::make_bool ();
      else if (ds._type == PRIM_CHAR && ds._size == 0)
        type = MatchTypeRef::make_char ();
      else if (ds._type == PRIM_INT) {
        if (ds._size == -1)
          type = MatchTypeRef::make_short ();
        else if (ds._size == 0)
          type = MatchTypeRef::make_int ();
        else if (ds._size == 1)
          type = MatchTypeRef::make_long ();
        else if (ds._size == 2)
          type = MatchTypeRef::make_long_long ();
      }
      else if (ds._type == PRIM_INT128 && ds._size == 0)
        type = MatchTypeRef::make_int128 ();
      else if (ds._type == PRIM_WCHAR_T && ds._size == 0)
        type = MatchTypeRef::make_wchar_t ();
      else if (ds._type == PRIM_FLOAT && ds._size == 0)
        type = MatchTypeRef::make_float ();
      else if (ds._type == PRIM_DOUBLE) {
        if (ds._size == 0)
          type = MatchTypeRef::make_double ();
        else if (ds._size == 1)
          type = MatchTypeRef::make_long_double ();
      }
      else if (ds._type == PRIM_VOID && ds._size == 0)
        type = MatchTypeRef::make_void ();
    }
    
  }
  
  // add const or volatile qualifiers to the type
  if (type.is_qualifiable ()) {
    if (ds._qual & QUAL_CONST) type.qualifiers ().qualify_const ();
    if (ds._qual & QUAL_VOLATILE) type.qualifiers ().qualify_volatile ();
  }

  if (type.is_undefined ()) {
    _state = ERR_INVALID_TYPE;
    return false;
  }
  
  // also return the parsed function specifiers
  fs = ds._fct_spec;
  s = ds._spec;
  
  return true;
}

bool MatchSignature::parse_decl_spec (const char *&str, DeclSpecs &ds) {
  // remember the current value of this flag as it might be changed
  bool name_allowed = ds._allow_name;
  
  // first skip any blank characters
  skip_blanks (str);

  // check for const or volatile
  CVQualifier qual = QUAL_NONE;
  if (next_word ("const", str))
    qual = QUAL_CONST;
  else if (next_word ("volatile", str))
    qual = QUAL_VOLATILE;
  if (qual) {
    if (ds._qual & qual) {
      _state = ERR_DUP_CV;
      return false;
    }
    ds._qual = (CVQualifier)(ds._qual | qual); // remember this qual
    return true;
  }
  
  // check for function specifiers; currently only virtual
  FctSpec fct_spec = FCT_NONE;
  Spec spec = DT_NONE;
  if (next_word ("virtual", str))
    fct_spec = FCT_VIRTUAL;
  else if (next_word ("static", str))
    spec = DT_STATIC;

  if( fct_spec || spec ) {
    if( ( ds._fct_spec & fct_spec ) || ( ds._spec & spec ) ) {
      _state = ERR_DUP_SPEC;
      return false;
    }
    else if (((ds._spec & DT_STATIC) && fct_spec == FCT_VIRTUAL) ||
        ((ds._fct_spec & FCT_VIRTUAL) && spec == DT_STATIC)) {
      _state = ERR_VIRT_STAT_FCT_SPEC;
      return false;
    }
    ds._fct_spec = (FctSpec)(ds._fct_spec | fct_spec); // remember this fct spec
    ds._spec = (Spec)(ds._spec | spec); // remember this spec
    return true;
  }
  
  ds._allow_name = false; // the following checked decl specs indicate a type.
                          // Therefore, no type name is allowed from now

  // check for C++ primitive types
  bool is_prim = true;
  if (next_word ("char", str))
    ds._type = PRIM_CHAR;
  else if (next_word ("int", str))
    ds._type = PRIM_INT;
  else if (next_word ("__int128", str))
    ds._type = PRIM_INT128;
  else if (next_word ("void", str))
    ds._type = PRIM_VOID;
  else if (next_word ("bool", str))
    ds._type = PRIM_BOOL;
  else if (next_word ("float", str))
    ds._type = PRIM_FLOAT;
  else if (next_word ("double", str))
    ds._type = PRIM_DOUBLE;
  else if (next_word ("wchar_t", str))
    ds._type = PRIM_WCHAR_T;
  else
    is_prim = false;
  if (is_prim)
    return true;

  // check for 'short' and 'long'
  int size = 0;
  if (next_word ("short", str))
    size = -1;
  else if (next_word ("long", str))
    size = 1;
  if (size != 0) {
    if ((ds._size < 0 && size > 0) || (ds._size > 0 && size < 0)) {
      _state = ERR_CONF_SIZE;
      return false;
    }
    ds._size += size;
    return true;
  }
  
  // check for 'signed' and 'unsigned'
  int sign = 0;
  if (next_word ("signed", str))
    sign = -1;
  else if (next_word ("unsigned", str))
    sign = 1;
  if (sign != 0) {
    if (ds._sign != 0) {
      _state = ERR_DUP_SIGN;
      return false;
    }
    ds._sign = sign;
    return true;
  }
  
  // check if this is a valid (qualified) name
  if (name_allowed) {
    const char *saved_pos = str;
    MatchName type_name;
    const char *curr = str;
    if (!parse_nested_name (curr, type_name))
      if (error ())
        return false;
    skip_blanks (curr);
    const char *name = curr;
    if (strncmp (curr, "<unnamed>", 9) == 0) {
      str = curr + 9;
      type_name.name ("<unnamed>");
      ds._name = type_name;
      return true;
    }
    while (is_id (*curr))
      curr++;
    if (curr != name) {
      str = curr;
      string ns (name, curr - name);
      // operator names are no type names
      if (ns == "operator") {
        str = saved_pos;
        return false;
      }
      else {
        type_name.name (ns);
        
        // are there any template parameters?
        MatchTemplateArgList *mtal;
        if (!parse_opt_template_argument_list (str, mtal))
          if (error ())
            return false;

        type_name.template_args (mtal);
        ds._name = type_name;
        return true;
      }
    }
  }
  
  // no valid declaration specifier
  return false;
}

bool MatchSignature::parse_nested_name (const char *&str, MatchName &match_name) {
  bool have_nested_name = false;
  while (parse_nested_name_elem (str, match_name))
    have_nested_name = true;
  return have_nested_name;
}

bool MatchSignature::parse_nested_name_elem (const char *&str, MatchName &match_name) {
  skip_blanks (str);
  // check for ellipses first
  if (str[0] == '.') {
    const char *curr = str;
    if (str[1] != '.' || str[2] != '.') {
      _state = ERR_INVALID_DOT;
      return false;
    }
    curr += 3;
    skip_blanks (curr);
    if (curr[0] != ':' || curr[1] != ':') {
      _state = ERR_INVALID_DOT;
      return false;
    }
    str = curr + 2; // consume '... ::'
    match_name.add_scope ("...");
    return true;
  }

  const char *curr = str;
  const char *str_end = 0;
  MatchTemplateArgList *mtal = 0;
  // an anonymous namespace?
  if (strncmp (str, "<unnamed>", 9) == 0) {
    curr += 9;
    str_end = curr;
  }
  else {
    // a class or namespace name?
    while (is_id (*curr))
      curr++;
    if (curr == str || (curr - str == 8 && strncmp (str, "operator", 8) == 0))
      return false;
    str_end = curr;

    // are there any template parameters?
    if (!parse_opt_template_argument_list (curr, mtal))
      if (error ())
        return false;
  }
  skip_blanks (curr);
  if (curr[0] != ':' || curr[1] != ':')
    return false;
  string scope_name (str, str_end - str);
  match_name.add_scope (scope_name, mtal);
  str = curr + 2;
  return true;
}

bool MatchSignature::parse_opt_template_argument_list (const char *&str,
                                                   MatchTemplateArgList *&mtal) {
  skip_blanks (str);
  mtal = 0;
  
  // check if the list starts with '<'  
  if (str[0] != '<' || strncmp (str, "<unnamed>", 9) == 0)
    return false;
  str++;
  
  mtal = new MatchTemplateArgList;
  if (!parse_template_argument_list (str, *mtal)) {
    if (error ()) {
      delete mtal;
      return false;
    }
  }
  if (*str != '>') {
    _state = ERR_INVALID_ARG_LIST;
    return false;
  }
  str++;
  return true;
}

bool MatchSignature::parse_template_argument_list (const char *&str,
                                               MatchTemplateArgList &mtal) {
  MatchTemplateArg *mta;
  if (!parse_template_argument (str, mta))
    return !error ();
  mtal.append (mta);
  skip_blanks (str);  
  
  while (str[0] == ',') {
    str++;
    if (!parse_template_argument (str, mta)) {
      _state = ERR_INVALID_ARG_LIST;
      return false;
    }
    mtal.append (mta);
    skip_blanks (str);
  }
  return true;
}

bool MatchSignature::parse_template_argument (const char *&str,
                                          MatchTemplateArg *&mta) {
  mta = 0;
  skip_blanks (str);

  // check for end of list
  if (str[0] == '>')
    return false;

  // first check for ellipses
  if (str[0] == '.') {
    if (str[1] == '.' && str[2] == '.') {
      str += 3;
      mta = new MTA_Ellipses;
      return true;
    }
    else {
      _state = ERR_INVALID_DOT;
      return false;
    }
  }
  
  // now check for 'any'
  if (str[0] == '%') {
    if (!_wildcards_allowed) {
      _state = ERR_WILDCARD;
      return false;
    }
    const char *curr = str;
    curr++;
    skip_blanks (curr);
    if (*curr == ',' || *curr == '>') {
      str = curr;
      mta = new MTA_Any;
      return true;
    }
  }

  // check for static function/object address as template arg
  if (str[0] == '&') {
    const char *curr = str;
    curr++;
    skip_blanks (curr);
    MatchName name;
    if (!parse_declarator_id (curr, name))
      return false;
    skip_blanks (curr);
    if (*curr == ',' || *curr == '>') {
      str = curr;
      mta = new MTA_Addr (name);
      return true;
    }
    else
      return false;
  }

  // check for an expression
  // TODO: decimal literals are not enough
  long long val;
  if (parse_dec_literal (str, val)) {
    mta = new MTA_Value (val);
    return true;
  }
  
  // check for a type parameter
  MatchTypeRef arg_type;
  if (!parse_declaration (str, true, arg_type))
    return false;

  mta = new MTA_Type (arg_type); // arg type will be copied
  return true;
}

/*bool MatchSignature::parse_dec_string_literal(const char *&str, std::string &val) {
  skip_blanks (str);

  const char *currStr = str;
  bool strLitFound = false;

  while(*currStr == '\\' && currStr[1] == '"') {
  strLitFound = true;
  currStr += 2;
  while(*currStr != '\\' || currStr[1] != '"') {
    val += *currStr;
    currStr++;
  }
  currStr += 2;
  skip_blanks(currStr);
  }

  str = currStr;
  return strLitFound;
}
*/
bool MatchSignature::parse_dec_literal (const char *&str, long long &val) {
  skip_blanks (str);
  long long sign = 1;
  
  if (*str == '-') {
    sign = -1;
    str++;
    skip_blanks (str);
  }
  
  if (!(*str >= '0' && *str <= '9'))
    return false;
    
  val = 0;
  while (*str >= '0' && *str <= '9') {
    val = 10LL * val + (long long)(*str - '0');
    str++;
  }
  val *= sign;
  // Skip ULL suffixes.
  while (*str == 'U' || *str == 'L')
    ++str;
  return true;
}

bool MatchSignature::parse_declarator (const char *&str, bool abstract,
                                  MatchTypeRef &type) {
  // first parse an optional sequence of pointer operators like '*'
  // the type from the decl specs is simply extended.
  while (parse_ptr_operator (str, type))
    if (error ())
      return false;

  skip_blanks (str);
  bool nested_declarator = false;
  const char *nested_str = str;
  if (*str == '(' && (!abstract || is_abstract_nested_declarator (str))) {
    // skip the nested declarator and parse it later
    nested_declarator = true;
    if (!skip_nested_declarator (str)) {
      _state = ERR_INVALID_DECLARATOR;
      return false;
    }
  }
  else {
    // parse the declarator id
    if (!parse_declarator_id (str, _name)) {
      if (error ())
        return false;
    }
    else if (abstract) {
      _state = ERR_DECL_NOT_ABSTRACT;
      return false;
    }
  }

  // now parse an optional sequence of [constant] or (...) cv
  parse_declarator_post (str, type);
  if (error ())
    return false;
  
  if (nested_declarator) {
    // parse a declarator in brackets
    nested_str++;
    if (!parse_declarator (nested_str, abstract, type)) {
      if (!error ())
        _state = ERR_INVALID_DECLARATOR;
      return false;
    }
    skip_blanks (nested_str);
    if (*nested_str != ')') {
      _state = ERR_MISSING_BRACKET;
      return false;
    }
  }
  
  // if we came here, everything is fine
  return true;
}

bool MatchSignature::is_abstract_nested_declarator (const char *str) {
  // skip sequence of opening brackets
  while (*str == '(') {
    str++;
    skip_blanks (str);
  }
  // create a type object, which is only needed temporarily
  MatchTypeRef dummy_type = MatchTypeRef::make_int ();
  // check whether the string starts with a pointer operator,
  // which is the only reason for a *nested* abstract declarator
  return parse_ptr_operator (str, dummy_type);
}

bool MatchSignature::skip_nested_declarator (const char *&str) {
  int brackets = 1;
  str++;
  while (*str) {
    if (*str == '(')
      brackets++;
    else if (*str == ')')
      brackets--;
    str++;
    if (brackets == 0)
      return true;
  }
  return false; 
}

bool MatchSignature::parse_declarator_post (const char *&str, MatchTypeRef &type) {
  skip_blanks (str);
  if (*str == '(') { // analyse a function declarator
    str++;

    // start with the argument list
    vector<MatchTypeRef> arg_types;
    bool var_args;
    parse_fct_args (str, arg_types, var_args);
    if (error ())
      return false;
      
    // check for the closing bracket
    skip_blanks (str);
    if (*str != ')') {
      _state = ERR_MISSING_BRACKET;
      return false;
    }
    str++;

    // now parse optional const or volatile qualifiers
    bool is_const = false, is_volatile = false;
    while (true) {
      skip_blanks (str);
      if (next_word ("const", str))
        is_const = true;
      else if (next_word ("volatile", str))
        is_volatile = true;
      else
        break;
    }

    // now parse other post declarators ("(...) cv" or "[ dim ]")
    parse_declarator_post (str, type);
    if (error ())
      return false;
    
    // make the current type a (qualified) function type
    type.to_function (arg_types, var_args);
    if (is_const) type.qualifiers ().qualify_const ();
    if (is_volatile) type.qualifiers ().qualify_volatile ();
    return true;
  }
  else if (*str == '[') { // analyse an array declarator
    str++;
    skip_blanks (str);

    // TODO: syntax (*foo)[] not yet implemented; however, this is no type
    bool any_size = false;
    unsigned long array_size = 0;
    if (*str == '%') {
      if (!_wildcards_allowed) {
        _state = ERR_WILDCARD;
        return false;
      }
      str++;
      any_size = true;
    }
    else if (*str >= '0' && *str <= '9') {
      while (*str >= '0' && *str <= '9') {
        array_size = 10uL * array_size + (unsigned long)(*str - '0');
        str++;
      }
    }
    else {
      _state = ERR_INVALID_ARRAY;
      return false;
    }
    
    // check for the closing bracket
    skip_blanks (str);
    if (*str != ']') {
      _state = ERR_MISSING_BRACKET;
      return false;
    }
    str++;

    // look for further post declarators and make the type
    parse_declarator_post (str, type);
    if (error ())
      return false;
    
    // make the current type an array type
    if (any_size)
      type.to_array ();
    else
      type.to_array (array_size);
      
    return true;
  }
  return false;
}

bool MatchSignature::parse_ptr_operator (const char *&str, MatchTypeRef &type) {
  skip_blanks (str);
  const char *curr = str;
  MatchName memb_ptr_scope;
  if (!parse_nested_name (curr, memb_ptr_scope))
    if (error ())
      return false;

  skip_blanks (curr);
  if (*curr == '*') {
    if (memb_ptr_scope.undefined ())
      type.to_pointer ();
    else
      type.to_memb_pointer (memb_ptr_scope);
    str = curr + 1;
    while (true) {
      skip_blanks (str);
      if (next_word ("const", str))
        type.qualifiers ().qualify_const ();
      else if (next_word ("volatile", str))
        type.qualifiers ().qualify_volatile ();
      else
        break;
    }
    return true;
  }
  else if (*curr == '&') {
    str = curr + 1;
    type.to_reference ();
    return true;
  }
  return false;
}

bool MatchSignature::parse_declarator_id (const char *&str, MatchName &match_name) {
  skip_blanks (str);
  // first check if this is a qualified name 
  const char *curr = str;
  if (!parse_nested_name (curr, match_name))
    if (error ())
      return false;
  skip_blanks (curr);
  // now check if a name follows
  const char *name_start = curr;
  while (is_id (*curr))
    curr++; 
  if (curr == name_start)
    return false;
  int len = curr - name_start;
  // check for operator <xx>
  if (len == 8 && strncmp (name_start, "operator", 8) == 0) {
    // yes, an operator
    skip_blanks (curr);
    MatchName::Operator oper;
    // check for operator +, etc.
    if (parse_operator_id (curr, oper)) {
      match_name.oper (oper);
    }
    else {
      // check for operator int(), etc.
      MatchTypeRef conv_type;
      if (parse_conv_id (curr, conv_type))
        match_name.conv_type (conv_type);
      else {
        _state = ERR_INVALID_OPER;
        return false; // this is an error!
      }
    }
  }
  else {
    // a normal function name
    string name (name_start, curr - name_start);
    match_name.name (name);
  }
  str = curr;

  // are there any template parameters?
  MatchTemplateArgList *mtal;
  if (!parse_opt_template_argument_list (str, mtal))
    if (error ())
      return false;
  // set the template argument list (might be 0)
  match_name.template_args (mtal);
  return true;
}

bool MatchSignature::parse_conv_id (const char *&str, MatchTypeRef &type) {
  // Here we allow more than the C++ grammar:
  //   decl_spec_seq_opt declarator_opt
  // =>
  // abstract-declaration / with look-ahead "(" ")"
  // This is because our signatures may be something like "void (*)()", which
  // would not be allowed by the C++.

  // search the empty argument list of the conversion operator
  int pos = (int)strlen(str);
  while (pos >= 0 && str[pos] != ')') pos--; // search for last ')'
  pos--;
  while(pos >= 0 && str[pos] == ' ') pos--; // skip blanks
  if (pos <= 0 || str [pos] != '(') {
    _state = ERR_CONV_FCT_ARGS;
    return false;
  }

  // create a new string that contains only the abstract declarator of the
  // conversion's target type
  string declarator_buffer(str, pos);
  const char *curr = declarator_buffer.c_str();
  
  // parse an abstract declaration
  if (!parse_declaration (curr, true, type)) {
    if (error ())
      return false;
    _state = ERR_INVALID_SPEC;
    return false;
  }

  str = str + pos;
  return true;
}

bool MatchSignature::parse_operator_id (const char *&str, MatchName::Operator &op) {
    
  // first check for new and delete
  bool op_new = false, op_delete = false;
  if (next_word ("new", str))
    op_new = true;
  else if (next_word ("delete", str))
    op_delete = true;

  skip_blanks (str);
  if (next_word ("[]", str)) {
    op = (op_new ? MatchName::OP_NEW_ARRAY :
                   (op_delete ? MatchName::OP_DELETE_ARRAY :
                                MatchName::OP_INDEX));
    return true;
  }

  op = (op_new ? MatchName::OP_NEW :
                 (op_delete ? MatchName::OP_DELETE:
                              MatchName::OP_UNDEFINED));
  if (op != MatchName::OP_UNDEFINED)
    return true;
  
  // check for the rest (and increment the pointer)
  const char *saved_pos = str;
  char first = *str;
  str++;
  if (first == '+') { // +, +=, ++
    if (*str == '=') {
      str++;
      op = MatchName::OP_ADD_EQ;
    }
    else if (*str == '+') {
      str++;
      op = MatchName::OP_INCR;
    }
    else
      op = MatchName::OP_PLUS;
  }
  else if (first == '-') { // -, -=, --, ->*, ->
    if (*str == '=') {
        str++;
        op = MatchName::OP_SUB_EQ;
    }
    else if (*str == '-') {
        str++;
        op = MatchName::OP_DECR;
    }
    else if (*str == '>') {
      str++;
      if (*str == '*') {
        str++;
        op = MatchName::OP_PTS_STAR;
      }
      else
        op = MatchName::OP_PTS;
    }
    else
      op = MatchName::OP_MINUS;
  }
  else if (first == '*') { // *, *=
    if (*str == '=') {
      str++;
      op = MatchName::OP_MUL_EQ;
    }
    else  
      op = MatchName::OP_MUL;
  }
  else if (first == '/') { // /, /=
    if (*str == '=') {
      str++;
      op = MatchName::OP_DIV_EQ;
    }
    else  
      op = MatchName::OP_DIV;
  }
  else if (first == '%') { // %, %%, %%= ; that's ugly! %% has to be used for %
    if (_wildcards_allowed) {
      if (*str == '%') {
        str++;
        if (*str == '=') {
          str++;
          op = MatchName::OP_MOD_EQ;
        }
        else
          op = MatchName::OP_MODULO;
      }
      else
        op = MatchName::OP_ANY;
    }
    else {
      if (*str == '=') {
        str++;
        op = MatchName::OP_MOD_EQ;
      }
      else  
        op = MatchName::OP_MODULO;
    }
  }
  else if (first == '^') { // ^, ^=
    if (*str == '=') {
      str++;
      op = MatchName::OP_XOR_EQ;
    }
    else  
      op = MatchName::OP_ROOF;
  }
  else if (first == '&') { // &, &=, &&
    if (*str == '&') {
      str++;
      op = MatchName::OP_AND_AND;
    }
    else if (*str == '=') {
      str++;
      op = MatchName::OP_AND_EQ;
    }
    else
      op = MatchName::OP_AND;
  }
  else if (first == '|') { // |, |=, ||
    if (*str == '|') {
      str++;
      op = MatchName::OP_OR_OR;
    }
    else if (*str == '=') {
      str++;
      op = MatchName::OP_IOR_EQ;
    }
    else
      op = MatchName::OP_OR;
  }
  else if (first == '~') { // ~
    op = MatchName::OP_TILDE;
  }
  else if (first == '!') { // !, !=
    if (*str == '=') {
      str++;
      op = MatchName::OP_NEQ;
    }
    else  
      op = MatchName::OP_NOT;
  }
  else if (first == '=') { // =, ==
    if (*str == '=') {
      str++;
      op = MatchName::OP_EQL;
    }
    else  
      op = MatchName::OP_ASSIGN;
  }
  else if (first == '<') { // <, <<, <=, <<=
    if (*str == '<') {
      str++;
      if (*str == '=') {
        str++;
        op = MatchName::OP_LSH_EQ;
      }
      else {
        op = MatchName::OP_LSH;
      }
    }
    else if (*str == '=') {
      str++;
      op = MatchName::OP_LEQ;
    }
    else
      op = MatchName::OP_LESS;
  }
  else if (first == '>') { // >, >>, >=, >>=
    if (*str == '>') {
      str++;
      if (*str == '=') {
        str++;
        op = MatchName::OP_RSH_EQ;
      }
      else
        op = MatchName::OP_RSH;
    }
    else if (*str == '=') {
      str++;
      op = MatchName::OP_GEQ;
    }
    else
      op = MatchName::OP_GREATER;
  }
  else if (first == ',') { // ,
    op = MatchName::OP_COMMA;
  }
  else if (first == '(') { // ()
    if (*str == ')') {
      str++;
      op = MatchName::OP_CALL;
    }
  }
  else if (first == '?') { // ?:
    if (*str == ':') {
      str++;
      op = MatchName::OP_COND;
    }
  }
  else if (first == '.') { // ., .*
    if (*str == '*') {
      str++;
      op = MatchName::OP_DOT_STAR;
    }
    else {
      op = MatchName::OP_DOT;
    }
  }

  if (op != MatchName::OP_UNDEFINED)
    return true;
    
  str = saved_pos;
  return false;
}

bool MatchSignature::parse_fct_args (const char *&str, vector<MatchTypeRef> &args,
                                bool &var_args) {
  int arg = 0;
  bool have_ellipses = false;
  var_args = false;
  // parse the elements of the argument list step-by-step
  while (true) {
    skip_blanks (str);
    // end of argument list?
    if (*str == ')')
      break;
    // check for the ',' and consume it
    if (arg > 0) {
      if (*str == ',') {
        if (have_ellipses) {
          _state = ERR_ARG_AFTER_ELLIPSES;
          return false;
        }
        str++;
        skip_blanks (str);
      }
      else {
        _state = ERR_INVALID_ARG_LIST;
        return false;
      }
    }
    // check for ellipses first
    if (parse_ellipses (str)) {
      have_ellipses = true;
      // set a flag that says that this function type has a var. arg. list
      var_args = true;
    }
    else {
      if (error ())
        return false;
      MatchTypeRef arg_type;
      if (!parse_declaration (str, true, arg_type)) {
        if (error ())
          return false;
        _state = ERR_INVALID_ARG_LIST;
        return false;
      }
      // add this type as an argument type;
      args.push_back (arg_type);
    }
    arg++;
  }
  return true;
}

bool MatchSignature::parse_ellipses (const char *&str) {
  const char *curr = str;
  if (*curr++ != '.')
    return false;
  if (*curr++ != '.') {
    _state = ERR_INVALID_DOT;
    return false;
  }
  if (*curr++ != '.') {
    _state = ERR_INVALID_DOT;
    return false;
  }
  skip_blanks (curr);
  if (*curr == ':')
    return false;
  str = curr;
  return true;
}

inline void MatchSignature::skip_blanks (const char *&str) const {
  while (*str == ' ') str++;
}

inline bool MatchSignature::next_word (const char *word, const char *&str) const {
  const char *curr = str;
  while (*word != '\0' && *curr == *word) {
    curr++;
    word++;
  }
  if (*word == '\0' && !is_id (*curr)) {
    str = curr;
    return true;
  }
  else
    return false;
}

inline bool MatchSignature::is_id (char c) const {
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          c == '_' || (_wildcards_allowed && c == '%') || (c >= '0' && c <= '9'));
}
