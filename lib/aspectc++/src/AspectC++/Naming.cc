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

#include <string.h>
#include <ctype.h>

#include "Naming.h"
#include "ACFileID.h"
#include "ACModel/Utils.h"
#include "AdviceInfo.h"
#include "JoinPoint.h"
#include "AspectInfo.h"

#include "Puma/SysCall.h"

void Naming::scope_name (ostream &out, ACM_Name &scope) {
  // check if we reached the global scope
  if (scope.get_name() == "::")
    return;

  // print the parent scope first
  if (scope.get_parent())
    scope_name (out, *(ACM_Name*)scope.get_parent());

  string name = scope.get_name();
  // print the scope name, preceeded by the length of the string
  if (name.substr(0, 5) == "<anon")
    out << "4ANON"; // handle this better!
  else if (name == "<unnamed>")
    out << "12_GLOBAL__N_1"; // same symbol as used by g++
  else if (name.rfind ('>') != string::npos)
    out << "4ANON"; // TODO: this is a hack - template parameters should be mangled!
  else
    out << scope.get_name().length() << scope.get_name();
}

bool Naming::is_unary_op (ACM_Function *func) {
  assert (func->get_name().substr(0,9) == "operator ");
  bool is_method = (func->get_kind () != FT_NON_MEMBER &&
      func->get_kind () != FT_STATIC_NON_MEMBER);
  return (is_method && func->get_arg_types ().size () == 0) ||
      (!is_method && func->get_arg_types ().size () == 1);
}

void Naming::op_name (ostream &out, ACM_Function *func) {
  string name = func->get_name ().substr (9);

  if (name == "new")
     out << "nw";
  else if (name == "new[]")
     out << "na";
  else if (name == "delete")
     out << "dl";
  else if (name == "delete[]")
     out << "da";
  else if (name == "-" && is_unary_op (func))
     out << "ng";
  else if (name == "&" && is_unary_op (func))
     out << "ad";
  else if (name == "*" && is_unary_op (func))
     out << "de";
  else if (name == "~")
     out << "co";
  else if (name == "+")
     out << "pl";
  else if (name == "-" && !is_unary_op (func))
     out << "mi";
  else if (name == "*" && !is_unary_op (func))
     out << "ml";
  else if (name == "/")
     out << "dv";
  else if (name == "%")
     out << "rm";
  else if (name == "&" && !is_unary_op (func))
     out << "an";
  else if (name == "|")
     out << "or";
  else if (name == "^")
     out << "eo";
  else if (name == "=")
     out << "aS";
  else if (name == "+=")
     out << "pL";
  else if (name == "-=")
     out << "mI";
  else if (name == "*=")
     out << "mL";
  else if (name == "/=")
     out << "dV";
  else if (name == "%=")
     out << "rM";
  else if (name == "&=")
     out << "aN";
  else if (name == "|=")
     out << "oR";
  else if (name == "^=")
     out << "eO";
  else if (name == "<<")
     out << "ls";
  else if (name == ">>")
     out << "rs";
  else if (name == "<<=")
     out << "lS";
  else if (name == ">>=")
     out << "rS";
  else if (name == "==")
     out << "eq";
  else if (name == "!=")
     out << "ne";
  else if (name == "<")
     out << "lt";
  else if (name == ">")
     out << "gt";
  else if (name == "<=")
     out << "le";
  else if (name == ">=")
     out << "ge";
  else if (name == "!")
     out << "nt";
  else if (name == "&&")
     out << "aa";
  else if (name == "||")
     out << "oo";
  else if (name == "++")
     out << "pp";
  else if (name == "--")
     out << "mm";
  else if (name == ",")
     out << "cm";
  else if (name == "->*")
     out << "pm";
  else if (name == "->")
     out << "pt";
  else if (name == "()")
     out << "cl";
  else if (name == "[]")
     out << "ix";
  else if (name == "?")
     out << "qu";
  else {
    // handle a conversion function
    out << "cv";
    mangle (out, func->get_result_type());
  }
}

