#ifndef __Utils_h__
#define __Utils_h__

#include "Elements.h"
#include <string>
using std::string;
#include <list>
using std::list;
#include <set>
using std::set;

// return various string representations

string signature(const ACM_Name &name);
string signature(const ACM_Code &code);

string format_type(ACM_Type &type);
string format_type(ACM_Type &type, const string &name);

// functions that return the entity for certain join-points
bool has_entity( const ACM_Code &jpl );
ACM_Name *get_entity( const ACM_Code &jpl );

bool has_entity_func( const ACM_Code &jpl );
ACM_Function *get_entity_func( const ACM_Code &jpl );

bool has_entity_var( const ACM_Code &jpl );
ACM_Variable *get_entity_var( const ACM_Code &jpl );

ACM_Type *get_entity_type( const ACM_Code &jpl );

// functions that return the number of argument types
int get_arg_count (ACM_Function &func);
int get_arg_count (ACM_Call &code);
int get_arg_count (ACM_Code &code);

// functions that return the argument/result types of certain join-points

void add_arg_types (ACM_Function &func, list<ACM_Type*> &type_list);
void get_arg_types (ACM_Function &func, list<ACM_Type*> &type_list);

void add_arg_types (ACM_Call &jpl, list<ACM_Type*> &type_list);
void get_arg_types (ACM_Call &jpl, list<ACM_Type*> &type_list);

void add_arg_types (ACM_Code &jpl, list<ACM_Type*> &type_list);
void get_arg_types (ACM_Code &jpl, list<ACM_Type*> &type_list);

bool has_result_type (ACM_Code &jpl);
ACM_Type *get_result_type (ACM_Code &jpl);

// check whether two functions have the same name and argument types
bool have_same_name_and_args (ACM_Function &l, ACM_Function &r);

// check whether a join-point location is only a pseudo join-point location
bool is_pseudo(ACM_Any &jpl);

// check if the given joinpoint is an implicit one
bool is_implicit( ACM_Code &jpl );

// get the parent of the explit joinpoint in code
ACM_Name *get_explicit_parent( ACM_Code &jpl );

// get the lexical scope of a code join-point as needed by 'within'
ACM_Name *lexical_scope (ACM_Code &jpl);

// check whether a given program entity needs an object to be meaningful
bool needs_this( ACM_Name &entity );

// check whether a function needs an object to be invoked ("this pointer")
bool needs_this (ACM_Function &func);

// check whether a variable is part of an object, thus needs a this pointer for an access
bool needs_this( ACM_Variable &var );

// check whether a name joinpoint is defined in the project
bool in_project (ACM_Name &name);

// describe the proceed code requirements for code join-points
bool proceed_needs_args(ACM_Code &jpl);
bool proceed_needs_result (ACM_Code &jpl);
bool proceed_needs_that (ACM_Code &jpl);
bool proceed_needs_target (ACM_Code &jpl);

// manage child name map of ACM_Name nodes
void map_refresh(ACM_Name &name);
ACM_Name *map_lookup(ACM_Name &parent, const string &key);
void map_insert(ACM_Name &parent, ACM_Name &child, const string &key);

// context information of a join-point
string filename (ACM_Any &jpl);
int line (ACM_Any &jpl);

// get the slice that is associated with on introduction
ACM_ClassSlice *get_slice (ACM_Introduction &intro);

// collect the set of abstract member functions and pointcuts of a class or aspect
void get_abstract_members (ACM_Class &cls, set<string> &abstract_members);

// check whether a class or aspect is abstract
bool is_abstract (ACM_Class &cls);

// calculate number of nested advice levels below the given code plan
int depth (ACM_CodePlan &plan);

// This function takes an AspectC++-Model-Function as ACM_Function and
// returns whether this function is an operator.
bool is_operator(ACM_Function*);

#endif // __Utils_h__
