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

#ifndef __MatchExpr_h__
#define __MatchExpr_h__

#include <cassert>

#include "MatchType.h"
#include "MatchTemplateArg.h"
#include "MatchName.h"

// This class is an abstraction for a C++ object signature (function, type,
// attribute).

class MatchSignature {

public:
  // object state 
  enum State {
    NEW    = 0, // this object is not initialized, yet
    NORMAL = 1, // this is a valid match expression, ready for 'match' calls
    // state of the expression ( < 0 means an error code)
    ERR_NO_DECL_SPEC       = -1,  // no declaration specifier given
    ERR_EMPTY_MATCH_EXPR   = -2,  // neither decl spec nor declarator given
    ERR_TOKENS_AFTER_END   = -3,  // tokens after end of match expression
    ERR_INVALID_DOT        = -4,  // invalid use of "..."
    ERR_INVALID_DECLARATOR = -5,  // invalid declarator
    ERR_MISSING_BRACKET    = -6,  // missing bracket
    ERR_MISSING_DECL_ID    = -7,  // missing declarator id (the name)
    ERR_DECL_NOT_ABSTRACT  = -8,  // argument declarator not abstract
    ERR_ARG_AFTER_ELLIPSES = -9,  // argument after "..."
    ERR_INVALID_ARG_LIST   = -10, // invalid argument list
    ERR_INVALID_ARRAY      = -11, // array size is neither "%" nor int constant
    ERR_DUP_CV             = -12, // duplicated const or volatile
    ERR_DUP_SIGN           = -13, // duplicated signed or unsigned
    ERR_CONF_SIZE          = -14, // conflicting short and long
    ERR_INVALID_TYPE       = -15, // invalid type
    ERR_INVALID_OPER       = -16, // invalid operator name
    ERR_TEMPLATE_ARG_LIST  = -17, // error in template arguments
    ERR_DUP_SPEC           = -18, // duplicate virtual, inline, or explicit
    ERR_INVALID_SPEC       = -19, // only functions can have function specifiers
    ERR_VIRT_STAT_FCT_SPEC = -20, // 'static' and 'virtual' can't be combined
    ERR_WILDCARD           = -21, // wildcard used in signature (not match expression)
    ERR_CONV_FCT_ARGS      = -22  // (empty) conversion function argument list not found
  };
  
protected:
  State _state;
  bool _wildcards_allowed;
  
private:
  // primitive types
  enum PrimType {
    PRIM_NONE, PRIM_CHAR, PRIM_INT, PRIM_INT128, PRIM_VOID, PRIM_BOOL, PRIM_FLOAT,
    PRIM_DOUBLE, PRIM_WCHAR_T
  };                 
  
  // const or volatile qualifiers
  enum CVQualifier { QUAL_NONE = 0, QUAL_CONST = 1, QUAL_VOLATILE = 2 } ;
 
  // general declaration specifiers
  enum Spec {
    DT_NONE = 0, DT_STATIC
  } _spec;
  // function specifiers, currently only 'virtual' is supported by the parser
  enum FctSpec {
    FCT_NONE = 0, FCT_VIRTUAL = 1, FCT_INLINE = 2, FCT_EXPLICIT = 4
  } _fct_spec;
  
  // type used for parsing and analysing the declaration specifier sequence
  struct DeclSpecs {
    CVQualifier _qual; // bitfield with const/volatile qualifiers
    Spec _spec; // bitfield with general specifiers
    FctSpec _fct_spec; // bitfield with function specifiers
    int _sign; // -1, 0, +1
    int _size; // -1 = short, 0 = no change, 1 = long, 2 = long long, ...
    bool _allow_name;
    PrimType _type;
    MatchName _name;
    DeclSpecs () :
      _qual (QUAL_NONE), _spec (DT_NONE), _fct_spec (FCT_NONE), _sign (0), _size (0),
      _allow_name (true), _type (PRIM_NONE) {}
  };
  
public:
  //! match expression type
  enum Type { ATTRIBUTE, FUNCTION, TYPE } _expr_type;
  
protected:
  //! the name pattern of the match expression
  MatchName _name;
  //! the type pattern
  MatchTypeRef _type;
  
public:
  // constructors/destructor
  inline MatchSignature ();
  inline MatchSignature (const string &str);
  
  // parse a match expression string
  // complain about errors => result false
  bool parse (const string &str);
  
  // check the state of this match expression
  inline bool error () const;
  inline bool state () const;
  inline bool is_new () const;
  inline bool is_function () const;
  inline bool is_virtual_function () const;
  inline bool is_static() const;
  inline bool is_attribute () const;
  inline bool is_type () const;

  // manipulate the state
  inline void declare_virtual_function ();
  inline void declare_static();
  
