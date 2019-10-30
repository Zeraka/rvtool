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

// AspectC++ includes
#include "PointCutExpr.h"
#include "PointCutContext.h"
#include "ACErrorStream.h"
#include "Binding.h"
#include "TransformInfo.h"

// ACModel library includes
#include "ACModel/Utils.h"

#include <assert.h>
#include <sstream>
using std::stringstream;
using std::endl;

using namespace Puma;

void PointCutExpr::sem_args (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  // recursively do a semantic analysis of child nodes
  for (int i = 0; i < args (); i++)
    if (arg (i))
      arg (i)->semantics (err, context, warn_compat);
}

void PointCutExpr::check_arg_types (ACErrorStream &err, const char *func,
				    PCE_Type expected, PointCutContext &context) {
  // it is an error if something is provided, which was not expected
  for (int i = 0; i < args (); i++) {
    if (arg (i) && arg (i)->type () != expected) {
      err << sev_error << context.location()
	      << "'" << func << "' expects a "
	      << (expected == PCE_NAME ? "name" : "code") << " pointcut"
	      << " as argument";
        if (args  () > 1)
          err << " " << (i + 1);
        err << endMessage;
    }
  }
}

void PointCutExpr::check_arg_types_equal (ACErrorStream &err, const char *func,
    PointCutContext &context) {
  if (!arg (0)) return;
  // it is an error if the types of all arguments are not equal
  PCE_Type first_type = arg (0)->type ();
  for (int i = 1; i < args (); i++) {
    if (arg (i) && arg (i)->type () != first_type) {
      err << sev_error << context.location()
	  << "'" << func << "' expects that all argument types are equal"
	  << endMessage;
    }
  }
}

const ACM_Arg *PointCutExpr::get_binding (const char *name,
						ACErrorStream &err,
						PointCutContext &context) {
  if (!context.pct_func ()) {
    err << sev_error << context.location()
	    << "context variable '" << name
	    << "' is not support in this kind of pointcut expression"
	    << endMessage;
    return 0;
  }
  ArgSet &curr_arg_bindings = *context.arg_bindings ().top ();
  ACM_Arg *arginfo = (ACM_Arg*)0;
  typedef ACM_Container<ACM_Arg, true> Container;
  Container &arguments = context.pct_func()->get_args();
  int a = 0;
  for (Container::iterator i = arguments.begin (); i != arguments.end (); ++i) {
    if ((*i)->get_name() == name) {
      arginfo = *i;
      break;
    }
    a++;
  }
  if (!arginfo) {
    err << sev_error << context.location()
	    << "'" << name << "' is not in argument list" << endMessage;
    return 0;
  }
  return curr_arg_bindings.lookup (a);
}

bool PCE_SingleArg::check_derived_class (ACM_Class &cls,
  PointCutContext &context, Binding &binding, Condition &cond) {
    
  Condition unused;
  bool do_subclasses = true;
  typedef ACM_Container<ACM_Class, false> Container;
  Container &derived = cls.get_derived();
  for (Container::const_iterator i = derived.begin (); i != derived.end (); ++i) {
    if (arg (0)->match (**i, context, binding, unused)) {
      if (context.in_that () || context.in_target ()) {
        cond.matches (**i);
        do_subclasses = false; // don't check subclasses, but
                               // siblings
      }
      else
        return true;
    }
    if (do_subclasses && check_derived_class (**i, context,
        binding, cond))
      return true;
  }
  return !do_subclasses;
}

bool PCE_SingleArg::check_derived_func (ACM_Class &cls, ACM_Function &func,
  PointCutContext &context, Binding &binding, Condition &cond) {
  Condition unused;
  bool do_subclasses = true;
  typedef ACM_Container<ACM_Class, false> Container;
  Container &derived = cls.get_derived();
  for (Container::const_iterator i = derived.begin (); i != derived.end (); ++i) {
    typedef ACM_Container<ACM_Any, true> Container;
    Container &children = (*i)->get_children();
    for (Container::const_iterator j = children.begin (); j != children.end (); ++j) {
      if ((*j)->type_val () != JPT_Function)
        continue;
      ACM_Function &curr = *(ACM_Function*)*j;
      if (!have_same_name_and_args (func, curr))
         continue;
      if (arg (0)->match (curr, context, binding, cond))
        return true;
    }

    if (do_subclasses && check_derived_func (**i, func, context, 
      binding, cond))
      return true;
  }
  return !do_subclasses;
}

bool PCE_SingleArg::check_derived_var (ACM_Class &cls, ACM_Variable &var,
  PointCutContext &context, Binding &binding, Condition &cond) {
  Condition unused;
  bool do_subclasses = true;
  typedef ACM_Container<ACM_Class, false> Container;
  Container &derived = cls.get_derived();
  for (Container::const_iterator i = derived.begin (); i != derived.end (); ++i) {
    typedef ACM_Container<ACM_Any, true> Container;
    Container &children = (*i)->get_children();
    for (Container::const_iterator j = children.begin (); j != children.end (); ++j) {
      if ((*j)->type_val () != JPT_Variable)
        continue;
      ACM_Variable &curr = *(ACM_Variable*)*j;
      if (var.get_name () != curr.get_name ())
         continue;
      if (arg (0)->match (curr, context, binding, cond))
        return true;
    }

    if (do_subclasses && check_derived_var (**i, var, context,
      binding, cond))
      return true;
  }
  return !do_subclasses;
}


bool PCE_SingleArg::check_base_class (ACM_Class &cls,
        PointCutContext &context, Binding &binding, Condition &cond) {
  if (arg (0)->match (cls, context, binding, cond)) {
    return true;
  }
  typedef ACM_Container<ACM_Class, false> Container;
  Container &bases = cls.get_bases();
  for (Container::const_iterator i = bases.begin (); i != bases.end (); ++i) {
    if (check_base_class(**i, context, binding, cond))
      return true;
  }
  return false;
}

bool PCE_SingleArg::check_base_func (ACM_Class &cls, ACM_Function &func,
        PointCutContext &context, Binding &binding, Condition &cond) {
  typedef ACM_Container<ACM_Any, true> Container;
  Container &children = cls.get_children ();
  for (Container::const_iterator i = children.begin (); i != children.end (); ++i) {
    if ((*i)->type_val () != JPT_Function)
      continue;
    ACM_Function &curr = *(ACM_Function*)*i;
    if (!have_same_name_and_args (func, curr))
       continue;
    if (arg (0)->match (curr, context, binding, cond))
      return true;
  }

  typedef ACM_Container<ACM_Class, false> BContainer;
  BContainer &bases = cls.get_bases();
  for (BContainer::const_iterator i = bases.begin (); i != bases.end (); ++i) {
    if (check_base_func(**i, func, context, binding, cond))
      return true;
  }
  return false;
}

