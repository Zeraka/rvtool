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

#include "Utils.h"

// return various string representations

string signature(const ACM_Name &name_param) {
  ACM_Name &name = (ACM_Name&)name_param;
  ACM_Name *parent = (ACM_Name*)name.get_parent();
  if (!parent)
    return "::";
  string result = signature (*parent);
  if (result == "::")
    result = "";
  else {
    result += "::";
    // ignore the anonymous namespace as parent
    if (result[0] == '%')
      result = "";
  }
  result += name.get_name();

  // the result so far seems to be a sane default so no explicit error handling neccesary, if below checks fall through
  if( name.type_val() == JPT_Attribute ) {
    result = "[[" + result + "]]";

    return result;
  }

  if( name.type_val() == JPT_Function ) {
    ACM_Function &func = (ACM_Function&)name;
    list<ACM_Type*> types;
    get_arg_types (func, types);
    result += "(";
    for (list<ACM_Type*>::iterator i = types.begin(); i != types.end(); ++i) {
      if (i != types.begin())
        result +=",";
      result += format_type(**i);
    }
    if (func.get_variadic_args())
      result += ",...";
    result += ")";
    if (func.get_cv_qualifiers() & CVQ_CONST)
      result += " const";
    if (func.get_cv_qualifiers() & CVQ_VOLATILE)
      result += " volatile";
    if (func.has_result_type()) {
      ACM_Type &result_type = *func.get_result_type();
      if (func.get_name().find("operator ") != 0)
        result = format_type (result_type, result);
      else {
        string result_str = format_type(result_type);
        if (func.get_name().find(string("operator ") + result_str) != 0)
          result = format_type (result_type, result);
      }
    }
  }
  else if( name.type_val() == JPT_Variable ) {
    ACM_Variable &var = (ACM_Variable &)name;

    ACM_Type &type = *( var.get_type() );
    result = format_type( type, result );
  }
  return result;
}

string signature( const ACM_Code &code ) {
  // signature of a code joinpoint is defined as signature of the matching entity
  if( has_entity( code ) )
    return signature( *get_entity( code ) );
  else
    return format_type( *get_entity_type( code ), "<unknown>" );
}

string format_type(ACM_Type &type) {
  string sig = type.get_signature();
  string::size_type pos = sig.find(" ?)");
  if (pos != string::npos) {
    sig.erase(pos, 2);
  }
  else {
    pos = sig.find("?");
    if (pos != string::npos) {
      sig.erase(pos, 1);
    }
  }
  return sig;
}

string format_type(ACM_Type &type, const string &name) {
  string sig = type.get_signature();
  string::size_type pos = sig.find("?");
  if (pos != string::npos) {
    sig.replace(pos, 1, name);
  }
  else {
    char last = sig[sig.length () -1];
    if (last != '*' && last != '&')
      sig += " ";
    sig += name;
  }
  return sig; 
}

// functions that return the entity for certain join-points
bool has_entity( const ACM_Code &jpl ) {
  return ( has_entity_func( jpl ) || has_entity_var( jpl ) );
}

ACM_Name *get_entity( const ACM_Code &jpl ) {
  if( has_entity_func( jpl ) ) // Code joinpoints that have a function as entity
    return get_entity_func( jpl );
  else if ( has_entity_var( jpl ) ) // Code joinpoints that have a variable as entity
    return get_entity_var( jpl );
  else { // illegal or new JPType -> make noise
    assert( false && "illegal and thus unhandled JPType" );
    return 0; // we need to return something if compiled without asserts
  }
}

bool has_entity_func( const ACM_Code &jpl ) {
  // Code joinpoints that have a function as entity:
  // JPT_Call
  // JPT_Builtin
  // JPT_Execution
  // JPT_Construction
  // JPT_Destruction
  return ( jpl.type_val() & ( JPT_Call | JPT_Builtin | JPT_Execution | JPT_Construction | JPT_Destruction ) );
}