void Naming::constr_name (ostream &out, ACM_Function *func) {
  out << "C1";
}

void Naming::destr_name (ostream &out, ACM_Function *func) {
  out << "D1";
}

// name mangling for variables and functions
void Naming::mangle (ostream &out, ACM_Name *obj) {

  // check if this object is a function:
  ACM_Function *func = (obj->type_val() == JPT_Function ? (ACM_Function*)obj : 0);

  // determine the scope
  ACM_Name *scope = (ACM_Name*)obj->get_parent();

//  // objects with C linkage and global namespace variables are not mangled
//  if (obj->Language () == CLanguage::LANG_C ||
//      (scope->GlobalScope () && !obj->FunctionInfo ())) {
//    out << obj->Name ();
//    return;
//  }
  // global namespace variables are not mangled (why not?)
  if (obj->type_val () != JPT_Function && scope->get_name () == "::") {
    out << obj->get_name ();
    return;
  }

  // mangle the function name
  out << "_Z";

  // print the (possibly nested) scope of the function
  if (scope->get_name () != "::") {
    out << "N";
    // possible CV qualifier of member functions must be printed here
    if (func) {
      if (func->get_cv_qualifiers () & CVQ_CONST)
        out << "K";
      if (func->get_cv_qualifiers () & CVQ_VOLATILE)
        out << "V";
    }
    scope_name (out, *scope);
  }

  if (func) {
    // print the function name (special handling)
    if (func->get_kind () == FT_CONSTRUCTOR)
      constr_name (out, func);
    else if (func->get_kind () == FT_DESTRUCTOR ||
        func->get_kind () == FT_VIRTUAL_DESTRUCTOR ||
        func->get_kind () == FT_PURE_VIRTUAL_DESTRUCTOR)
      destr_name (out, func);
    else if (func->get_name ().substr (0, 9) == "operator ") {
      op_name (out, func);
    }
    else
      out << func->get_name ().length () << func->get_name ();
  }
  else {
    out << obj->get_name ().length () << obj->get_name ();
  }

  // if this was a nested name, add the 'E'
  if (scope->get_name () != "::")
    out << "E";

  if (func) {
    // print the argument types (also mangled)
    if (get_arg_count (*func) == 0)
      out << "v";
    else {
      typedef ACM_Container<ACM_Type, true> C;
      C &arg_types = func->get_arg_types ();
      for (C::iterator i = arg_types.begin (); i != arg_types.end (); ++i)
        mangle (out, *i);
//        TI_Type::of (**i)->type_info ()->Mangled (out);
    }
  }
}

void Naming::mangle (ostream &out, ACM_Type *type) {
  MatchSignature &match_sig = type->get_match_sig();
  if (match_sig.is_new())
    match_sig.parse(format_type (*type));
  match_sig.type().mangle(out);
}

void Naming::mangle (ostream &out, const ACM_Arg *arg) {
  // we only mangle the type of the argument, not its name
  MatchSignature match_sig;
  match_sig.parse(arg->get_type());
  match_sig.type().mangle(out);
}

void Naming::bypass_id_class (ostream& out, ACM_Call *jpl) {
  out << "__ID";
  mangle (out, (ACM_Name*)jpl->get_parent ());
  if (jpl->get_lid () >= 0)
    out << "_" << jpl->get_lid ();
}

string Naming::bypass_caller_class (ACM_Class *cls) {
  ostringstream out;
  out << "__BYPASS";
  mangle (out, cls);
  return out.str ();
}

void Naming::access_wrapper (ostream& out, ACM_Access *jpl, unsigned depth, int wrapper_number) {
  out << "__" << jpl->type_str() << "_";

  ACM_Name *parent = get_explicit_parent( *jpl );
  mangle( out, parent );

  if (jpl->get_lid () >= 0)
    out << "_" << jpl->get_lid ();
  out << "_" << depth;
  if(wrapper_number >= 0) {
    out << "_" << wrapper_number;
  }
}