bool PCE_SingleArg::check_base_var (ACM_Class &cls, ACM_Variable &var,
        PointCutContext &context, Binding &binding, Condition &cond) {
  typedef ACM_Container<ACM_Any, true> Container;
  Container &children = cls.get_children ();
  for (Container::const_iterator i = children.begin (); i != children.end (); ++i) {
    if ((*i)->type_val () != JPT_Variable)
      continue;
    ACM_Variable &curr = *(ACM_Variable*)*i;
    if (var.get_name () != curr.get_name ())
       continue;
    if (arg (0)->match (curr, context, binding, cond))
      return true;
  }

  typedef ACM_Container<ACM_Class, false> BContainer;
  BContainer &bases = cls.get_bases();
  for (BContainer::const_iterator i = bases.begin (); i != bases.end (); ++i) {
    if (check_base_var(**i, var, context, binding, cond))
      return true;
  }
  return false;
}

bool PCE_SingleArg::check_scopes (ACM_Name *scope,
        PointCutContext &context, Binding &binding, Condition &cond) {
  do {
    // check whether the argument of the pointcut function matches 'scope'
    if (arg (0)->match (*scope, context, binding, cond))
      return true;
    // go to the next scope level
    scope = (ACM_Name*)scope->get_parent ();
  } while (scope);

  return false;
}

PCE_Type PCE_Classes::type () const {
  return PCE_NAME;
}

void PCE_Classes::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args (err, context, warn_compat);
  check_arg_types (err, "classes", PCE_NAME, context);
  _possible_types = JPT_Class;
}

bool PCE_Classes::evaluate (ACM_Any &jpl, PointCutContext &context,
                            Binding &binding, Condition &cond) {
//  cout << "classes: " << jpl.type_val () << " ";
//  if (jpl.type_val() & JPT_Code)
//   cout << signature((ACM_Code&)jpl);
//  else
//   cout << signature((ACM_Name&)jpl);
//  cout << endl;
  return jpl.type_val () == JPT_Class &&
    arg (0)->match (jpl, context, binding, cond);
}

PCE_Type PCE_Base::type () const {
  return PCE_NAME;
}

void PCE_Base::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args (err, context, warn_compat);
  check_arg_types (err, "base", PCE_NAME, context);
  _possible_types = (JoinPointType)arg (0)->possible_types ();
}

bool PCE_Base::evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond) {
  if (!(jpl.type_val () & (int)_possible_types))
    return false;
    
  if (jpl.type_val () & (JPT_Class | JPT_Aspect)) {
    // the class is a base of the argument class(es) if one of its derived
    // classes is described by the argument
    return check_derived_class ((ACM_Class&)jpl, context, binding, cond);
  }
  else if ((jpl.type_val () == JPT_Function)) {
    ACM_Function &func = (ACM_Function&)jpl;
    if (((ACM_Any*)func.get_parent ())->type_val () & (JPT_Class | JPT_Aspect)) {
      ACM_Class &cls = *(ACM_Class*)func.get_parent ();
      return check_derived_class (cls, context, binding, cond) ||
          check_derived_func (cls, func, context, binding, cond);
    }
  }
  else if ((jpl.type_val () == JPT_Variable)) {
    ACM_Variable &var = (ACM_Variable&)jpl;
    if (((ACM_Any*)var.get_parent ())->type_val () & (JPT_Class | JPT_Aspect)) {
      ACM_Class &cls = *(ACM_Class*)var.get_parent ();
      return check_derived_class (cls, context, binding, cond) ||
          check_derived_var (cls, var, context, binding, cond);
    }
  }
  return false;
}

PCE_Type PCE_Derived::type () const {
  return PCE_NAME;
}

void PCE_Derived::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args (err, context, warn_compat);
  check_arg_types (err, "derived", PCE_NAME, context);
  _possible_types = (JoinPointType)arg (0)->possible_types ();
}

bool PCE_Derived::evaluate (ACM_Any &jpl, PointCutContext &context,
                            Binding &binding, Condition &cond) {
  if (!(jpl.type_val () & (int)_possible_types))
    return false;
    
  if (jpl.type_val () & (JPT_Class | JPT_Aspect)) {
    // the class is derived of the argument class(es) if the class itself or
    // one of its base classes is described by the argument
    return check_base_class ((ACM_Class&)jpl, context, binding, cond);
  }
  else if ((jpl.type_val () == JPT_Function)) {
    ACM_Function &func = (ACM_Function&)jpl;
    if (((ACM_Any*)func.get_parent ())->type_val () & (JPT_Class | JPT_Aspect)) {
      ACM_Class &cls = *(ACM_Class*)func.get_parent ();
      return check_base_class (cls, context, binding, cond) ||
          check_base_func (cls, func, context, binding, cond);
    }
  }
  else if ((jpl.type_val () == JPT_Variable)) {
    ACM_Variable &var = (ACM_Variable&)jpl;
    if (((ACM_Any*)var.get_parent ())->type_val () & (JPT_Class | JPT_Aspect)) {
      ACM_Class &cls = *(ACM_Class*)var.get_parent ();
      return check_base_class (cls, context, binding, cond) ||
          check_base_var (cls, var, context, binding, cond);
    }
  }
  return false;
}

PCE_Type PCE_Within::type () const {
  return PCE_CODE;
}

void PCE_Within::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args (err, context, warn_compat);
  check_arg_types (err, "within", PCE_NAME, context);
  _possible_types = JPT_Code;
}

bool PCE_Within::evaluate (ACM_Any &jpl, PointCutContext &context,
                           Binding &binding, Condition &cond) {

  // 'within' matches only code join points
  if (!(jpl.type_val () & (int)JPT_Code))
    return false;
  ACM_Code &jpl_code = (ACM_Code&)jpl;
    
  // accept pseudo method calls
  if (is_pseudo (jpl_code)) {
    context.pseudo_true (true);
    return true;
  }

  ACM_Name *scope = lexical_scope (jpl_code);
  if (scope->type_val () == JPT_Function) {
    ACM_Function *func = (ACM_Function*)scope;
    if (func->get_kind() == FT_CONSTRUCTOR ||
        func->get_kind() == FT_DESTRUCTOR ||
        func->get_kind() == FT_VIRTUAL_DESTRUCTOR ||
        func->get_kind() == FT_PURE_VIRTUAL_DESTRUCTOR)
      scope = (ACM_Name*)func->get_parent ();
  }
  return arg(0)->match (*scope, context, binding, cond);
}

PCE_Type PCE_Member::type() const {
  return PCE_NAME;
}

void PCE_Member::semantics( ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args( err, context, warn_compat);
  check_arg_types( err, "member", PCE_NAME, context );
  _possible_types = JPT_Name;
}