ACM_Function *get_entity_func( const ACM_Code &jpl ) {
  switch( jpl.type_val() ) {
  case JPT_Call:
    return const_cast<ACM_Call &>( static_cast<const ACM_Call &>( jpl ) ).get_target(); // for some reason the getter is not defined const, but should be
  case JPT_Builtin:
    return const_cast<ACM_Builtin &>( static_cast<const ACM_Builtin &>( jpl ) ).get_target();
  case JPT_Execution: // inner joinpoints have there entity as parent
  case JPT_Construction:
  case JPT_Destruction:
    return static_cast<ACM_Function *>( jpl.get_parent() );
  default: // illegal or new JPType -> make noise
    assert( false && "illegal and thus unhandled JPType" );
    return 0; // we need to return something if compiled without asserts
  }
}

bool has_entity_var( const ACM_Code &jpl ) {
  // Code joinpoints that have a variable as entity:
  // JPT_Get
  // JPT_Set
  // JPT_Ref
  return ( jpl.type_val() & ( JPT_Get | JPT_Set | JPT_Ref ) );
};

ACM_Variable *get_entity_var( const ACM_Code &jpl ) {
  switch( jpl.type_val() ) {
  case JPT_Get:
    return const_cast<ACM_Get &>( static_cast<const ACM_Get &>( jpl ) ).get_variable(); // for some reason the getter is not defined const, but should be
  case JPT_Set:
    return const_cast<ACM_Set &>( static_cast<const ACM_Set &>( jpl ) ).get_variable(); // for some reason the getter is not defined const, but should be
  case JPT_Ref:
    return const_cast<ACM_Ref &>( static_cast<const ACM_Ref &>( jpl ) ).get_variable(); // for some reason the getter is not defined const, but should be
  default: // illegal or new JPType -> make noise
    assert( false && "illegal and thus unhandled JPType" );
    return 0; // we need to return something if compiled without asserts
  }
}

ACM_Type *get_entity_type( const ACM_Code &jpl ) {
  switch( jpl.type_val() ) {
  case JPT_Get:
    // FIXME: if the get join point accesses an array, the entity type must be the element type (and not the array type)
    return const_cast<ACM_Get &>( static_cast<const ACM_Get &>( jpl ) ).get_variable()->get_type(); // for some reason the getter is not defined const, but should be
  case JPT_Set:
    // FIXME: if the set join point accesses an array, the entity type must be the element type (and not the array type)
    return const_cast<ACM_Set &>( static_cast<const ACM_Set &>( jpl ) ).get_variable()->get_type(); // for some reason the getter is not defined const, but should be
  case JPT_Ref:
    return const_cast<ACM_Ref &>( static_cast<const ACM_Ref &>( jpl ) ).get_variable()->get_type(); // for some reason the getter is not defined const, but should be
  case JPT_GetRef:
    return const_cast<ACM_GetRef &>( static_cast<const ACM_GetRef &>( jpl ) ).get_type();
  case JPT_SetRef:
    return const_cast<ACM_SetRef &>( static_cast<const ACM_SetRef &>( jpl ) ).get_type();
  case JPT_CallRef:
    return const_cast<ACM_CallRef &>( static_cast<const ACM_CallRef &>( jpl ) ).get_type();
  default: // illegal or new JPType -> make noise
    assert( false && "illegal and thus unhandled JPType" );
    return 0; // we need to return something if compiled without asserts
  }
}

// functions that return the number of argument types

int get_arg_count (ACM_Function &func) {
  return func.get_arg_types().size();
}

int get_arg_count (ACM_Call &call) {
  ACM_Function *func = call.get_target();
  if (call.has_default_args())
    return get_arg_count (*func) - call.get_default_args();
  else
    return get_arg_count (*func) + call.get_variadic_arg_types().size();
}

