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

#ifndef __PointCutExpr_h__
#define __PointCutExpr_h__

#ifdef FRONTEND_PUMA
namespace Puma {
  class CTypeInfo;
}
#else // FRONTEND_CLANG
#include "clang/AST/Type.h"
#endif

#include "Condition.h"
#include "PointCut.h"
#include "ACErrorStream.h"

#include "ACModel/MatchExpr.h"
#include "ACModel/Elements.h"
#include "ACModel/Utils.h"

#include <list>
#include <string>
#include "Puma/Array.h" // TODO: replace by STL container

class Binding;
class PointCutContext;

// Pointcut expression types are evaluated in a context-independant manner.
// For every pointcut expression a type can always be determined.
enum PCE_Type { 
  PCE_NAME, // NAME represents a name pointcut
  PCE_CODE // CODE represents a code pointcut
};

class PointCutExpr : public PointcutExpressionNode {

protected:

  enum { PRE_EVAL_OFF, PRE_EVAL_ON, PRE_EVAL_DONE } _pre_evaluate;
  ACM_Any *_jpl;
  JoinPointType _possible_types;

  // do a semantic analysis of the child nodes
  void sem_args (ACErrorStream &err, PointCutContext &context, int warn_compat);

  // check if the argument pointcut types are as expected
  void check_arg_types (ACErrorStream &err, const char *func, PCE_Type expected,
      PointCutContext &context);

  // check if all argument types are equal
  void check_arg_types_equal (ACErrorStream &err, const char *func,
      PointCutContext &context);

  // get the real argument binding of a context variable
  const ACM_Arg *get_binding (const char *name, ACErrorStream &err,
      PointCutContext &context);
public:

  PointCutExpr () : _pre_evaluate (PRE_EVAL_OFF), _jpl (0),
    _possible_types((JoinPointType)0) {}
  virtual ~PointCutExpr () {}

  // Return the type of pointcut described by this expression. This function
  // does not check any argument types etc. -> semantics!
  virtual PCE_Type type () const = 0;

  // return a bit mask of join point types that could match; others can't!
  int possible_types () const { return (int)_possible_types; }

  // run a semantic analysis (e.g. check types)
  virtual void semantics (ACErrorStream &err, PointCutContext &context,
      int warn_compat) = 0;

  // evaluate an expression, i.e. check if the expression evaluates as 'true'
  // for a particular join point
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond) = 0;

  // if the expression supports pre-evaluation, do it in this function
  virtual void pre_evaluate (PointCutContext &context) {}

  bool match (ACM_Any &jpl, PointCutContext &context,
              Binding &binding, Condition &cond) {
    if (_pre_evaluate == PRE_EVAL_ON)
      pre_evaluate(context);
    if (_pre_evaluate == PRE_EVAL_OFF)
      return evaluate (jpl, context, binding, cond);
    return (&jpl == _jpl);
  }
                         
  // mangle a string that represents a type check (for that/target arguments)                         
  virtual void mangle_type_check (ostream &) = 0;

  // helper functions to get the right result type
  virtual PointCutExpr* duplicate () = 0;
  virtual PointCutExpr *arg (int i) const = 0;
};

class PCE_SingleArg : public PointCutExpr {
  PointCutExpr *_arg;
public:
  PCE_SingleArg (PointCutExpr *arg) : _arg (arg) {}

  virtual int args () const { return 1; }
  virtual PointCutExpr *arg (int i) const { return (i == 0) ? _arg : 0; }
  void replace_arg (PointCutExpr *expr) { if (_arg) _arg->ref(-1); _arg = expr; }

  bool check_derived_class (ACM_Class &cls,
    PointCutContext &context, Binding &binding, Condition &cond);
  bool check_derived_func (ACM_Class &cls, ACM_Function &func, 
    PointCutContext &context, Binding &binding, Condition &cond);
  bool check_derived_var (ACM_Class &cls, ACM_Variable &var,
    PointCutContext &context, Binding &binding, Condition &cond);
  bool check_base_class (ACM_Class &cls, PointCutContext &context,
    Binding &binding, Condition &cond);
  bool check_base_func (ACM_Class &cls, ACM_Function &func, 
    PointCutContext &context, Binding &binding, Condition &cond);
  bool check_base_var (ACM_Class &cls, ACM_Variable &var,
    PointCutContext &context, Binding &binding, Condition &cond);
  bool check_scopes (ACM_Name *scope,
    PointCutContext &context, Binding &binding, Condition &cond);
  
  virtual void mangle_type_check (ostream &out) {
    out << 1;
    _arg->mangle_type_check (out);
  }
};