bool PCE_Member::evaluate( ACM_Any &jpl, PointCutContext &context, Binding &binding, Condition &cond ) {

  // 'member' matches only name join points
  if( ! ( jpl.type_val() & JPT_Name ) )
    return false;

  // avoid conflict with old style scope checks
  if( context.in_old_style_scopematch() )
    return false;

  ACM_Name &named = static_cast<ACM_Name &>( jpl );
  JoinPointType pt = (JoinPointType) arg(0)->possible_types();

  // check the scope of the named program entity and all its parent scopes
  if( pt & ( JPT_Aspect | JPT_Class | JPT_Namespace ) ) {
    ACM_Any *scope = static_cast<ACM_Any *>( named.get_parent() );
    if( scope == 0 || ! ( scope->type_val() & JPT_Name ) )
      return false;

    return check_scopes( static_cast<ACM_Name *>( scope ), context, binding, cond );
  }

  return false;
}

PCE_Type PCE_Execution::type () const {
  return PCE_CODE;
}

void PCE_Execution::semantics (ACErrorStream &err,
			       PointCutContext &context, int warn_compat) {
  sem_args (err, context, warn_compat);
  check_arg_types (err, "execution", PCE_NAME, context);
  _possible_types = JPT_Execution;
}

bool PCE_Execution::evaluate (ACM_Any &jpl, PointCutContext &context,
                              Binding &binding, Condition &cond) {
  // consider execution join point only
  if (jpl.type_val () != JPT_Execution)
    return false;

  JoinPointType pt = (JoinPointType)arg (0)->possible_types ();

  // check if the executed function's name matches the argument
  if (pt & JPT_Function) {
    ACM_Function *function = (ACM_Function*)jpl.get_parent();
    assert(!function->get_builtin());
    return arg (0)->match (*function, context, binding, cond);
  }
  return false;
}

PCE_Type PCE_Call::type () const {
  return PCE_CODE;
}

void PCE_Call::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args (err, context, warn_compat);
  check_arg_types (err, "call", PCE_NAME, context);
  _possible_types = JPT_Call;
}

bool PCE_Call::evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond) {
  if (jpl.type_val () != JPT_Call)
    return false;

  JoinPointType pt = (JoinPointType)arg (0)->possible_types ();

  // check if the called function's name matches the argument
  if (pt & JPT_Function) {
    ACM_Function *target_function = ((ACM_Call&)jpl).get_target();
    assert(!target_function->get_builtin());
    return arg (0)->match (*target_function, context, binding, cond);
  }
  return false;
}

PCE_Type PCE_Builtin::type () const {
  return PCE_CODE;
}

void PCE_Builtin::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args (err, context, warn_compat);
  check_arg_types (err, "builtin", PCE_NAME, context);
  _possible_types = JPT_Builtin;
}

bool PCE_Builtin::evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond) {
  if (jpl.type_val () != JPT_Builtin)
    return false;

  JoinPointType pt = (JoinPointType)arg (0)->possible_types ();

  // check if the called operator's name matches the argument
  if (pt & JPT_Function) {
    ACM_Function *target_function = ((ACM_Builtin&)jpl).get_target();
    assert(target_function->get_builtin());
    return arg (0)->match (*target_function, context, binding, cond);
  }
  return false;
}

PCE_Type PCE_Get::type () const {
  return PCE_CODE;
}

void PCE_Get::semantics( ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args( err, context, warn_compat);
  check_arg_types( err, "get", PCE_NAME, context );
  _possible_types = static_cast<JoinPointType>( JPT_Get | JPT_GetRef );
}

bool PCE_Get::evaluate( ACM_Any &jpl, PointCutContext &context, Binding &binding, Condition &cond ) {
  if( ! ( jpl.type_val() & _possible_types ) )
    return false;

  JoinPointType pt = (JoinPointType)arg(0)->possible_types();

  if( jpl.type_val() == JPT_Get ) {
    // check if the accessed variables's name matches the argument
    ACM_Variable *target_variable = ((ACM_Get&)jpl).get_variable();
    if( pt & JPT_Variable )
      if( arg(0)->match( *target_variable, context, binding, cond ) )
        return true;
  }
  else if( jpl.type_val() == JPT_GetRef ) {
    // check alias-based joinpoints
    ACM_Type *var_type = static_cast<ACM_GetRef &>( jpl ).get_type();
    if( pt == JPT_Variable ) // match only vars as alias is for nothing else here, especially exclude types
      if( arg(0)->match( *var_type, context, binding, cond ) )
        return true;
  }

  return false;
}

PCE_Type PCE_Set::type () const {
  return PCE_CODE;
}

void PCE_Set::semantics( ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args( err, context, warn_compat);
  check_arg_types( err, "set", PCE_NAME, context );
  _possible_types = static_cast<JoinPointType>( JPT_Set | JPT_SetRef );
}

bool PCE_Set::evaluate( ACM_Any &jpl, PointCutContext &context, Binding &binding, Condition &cond ) {
  if( ! ( jpl.type_val() & _possible_types ) )
    return false;

  JoinPointType pt = (JoinPointType)arg(0)->possible_types();

  if( jpl.type_val() == JPT_Set ) {
    // check if the called function's name matches the argument
    ACM_Variable *target_variable = ((ACM_Set&)jpl).get_variable();
    if( pt & JPT_Variable )
      if( arg(0)->match( *target_variable, context, binding, cond ) )
        return true;
  }
  else if( jpl.type_val() == JPT_SetRef ) {
    // check alias-based joinpoints
    ACM_Type *var_type = static_cast<ACM_SetRef &>( jpl ).get_type();
    if( pt == JPT_Variable ) // match only vars as alias is for nothing else here, especially exclude types
      if( arg(0)->match( *var_type, context, binding, cond ) )
        return true;
  }

  return false;
}

PCE_Type PCE_Ref::type () const {
  return PCE_CODE;
}

void PCE_Ref::semantics( ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args( err, context, warn_compat);
  check_arg_types( err, "ref", PCE_NAME, context );
  _possible_types = JPT_Ref;
}

bool PCE_Ref::evaluate( ACM_Any &jpl, PointCutContext &context, Binding &binding, Condition &cond ) {
  if( jpl.type_val () != JPT_Ref )
    return false;

  JoinPointType pt = (JoinPointType)arg(0)->possible_types();

  // check if the called function's name matches the argument
  ACM_Variable *target_variable = ((ACM_Ref&)jpl).get_variable();
  if( pt & JPT_Variable )
    if( arg(0)->match( *target_variable, context, binding, cond ) )
      return true;

  return false;
}

PCE_Type PCE_Alias::type () const {
  return PCE_NAME;
}

void PCE_Alias::semantics( ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args( err, context, warn_compat);
  check_arg_types( err, "alias", PCE_NAME, context );
  _possible_types = JPT_Variable;
}

