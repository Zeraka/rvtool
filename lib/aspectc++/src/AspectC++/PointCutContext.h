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

#ifndef __PointCutContext_h__
#define __PointCutContext_h__

#include <set>
using namespace std;

#include "Puma/Array.h"
#include "Puma/Stack.h"

#include "Binding.h"
#include "ACModel/Elements.h"
#include "TransformInfo.h"
#include "ACConfig.h"

class PointCutExpr;

typedef Puma::Array<const ACM_Arg*> ArgSet;

class PointCutContext {
  ACConfig& _config;
  ProjectModel &_jpm;
  ACM_Aspect *_aspect;
  ACM_Pointcut *_pct_func;
  int _per_aspect_cflows;

  // new pointcut evaluation data members
  Puma::Stack<ArgSet*> _arg_bindings;
  int _in_arg;
  bool _in_result;
  bool _in_that;
  bool _in_target;
  bool _pseudo_true;
  bool _in_old_style_scopematch;
  set<PointCutExpr*> _cflow_exprs;
  // Messages that will be output to the AC++ error stream
  vector<pair<Puma::ErrorSeverity, string>> _messages;

public:

  PointCutContext (ProjectModel &jpm, ACConfig& config) :
    _config(config), _jpm (jpm), _aspect (0), _pct_func (0), _per_aspect_cflows (0),
    _in_arg (-1), _in_result (false), _in_that (false), _in_target (false),
    _pseudo_true (false), _in_old_style_scopematch( false ) {
  }

  PointCutExpr *lookup_virtual_pct_func (ACM_Class *scope, ACM_Pointcut *pct_func);
  PointCutExpr *lookup_virtual_pct_func (ACM_Pointcut *pct_func) {
    return lookup_virtual_pct_func(_aspect, pct_func);
  }

  ProjectModel &jpm () { return _jpm; }

  ACConfig& config() { return _config;}

  ACM_Pointcut *pct_func (ACM_Pointcut *new_pct_func) {
    ACM_Pointcut *result = _pct_func;
    _pct_func = new_pct_func;
    return result;
  }
  ACM_Pointcut *pct_func () const { return _pct_func; }
  void concrete_aspect (ACM_Aspect &ai) { _aspect = &ai; _per_aspect_cflows = 0; }

  // return the location of the current pointcut function (for error messages)
#ifdef FRONTEND_PUMA
  Puma::Location location () const {
    static Puma::Location loc;
    if (!_pct_func) return loc;
#else
  clang::FullSourceLoc location () const {
    if (!_pct_func) return clang::FullSourceLoc();
#endif
    return TI_Pointcut::of(*_pct_func)->get_location();
  }

  // used for semantic analysis of pointcut expressions
  Puma::Stack<ArgSet*> &arg_bindings () { return _arg_bindings; }

  // used for context binding in args()
  bool in_arg () const { return _in_arg >= 0; }
  void set_arg (int a) { _in_arg = a; }
  int get_arg () const { return _in_arg; }

  // used for context binding in result()
  bool in_result () const { return _in_result; }
  void enter_result () { _in_result = true; }
  void leave_result () { _in_result = false; }

  // used for context binding in that()
  bool in_that () const { return _in_that; }
  void enter_that () { _in_that = true; }
  void leave_that () { _in_that = false; }

  // used for context binding in target()
  bool in_target () const { return _in_target; }
  void enter_target () { _in_target = true; }
  void leave_target () { _in_target = false; }

  // that or within was evaluated as true, because we have a pseudo call
  void pseudo_true (bool val) { _pseudo_true = val; }
  bool is_pseudo_true () const { return _pseudo_true; }

  // needed to suppress matching of PCE_Member if doing it the old way
  bool in_old_style_scopematch() const { return _in_old_style_scopematch; }
  void enter_old_style_scopematch() { _in_old_style_scopematch = true; }
  void leave_old_style_scopematch() { _in_old_style_scopematch = false; }

  // cflow trigger management
  int cflow (PointCutExpr *pce) {
    _cflow_exprs.insert (pce);
    return _per_aspect_cflows++;
  }
  const set<PointCutExpr*> &cflows () { return _cflow_exprs; }
  void cflow_reset () { _cflow_exprs.clear (); }

  // This is used in PCE_X::evaluate as there is no AC++ error stream available.
  vector<pair<Puma::ErrorSeverity,string>>& messages() { return _messages; }
};

#endif // __PointCutContext_h__