class PCE_DoubleArg : public PointCutExpr {
  PointCutExpr *_args[2];
public:
  PCE_DoubleArg (PointCutExpr *arg0, PointCutExpr *arg1) {
    _args[0] = arg0; _args[1] = arg1; 
  }
  virtual int args () const { return 2; }
  virtual PointCutExpr *arg (int i) const { 
    return (i >= 0 && i < 2) ? _args[i] : 0; 
  }
  virtual void mangle_type_check (ostream &out) {
    out << 2;
    _args[0]->mangle_type_check (out);
    _args[1]->mangle_type_check (out);
  }
};

class PCE_VarArgs : public PointCutExpr {
  Puma::Array<PointCutExpr*> _args; // TODO: replace by STL container
public:
  virtual ~PCE_VarArgs () {}
  void add_arg (PointCutExpr *arg) { _args.append (arg); }
  virtual int args () const { return _args.length (); }
  virtual PointCutExpr *arg (int i) const { return _args.lookup (i); }
  virtual void mangle_type_check (ostream &out) {
    out << args ();
    for (int i = 0; i < args (); i++)
      arg (i)->mangle_type_check (out);
  }
};

class PCE_Classes : public PCE_SingleArg {
public:
  PCE_Classes (PointCutExpr *arg) : PCE_SingleArg (arg) {}
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "classes_";
    PCE_SingleArg::mangle_type_check (out);
  }
  virtual PCE_Classes* duplicate () { return new PCE_Classes(arg(0)->duplicate ()); }
};

class PCE_Base : public PCE_SingleArg {
public:
  PCE_Base (PointCutExpr *arg) : PCE_SingleArg (arg) {}
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "base_";
    PCE_SingleArg::mangle_type_check (out);
  }
  virtual PCE_Base* duplicate () { return new PCE_Base(arg(0)->duplicate ()); }
};

class PCE_Derived : public PCE_SingleArg {
public:
  PCE_Derived (PointCutExpr *arg) : PCE_SingleArg (arg) {}
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "derived_";
    PCE_SingleArg::mangle_type_check (out);
  }
  virtual PCE_Derived* duplicate () { return new PCE_Derived(arg(0)->duplicate ()); }
};

class PCE_Within : public PCE_SingleArg {
public:
  PCE_Within (PointCutExpr *arg) : PCE_SingleArg (arg) {}
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "within_";
    PCE_SingleArg::mangle_type_check (out);
  }
  virtual PCE_Within* duplicate () { return new PCE_Within(arg(0)->duplicate ()); }
};

class PCE_Member : public PCE_SingleArg {
public:
  PCE_Member( PointCutExpr *arg ) : PCE_SingleArg( arg ) {}
  virtual PCE_Type type() const;
  virtual void semantics( ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate( ACM_Any &jpl, PointCutContext &context, Binding &binding, Condition &cond );
  virtual void mangle_type_check( ostream &out ) {
    out << "member_";
    PCE_SingleArg::mangle_type_check( out );
  }
  virtual PCE_Member* duplicate () { return new PCE_Member( arg(0)->duplicate () ); }
};

class PCE_Execution : public PCE_SingleArg {
public:
  PCE_Execution (PointCutExpr *arg) : PCE_SingleArg (arg) {}
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "execution_";
    PCE_SingleArg::mangle_type_check (out);
  }
  virtual PCE_Execution* duplicate () { return new PCE_Execution(arg(0)->duplicate ()); }
};

class PCE_Call : public PCE_SingleArg {
public:
  PCE_Call (PointCutExpr *arg) : PCE_SingleArg (arg) {}
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "call_";
    PCE_SingleArg::mangle_type_check (out);
  }
  virtual PCE_Call* duplicate () { return new PCE_Call(arg(0)->duplicate ()); }
};

class PCE_Builtin : public PCE_SingleArg {
public:
  PCE_Builtin (PointCutExpr *arg) : PCE_SingleArg (arg) {}
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "builtin";
    PCE_SingleArg::mangle_type_check (out);
  }
  virtual PCE_Builtin* duplicate () { return new PCE_Builtin(arg(0)->duplicate ()); }
};