bool PCE_Alias::evaluate( ACM_Any &jpl, PointCutContext &context, Binding &binding, Condition &cond ) {
  if( jpl.type_val () != JPT_Type ) // only type based matching yet
    return false;

  JoinPointType pt = (JoinPointType)arg(0)->possible_types();

  if( pt & JPT_Variable ) {
    ACM_Type &type = static_cast<ACM_Type &>( jpl );
    ProjectModel::Selection refs;
    context.jpm().select( JPT_Ref, refs );

    for( ProjectModel::Selection::iterator iter = refs.begin(); iter != refs.end(); ++iter ) { // check all possible variables
      ACM_Variable &var = *( static_cast<ACM_Ref &>( **iter ).get_variable() );
      if( ( type.get_signature() == var.get_type()->get_signature() ) && arg(0)->match( var, context, binding, cond ) )
        return true;
    }
  }

  return false;
}

PCE_Type PCE_Construction::type () const {
  return PCE_CODE;
}

void PCE_Construction::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args (err, context, warn_compat);
  check_arg_types (err, "construction", PCE_NAME, context);
  _possible_types = JPT_Construction;
}

bool PCE_Construction::evaluate (ACM_Any &jpl, PointCutContext &context,
                                 Binding &binding, Condition &cond) {
  if (jpl.type_val () != JPT_Construction)
    return false;
  ACM_Name *scope = lexical_scope ((ACM_Construction&)jpl);
  return arg (0)->match (*scope, context, binding, cond);
}

PCE_Type PCE_Destruction::type () const {
  return PCE_CODE;
}

void PCE_Destruction::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args (err, context, warn_compat);
  check_arg_types (err, "destruction", PCE_NAME, context);
  _possible_types = JPT_Destruction;
}

bool PCE_Destruction::evaluate (ACM_Any &jpl, PointCutContext &context,
                                Binding &binding, Condition &cond) {
  if (jpl.type_val () != JPT_Destruction)
    return false;
  ACM_Name *scope = lexical_scope ((ACM_Destruction&)jpl);
  return arg (0)->match (*scope, context, binding, cond);
}


PCE_Type PCE_That::type () const {
  return PCE_CODE;
}

void PCE_That::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  context.enter_that ();
  sem_args (err, context, warn_compat);
  context.leave_that ();
  check_arg_types (err, "that", PCE_NAME, context);
  _possible_types = JPT_Code;
}

bool PCE_That::evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond) {
  // 'that' matches only code join points
  if (!(jpl.type_val () & (int)JPT_Code))
    return false;
  ACM_Code &jpl_code = (ACM_Code&)jpl;
  if (is_pseudo(jpl)) {
    context.pseudo_true (true);
    return true;
  }

  // get the function that is executed when the join point is reached
  ACM_Name *parent = (ACM_Name*)jpl_code.get_parent ();
  if (parent && parent->type_val() == JPT_Builtin)
    parent = (ACM_Name*)parent->get_parent();
  if (parent->type_val () != JPT_Function)
    return false;
  ACM_Function *jpl_function = (ACM_Function*)parent;

  // static (member) functions and non-member functions don't have a 'this' pointer
  if (jpl_function->get_kind() == FT_NON_MEMBER ||
      jpl_function->get_kind() == FT_STATIC_NON_MEMBER ||
      jpl_function->get_kind() == FT_STATIC_MEMBER)
    return false;

  // get the class of which the function is a member
  ACM_Class *class_loc = (ACM_Class*)jpl_function->get_parent ();
  
  context.enter_that ();
  binding._this = (ACM_Arg*)0;

  // if any of the class' base classes matches the argument the current class
  // is definitely an object of the required type
  if (!check_base_class (*class_loc, context, binding, cond)) {
    // in a construction or destruction the object has exactly the type
    // of the constructor/destructor scope => no run-time check

    if (jpl_function->get_kind() == FT_CONSTRUCTOR ||
        jpl_function->get_kind() == FT_DESTRUCTOR  ||
        jpl_function->get_kind() == FT_VIRTUAL_DESTRUCTOR ||
        jpl_function->get_kind() == FT_PURE_VIRTUAL_DESTRUCTOR) {
      context.leave_that ();
      return false;
    }
      
    // create a 'that' condition with a mangled name
    stringstream mangled_check;
    arg (0)->mangle_type_check (mangled_check);
    cond.that (class_loc, mangled_check.str ());

    // find the derived classes that match the argument
    check_derived_class (*class_loc, context, binding, cond);
  }
  context.leave_that ();
  return true;
}

PCE_Type PCE_Target::type () const {
  return PCE_CODE;
}

void PCE_Target::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  context.enter_target ();
  sem_args (err, context, warn_compat);
  context.leave_target ();
  check_arg_types (err, "target", PCE_NAME, context);
  _possible_types = JPT_Call|JPT_Set|JPT_Get|JPT_Ref;
}

bool PCE_Target::evaluate (ACM_Any &jpl, PointCutContext &context,
                           Binding &binding, Condition &cond) {
  ACM_Class *class_loc = 0;

  if (jpl.type_val () == JPT_Call) {
    ACM_Call &jpl_call = (ACM_Call&)jpl;

    ACM_Function *target_function = jpl_call.get_target();

    // only non-static member functions are relevant here
    // TODO: how to handle explicit destructor calls?
    if (target_function->get_kind() != FT_MEMBER &&
        target_function->get_kind() != FT_VIRTUAL_MEMBER &&
        target_function->get_kind() != FT_PURE_VIRTUAL_MEMBER)
      return false;

    // get the class of which the function is a member
    class_loc = (ACM_Class*)target_function->get_parent ();
  }
  else if (jpl.type_val() & (JPT_Set|JPT_Get|JPT_Ref)) {
    ACM_Variable *target_var = 0;
    if (jpl.type_val () == JPT_Set) {
      ACM_Set &jpl_set = (ACM_Set&)jpl;
      target_var = jpl_set.get_variable();
    }
    else if (jpl.type_val () == JPT_Get) {
      ACM_Get &jpl_get = (ACM_Get&)jpl;
      target_var = jpl_get.get_variable();
    }
    else if (jpl.type_val () == JPT_Ref) {
      ACM_Ref &jpl_ref = (ACM_Ref&)jpl;
      target_var = jpl_ref.get_variable();
    }
    if (!target_var || target_var->get_kind() != VT_MEMBER)
      return false;

    // get the class of which the variable is a member
    class_loc = (ACM_Class*)target_var->get_parent ();
  }

  if (!class_loc)
    return false; // shouldn't happen
  
  context.enter_target ();
  binding._target = (ACM_Arg*)0;
  // if any of the class' base classes matches the argument the current class
  // is definitely an object of the required type
  if (!check_base_class (*class_loc, context, binding, cond)) {
    // create a 'that' condition with a mangled name
    stringstream mangled_check;
    arg (0)->mangle_type_check (mangled_check);
    cond.target (class_loc, mangled_check.str ());

    // find the derived classes that match the argument
    check_derived_class (*class_loc, context, binding, cond);
  }
  context.leave_target ();
  return true;
}

PCE_Type PCE_CFlow::type () const {
  return PCE_CODE;
}

