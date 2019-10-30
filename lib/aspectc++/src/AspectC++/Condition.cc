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

#include <iostream>
using namespace std;

#include "Condition.h"
#include "AdviceInfo.h"
#include "TransformInfo.h"

void Condition::That::print (ostream &out) const {
  out << "that(\"" << name  () << "\")";
}

void Condition::That::gen_check (ostream &out, AdviceInfo *ai,
                                 const char *srcthis,
                                 const string*) const {
  out << srcthis << "->" << name () << " ()";
}

void Condition::Target::print (ostream &out) const {
  out << "target(\"" << name () << "\")";
}

void Condition::Target::gen_check (ostream &out, AdviceInfo *ai,
                                   const char*,
                                   const string*) const {
  out << "dst." << name () << " ()";
}

void Condition::CFlow::print (ostream &out) const {
  out << "cflow[" << _index << "]";
}

void Condition::CFlow::gen_check (ostream &out, AdviceInfo *ai,
                                  const char*,
                                  const string*) const {
  Naming::cflow (out, ai->aspect (), _index);
  out << "::active ()";
}

void Condition::And::print (ostream &out) const {
  out << "(";
  _left->print (out);
  out << " && ";
  _right->print (out);
  out << ")";
}

void Condition::And::gen_check (ostream &out, AdviceInfo *ai,
                                const char *srcthis,
                                const string*) const {
  out << "(";
  _left->gen_check (out, ai, srcthis);
  out << "&&";
  _right->gen_check (out, ai, srcthis);
  out << ")";
}

void Condition::Or::print (ostream &out) const {
  out << "(";
  _left->print (out);
  out << " || ";
  _right->print (out);
  out << ")";
}

void Condition::Or::gen_check (ostream &out, AdviceInfo *ai,
                               const char *srcthis,
                               const string*) const {
  out << "(";
  _left->gen_check (out, ai, srcthis);
  out << "||";
  _right->gen_check (out, ai, srcthis);
  out << ")";
}

void Condition::Not::print (ostream &out) const {
  out << "!";
  _arg->print (out);
}

void Condition::Not::gen_check (ostream &out, AdviceInfo *ai,
                                const char *srcthis,
                                const string*) const {
  out << "!";
  _arg->gen_check (out, ai, srcthis);
}

void Condition::NeededShortCircuitArg::print (ostream &out) const {
  out << "{" << _index_of_needed_sc_arg << "th argument is needed}";
}

void Condition::NeededShortCircuitArg::gen_check (ostream &out, AdviceInfo *ai,
                                                  const char*,
                                                  const string* tjp_prefix) const {
  out << "(" << *tjp_prefix << "_args["
      << _index_of_needed_sc_arg << "] > 0)";
}

// create a 'that' condition, i.e. the current object is an instance
// of a specific class that matches the argument of 'that'
void Condition::that (ACM_Class *in_class, const string &mangled_check) {
  stringstream cond_name;
  Naming::type_check_func (cond_name, *in_class, mangled_check.c_str ());
  _cond = new That (cond_name.str ());
}

// the target object is an instance of a specific class
void Condition::target (ACM_Class *in_class, const string &mangled_check) {
  stringstream cond_name;
  Naming::type_check_func (cond_name, *in_class, mangled_check.c_str ());
  _cond = new Target (cond_name.str ());
}

void Condition::matches (const ACM_Class &cls_loc) {
  // this is a hack! I assume that the top-level node is a TypeCond.
  if (_cond)
    ((TypeCond*)_cond)->matches (&cls_loc);
}
