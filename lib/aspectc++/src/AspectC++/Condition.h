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

#ifndef __Condition_h__
#define __Condition_h__

#include <set>
#include <iostream>
#include <string>
using namespace std;

#include "ACModel/Elements.h"
#include "Naming.h"

class PointCutExpr;
class AdviceInfo;
class ACM_Class;

typedef pair<const ACM_Class*, string> TypeCheck;
typedef set<TypeCheck> TypeCheckSet;
typedef set<const ACM_Class*> RecordSet;
typedef set<string> StringSet;

// this is the class, which users should apply to manage runtime conditions
// for joinpoints. It is responsible for the memory management.

class Condition {

  class Base {
  public:
    virtual ~Base () {}
    virtual void print (ostream&) const = 0;
    virtual void gen_check (ostream &, AdviceInfo*, const char*,
                            const std::string* = 0) const = 0;
    virtual void checks_for_that (TypeCheckSet &checks) const {}
    virtual void checks_for_target (TypeCheckSet &checks) const {}
    virtual void names_for_that (StringSet &names) const {}
    virtual void names_for_target (StringSet &names) const {}
  };

  class TypeCond : public Base {
    RecordSet _matching;
    string _name;
  public:
    TypeCond (const string &name) : _name (name) {}
    virtual ~TypeCond () {}
    void matches (const ACM_Class *cls) { _matching.insert (cls); }
    const RecordSet &matching () const { return _matching; }
    const string &name () const { return _name; }
    void type_checks (TypeCheckSet &checks) const {
      for (RecordSet::const_iterator iter = _matching.begin ();
          iter != _matching.end (); ++iter)
        checks.insert (TypeCheck (*iter, _name));
    }
  };
  
  class That : public TypeCond {
  public:
    That (const string &name) : TypeCond (name) {}
    virtual void print (ostream&) const;
    virtual void gen_check (ostream &, AdviceInfo*, const char*,
                            const std::string* = 0) const;
    virtual void checks_for_that (TypeCheckSet &checks) const {
      type_checks (checks);
    }
    virtual void names_for_that (StringSet &names) const {
      names.insert (name ());
    }
  };

  class Target : public TypeCond {
  public:
    Target (const string &name) : TypeCond (name) {}
    virtual void print (ostream&) const;
    virtual void gen_check (ostream &, AdviceInfo*, const char*,
                            const std::string* = 0) const;
    virtual void checks_for_target (TypeCheckSet &checks) const {
      type_checks (checks);
    }
    virtual void names_for_target (StringSet &names) const {
      names.insert (name ());
    }
  };

  class CFlow : public Base {
    int _index;
  public:
    CFlow (int i) : _index (i) {}
    virtual void print (ostream&) const;
    virtual void gen_check (ostream &, AdviceInfo*, const char*,
                            const std::string* = 0) const;
  };
  
  class Binary : public Base {
  protected:
    Base *_left;
    Base *_right;
  public:
    Binary (Base *left, Base *right) : _left (left), _right (right) {}
    virtual void checks_for_that (TypeCheckSet &result) const {
      _left->checks_for_that (result);
      _right->checks_for_that (result);
    }
    virtual void checks_for_target (TypeCheckSet &result) const {
      _left->checks_for_target (result);
      _right->checks_for_target (result);
    }
    virtual void names_for_that (StringSet &names) const {
      _left->names_for_that (names);
      _right->names_for_that (names);
    }
    virtual void names_for_target (StringSet &names) const {
      _left->names_for_that (names);
      _right->names_for_that (names);
    }
  };
  
  class And : public Binary {
  public:
    And (Base *left, Base *right) : Binary (left, right) {}
    virtual void print (ostream&) const;
    virtual void gen_check (ostream &, AdviceInfo*, const char*,
                            const std::string* = 0) const;
  };