int get_arg_count (ACM_Code &code) {
  switch( code.type_val() ) {
  // first cases which have an associated function
  case JPT_Call: // handle seperately as adjustments are needed
    return get_arg_count( static_cast<ACM_Call &>( code ) );
  case JPT_Builtin:
  case JPT_Execution:
  case JPT_Construction:
  case JPT_Destruction:
    return get_arg_count( *get_entity_func( code ) );
  // cases that behave like a call but are none
  // currently these values need to be hard coded
  // JPT_Get( T x ) -> T get()
  case JPT_Get:
  case JPT_GetRef:
    return 0;
  // JPT_Set( T x ) -> void set( T val )
  case JPT_Set:
  case JPT_SetRef:
    return 1;
  // JPT_Ref( T x ) -> T(&|*) ref()
  case JPT_Ref:
    return 0;
  case JPT_CallRef:
    return 0; // FIXME: not implemented, yet. Needs handling similar to JPT_Call.
  default: // illegal or new JPType -> make noise
    assert( false && "illegal and thus unhandled JPType" );
    return 0; // we need to return something if compiled without asserts
  }
}

// functions that return the argument/result types of certain join-points

void add_arg_types (ACM_Function &func, list<ACM_Type*> &type_list) {
  typedef ACM_Container<ACM_Type, true> Container;
  Container &types = func.get_arg_types();
  for (Container::iterator i = types.begin (); i != types.end(); ++i)
    type_list.push_back (*i);
}

void get_arg_types (ACM_Function &func, list<ACM_Type*> &type_list) {
  type_list.clear();
  add_arg_types (func, type_list);
}


void add_arg_types (ACM_Call &jpl, list<ACM_Type*> &type_list) {
  typedef ACM_Container<ACM_Type, true> Container;

  int args = get_arg_count (jpl);
  Container &types = jpl.get_target()->get_arg_types();
  int arg = 0;
  for (Container::iterator i = types.begin (); i != types.end() && arg < args;
       ++i, ++arg)
    type_list.push_back (*i);
  if (arg < args) {
    Container &var_types = jpl.get_variadic_arg_types();
    for (Container::iterator i = var_types.begin (); i != var_types.end(); ++i)
      type_list.push_back (*i);
  }
}

void get_arg_types (ACM_Call &jpl, list<ACM_Type*> &type_list) {
  type_list.clear();
  add_arg_types (jpl, type_list);
}


void add_arg_types( ACM_Code &jpl, list<ACM_Type *> &type_list ) {
  switch( jpl.type_val() ) {
  // first cases which have an associated function
  case JPT_Call: // handle seperately as adjustments are needed
    add_arg_types( static_cast<ACM_Call &>( jpl ), type_list );
    return;
  case JPT_Builtin:
  case JPT_Execution:
  case JPT_Construction:
  case JPT_Destruction:
    add_arg_types( *get_entity_func( jpl ), type_list );
    return;
  // cases that behave like a call but are none
  // currently these values need to be hard coded
  // JPT_Get( T x ) -> T get()
  case JPT_Get:
  case JPT_GetRef:
    return; // no args
  // JPT_Set( T x ) -> void set( T val )
  case JPT_Set:
  case JPT_SetRef:
    type_list.push_back( get_entity_type( jpl ) ); // only arg has type of variable
    return;
  // JPT_Ref( T x ) -> T(&|*) ref()
  case JPT_Ref:
    return; // no args
  default: // illegal or new JPType -> make noise
    assert( false && "illegal and thus unhandled JPType" );
  }
}

void get_arg_types (ACM_Code &jpl, list<ACM_Type*> &type_list) {
  type_list.clear();
  add_arg_types (jpl, type_list);
}

bool has_result_type( ACM_Code &jpl ) {
  switch( jpl.type_val() ) {
  // first cases which have an associated function
  case JPT_Call:
  case JPT_Builtin:
  case JPT_Execution:
  case JPT_Construction:
  case JPT_Destruction:
    return get_entity_func( jpl )->has_result_type();
  // cases which operate on data
  // currently these values need to be hard coded
  // JPT_Get( T x ) -> T get()
  case JPT_Get:
  case JPT_GetRef:
    return true;
  // JPT_Set( T x ) -> void set( T val )
  case JPT_Set:
  case JPT_SetRef:
    return false;
  // JPT_Ref( T x ) -> T(&|*) ref()
  case JPT_Ref:
    return true;
  default: // illegal or new JPType -> make noise
    assert( false && "illegal and thus unhandled JPType" );
    return false; // something needs to be returned for builds without asserts
  }
}