void PCE_CFlow::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args (err, context, warn_compat);
  check_arg_types (err, "cflow", PCE_CODE, context);
  // evaluate the argument pointcut
  ProjectModel::Selection all;
  context.jpm ().select (JPT_Code, all);
  for (ProjectModel::Selection::iterator iter = all.begin ();
       iter != all.end (); ++iter) {
    ACM_Any &jpl = *(ACM_Any*)*iter;
    Binding cflow_binding;
    Condition cflow_condition;
    context.pseudo_true (false); // todo sichern!
    if (arg (0)->match (jpl, context, cflow_binding, cflow_condition)) {
      if (cflow_binding._used) {
        err << sev_error << context.location()
            << "context variables not supported in cflows" << endMessage;
        break;
      }
      if (cflow_condition) {
        err << sev_error << context.location()
            << "runtime conditions not supported in cflows" << endMessage;
        break;
      }
      _arg_pointcut.append (*new JoinPoint (&jpl, cflow_condition));
    }
  }
  _index = context.cflow (this); // remember this node in the context
  _possible_types = JPT_Code;
}

bool PCE_CFlow::evaluate (ACM_Any &jpl, PointCutContext &context,
                          Binding &binding, Condition &cond) {
  // check if this join point is one of the cflow starting points
  if (_arg_pointcut.find (&jpl) != _arg_pointcut.end ())
    return true; // no runtime condition check!
    
  // every other joinpoint might be in the cflow (space for improvement)
  cond.cflow (_index);
  return true;
}

PCE_Type PCE_Args::type () const {
  return PCE_CODE;
}

void PCE_Args::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  check_arg_types (err, "args", PCE_NAME, context);

  // recursively do a semantic analysis of child nodes
  int saved_arg = context.get_arg ();
  for (int i = 0; i < args (); i++) {
    context.set_arg (i);
    if (arg (i))
      arg (i)->semantics (err, context,warn_compat);
  }
  context.set_arg (saved_arg);
  _possible_types = JPT_Code;
}

bool PCE_Args::evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond) {

  // ignore all non-code join points
  if (!(jpl.type_val () & (int)JPT_Code))
    return false;
  ACM_Code *jpl_code = (ACM_Code*)&jpl;

  // check if the number of arguments is ok -> not compatible ("...")
  if (args () != get_arg_count(*jpl_code))
    return false;

  // check if all argument types match
  int saved_arg = context.get_arg ();
  list<ACM_Type*> type_list;
  get_arg_types (*jpl_code, type_list);
  unsigned a = 0;
  for (list<ACM_Type*>::iterator i = type_list.begin(); i != type_list.end(); ++i) {
    context.set_arg (a);
    binding._args[a] = (ACM_Arg*)0;
    ACM_Type &type_loc = (ACM_Type&)**i;
    if (!arg (a)->match (type_loc, context, binding, cond)) {
      context.set_arg (saved_arg);
      return false;
    }
    a++;
  }
  context.set_arg (saved_arg);

  // args should not produce runtime checks until here
  assert(!cond);

#ifdef FRONTEND_CLANG
  // Check whether the join-point uses short-circuit-evaluation:
  if(jpl_code->type_val() == JPT_Builtin){
    const TI_Builtin* transform_info = TI_Builtin::of(*static_cast<ACM_Builtin*>(jpl_code));
    // Check whether this is no pseudo-call-join-point and a short-circuiting-join-point:
    if(transform_info->TI_Access::tree_node() > 0 && transform_info->is_short_circuiting()) {
      // On join-points with short-circuit evaluation it is not guaranteed
      // that the second (or third in case of "?:"-operator) argument is available. Because the
      // decision about the evaluation of the second (or third) argument
      // comes at runtime, we first determine the short-circuit argument, that is bound by the
      // current args pointcut function, and afterwards we create a condition that compares the
      // available short-circuit argument with the needed argument at runtime.

      int needed_arg = -1;
      if(transform_info->is_ternary_expr()) {
        // Operator "?:"
        if(binding._args[1] > 0 && binding._args[2] > 0) {
          // Both short-circuiting arguments are bound, but it is not possible that
          // both short-circuiting arguments are available at the same time. We need
          // no runtime check and generate a warning if wanted. The warning will be
          // created only once per args pointcut function.
          if(context.config().warn_limitations() && !_both_sc_args_bound_warning_created) {
            context.messages().push_back(pair<ErrorSeverity, string>(
                sev_warning, string("args pointcut function: join points at calls to the "
                "short-circuiting operator '?:' do not match (reason: both short-circuit "
                "arguments were bound, but they are never available at the same time).")));
            _both_sc_args_bound_warning_created = true;
          }
          return false;
        }
        else if(binding._args[1] == 0 && binding._args[2] == 0) {
          // Both short-circuiting arguments are *not* bound: We do not need a runtime check
          return true;
        }
        // Otherwise determine the needed/bound argument:
        needed_arg = binding._args[1] > 0 ? 1 : 2;
      }
      else {
        // Operator "&&" or "||": Check for first argument:
        if(binding._args[1] > 0) {
          needed_arg = 1;
        }
      }

      // Check if an short-circuit argument was bound and we therefore need an argument at
      // runtime:
      if(needed_arg != -1) {
        // We need an argument: create condition:
        cond.needed_short_circuit_arg(needed_arg);
        // If wanted we create a warning. This warning will be created only once per
        // args pointcut function.
        if(context.config().warn_limitations() && !_sc_arg_bound_warning_created) {
          context.messages().push_back(pair<ErrorSeverity, string>(
          sev_warning, string("args pointcut function: matching of join points at calls "
          "to short-circuiting operators '&&', '||' and '?:' is runtime dependent "
          "(reason: a short-circuit argument is bound by the args pointcut function and "
          "a match only occurs if the bound argument is available at runtime).")));
          _sc_arg_bound_warning_created = true;
        }
      }
    }
  }
#endif // FRONTEND_CLANG

  return true;
}

PCE_Type PCE_Result::type () const {
  return PCE_CODE;
}

void PCE_Result::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  context.enter_result ();
  sem_args (err, context, warn_compat);
  context.leave_result ();
  check_arg_types (err, "result", PCE_NAME, context);
  _possible_types = (JoinPointType)(JPT_Execution|JPT_Call|JPT_Builtin|JPT_Get);
}

bool PCE_Result::evaluate (ACM_Any &jpl, PointCutContext &context,
                           Binding &binding, Condition &cond) {

  // only execution, call and builtin join points have a result
  if (jpl.type_val () != JPT_Execution &&
      jpl.type_val () != JPT_Call &&
      jpl.type_val () != JPT_Builtin &&
      jpl.type_val () != JPT_Get)
    return false;
  
  ACM_Code *jpl_code = (ACM_Code*)&jpl;
  assert (has_result_type(*jpl_code));

  ACM_Type &type_loc = *get_result_type(*jpl_code);

  context.enter_result ();
  binding._result = (ACM_Arg*)0;
  bool result = arg (0)->match (type_loc, context, binding, cond);
  context.leave_result ();
  return result;
}