  class Or : public Binary {
  public:
    Or (Base *left, Base *right) : Binary (left, right) {}
    virtual void print (ostream&) const;
    virtual void gen_check (ostream &, AdviceInfo*, const char*,
                            const std::string* = 0) const;
  };

  class Not : public Base {
    Base *_arg;
  public:
    Not (Base *arg) : _arg (arg) {}
    virtual void print (ostream&) const;
    virtual void gen_check (ostream &, AdviceInfo*, const char*,
                            const std::string* = 0) const;
    virtual void checks_for_that (TypeCheckSet &result) const {
      _arg->checks_for_that (result);
    }
    virtual void checks_for_target (TypeCheckSet &result) const {
      _arg->checks_for_target (result);
    }
    virtual void names_for_that (StringSet &names) const {
      _arg->names_for_that (names);
    }
    virtual void names_for_target (StringSet &names) const {
      _arg->names_for_target (names);
    }
  };
  
  class NeededShortCircuitArg : public Base {
    int _index_of_needed_sc_arg;
  public:
    NeededShortCircuitArg (int index_of_needed_sc_arg)
        : _index_of_needed_sc_arg(index_of_needed_sc_arg) {}
    virtual void print (ostream&) const;
    virtual void gen_check (ostream &, AdviceInfo*, const char*,
                            const std::string* = 0) const;
  };

  Base *_cond;
  
  Condition (Base *cond) : _cond (cond) {}
  
public:
  // create an unused condition
  Condition () : _cond (0) {}
  
  // delete a condition
  ~Condition () {
    // heap cleanup
    if (_cond)
      delete _cond;
  }
  
  // operator bool returns whether a condition is used
  operator bool () const { return _cond != 0; }

  // print the condition in a human readable way
  void print (ostream &out) {
    if (_cond)
      _cond->print (out);
    else
      out << "no condition";
  }

  void gen_check (ostream &out, AdviceInfo* ai, const char *srcthis,
                  const std::string* tjp_prefix) const {
    if (_cond)
      _cond->gen_check (out, ai, srcthis, tjp_prefix);
    else
      out << "true";
  }

  // return the set of checks needed for 'that' and target
  void checks_for_that (TypeCheckSet &result) const {
    if (_cond) _cond->checks_for_that (result);
  }
  void checks_for_target (TypeCheckSet &result) const {
    if (_cond) _cond->checks_for_target (result);
  }

  // return the names of all 'that' and 'target' checks in this condition
  void names_for_that (StringSet &names) const {
    if (_cond) _cond->names_for_that (names);
  }
  void names_for_target (StringSet &names) const {
    if (_cond) _cond->names_for_target (names);
  }
    
  // create a 'that' condition, i.e. the current object is an instance
  // of a specific class that matches the argument of 'that'
  void that (ACM_Class *in_class, const string &mangled_check);

  // the target object is an instance of a specific class
  void target (ACM_Class *in_class, const string &mangled_check);
  
  // we are currently running in a control flow from ...
  void cflow (int index) {
    _cond = new CFlow (index);
  }
  
  // assgin a condition
  void assign (Condition &other) {
    _cond = other._cond;
    other._cond = 0;
  }
  
  // combine two conditions with &&
  void op_and (Condition &other) {
    _cond = new And (_cond, other._cond);
    other._cond = 0;
  }

  // combine two conditions with ||
  void op_or (Condition &other) {
    _cond = new Or (_cond, other._cond);
    other._cond = 0;
  }

  // check if a condition is *not* true
  void op_not () {
    _cond = new Not (_cond);
  }

  // Create condition that checks, whether a short-circuit argument, that
  // is needed by an args pointcut function, is available at runtime
  void needed_short_circuit_arg (int arg_index) {
    _cond = new NeededShortCircuitArg(arg_index);
  }

  void matches (const ACM_Class &cls_loc);
};

inline ostream& operator<< (ostream& o, Condition &cond) {
  cond.print (o);
  return o;
}

#endif // __Condition_h__