ACM_Type *get_result_type( ACM_Code &jpl ) {
  switch( jpl.type_val() ) {
  // first cases which have an associated function
  case JPT_Call:
  case JPT_Builtin:
  case JPT_Execution:
  case JPT_Construction:
  case JPT_Destruction:
    return get_entity_func( jpl )->get_result_type();
  // cases which operate on data
  // currently these values need to be hard coded
  // JPT_Get( T x ) -> T get()
  case JPT_Get:
  case JPT_GetRef:
    return get_entity_type( jpl );
  // JPT_Set( T x ) -> void set( T val )
  case JPT_Set:
  case JPT_SetRef:
    return 0;
  // JPT_Ref( T x ) -> T(&|*) ref()
  case JPT_Ref:
    return const_cast<ACM_Ref &>( static_cast<const ACM_Ref &>( jpl ) ).get_type(); // for some reason the getter is not defined const, but should be
  default: // illegal or new JPType -> make noise
    assert( false && "illegal and thus unhandled JPType" );
    return 0; // something needs to be returned for builds without asserts
  }
}

// check whether two functions have the same name and argument types
bool have_same_name_and_args (ACM_Function &l, ACM_Function &r) {
  if (get_arg_count (l) != get_arg_count (r) ||
      l.get_variadic_args() != r.get_variadic_args() || 
      l.get_name() != r.get_name())
    return false;
  typedef ACM_Container<ACM_Type, true> Container;
  Container &ltypes = l.get_arg_types();
  Container &rtypes = r.get_arg_types();
  for (Container::const_iterator liter = ltypes.begin(), riter = rtypes.begin();
       liter != ltypes.end(); ++liter, ++riter)
    if ((*liter)->get_signature() != (*riter)->get_signature())
      return false;
  return true;
}

// check whether a join-point location is only a pseudo join-point location
bool is_pseudo(ACM_Any &jpl) {
  if (jpl.type_val() == JPT_Call && !jpl.get_parent ())
    return true;
  return false;
}

// check if the given joinpoint is an implicit one
bool is_implicit( ACM_Code &jpl ) {
  ACM_Node *parent = static_cast<ACM_Node *>( jpl.get_parent() );
  return parent && ( parent->type_val() & JPT_Code );
}

// get the parent of the explit joinpoint in code
ACM_Name *get_explicit_parent( ACM_Code &jpl ) {
  ACM_Code *code = &jpl;
  while( code && is_implicit( *code ) )
    code = static_cast<ACM_Code *>( code->get_parent() );

  assert( ! code || ( static_cast<ACM_Node *>( code->get_parent() )->type_val() & JPT_Name ) );
  return code ? static_cast<ACM_Name *>( code->get_parent() ) : 0;
}

// get the lexical scope of a code join-point as needed by 'within'
ACM_Name *lexical_scope (ACM_Code &jpl) {
  ACM_Name *parent = get_explicit_parent( jpl );
  assert( parent );
  if (jpl.type_val() & JPT_Access) {
    if (parent->type_val() == JPT_Variable)
      parent = (ACM_Name*)parent->get_parent();
  }
  else
    parent = (ACM_Name*)parent->get_parent();
  return parent;
}

// check whether a given program entity needs an object to be meaningful
bool needs_this( ACM_Name &entity ) {
  if( entity.type_val() == JPT_Function )
    return needs_this( static_cast<ACM_Function &>( entity ) );
  else if( entity.type_val() == JPT_Variable )
    return needs_this( static_cast<ACM_Variable &>( entity ) );
  else {
    assert( false && "Unknown and/or not supported entity type !" );
    return false; // something needs to be returned for builds without asserts
  }
}

