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

#ifndef __ThisJoinPoint_h__
#define __ThisJoinPoint_h__

#include <string>
#include <vector>
#include <cassert>
using namespace std;

#include "BackEndProblems.h"

class Binding;
class ACM_Code;
class ACM_Function;
class ACM_CodePlan;
class Condition;

class ThisJoinPoint {
  enum
    {
      // these values define required information
      SIGNATURE=0x1, ARGS=0x2, ARG=0x4, ARG_TYPE=0x8,
      TYPE=0x10, ID=0x20, RESULT_TYPE=0x40, RESULT=0x80,
      THAT=0x100, TARGET=0x200, JP_TYPE=0x400, ACTION=0x800,
      WRAPPER=0x1000, PROCEED_ADVICE=0x2000, FILENAME=0x4000, LINE=0x8000,
      // does the advice need a pointer 'tjp'
      PTR_ADVICE_NEEDED=0x10000,
      // does the advice need an alias 'thisJoinPoint' for 'tjp'
      PTR_ALIAS_NEEDED=0x20000,
      // does the advice need the type JoinPoint
      TYPE_ADVICE_NEEDED=0x40000,
      ENTITY=0x80000, MEMBERPTR=0x100000, ARRAY=0x200000, IDX=0x400000,
      // does the internal code needs a pointer 'tjp'
      PTR_INTERNAL_NEEDED=0x800000,
      // does the internal code needs the type JoinPoint
      TYPE_INTERNAL_NEEDED=0x1000000,
      // does the internal code needs the proceed function
      PROCEED_INTERNAL_NEEDED=0x2000000
    };
  
  unsigned int _used;
  unsigned int _proceed_calls;
  unsigned int _force_const;
  unsigned int _force_const_args;

  bool _enable_entity;
  bool _need_wormhole;
  
  static ACM_Function *that_func (ACM_Code *loc);

public:
  ThisJoinPoint () : _used (0), _proceed_calls (0), _force_const( 0 ), _force_const_args( 0 ), _enable_entity( false ), _need_wormhole( false ) {}
  
  void setup (const Binding &binding);
  void check_field (const char* field, bool dyn_only = false);
  bool check_type (const string &name);
  bool check_obj (const string &name);
  void check_condition(const Condition&);

  void conditional () { _used |= (PROCEED_INTERNAL_NEEDED|TYPE_INTERNAL_NEEDED|PTR_INTERNAL_NEEDED); }
    
  bool signature () const { return (_used & SIGNATURE); }
  bool filename () const { return (_used & FILENAME); }
  bool line () const { return (_used & LINE); }
  bool args () const { return (_used & ARGS); }
  bool arg () const { return (_used & ARG); }
  bool argtype() const { return (_used & ARG_TYPE); }
  bool type() const { return (_used & TYPE); }
  bool id() const { return (_used & ID); }
  bool resulttype() const { return (_used & RESULT_TYPE); }
  bool that() const { return (_used & THAT); }
  bool target() const { return (_used & TARGET); }
  bool result() const { return (_used & RESULT); }
  bool entity() const { return (_used & ENTITY); }
  bool memberptr() const { return (_used & MEMBERPTR); }
  bool array() const { return ( _used & ARRAY ); }
  bool idx() const { return ( _used & IDX ); }
  bool jptype() const { return (_used & JP_TYPE); }
  bool action() const { return (_used & ACTION); }
  bool wrapper() const { return (_used & WRAPPER); }

  bool proceed() const { return proceed_advice() || proceed_internal(); }
  bool proceed_internal() const { return (_used & PROCEED_INTERNAL_NEEDED); }
  bool proceed_advice() const { return (_used & PROCEED_ADVICE); }

  unsigned int proceed_calls () const { return _proceed_calls; }

  void require_entity() { _used |= ENTITY; }
  void require_idx() { _used |= IDX; }
  
  void gen_tjp_struct (ostream &code, ACM_Code *loc,
                       BackEndProblems &bep, int depth) const;
  void gen_tjp_init (ostream &code, ACM_Code *loc,
                     BackEndProblems &bep, int depth, bool is_dep = false,
                     vector<string> *arg_names = 0, int wrapper_number = -1 ) const;
  void merge_flags (ACM_CodePlan &plan);
  void merge_implicit( ACM_Code &jpl );
  void dump (ostream &os) const;
  
  bool pointer_needed () const { return pointer_advice_needed() || pointer_internal_needed(); }
  bool pointer_internal_needed () const { return (_used & PTR_INTERNAL_NEEDED); }
  bool pointer_advice_needed () const { return (_used & PTR_ADVICE_NEEDED); }

  bool pointer_alias_needed () const { return (_used & PTR_ALIAS_NEEDED); }

  bool type_needed () const { return type_advice_needed() || type_internal_needed(); }
  bool type_internal_needed () const { return (_used & TYPE_INTERNAL_NEEDED); }
  bool type_advice_needed () const { return (_used & TYPE_ADVICE_NEEDED); }

  bool arg_needed (ACM_Code *loc) const;
  bool that_needed( ACM_Code *loc ) const;
  bool target_needed( ACM_Code *loc ) const;
  bool result_needed( ACM_Code *loc ) const;
  bool entity_needed( ACM_Code *loc ) const;
  bool memberptr_needed( ACM_Code *loc ) const;
  bool memberptr_defined( ACM_Code *loc ) const;
  bool array_needed( ACM_Code *loc ) const;
  bool array_defined( ACM_Code *loc ) const;
  bool idx_needed( ACM_Code *loc ) const;

  bool forced_const_arg( unsigned int i ) const { assert( i < 32 ); return ( _force_const_args & ( 1 << i ) ); }
  bool forced_const_args() const { return _force_const_args; }
  void force_const_arg( unsigned int i ) { _force_const_args |= ( 1 << i ); }
  bool forced_const_result() const { return (_force_const & RESULT ); }
  void force_const_result() { _force_const |= RESULT; }
  
  bool useAction() const { return action(); }

  void merge (const ThisJoinPoint &from) { _used |= from._used; }
  void clear () { _used = 0; }

#ifdef FRONTEND_CLANG
  void enable_entity() { _enable_entity = true; }
#endif
  void request_wormhole() { _need_wormhole = true; }
  bool has_wormhole() const { return _need_wormhole; }
};
  
#endif // __ThisJoinPoint_h__
