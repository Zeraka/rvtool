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

#ifndef __MatchType_h__
#define __MatchType_h__

#include <iostream>
using std::ostream;
using std::endl;
#include <vector>
using std::vector;

class MatchName; // ugly cyclic relationship needed for conv type names

// common base class for all qualified types

class MTQual {
  bool _const;
  bool _volatile;

public:
  MTQual () : _const (false), _volatile (false) {}
  
  void qualify_const () { _const = true; }
  void qualify_volatile () { _volatile = true; }
  void unqualify () { _const = false; _volatile = false; }
  bool is_const () const { return _const; }
  bool is_volatile () const { return _volatile; }
  bool is_qualified () const { return _const || _volatile; }
  
  // print the string representation of this type
  void print_qual (ostream &) const;

  // print the 'mangled' string representation of this type
  void mangle (ostream &) const;

  // perform a type match
  bool qual_matches (const MTQual *qual) const;
};

// this is a common base class for all nodes in a tree that represents
// a match pattern for a type. Despite normal type nodes this tree might
// also contain wildcards.

class MatchType {
  
protected:

  // an id for the different kinds of types
  typedef const char *TID;

  // get the kind of type, e.g. bool, int, etc.
  virtual TID type () const = 0;  

public:
  // virtual destructor
  virtual ~MatchType () {}
  
  // is it the same kind of type?
  virtual bool operator ==(const MatchType &) const;
  inline bool operator !=(const MatchType &t) const { return !(*this == t); }
  
  //! print the string representation of this type
  virtual void print (ostream &, const char* = 0, bool = false) const = 0;
  
  // print the 'mangled' string representation of this type
  virtual void mangle (ostream &os) const { os << "X"; }

  //! check whether this is a function type
  virtual bool is_function () const { return false; }
  //! check whether this is an array type
  virtual bool is_array () const { return false; }
  //! check whether this type is 'void'
  virtual bool is_void () const { return false; }
  //! check whether this type is primitive
  virtual bool is_primitive () const { return false; }
  //! check whether this type is a reference
  virtual bool is_reference () const { return false; }

  //! create a clone of this match type
  virtual MatchType *clone () const = 0;
  
  //! get the base type if this is a derived type
  virtual MatchType *base () const { return 0; }
  
  // perform a type match
  virtual bool matches (const MatchType &type) const = 0;
  
  // check whether this type mattern can only match a single name type
  virtual bool is_trivial_match () const { return false; }

  // adjust the argument types according to �8.3.5.2 and �8.3.5.3 of ISO C++
  virtual void adjust_args (bool &f, bool &a, bool &q, bool &v) {}
  
  // converts the type into an MTQual if it is one
  virtual MTQual *qualified () { return 0; }
  virtual const MTQual *qualified () const { return 0; }
};

// should later become "MatchType"
class MatchTypeRef {
  MatchType *_type_info;
private:
  // only members of this class can create instances without clear()
  MatchTypeRef (MatchType *t) : _type_info (t) {}
public:
  MatchTypeRef ();
  MatchTypeRef (const MatchTypeRef &copy) {
    _type_info = copy._type_info->clone();
  }
  MatchTypeRef &operator = (const MatchTypeRef &copy) {
    clear ();
    _type_info = copy._type_info->clone();
    return *this;
  }
  ~MatchTypeRef () { clear (); }
  void clear () { delete _type_info; }
  MatchType *type_info () const { return _type_info; }
  
  // functions used to instantiate built-in primitive types
  static MatchTypeRef make_any ();
  static MatchTypeRef make_void ();
  static MatchTypeRef make_bool ();
  static MatchTypeRef make_signed_char ();
  static MatchTypeRef make_unsigned_char ();
  static MatchTypeRef make_char ();
  static MatchTypeRef make_wchar_t ();
  static MatchTypeRef make_unsigned_short ();
  static MatchTypeRef make_short ();
  static MatchTypeRef make_unsigned_int ();
  static MatchTypeRef make_int ();
  static MatchTypeRef make_unsigned_long ();
  static MatchTypeRef make_long ();
  static MatchTypeRef make_unsigned_long_long ();
  static MatchTypeRef make_long_long ();
  static MatchTypeRef make_unsigned_int128 ();
  static MatchTypeRef make_int128 ();
  static MatchTypeRef make_float ();
  static MatchTypeRef make_double ();
  static MatchTypeRef make_long_double ();
  
  // function used to instantiate a new named type
  static MatchTypeRef make_named (const MatchName &mn);
  
  // functions to construct complex types from built-in or named types
  void to_pointer ();
  void to_memb_pointer (const MatchName &mps);
  void to_reference ();
  void to_function (const vector<MatchTypeRef> &args, bool var_args);
  void to_array (unsigned long dim);
  void to_array ();
  
  // common type interface

  // perform a type match
  bool matches (MatchTypeRef type) const {
    return _type_info->matches (*type.type_info ());
  }
  
  // adjust the argument types according to �8.3.5.2 and �8.3.5.3 of ISO C++
  void adjust_args (bool &f, bool &a, bool &q, bool &v) {
    _type_info->adjust_args (f, a, q, v);
  }
  
  // print the string representation of this type
  void print (ostream &os, const char *prefix = 0) const {
    _type_info->print (os, prefix);
  }

  // print the 'mangled' string representation of this type
  void mangle (ostream &os) const {
    _type_info->mangle(os);
  }

  // check and get specific properties of this type
  
  bool is_undefined () const {
    return *type_info () == *MatchTypeRef ()._type_info;
  }
  bool is_qualifiable () const { return _type_info->qualified (); }
  MTQual &qualifiers () const { return *_type_info->qualified (); }

  bool is_function () const { return _type_info->is_function (); }
  bool is_array () const { return _type_info->is_array (); }
  bool is_void () const { return _type_info->is_void(); }
  bool is_primitive () const { return _type_info->is_primitive(); }
  bool is_reference () const { return _type_info->is_reference(); }

  MatchTypeRef base () const {
    MatchType *b = _type_info->base ();
    return b ? MatchTypeRef (b->clone ()) : MatchTypeRef ();
  }

  // check whether this type mattern can only match a single name type
  bool is_trivial_match () const { return _type_info->is_trivial_match(); }
  // get the associated name object
  const MatchName &name () const;
};

inline ostream &operator << (ostream &out, const MatchTypeRef &mt) {
  mt.print (out);
  return out;
}

#endif // __MatchType_h__