// check whether a function needs an object to be invoked ("this pointer")
bool needs_this (ACM_Function &func) {
  switch (func.get_kind()) {
  case FT_NON_MEMBER:
  case FT_STATIC_NON_MEMBER:
  case FT_STATIC_MEMBER:
    return false;
  case FT_CONSTRUCTOR:
  case FT_DESTRUCTOR:
  case FT_VIRTUAL_DESTRUCTOR:
  case FT_PURE_VIRTUAL_DESTRUCTOR:
    return true;
  case FT_MEMBER:
  case FT_VIRTUAL_MEMBER:
  case FT_PURE_VIRTUAL_MEMBER:
    {
    if( ! is_operator( &func ) )
      return true;

    string name = func.get_name().substr(9);
    return name != "new" && name != "new[]" && name != "delete" && name != "delete[]";
    }
  default: // illegal or new FType -> make noise
    assert( false && "illegal and thus unhandled FType" );
    return false; // something needs to be returned for builds without asserts
  }
}

// check whether a variable is part of an object, thus needs a this pointer for an access
bool needs_this( ACM_Variable &var ) {
  switch( var.get_kind() ) {
  case VT_NON_MEMBER:
  case VT_STATIC_NON_MEMBER:
  case VT_STATIC_MEMBER:
    return false;
  case VT_MEMBER:
    return true;
  default: // illegal or new VType -> make noise
    assert( false && "illegal and thus unhandled VType" );
    return false; // something needs to be returned for builds without asserts
  }
}

// check whether a name joinpoint is defined in the project
bool in_project (ACM_Name &name) {
  return name.get_tunits().empty();
}

// describe the proceed code requirements for code join-points
bool proceed_needs_args(ACM_Code &jpl) {
  return get_arg_count (jpl) > 0;
}

bool proceed_needs_result (ACM_Code &jpl) {
  return has_result_type (jpl);
}

bool proceed_needs_that (ACM_Code &jpl) {
  if( jpl.type_val() & JPT_Access ) // outer joinoints ...
    return false; // ... never need it
  else // inner joinpoints ...
    return needs_this( *get_entity( jpl ) ); // determine by entity
}

bool proceed_needs_target (ACM_Code &jpl) {
  if( jpl.type_val() & JPT_Access ) // outer joinoints ...
    return needs_this( *get_entity( jpl ) ); // determine by entity
  else // inner joinpoints ...
    return false; // ... never need it
}

// manage child name map of ACM_Name nodes

static string map_key (ACM_Name &name) {
  string result = name.get_name();
  if (name.type_val() & JPT_Function) {
    list<ACM_Type*> types;
    get_arg_types ((ACM_Function &)name, types);
    result += "(";
    for (list<ACM_Type*>::iterator i = types.begin(); i != types.end(); ++i) {
      if (i != types.begin())
        result +=", ";
      result += (*i)->get_signature();
    }
    result += ")";
  }
  return result;
}

void map_refresh(ACM_Name &name) {
  typedef ACM_Container<ACM_Any, true> Container;
  NameMap &map = name.get_name_map();
  Container &children = name.get_children();
  map.clear();
  for (Container::iterator i = children.begin(); i != children.end(); ++i) {
    if ((*i)->type_val() & JPT_Name) {
      ACM_Name *child = (ACM_Name*)*i;
      map.insert(NameMapPair(map_key(*child), child));
    }
  }
}

ACM_Name *map_lookup(ACM_Name &parent, const string &key) {
  NameMap &map = parent.get_name_map();
  if (map.size() == 0 && parent.get_children().size() > 0)
    map_refresh(parent);
  NameMap::iterator i = map.find(key);
  if (i != map.end())
    return i->second;
  return 0;
}

void map_insert(ACM_Name &parent, ACM_Name &child, const string &key) {
  NameMap &map = parent.get_name_map();
  map.insert(NameMapPair(key, &child));
}

// context information of a join-point

string filename (ACM_Any &jpl) {
  typedef ACM_Container<ACM_Source, true> Container;
  Container &source = (jpl.type_val() &
      (JPT_Construction|JPT_Destruction|JPT_Execution)) ?
          ((ACM_Code*)jpl.get_parent())->get_source() : jpl.get_source();
  if (source.size() == 0)
    return "<unknown>";

  Container::iterator last;
  for (Container::iterator i = source.begin (); i != source.end (); ++i) {
    last = i;
    if ((*i)->get_kind () == SLK_DEF)
      break;
  }
  return (*last)->get_file()->get_filename();
}