PCE_Type PCE_Or::type () const {
  return arg (0)->type ();
}

void PCE_Or::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args (err, context, warn_compat);
  check_arg_types_equal (err, "||", context);
  _possible_types = (JoinPointType)
    (arg(0)->possible_types () | arg(1)->possible_types ());
}

bool PCE_Or::evaluate (ACM_Any &jpl, PointCutContext &context,
                       Binding &binding, Condition &cond) {
  Condition subcond0, subcond1;
  bool subresult0, subresult1;
  subresult0 = arg (0)->match (jpl, context, binding, subcond0);
  subresult1 = arg (1)->match (jpl, context, binding, subcond1);
  // if both subresults are false the disjunction is false, too
  if (!(subresult0 || subresult1))
    return false;
    
  // if any of the subresults was an unconditional true the result is true
  if ((subresult0 && !subcond0) || (subresult1 && !subcond1))
    return true;
    
  // at least one subresult was true, now we consider possible conditions
  if (subcond0) {
    cond.assign (subcond0);
    if (subcond1)
      cond.op_or (subcond1);
  }
  else if (subcond1) {
    cond.assign (subcond1);
  }
  return true;
}

PCE_Type PCE_And::type () const {
  return arg (0)->type ();
}

void PCE_And::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args (err, context, warn_compat);
  check_arg_types_equal (err, "&&", context);
  _possible_types = (JoinPointType)
    (arg(0)->possible_types () & arg(1)->possible_types ());
  if (_possible_types == (JoinPointType)0) {
    err << sev_warning << context.location()
        << "Result of '&&' always false! Check the types of the argument pointcut expressions."
        << endMessage;
    return;
  }
}

bool PCE_And::evaluate (ACM_Any &jpl, PointCutContext &context,
                        Binding &binding, Condition &cond) {
  Condition subcond0, subcond1;
  bool subresult0, subresult1;
  subresult0 = arg (0)->match (jpl, context, binding, subcond0);
  bool pseudo_true0 = context.is_pseudo_true ();
  context.pseudo_true (false);
  subresult1 = arg (1)->match (jpl, context, binding, subcond1);
  bool pseudo_true1 = context.is_pseudo_true ();
  context.pseudo_true (pseudo_true0 && pseudo_true1);
  
  // if any subresult was false the conjunction is false, too
  if (!(subresult0 && subresult1))
    return false;
  // both subresult were true, now we consider possible conditions
  if (subcond0) {
    cond.assign (subcond0);
    if (subcond1)
      cond.op_and (subcond1);
  }
  else if (subcond1) {
    cond.assign (subcond1);
  }
  return true;
}

PCE_Type PCE_Not::type () const {
  return arg (0)->type ();
}

void PCE_Not::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  sem_args (err, context, warn_compat);
  check_arg_types (err, "!", type (), context);
  if (type () == PCE_NAME)
    _possible_types = JPT_Name;
  else if (type () == PCE_CODE)
    _possible_types = JPT_Code;
  else
    _possible_types = (JoinPointType)0;
}

bool PCE_Not::evaluate (ACM_Any &jpl, PointCutContext &context,
                        Binding &binding, Condition &cond) {
  Condition subcond;
  bool subresult = arg (0)->match (jpl, context, binding, subcond);
  // if the argument was true because we evaluate a pseudo call return true
  if (context.is_pseudo_true ())
    return true;
  // if the subexpression evaluates to an unconditional 'true' or false the
  // result must be inverted
  if (!subcond)
    return !subresult;
  // otherwise the result is 'true' but the condition must be negated
  else {
    cond.assign (subcond);
    cond.op_not ();
    return true;
  }
}

PCE_Type PCE_Named::type () const {
  assert(_expr.get ());
  return ((PointCutExpr*)_expr.get ())->type ();
}

void PCE_Named::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {

  // link invocations of virtual pointcut functions to the right implementation
  if (_pct_func->get_kind() == PT_VIRTUAL ||
      _pct_func->get_kind() == PT_PURE_VIRTUAL) {
    PointCutExpr *late_bound_pct = context.lookup_virtual_pct_func(_pct_func);
    if (!late_bound_pct) { // TODO: error handling
      err << sev_fatal << context.location() << "Virtual named pointcut '"
          << _pct_func->get_name().c_str() << "' not found." << endMessage;
      return;
    }
    // TODO: better let 'lookup...' return the pointcut function and use the
    // copy operator of 'PointcutExpression' here.
    _expr.set(late_bound_pct->duplicate());
  }
  else
    _expr = _pct_func->get_parsed_expr();

  ArgSet new_arg_bindings;
  for (std::list<std::string>::iterator i = params().begin ();
      i != params().end(); ++i) {
    // Argument must be a formal paramete in the current context, lookup it up
    ACM_Arg *argument = (ACM_Arg*)get_binding((*i).c_str (), err, context);
    if (!argument) {
      err << sev_error << context.location() << "Argument '" << (*i).c_str()
      << "' undefined." << endMessage;
    }
    new_arg_bindings.append(argument);
  }

  // set the current function pointer for context var lookups
  ACM_Pointcut *saved_pct_func = context.pct_func (_pct_func);

  // push the argument binding
  context.arg_bindings ().push (&new_arg_bindings);

  // analyze referenced expression tree
  ((PointCutExpr*)_expr.get ())->semantics(err, context, warn_compat);

  // pop the argument binding from the stack again
  context.arg_bindings ().pop ();

  context.pct_func (saved_pct_func); // restore old function pointer
  
  check_arg_types (err, "pointcut", type (), context);

  _possible_types = ((PointCutExpr*)_expr.get ())->possible_types ();
}

bool PCE_Named::evaluate (ACM_Any &jpl, PointCutContext &context,
                          Binding &binding, Condition &cond) {
  assert(_expr.get ());
  return ((PointCutExpr*)_expr.get ())->match (jpl, context, binding, cond);
}

PCE_Type PCE_ContextVar::type () const {
  return PCE_NAME;
}

bool PCE_CXX11Attr::evaluate(ACM_Any &jpl, PointCutContext &context, Binding &binding, Condition &cond)
{
  // check whether this is a name join point. Only name joinpoints can have attributes!
  if ((jpl.type_val() & JPT_Name) == 0)
    return false;

  ACM_Name *jpl_name = (ACM_Name*)&jpl;
  while (true) {

    // test if this attributes was annotated at the join point
    for (ACM_Attribute *attr :jpl_name->get_attributes())
      if(*attr == *_pct_attr)
        return true;

    // otherwise check the parent scope of the current name join point,
    // but omit the parent classes of nested classes
    ACM_Name *parent = (ACM_Name*)jpl_name->get_parent();
    if (jpl_name->type_val () == JPT_Class || jpl_name->type_val () == JPT_Aspect) {
      while (parent && (parent->type_val () == JPT_Class || parent->type_val () == JPT_Aspect))
        parent = (ACM_Name*)parent->get_parent();
    }
    // root scope detected -> stop here
    if (!parent || parent == context.jpm().get_root())
      break;

    // continue by matching the parent scope
    jpl_name = parent;
  }

  // not found, even in any parent scope
  return false;
}

