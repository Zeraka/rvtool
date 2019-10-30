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

#ifndef __MatchName_h__
#define __MatchName_h__

#include <iostream>
using std::ostream;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include "lexertl/state_machine.hpp"

#include "MatchType.h" // ugly cyclic relationship for conversion oper names

class MatchTemplateArgList;

class MatchName {
  
  class Matcher {
  public:
    virtual bool matches (const string &name) const = 0;
    virtual ~Matcher () {}
  };

  class MRegComp : public Matcher {
    string  _expr;
    lexertl::state_machine _compiled; // internal: lexertl compiled regular expression
    bool    _compiled_used;
    bool    _error;     // true, if any operation failed
  public:
    MRegComp () : _compiled_used(false), _error (false) {}
    MRegComp (const string &str) :
      _expr(str), _compiled_used(false), _error (false) { setup(str); }
    MRegComp (const MRegComp &copy) :
      _expr(copy._expr), _error(false) {
      if (_compiled_used) {
        _compiled.clear();
        _compiled_used = false;
      }
    }
    MRegComp &operator = (const MRegComp &copy) {
      if (_compiled_used) _compiled.clear ();
      _expr = copy._expr;
      _error = false;
      _compiled_used = false;
      return *this;
    }
    void setup (const string &str);
    virtual bool matches (const string &name) const;
  };
  
  class MTrue : public Matcher {
  public:
    virtual bool matches (const string &name) const;
  };
    
  class MStrComp : public Matcher {
    string _str;
  public:
    MStrComp (const string &str) : _str (str) {}
    virtual bool matches (const string &name) const;
  };
    
  class Name {
    Matcher *_matcher; // not copied, but regenerated on demand
    string _str;
    void make_matcher ();
  public:
    Name () : _matcher (0) {}
    Name (const string &s) : _matcher (0), _str (s) {}
    Name (const Name &copy) { *this = copy; }
    Name &operator = (const Name &copy) {
      _matcher = 0; // regenerated on demand
      _str     = copy._str;
      return *this;
    }
    ~Name () { if (_matcher) delete _matcher; }
    bool matches (const string &name) {
      if (!_matcher)
        make_matcher ();
      return _matcher->matches (name);
    }
    Name &operator = (const string &str) {
      _matcher = 0;
      _str     = str;
      return *this;
    }
    string str () const { return _str; }
  };
  
public:
  enum Operator {
    OP_UNDEFINED = -1, OP_ANY = 0, 
    OP_PLUS, OP_MINUS, OP_MUL, OP_DIV, OP_MODULO, OP_ROOF,
    OP_AND, OP_OR, OP_TILDE, OP_NOT, OP_LESS, OP_GREATER,
    OP_EQL, OP_GEQ, OP_LEQ, OP_NEQ, OP_AND_AND, OP_OR_OR,
    OP_LSH, OP_RSH, OP_DECR, OP_INCR, OP_ASSIGN, OP_COMMA,
    OP_ADD_EQ, OP_SUB_EQ, OP_MUL_EQ, OP_DIV_EQ, OP_MOD_EQ,
    OP_AND_EQ, OP_IOR_EQ, OP_LSH_EQ, OP_RSH_EQ, OP_XOR_EQ,
    OP_PTS_STAR, OP_PTS, OP_NEW, OP_DELETE, OP_NEW_ARRAY,
    OP_DELETE_ARRAY, OP_CALL, OP_INDEX,
    OP_COND, OP_DOT, OP_DOT_STAR
  };
  
private:
  vector<Name> _scopes;  // scope names ('X::Y::Z::')
  vector<MatchTemplateArgList*> _scope_template_args;
  Name _name;            // either name
  MatchTemplateArgList *_name_template_args;
  Operator _oper;          //    ... or operator name
  MatchTypeRef _conv_type; //    ... or conversion type id

  bool oper_matches (Operator oper);
  bool conv_matches (MatchTypeRef type);
  bool name_matches (Name &name, MatchTemplateArgList *name_template_args);

public:
  
  MatchName () : _name_template_args (0), _oper (OP_UNDEFINED) {}
  ~MatchName ();
  MatchName (const MatchName &copy) { *this = copy; }
  MatchName &operator = (const MatchName &copy);
  
  // print the contents of this match name object
  void print (ostream &) const;
  // print the 'mangled' string representation of this type
  void mangle (ostream &os) const;

  // define a match name
  void name (const string &n) {
    _name = n;
  }
  void template_args (MatchTemplateArgList *mtal) {
    _name_template_args = mtal;
  }
  void oper (Operator o) { _oper = o; }
  void conv_type (const MatchTypeRef &ct) { _conv_type = ct; }
  void add_scope (const string &s, MatchTemplateArgList *mtal = 0) {
    _scopes.push_back (Name (s));
    _scope_template_args.push_back (mtal);
  }
  // get the object contents
  int scopes () const { return _scopes.size (); }
  string scope (int i) const { return _scopes[i].str (); }
  string name () const { return _name.str (); }
  Operator oper () const { return _oper; }
  MatchTypeRef conv_type () const { return _conv_type; }
  bool undefined () const { 
    return scopes () == 0 && _name.str ().empty () &&
           _oper == OP_UNDEFINED && _conv_type.is_undefined ();
  }
  bool scope_matches (vector<Name> &, vector<MatchTemplateArgList*> &,
    int match_pos, int sig_pos);
  bool matches (MatchName &match_name);

  // check whether the name will match only a single entity
  bool is_trivial () const;
};

inline ostream &operator << (ostream &os, const MatchName &mn) {
  mn.print (os);
  return os;
}

#endif // __MatchName_h__