int line (ACM_Any &jpl) {
  typedef ACM_Container<ACM_Source, true> Container;
  Container &source = (jpl.type_val() &
      (JPT_Construction|JPT_Destruction|JPT_Execution)) ?
          ((ACM_Code*)jpl.get_parent())->get_source() : jpl.get_source();
  if (source.size() == 0)
    return -1;

  Container::iterator last;
  for (Container::iterator i = source.begin (); i != source.end (); ++i) {
    last = i;
    if ((*i)->get_kind () == SLK_DEF)
      break;
  }
  return (*last)->get_line();
}

// get the slice that is associated with on introduction
ACM_ClassSlice *get_slice (ACM_Introduction &intro) {
  ACM_ClassSlice *cs = 0;
  if (intro.has_named_slice ())
    cs = intro.get_named_slice ();
  else if (intro.has_anon_slice ())
    cs = intro.get_anon_slice ();
  return cs;
}

// collect the set of abstract member functions and pointcuts of a class or aspect
void get_abstract_members (ACM_Class &cls, set<string> &abstract_members) {
  // iterate over all base classes and get the inherited abstract members
  typedef ACM_Container<ACM_Class, false> Container;
  Container &bases = cls.get_bases();
  for (Container::iterator i = bases.begin (); i != bases.end (); ++i) {
    set<string> base_members; // key strings
    get_abstract_members (**i, base_members);
    // now check each abstract base class member in the current class
    for (set<string>::const_iterator ki = base_members.begin ();
        ki != base_members.end (); ++ki) {
      ACM_Any *elem = map_lookup(cls, *ki);
      bool redefined = false;
      if ((*ki)[0] == '~')
        redefined = true; // pure virtual destructor are defined implicitly
      else if (elem) {
        // check the found member
        if (elem->type_val() == JPT_Pointcut &&
            ((ACM_Pointcut*)elem)->get_kind() != PT_PURE_VIRTUAL)
          redefined = true;
        else if (elem->type_val() == JPT_Function &&
             ((ACM_Function*)elem)->get_kind() == FT_VIRTUAL_MEMBER)
          redefined = true;
      }
      // if it is not redefined here as non-abstract, keep it
      if (!redefined)
        abstract_members.insert (*ki);
    }
  }

  // non check all member functions and pointcuts of this class
  NameMap &name_map = cls.get_name_map();
  for (NameMap::const_iterator i = name_map.begin (); i != name_map.end (); ++i) {
    string key = i->first;
    ACM_Any *elem = i->second;
    if ((elem->type_val() == JPT_Pointcut &&
        ((ACM_Pointcut*)elem)->get_kind() == PT_PURE_VIRTUAL) ||
        (elem->type_val() == JPT_Function &&
        (((ACM_Function*)elem)->get_kind() == FT_PURE_VIRTUAL_MEMBER ||
        ((ACM_Function*)elem)->get_kind() == FT_PURE_VIRTUAL_DESTRUCTOR)))
      abstract_members.insert (key);
  }

//  cout << "result for " << cls.get_name() << endl;
//  for (set<string>::const_iterator ki = abstract_members.begin ();
//      ki != abstract_members.end (); ++ki) {
//    cout << "  " << *ki << endl;
//  }
}

// check whether a class or aspect is abstract
bool is_abstract (ACM_Class &cls) {
  set<string> abstract_members;
  get_abstract_members (cls, abstract_members);
  return abstract_members.size () != 0;
}

// calculate number of nested advice levels below the given code plan
int depth (ACM_CodePlan &plan) {
  return plan.has_next_level() ? depth (*plan.get_next_level()) + 1 : 0;
}

// This function takes an AspectC++-Model-Function as ACM_Function and
// returns whether this function is an operator.
bool is_operator(ACM_Function* acm_function) {
  return acm_function->get_name().substr(0,9) == "operator ";
}