class PCE_Get : public PCE_SingleArg {
public:
  PCE_Get( PointCutExpr *arg ) : PCE_SingleArg( arg ) {}
  virtual PCE_Type type () const;
  virtual void semantics( ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate( ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond );
  virtual void mangle_type_check( ostream &out ) {
    out << "get_";
    PCE_SingleArg::mangle_type_check( out );
  }
  virtual PCE_Get* duplicate() { return new PCE_Get( arg(0)->duplicate() ); }
};

class PCE_Set : public PCE_SingleArg {
public:
  PCE_Set( PointCutExpr *arg ) : PCE_SingleArg( arg ) {}
  virtual PCE_Type type () const;
  virtual void semantics( ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate( ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond );
  virtual void mangle_type_check( ostream &out ) {
    out << "set_";
    PCE_SingleArg::mangle_type_check( out );
  }
  virtual PCE_Set* duplicate() { return new PCE_Set( arg(0)->duplicate() ); }
};

class PCE_Ref : public PCE_SingleArg {
public:
  PCE_Ref( PointCutExpr *arg ) : PCE_SingleArg( arg ) {}
  virtual PCE_Type type () const;
  virtual void semantics( ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate( ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond );
  virtual void mangle_type_check( ostream &out ) {
    out << "ref_";
    PCE_SingleArg::mangle_type_check( out );
  }
  virtual PCE_Ref* duplicate() { return new PCE_Ref( arg(0)->duplicate() ); }
};

class PCE_Alias : public PCE_SingleArg {
public:
  PCE_Alias( PointCutExpr *arg ) : PCE_SingleArg( arg ) {}
  virtual PCE_Type type () const;
  virtual void semantics( ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate( ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond );
  virtual void mangle_type_check( ostream &out ) {
    out << "alias_";
    PCE_SingleArg::mangle_type_check( out );
  }
  virtual PCE_Alias* duplicate() { return new PCE_Alias( arg(0)->duplicate() ); }
};

class PCE_Construction : public PCE_SingleArg {
public:
  PCE_Construction (PointCutExpr *arg) : PCE_SingleArg (arg) {}
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "construction_";
    PCE_SingleArg::mangle_type_check (out);
  }
  virtual PCE_Construction* duplicate () { return new PCE_Construction(arg(0)->duplicate ()); }
};

class PCE_Destruction : public PCE_SingleArg {
public:
  PCE_Destruction (PointCutExpr *arg) : PCE_SingleArg (arg) {}
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "destruction_";
    PCE_SingleArg::mangle_type_check (out);
  }
  virtual PCE_Destruction* duplicate () { return new PCE_Destruction(arg(0)->duplicate ()); }
};

class PCE_That : public PCE_SingleArg {
public:
  PCE_That (PointCutExpr *arg) : PCE_SingleArg (arg) {}
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "that_";
    PCE_SingleArg::mangle_type_check (out);
  }
  virtual PCE_That* duplicate () { return new PCE_That(arg(0)->duplicate ()); }
};

class PCE_Target : public PCE_SingleArg {
public:
  PCE_Target (PointCutExpr *arg) : PCE_SingleArg (arg) {}
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "target_";
    PCE_SingleArg::mangle_type_check (out);
  }
  virtual PCE_Target* duplicate () { return new PCE_Target(arg(0)->duplicate ()); }
};

class PCE_CFlow : public PCE_SingleArg {
  int _index;
  PointCut _arg_pointcut;
public:
  PCE_CFlow (PointCutExpr *arg) : PCE_SingleArg (arg) {}
  virtual ~PCE_CFlow () {}
  virtual PCE_Type type () const;
  PointCut &arg_pointcut () { return _arg_pointcut; }
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "cflow_";
    PCE_SingleArg::mangle_type_check (out);
  }
  virtual PCE_CFlow* duplicate () { return new PCE_CFlow(arg(0)->duplicate ()); }
};

class PCE_Args : public PCE_VarArgs {
  bool _both_sc_args_bound_warning_created;
  bool _sc_arg_bound_warning_created;
public:
  PCE_Args () : _both_sc_args_bound_warning_created(false),
                _sc_arg_bound_warning_created(false) {}
  PCE_Args (bool both_sc_args_bound_warning_created,
            bool sc_arg_bound_warning_created) :
        _both_sc_args_bound_warning_created(both_sc_args_bound_warning_created),
        _sc_arg_bound_warning_created(sc_arg_bound_warning_created){}
  virtual ~PCE_Args () {}
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "args_";
    PCE_VarArgs::mangle_type_check (out);
  }
  virtual PCE_Args* duplicate () {
    PCE_Args *result = new PCE_Args(_both_sc_args_bound_warning_created,
        _sc_arg_bound_warning_created);
    for (int i = 0; i < args(); i++)
      result->add_arg(arg(i)->duplicate ());
    return result;
  }
};

