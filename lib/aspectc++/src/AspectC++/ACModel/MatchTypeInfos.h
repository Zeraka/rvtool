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

#ifndef __MatchTypeInfos_h__
#define __MatchTypeInfos_h__

#include <iostream>
using std::ostream;
using std::endl;
#include <vector>
using std::vector;

#include "MatchType.h"
#include "MatchName.h"

// functions without result type (conversion operators) have this type
class MTUndefined : public MatchType {
  static TID _tid;

protected:
  virtual TID type () const;
public:
  virtual MTUndefined *clone () const;
  virtual void print (ostream &, const char* = 0, bool = false) const;
  virtual bool matches (const MatchType &type) const;
};

// commonalities of all types made by declarators
class MTDeclarator : public MatchType {
  MatchType *_base;
  static TID _tid;

protected:
  virtual TID type () const;
  // only derived types may construct/destruct a declarator type
  MTDeclarator (MatchType *b) : _base (b) {}
  MTDeclarator (const MTDeclarator &copy) { *this = copy; }
  MTDeclarator &operator = (const MTDeclarator &copy) {
    _base = copy._base->clone ();
    return *this;
  }
  ~MTDeclarator () { delete _base; }
  
public:
  // set and get the base type
  virtual MatchType *base () const { return _base; }
  // adjust the argument types according to �8.3.5.2 and �8.3.5.3 of ISO C++
  virtual void adjust_args (bool &f, bool &a, bool &q, bool &v) {
    _base->adjust_args (f, a, q, v);
  }
};

// pointer types
class MTPointer : public MTDeclarator, public MTQual {
  static TID _tid;

protected:
  virtual TID type () const;

public:
  MTPointer (MatchType *b = 0) : MTDeclarator (b) {}
  // print the string representation of this type
  virtual MTPointer *clone () const;
  virtual void print (ostream &, const char* = 0, bool = false) const;
  virtual void mangle (ostream &) const;
  virtual bool matches (const MatchType &type) const;
  virtual MTQual *qualified () { return this; }
  virtual const MTQual *qualified () const { return this; }
};

// member pointer types
class MTMembPointer : public MTPointer {
  static TID _tid;
  mutable MatchName _memb_ptr_scope;

protected:
  virtual TID type () const;

public:
  MTMembPointer (const MatchName &mps, MatchType *b = 0) :
    MTPointer (b), _memb_ptr_scope (mps) {}
  virtual MTMembPointer *clone () const;
  // print the string representation of this type
  virtual void print (ostream &, const char* = 0, bool = false) const;
  virtual void mangle (ostream &) const;
  virtual bool matches (const MatchType &type) const;
};


// reference types
class MTReference : public MTDeclarator {
  static TID _tid;

protected:
  virtual TID type () const;

public:
  MTReference (MatchType *b = 0) : MTDeclarator (b) {}
  virtual MTReference *clone () const;
  // print the string representation of this type
  virtual void print (ostream &, const char* = 0, bool = false) const;
  virtual void mangle (ostream &) const;
  virtual bool matches (const MatchType &type) const;
  virtual bool is_reference () const { return true; }
};

// function types
class MTFunction : public MTDeclarator, public MTQual {
  static TID _tid;
  vector<MatchTypeRef> _arg_types;
  bool _varargs;

protected:
  virtual TID type () const;

public:
  MTFunction (MatchType *b = 0) : MTDeclarator (b), _varargs (false) {}
  MTFunction (const vector<MatchTypeRef> &args, bool va, MatchType *b) :
    MTDeclarator (b), _arg_types (args), _varargs (va) {}
//  virtual ~MTFunction ();
  virtual MTFunction *clone () const;
  // print the string representation of this type
  virtual void print (ostream &, const char* = 0, bool = false) const;
  // print the 'mangled' string representation of this type
  virtual void mangle (ostream &os) const;
  virtual bool is_function () const { return true; }
  void varargs () { _varargs = true; }
  bool varargs () const { return _varargs; }
  int args () const { return (int)_arg_types.size (); }
  void arg (MatchTypeRef a) { _arg_types.push_back (a); }
  const MatchTypeRef &arg (int index) const { return _arg_types[index]; }
  // perform a type match
  virtual bool matches (const MatchType &type) const;
  // adjust the argument types according to �8.3.5.2 and �8.3.5.3 of ISO C++
  virtual void adjust_args (bool &f, bool &a, bool &q, bool &v);
  virtual MTQual *qualified () { return this; }
  virtual const MTQual *qualified () const { return this; }
};