void PCE_CXX11Attr::semantics(ACErrorStream &err, PointCutContext &context, int warn_compat)
{
  _possible_types = JPT_Any;
}

void PCE_ContextVar::semantics (ACErrorStream &err,
				PointCutContext &context, int warn_compat) {
  if (!(_bound_arg  = get_binding (_name.c_str (), err, context)))
    return;
  // TODO: can we get rid of the Puma/parser dependency?
#ifdef FRONTEND_PUMA
  _check_type = TI_Arg::of(*_bound_arg)->type_info();

  if (context.in_that () || context.in_target ()) {
    if (_check_type->isPointer () && _check_type->BaseType ()->is_void ()) {
      // "void*" matches any class!
      _check_type = 0;
    }
    else {
      if (_check_type->isPointer () || _check_type->isAddress ())
        _check_type = _check_type->BaseType ();
      if (!_check_type->isClass ()) {
#else // FRONTEND_CLANG
  _check_type = TI_Arg::of(*_bound_arg)->type();

  if (context.in_that () || context.in_target ()) {
    if (_check_type->isPointerType() && _check_type->isVoidPointerType()) {
      // "void*" matches any class!
      _check_type = clang::QualType();
    }
    else {
      if (_check_type->isPointerType() || _check_type->isReferenceType())
        _check_type = _check_type->getPointeeType().getCanonicalType ();
      if (!_check_type->isRecordType ()) {
#endif
        err << sev_error << context.location()
            << "argument of 'that' or 'target' must be a class, pointer to "
            << "class, or reference to class" << endMessage;
      }
    }
    _bound_to = context.in_that () ? CV_THAT : CV_TARGET;
  }
  else if (context.in_result ()) {
    _bound_to = CV_RESULT;
  }
  else {
    _bound_to = CV_ARG;
    _arg = context.get_arg ();
  }
  if (_bound_to == CV_ARG || _bound_to == CV_RESULT)
    _possible_types = JPT_Type;
  else if (_bound_to == CV_TARGET || _bound_to == CV_THAT)
    _possible_types = (JoinPointType)
      (JPT_Class|JPT_Aspect);
  else
    _possible_types = (JoinPointType)0;
}

bool PCE_ContextVar::evaluate (ACM_Any &jpl, PointCutContext &context,
                               Binding &binding, Condition &cond) {

  assert (((_bound_to == CV_ARG || _bound_to == CV_RESULT) &&
           jpl.type_val () == JPT_Type) ||
	        ((_bound_to == CV_TARGET || _bound_to == CV_THAT) && 
	         (jpl.type_val () & (JPT_Class | JPT_Aspect))));

  // determine the current type
#ifdef FRONTEND_PUMA
  CTypeInfo *curr_type = 0;
  if (jpl.type_val () == JPT_Type)
    curr_type = TI_Type::of ((ACM_Type&)jpl)->type_info ();
  else if (jpl.type_val () == JPT_Class ||
           jpl.type_val () == JPT_Aspect) {
    curr_type = ((TI_Class*)((ACM_Class&)jpl).transform_info ())->class_info ()->TypeInfo ();
  }

  assert (curr_type);
  if (!curr_type)
    return false;

  // check if the current type matches
  if (_check_type && *_check_type != *curr_type) {
    return false;
  }
#else // FRONTEND_CLANG
  clang::QualType curr_type;
  if (jpl.type_val () == JPT_Type)
    curr_type = TI_Type::of ((ACM_Type&)jpl)->type ();
  else if (jpl.type_val () == JPT_Class ||
           jpl.type_val () == JPT_Aspect) {
    curr_type =
        clang::QualType(((TI_Class *)((ACM_Class &)jpl).transform_info())
                            ->decl()->getTypeForDecl(),
                        0);
  }

  assert (!curr_type.isNull());
  if (curr_type.isNull())
    return false;
    
  // check if the current type matches
  if (!_check_type.isNull() &&
      _check_type.getCanonicalType () != curr_type.getCanonicalType ()) {
    return false;
  }
#endif

  // do the binding
  binding._used = true;
  switch (_bound_to) {
  case CV_THAT:
    binding._this = (ACM_Arg*)_bound_arg;
    break;
  case CV_TARGET:
    binding._target = (ACM_Arg*)_bound_arg;
    break;
  case CV_RESULT:
    binding._result = (ACM_Arg*)_bound_arg;
    break;
  case CV_ARG:
    binding._args[_arg] = (ACM_Arg*)_bound_arg;
    break;
  }
  return true;
}

void PCE_ContextVar::mangle_type_check (ostream &out) {
  Naming::mangle(out, _bound_arg);
}


PCE_Type PCE_Match::type () const {
  return PCE_NAME;
}

bool PCE_Match::parse () {
  return _match_expr.parse (_str);
}

void PCE_Match::semantics (ACErrorStream &err, PointCutContext &context, int warn_compat) {
  if (_match_expr.is_new() && !_match_expr.parse (_str))
    return; // TODO: What about error handling here?
  if (_match_expr.is_function ()) {
//    if (_match_expr.name ().is_trivial()) {
//      _pre_evaluate = PRE_EVAL_ON;
//    }
    _possible_types = JPT_Function;
  }
  else if( _match_expr.is_attribute() ) {
//    if( _match_expr.name().is_trivial() ) {
//      _pre_evaluate = PRE_EVAL_ON;
//    }
    _possible_types = JPT_Variable;
  }
  else if (_match_expr.is_type ()) {
//    if (_match_expr.type().is_trivial_match())
//      _pre_evaluate = PRE_EVAL_ON;
    _possible_types = (JoinPointType)( JPT_Class | JPT_Aspect | JPT_Namespace | JPT_Variable | JPT_Function);
  }
  else
    _possible_types = (JoinPointType)0;
}

void PCE_Match::pre_evaluate (PointCutContext &context) {
  if (_match_expr.is_function ())
    pre_evaluate (_match_expr.name (), context);
  else if( _match_expr.is_attribute() )
    pre_evaluate( _match_expr.name(), context );
  else if (_match_expr.is_type ())
    pre_evaluate (_match_expr.type().name (), context);
}

void PCE_Match::pre_evaluate (const MatchName &name, PointCutContext &context) {
  ACM_Name *scope = context.jpm().get_root();

  if( name.scopes() == 0 && name.name() == "::" ) {
    _pre_evaluate = PRE_EVAL_DONE;
    _jpl = scope;
    return;
  }

  for (int i = 0; scope && i < name.scopes (); i++) {
    scope = map_lookup (*scope, name.scope (i));
  }
  _pre_evaluate = PRE_EVAL_OFF;
  if (scope) {
    if (_match_expr.is_type()) {
      _pre_evaluate = PRE_EVAL_DONE;
      _jpl = map_lookup (*scope, name.name ());
    }
    else if (_match_expr.is_function()) {
      _pre_evaluate = PRE_EVAL_DONE;
      _jpl = 0;
      typedef ACM_Container<ACM_Any, true> CList;
      const CList &clist = scope->get_children();
      for (CList::const_iterator i = clist.begin (); i != clist.end(); ++i) {
        if ((*i)->type_val () != JPT_Function)
          continue;
        ACM_Function *func = (ACM_Function*)(*i);
        if (func->get_kind() == FT_CONSTRUCTOR ||
            func->get_kind() == FT_DESTRUCTOR ||
            func->get_kind() == FT_VIRTUAL_DESTRUCTOR ||
            func->get_kind() == FT_PURE_VIRTUAL_DESTRUCTOR)
          continue;
        MatchSignature &sig = ((ACM_Function*)(*i))->get_match_sig();
        if (sig.is_new ()) {
          if (!sig.parse (signature (*func))) {
            continue;
          }
          if (func->get_kind() == FT_VIRTUAL_MEMBER ||
              func->get_kind() == FT_PURE_VIRTUAL_MEMBER)
            sig.declare_virtual_function();
          else if (func->get_kind() == FT_STATIC_MEMBER ||
              func->get_kind() == FT_STATIC_NON_MEMBER) {
            sig.declare_static();
          }
        }
        if (_match_expr.matches(sig)) {
          if (_jpl) {
            _pre_evaluate = PRE_EVAL_OFF; // more than one match :-(
            break;
          }
          _jpl = *i;
        }
      }
    }
    else if( _match_expr.is_attribute() ) {
      _pre_evaluate = PRE_EVAL_DONE;
      _jpl = 0;

      typedef ACM_Container<ACM_Any, true> CList;
      const CList &clist = scope->get_children();
      for( CList::const_iterator i = clist.begin(); i != clist.end(); i++ ) {
        if( (*i)->type_val() != JPT_Variable )
          continue;
        ACM_Variable *var = (ACM_Variable*)(*i);

	MatchSignature &sig = var->get_match_sig();
        if( sig.is_new () ) {
          if( !sig.parse( signature( *var ) ) ) {
            continue;
	  }
	  if( var->get_kind() == VT_STATIC_MEMBER ||
	      var->get_kind() == VT_STATIC_NON_MEMBER ) {
	    sig.declare_static();
	  }
	}

	if( _match_expr.matches( sig ) ) {
          if( _jpl ) {
            _pre_evaluate = PRE_EVAL_OFF; // more than one match :-(
            break;
          }
          _jpl = *i;
        }
      }
    }
  }
  // no match possible at this point
  else {
    _pre_evaluate = PRE_EVAL_DONE;
    _jpl = 0;
  }
}


bool PCE_Match::evaluate (ACM_Any &jpl, PointCutContext &context,
                          Binding &binding, Condition &cond) {
  assert (!_match_expr.error ());

  // in case "::" we can immediately return 'true', because it matches the
  // global scope and all its potentially nested contents
  if (_str == "::")
    return true;

  if( &jpl == context.jpm().get_root() )
    return false; // root namespace is already handled in preeval completely

  bool result = false;
  if (jpl.type_val () == JPT_Type) {
    if (_match_expr.is_type ()) {
      ACM_Type &jpl_type = (ACM_Type&)jpl;
      MatchSignature &match_sig = jpl_type.get_match_sig();
      if (match_sig.is_new())
        match_sig.parse(format_type (jpl_type));
      result = _match_expr.matches (match_sig);
    }
  }
  else if ((jpl.type_val () & JPT_Name) != 0) {
    ACM_Name *jpl_name = (ACM_Name*)&jpl;
    while (true) {
      if(    (   jpl_name->type_val()  == JPT_Function                              && _match_expr.is_function()  )
          || (   jpl_name->type_val()  == JPT_Variable                              && _match_expr.is_attribute() )
          || ( ( jpl_name->type_val () & (JPT_Class | JPT_Aspect | JPT_Namespace) ) && _match_expr.is_type ()     )
        ) {
        MatchSignature &match_sig = jpl_name->get_match_sig();
        if (match_sig.is_new()) {
          if (!match_sig.parse(signature (*jpl_name))) {
            cout << "problematic signature: " << signature(*jpl_name) << endl;
            assert(false && "Parsing of signature failed. Maybe because of anonymous types? Signature was printed above.");
            return false;
          }
          if (jpl_name->type_val () == JPT_Function) {
            ACM_Function& jpl_func = static_cast<ACM_Function &>( *jpl_name );
            if( jpl_func.get_kind() == FT_VIRTUAL_MEMBER ||
                jpl_func.get_kind() == FT_PURE_VIRTUAL_MEMBER )
              match_sig.declare_virtual_function();
            else if( jpl_func.get_kind() == FT_STATIC_MEMBER ||
                     jpl_func.get_kind() == FT_STATIC_NON_MEMBER ) {
              match_sig.declare_static();
            }
          }
          else if( jpl_name->type_val () == JPT_Variable ) {
            ACM_Variable &jpl_var =  static_cast<ACM_Variable &>( *jpl_name );
            if( jpl_var.get_kind() == VT_STATIC_MEMBER ||
                jpl_var.get_kind() == VT_STATIC_NON_MEMBER ) {
              match_sig.declare_static();
            }
          }
        }
        result = _match_expr.matches (match_sig);

        // if we have a match, leave the loop
        if (result)
          break;
      }

      // otherwise check the parent scope of the current name join point,
      // but omit the parent classes of nested classes
      ACM_Name *parent = (ACM_Name*)jpl_name->get_parent();
      if (jpl_name->type_val () == JPT_Class || jpl_name->type_val () == JPT_Aspect) {
        while (parent && (parent->type_val () == JPT_Class || parent->type_val () == JPT_Aspect))
          parent = (ACM_Name*)parent->get_parent();
      }
      // root scope detected -> stop here
      if (!parent || parent == context.jpm().get_root())
        break;

      // continue by matching the parent scope
      jpl_name = parent;
    }
  }
  return result;
}

void PCE_Match::mangle_type_check (ostream &out) {
  const char *str = _str.c_str ();
  while (*str) {
    switch (*str) {
      case ':' : out << "___S___"; break;
      case '%' : out << "___A___"; break;
      case '(' : out << "___L___"; break;
      case ')' : out << "___R___"; break;
      case ',': out << "___C___"; break;
      default: out << *str;
    }
    str++;
  }
}