class PCE_Result : public PCE_SingleArg {
public:
  PCE_Result (PointCutExpr *arg) : PCE_SingleArg (arg) {}
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "result_";
    PCE_SingleArg::mangle_type_check (out);
  }
  virtual PCE_Result* duplicate () { return new PCE_Result(arg(0)->duplicate ()); }
};

class PCE_Or : public PCE_DoubleArg {
public:
  PCE_Or (PointCutExpr *arg0, PointCutExpr *arg1) : 
    PCE_DoubleArg (arg0, arg1) {}
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "or_";
    PCE_DoubleArg::mangle_type_check (out);
  }
  virtual PCE_Or* duplicate () { return new PCE_Or(arg(0)->duplicate (), arg(1)->duplicate()); }
};

class PCE_And : public PCE_DoubleArg {
public:
  PCE_And (PointCutExpr *arg0, PointCutExpr *arg1) : 
    PCE_DoubleArg (arg0, arg1) {}
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "and_";
    PCE_DoubleArg::mangle_type_check (out);
  }
  virtual PCE_And* duplicate () { return new PCE_And(arg(0)->duplicate (), arg(1)->duplicate()); }
};

class PCE_Not : public PCE_SingleArg {
public:
  PCE_Not (PointCutExpr *arg) : PCE_SingleArg (arg) {}
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "not_";
    PCE_SingleArg::mangle_type_check (out);
  }
  virtual PCE_Not* duplicate () { return new PCE_Not(arg(0)->duplicate ()); }
};

class PCE_Named : public PointCutExpr {
  ACM_Pointcut *_pct_func;
  PointcutExpression _expr;
  std::list<std::string> _params;
public:
  PCE_Named (ACM_Pointcut *pct_func) : _pct_func (pct_func) {}
  virtual PCE_Type type () const;
  std::list<std::string> &params () { return _params; }
  PointcutExpression &expr () { return _expr; }
  virtual int args () const { return 0; }
  virtual PointCutExpr *arg (int) const { return 0; }
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "named_";
    ((PointCutExpr*)_expr.get())->mangle_type_check (out);
  }
  virtual PCE_Named* duplicate () {
    PCE_Named *result = new PCE_Named(_pct_func);
    result->_expr = _expr;
    result->_params = _params;
    return result;
  }
};

class PCE_CXX11Attr : public PointCutExpr {
  ACM_Attribute *_pct_attr;
  std::list<std::string> _params;
public:
  PCE_CXX11Attr ( ACM_Attribute *pct_attr ) : _pct_attr(pct_attr) {}

  virtual PCE_Type type () const {return PCE_NAME;}
  std::list<std::string> &params () { return _params; }
  virtual int args() const {return 0;} //Is this right?
  virtual PointCutExpr *arg(int) const {return 0;} //Is this right?
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out) {
    out << "named_";
    arg(0)->mangle_type_check (out);
  }
  virtual PCE_CXX11Attr* duplicate () {
    PCE_CXX11Attr *result = new PCE_CXX11Attr(_pct_attr);
    result->_params = _params;
    return result;
  }
};

class PCE_ContextVar : public PointCutExpr {
  string _name;
  const ACM_Arg *_bound_arg;
#ifdef FRONTEND_PUMA  // TODO: get rid of frontend types here
  const Puma::CTypeInfo *_check_type;
#else // FRONTEND_CLANG
  clang::QualType _check_type;
#endif
  enum { CV_THAT, CV_TARGET, CV_ARG, CV_RESULT } _bound_to;
  int _arg;
public:
  PCE_ContextVar (const string &name) : _name (name) {}
  virtual int args () const { return 0; }
  virtual PointCutExpr *arg (int) const { return 0; }
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out);
  virtual PCE_ContextVar* duplicate () { return new PCE_ContextVar(_name); }
};

class PCE_Match : public PointCutExpr {
  MatchExpr _match_expr;
  string _str;
public:
  PCE_Match (const string &str) : _str (str) {}
  virtual ~PCE_Match () {}
  virtual int args () const { return 0; }
  virtual PointCutExpr *arg (int) const { return 0; }
  virtual PCE_Type type () const;
  virtual void semantics (ACErrorStream &err, PointCutContext &context, int warn_compat);
  bool parse ();

  // if the expression supports pre-evaluation, do it in this function
  virtual void pre_evaluate (PointCutContext &context);
  void pre_evaluate (const MatchName &name, PointCutContext &context);
  virtual bool evaluate (ACM_Any &jpl, PointCutContext &context,
                         Binding &binding, Condition &cond);
  virtual void mangle_type_check (ostream &out);
  virtual PCE_Match* duplicate () { ref(1); return this; }
};

#endif // __PointCutExpr_h__