// array types
class MTArray : public MTDeclarator {
  static TID _tid;
  unsigned long _dim;
  bool _any_size;
  
protected:
  virtual TID type () const;

public:
  MTArray (MatchType *b) : MTDeclarator (b), _dim (0uL),
    _any_size (true) {}
  MTArray (unsigned long dim, MatchType *b) : MTDeclarator (b), _dim (dim),
    _any_size (false) {}
  virtual MTArray *clone () const;
  // print the string representation of this type
  virtual void print (ostream &, const char* = 0, bool = false) const;
  // print the 'mangled' string representation of this type
  virtual void mangle (ostream &os) const;
  void dimension (unsigned long d) { _dim = d; }
  unsigned long dimension () const { return _dim; }
  void any_size () { _any_size = true; }
  bool any_size () const { return _any_size; }
  virtual bool matches (const MatchType &type) const;
  virtual bool is_array () const { return true; }
};

// user-defined names types
class MTNamed : public MatchType, public MTQual {
  mutable MatchName _name;
  static TID _tid;
protected:
  virtual TID type () const;
public:
  MTNamed (const MatchName &mn) : _name (mn) {}
  virtual MTNamed *clone () const;
  // print the string representation of this type
  virtual void print (ostream &, const char* = 0, bool = false) const;
  // print the 'mangled' string representation of this type
  virtual void mangle (ostream &os) const;
  const MatchName &name () const { return _name; }
  // perform a type match
  virtual bool matches (const MatchType &type) const;
  virtual MTQual *qualified () { return this; }
  virtual const MTQual *qualified () const { return this; }
  // check whether this type mattern can only match a single name type
  virtual bool is_trivial_match () const { return _name.is_trivial(); }
};

//! commonalities of all primitive types
class MTPrim : public MatchType, public MTQual {
public:
  //! print the string representation of this type
  virtual void print (ostream &, const char* = 0, bool = false) const;
  virtual MTQual *qualified () { return this; }
  virtual const MTQual *qualified () const { return this; }
  virtual bool is_primitive () const { return true; }
  // print the 'mangled' string representation of this type
  virtual void mangle (ostream &os) const;
};  

// type <any> (used for matching types>
class MTAny : public MTPrim {
  static TID _tid;
protected:
  virtual TID type () const;
public:
  virtual MTAny *clone () const;
  // perform a type match
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type bool
class MTBool : public MTPrim {
  static TID _tid;
protected:
  virtual MTBool *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type signed char
class MTSignedChar : public MTPrim {
  static TID _tid;
protected:
  virtual MTSignedChar *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type ungined char
class MTUnsignedChar : public MTPrim {
  static TID _tid;
protected:
  virtual MTUnsignedChar *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type char
class MTChar : public MTPrim {
  static TID _tid;
protected:
  virtual MTChar *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type unsigned short
class MTUnsignedShort : public MTPrim {
  static TID _tid;
protected:
  virtual MTUnsignedShort *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type short
class MTShort : public MTPrim {
  static TID _tid;
protected:
  virtual MTShort *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type unsigned int
class MTUnsignedInt : public MTPrim {
  static TID _tid;
protected:
  virtual MTUnsignedInt *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type int
class MTInt : public MTPrim {
  static TID _tid;
protected:
  virtual MTInt *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type wchar_t
class MTWCharT : public MTPrim {
  static TID _tid;
protected:
  virtual MTWCharT *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type unsigned long
class MTUnsignedLong : public MTPrim {
  static TID _tid;
protected:
  virtual MTUnsignedLong *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type long
class MTLong : public MTPrim {
  static TID _tid;
protected:
  virtual MTLong *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type unsigned long long
class MTUnsignedLongLong : public MTPrim {
  static TID _tid;
protected:
  virtual MTUnsignedLongLong *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type long long
class MTLongLong : public MTPrim {
  static TID _tid;
protected:
  virtual MTLongLong *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type unsigned __int128
class MTUnsignedInt128 : public MTPrim {
  static TID _tid;
protected:
  virtual MTUnsignedInt128 *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type __int128
class MTInt128 : public MTPrim {
  static TID _tid;
protected:
  virtual MTInt128 *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type float
class MTFloat : public MTPrim {
  static TID _tid;
protected:
  virtual MTFloat *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type double
class MTDouble : public MTPrim {
  static TID _tid;
protected:
  virtual MTDouble *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type long double
class MTLongDouble : public MTPrim {
  static TID _tid;
protected:
  virtual MTLongDouble *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual void mangle (ostream &os) const;
};

// type void
class MTVoid : public MTPrim {
  static TID _tid;
protected:
  virtual MTVoid *clone () const;
  virtual TID type () const;
  virtual bool matches (const MatchType &type) const;
  virtual bool is_void () const { return true; }
  virtual void mangle (ostream &os) const;
};

#endif // __MatchTypeInfos_h__