void Naming::exec_inner (ostream& out, ACM_Code *jpl) {
  ACM_Function *func = (ACM_Function*)jpl->get_parent ();
  assert (func && (func->type_val () == JPT_Function));

  out << "__exec_old_";
  if (func->get_kind () == FT_CONSTRUCTOR)
    constr_name (out, func);
  else if (func->get_kind () == FT_DESTRUCTOR ||
      func->get_kind () == FT_VIRTUAL_DESTRUCTOR ||
      func->get_kind () == FT_PURE_VIRTUAL_DESTRUCTOR)
    destr_name (out, func);
  else if (func->get_name ().substr (0, 9) == "operator ") {
    op_name (out, func);
  }
  else
    out << func->get_name ();
}

void Naming::action_wrapper (ostream &out, ACM_Any *loc, unsigned depth) {
  out << "__action_func";
}

void Naming::exec_advice (ostream& out, ACM_Execution *jpl, AdviceInfo *adv) {
  out << "__" << (adv->name ().c_str() + 1);
}

void Naming::call_advice (ostream& out, ACM_Call *jpl, AdviceInfo *adv) {
  out << "__" << (adv->name ().c_str() + 1);
}

void Naming::local_id (ostream& out, ACM_Code *jpl) {
  if (jpl->type_val () & JPT_Access) {
    ACM_Access *jpl_code = (ACM_Access*)jpl;
    if (jpl_code->get_lid () >= 0)
      out << "_" << jpl_code->get_lid();
  }
}

void Naming::tjp_struct(ostream& out, ACM_Code *loc, int depth) {
  out << "TJP_";

  ACM_Name *parent = get_explicit_parent( *loc );
  mangle( out, parent );

  local_id (out, loc);
  out << "_" << depth;
}

void Naming::tjp_instance(ostream& out, ACM_Code *loc) {
  out << "tjp";
}

void Naming::tjp_args_array(ostream& out, ACM_Code *loc) {
  out << "args_";

  ACM_Name *parent = get_explicit_parent( *loc );
  mangle( out, parent );

  local_id (out, loc);
}

void Naming::tjp_argtypes(ostream& out, ACM_Code *loc) {
  out << "argtypes_";

  ACM_Name *parent = get_explicit_parent( *loc );
  mangle( out, parent );

  local_id (out, loc);
}

void Naming::cflow (ostream& out, ACM_Aspect &jpl_aspect, int index) {
  out << "::AC::CFlow<" << signature(jpl_aspect) << "," << index << ">";
}

bool Naming::is_tjp_object (const char *candidate) {
  return strcmp (candidate, "tjp") == 0 ||
    strcmp (candidate, "thisJoinPoint") == 0;
}

void Naming::tjp_typedef (ostream& out, const char *name) {
  out << "__JP_" << name;
}

void Naming::type_check_func (ostream &out, ACM_Class &in, const string &name) {
  out << "__ac_";
  scope_name (out, in);
  out << "is_" << name;
}

void Naming::guard (ostream &out, ACFileID unit) {
  out << "__ac_guard_";
  mangle_file (out, unit);
}

void Naming::mangle_file (ostream &out, ACFileID file_id) {
  // TODO: better is unit->absolutePath() but the implementation is strange and
  // has to be changed.
  Puma::Filename name;
  if (Puma::SysCall::canonical (file_id.name ().c_str (), name))
    mangle_file (out, name.name ());
}

void Naming::mangle_file (ostream &out, const char *name) {
  const char *curr = name;
  while (*curr) {
    if (*curr == '_' || isalnum (*curr))
      out << *curr;
    else if (*curr == '/' || *curr == '\\' || *curr == ':' || *curr == '.')
      out << "_";
    else
      out << 'X' << (unsigned int)*(unsigned char*)curr;
    curr++;
  }
  out << "__";
}