  // get more detailed infos about the match expression
  inline MatchName &name ();
  inline MatchTypeRef &type ();
  
private:
  // internal parser functions that analyse a match expr string
  bool parse_match_expr (const char *&str);
  bool parse_declaration (const char *&str, bool abstract, MatchTypeRef &type);
  bool parse_decl_spec_seq( const char *&str, MatchTypeRef &type, FctSpec &fs, Spec &s );
  bool parse_decl_spec (const char *&str, DeclSpecs &ds);
  bool parse_nested_name (const char *&str, MatchName &match_name);
  bool parse_nested_name_elem (const char *&str, MatchName &match_name);  
  bool parse_opt_template_argument_list (const char *&str,
                                         MatchTemplateArgList *&mtal);
  bool parse_template_argument_list (const char *&str, 
                                     MatchTemplateArgList &mtal);
  bool parse_template_argument (const char *&str, MatchTemplateArg *&mta);
//bool parse_dec_string_literal(const char *&str, std::string &val);
  bool parse_dec_literal (const char *&str, long long &val);
  bool parse_declarator (const char *&str, bool abstract, MatchTypeRef &type);
  bool is_abstract_nested_declarator (const char *str);
  bool skip_nested_declarator (const char *&str);
  bool parse_declarator_post (const char *&str, MatchTypeRef &type);
  bool parse_ptr_operator (const char *&str, MatchTypeRef &type);
  bool parse_declarator_id (const char *&str, MatchName &match_name);
  bool parse_conv_id (const char *&str, MatchTypeRef &type);
  bool parse_operator_id (const char *&str, MatchName::Operator &op);
  bool parse_fct_args (const char *&str, vector<MatchTypeRef> &args, bool &va);
  bool parse_ellipses (const char *&str);
  inline void skip_blanks (const char *&str) const;
  inline bool next_word (const char *word, const char *&str) const;
  inline bool is_id (char c) const;
};

// implementations of inline constructors/destructor
inline MatchSignature::MatchSignature () : _state (NEW), _wildcards_allowed (false) {}
inline MatchSignature::MatchSignature (const string &str) : _state (NEW), _wildcards_allowed (false) {
  parse (str);
}

// check the state of this match expression
inline bool MatchSignature::error () const { return _state < 0; }
inline bool MatchSignature::state () const { return _state; }
inline bool MatchSignature::is_new () const { return _state == NEW; }
inline bool MatchSignature::is_function () const { return _expr_type == FUNCTION; }
inline bool MatchSignature::is_virtual_function () const {
  return is_function () && ((_fct_spec & FCT_VIRTUAL) != 0);
}
inline bool MatchSignature::is_static() const {
  return ( is_function() || is_attribute() ) && ( ( _spec & DT_STATIC ) != 0 );
}
inline bool MatchSignature::is_attribute () const { return _expr_type == ATTRIBUTE; }
inline bool MatchSignature::is_type () const { return _expr_type == TYPE; }

// manipulate the state
inline void MatchSignature::declare_virtual_function () {
  assert (is_function ());
  _fct_spec = (FctSpec)(FCT_VIRTUAL | _fct_spec);
}
inline void MatchSignature::declare_static() {
  assert( is_function () || is_attribute() );
  _spec = (Spec)(DT_STATIC | _spec);
}

// get more detailed infos about the match expression
// TODO: const correctness
inline MatchName &MatchSignature::name () { return _name; }
inline MatchTypeRef &MatchSignature::type () { return _type; }

// this is an abstraction for AspectC++ match expressions. These expressions
// describe signature patters, which can be matched agains the signatures
// of C++ objects. Match expressions are 'Signatures' that may contain
// special wildcard symbols ("%", "...").

class MatchExpr : public MatchSignature {
public:
  
  // constructors/destructor
  inline MatchExpr ();
  inline MatchExpr (const string &str);

  // check if a certain signature is matched by this expression,
  // obj is passed to check the name part of the signature
  // true is returned if the signature is matched
  inline bool matches (MatchSignature &sig);
};

// implementations of inline constructors/destructor
inline MatchExpr::MatchExpr () { _wildcards_allowed = true; }
inline MatchExpr::MatchExpr (const string &str) { _wildcards_allowed = true; parse (str); }

// check if the match expression matches a passed match signature
inline bool MatchExpr::matches (MatchSignature &sig) {
  assert (_state == NORMAL && _expr_type == sig._expr_type && !_type.is_undefined ());

  if ((is_function() || is_attribute()) && !_name.matches (sig.name ()))
    return false;
  if (!_type.matches (sig.type ()))
    return false;
  if (is_virtual_function () && !sig.is_virtual_function ())
    return false;
  if( is_static() && !sig.is_static() )
    return false;
  return true;
}

#endif // __MatchExpr_h__
