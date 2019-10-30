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

#ifndef __MatchTemplateArg_h__
#define __MatchTemplateArg_h__

#include <list>
using std::list;

#include "MatchType.h"
#include "MatchName.h"

// Common interface for all class that represent match expressions
// for template arguments.

class MatchTemplateArg {
public:
  virtual ~MatchTemplateArg () {}
  virtual void print (ostream &out) const = 0;
  virtual void mangle (ostream &out) const = 0;
  virtual bool matches (MatchTemplateArg &arg) const = 0;
  virtual MatchTemplateArg *clone () const = 0;
  virtual bool is_any () const { return false; }
  virtual bool is_ellipses () const { return false; }
  virtual bool is_type () const { return false; }
  virtual bool is_value () const { return false; }
  virtual bool is_addr () const { return false; }
};

// streamed output for template argument match expressions
inline ostream &operator << (ostream &out, const MatchTemplateArg &mta) {
  mta.print (out);
  return out;
}


// A list of template argument match expressions
class MatchTemplateArgList : private list <MatchTemplateArg*> {
public:
  MatchTemplateArgList () {}
  ~MatchTemplateArgList () {
    for (iterator iter = begin (); iter != end (); ++iter)
      delete *iter;
  }
  MatchTemplateArgList (const MatchTemplateArgList& mtal) { *this = mtal; }
  MatchTemplateArgList &operator = (const MatchTemplateArgList& mtal) {
    for (const_iterator iter = mtal.begin (); iter != mtal.end (); ++iter)
      append ((*iter)->clone ());
    return *this;
  } 
  void append (MatchTemplateArg *mta) { push_back (mta); }
  void print (ostream &out) const {
    out << "<";
    bool first = true;
    for (const_iterator iter = begin (); iter != end (); ++iter) {
      if (first)
        first = false;
      else
        out << ",";
      out << **iter;
    }
    out << ">";
  }
  void mangle (ostream &os) const {
    os << "I";
    for (const_iterator iter = begin (); iter != end (); ++iter)
      (*iter)->mangle (os);
    os << "E";
  }
  bool matches (MatchTemplateArgList &mtal) const;
};

inline ostream &operator << (ostream &out, const MatchTemplateArgList &mtal) {
  mtal.print (out);
  return out;
}


// Template argument match expression that matches any template argument
class MTA_Any : public MatchTemplateArg {
public:
  void print (ostream &out) const { out << "%"; }
  void mangle (ostream &out) const { out << "_X_"; }
  bool matches (MatchTemplateArg &arg) const { return true; }
  MTA_Any *clone () const { return new MTA_Any (*this); }
  bool is_any () const { return true; }
};


// Template argument match expression that matches any number of
// arbitrary template argument
class MTA_Ellipses : public MatchTemplateArg {
public:
  void print (ostream &out) const { out << "..."; }
  void mangle (ostream &out) const {}
  // TODO: not correct
  bool matches (MatchTemplateArg &arg) const { return true; }
  MTA_Ellipses *clone () const { return new MTA_Ellipses (*this); }
  bool is_ellipses () const { return true; }
};


// This class represents match expressions for types in template
// argument lists.
class MTA_Type : public MatchTemplateArg {
  MatchTypeRef _match_type;
public:
  MTA_Type (const MatchTypeRef mt) : _match_type (mt) {}
  void print (ostream &out) const { out << _match_type; }
  void mangle (ostream &out) const { _match_type.mangle (out); }
  bool matches (MatchTemplateArg &arg) const;
  MTA_Type *clone () const { return new MTA_Type (*this); }
  bool is_type () const { return true; }
};


// This class represents match expressions for value template arguments.
class MTA_Value : public MatchTemplateArg {
  long long _val;
public:
  MTA_Value (long long val) : _val (val) {}
  void print (ostream &out) const { out << _val; }
  void mangle (ostream &out) const {
    if (_val < 0)
      out << "n" << -_val;
    else
      out << _val;
  }
  bool matches (MatchTemplateArg &arg) const;
  MTA_Value *clone () const { return new MTA_Value (*this); }
  bool is_value () const { return true; }
};

// This class represents match expressions for value template arguments.
class MTA_Addr : public MatchTemplateArg {
  MatchName _name;
public:
  MTA_Addr (const MatchName &name) : _name (name) {}
  void print (ostream &out) const { out << "&" << _name; }
  // TODO: name mangling not correct, e.g. "_Z" missing for functions
  void mangle (ostream &out) const { out << "L"; _name.mangle (out); out << "E"; }
//  bool matches (DeducedArgument *arg) const;
  bool matches (MatchTemplateArg &arg) const;
  MTA_Addr *clone () const { return new MTA_Addr (*this); }
  bool is_addr () const { return true; }
};

#endif // __MatchTemplateArg_h__
