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

#include "ThisJoinPoint.h"
#include "Naming.h"
#include "Binding.h"
#include "ACModel/Utils.h"
#include "ACResultBuffer.h"
#include "AdviceInfo.h"

#include <sstream>
using std::stringstream;
using std::endl;

// special setup function for bindings
void ThisJoinPoint::setup (const Binding &binding) {
  if (binding._used) {
    _used |= (binding._this != 0) ? THAT : 0;
    _used |= (binding._target != 0) ? TARGET : 0;
    _used |= (binding._result != 0) ? RESULT : 0;
    _used |= (binding._args.length () > 0) ? ARG : 0;
    _used |= (PTR_INTERNAL_NEEDED|TYPE_INTERNAL_NEEDED);
  }
}

void ThisJoinPoint::check_field (const char* field, bool dyn_only) {
  if (strcmp (field, "signature") == 0)
    _used |= SIGNATURE;
  else if (strcmp (field, "filename") == 0)
    _used |= FILENAME;
  else if (strcmp (field, "line") == 0)
    _used |= LINE;
  else if (strcmp (field, "args") == 0)
    _used |= ARGS;
  else if (strcmp (field, "arg") == 0)
    _used |= ARG;
  else if (strcmp (field, "argtype") == 0)
    _used |= ARG_TYPE;
  else if (strcmp (field, "type") == 0)
    _used |= TYPE;
  else if (strcmp (field, "id") == 0 || (!dyn_only && strcmp (field, "JPID") == 0))
    _used |= ID;
  else if (strcmp (field, "resulttype") == 0)
    _used |= RESULT_TYPE;
  else if (strcmp (field, "that") == 0)
    _used |= THAT;
  else if (strcmp (field, "target") == 0)
    _used |= TARGET;
  else if (strcmp (field, "result") == 0)
    _used |= RESULT;
  else if (strcmp (field, "entity") == 0)
    _used |= ENTITY;
  else if (strcmp (field, "memberptr") == 0)
    _used |= MEMBERPTR;
  else if (strcmp (field, "array") == 0)
    _used |= ARRAY;
  else if (strcmp (field, "idx") == 0)
    _used |= IDX;
  else if (strcmp (field, "jptype") == 0)
    _used |= JP_TYPE;
  else if (strcmp (field, "action") == 0)
    _used |= ACTION;
  else if (strcmp (field, "wrapper") == 0)
    _used |= WRAPPER;
  else if (strcmp (field, "proceed") == 0) {
    _used |= PROCEED_ADVICE;
    _proceed_calls++;
  }
}

bool ThisJoinPoint::check_type (const string &name) {
  if (name == "JoinPoint" || name.substr(0, 11) == "JoinPoint::") {
    _used |= TYPE_ADVICE_NEEDED;
    return true;
  }
  return false;
}

bool ThisJoinPoint::check_obj (const string &name) {
  if (name == "tjp") {
    _used |= (PTR_ADVICE_NEEDED | TYPE_ADVICE_NEEDED);
    return true;
  }
  else if (name == "thisJoinPoint") {
    _used |= (PTR_ADVICE_NEEDED | PTR_ALIAS_NEEDED | TYPE_ADVICE_NEEDED);
    return true;
  }
  return false;
}

// Checks if it is necessary to weave additionally things like call wrapper
// arguments to ensure correct runtime checks
void ThisJoinPoint::check_condition(const Condition& condition) {
  if (condition) {
    // that needed?
    TypeCheckSet tcs;
    condition.checks_for_that(tcs);
    if(!tcs.empty()) {
      _used |= THAT;
    }
  }
}

void ThisJoinPoint::gen_tjp_struct (ostream &code, ACM_Code *loc,
                                    BackEndProblems &bep, int depth) const {
  // the level 0 struct -> contains all the data and types
  code << "template <typename TResult, typename TThat, typename TTarget, ";
#ifdef FRONTEND_CLANG
  // Entity is only supported in the Clang variant and if enabled on cmdline
  if( _enable_entity ) {
    code << "typename TEntity, ";
    if( loc->type_val() & ( JPT_Access & ~ ( JPT_Call | JPT_Builtin ) ) )
      code << "typename TDims, ";
  }
#endif
  code << "typename TArgs";
  if( _need_wormhole )
    code << ", typename TWORMHOLE";
  code << "> struct ";
  Naming::tjp_struct (code, loc, depth);
  if (depth > 0) {
    // the level N > 0 structs -> are derived from level N - 1
    code << " : ";
    Naming::tjp_struct (code, loc, depth - 1);
    code << "<TResult, TThat, TTarget, ";
#ifdef FRONTEND_CLANG
    // Entity is only supported in the Clang variant
    if( _enable_entity ) {
      code << "TEntity, ";
      if( loc->type_val() & ( JPT_Access & ~ ( JPT_Call | JPT_Builtin ) ) )
        code << "TDims, ";
    }
#endif
    code << "TArgs";
    if( _need_wormhole )
      code << ", TWORMHOLE";
    code << ">";
  }
  else {
    // the level N == 0 structs are derived from AC::Action, if action() used
    if (useAction())
      code << " : AC::Action";
  }
  code << " {" << endl;
  code << "  typedef ";
  Naming::tjp_struct (code, loc, depth);
  code << " __TJP;" << endl; // internal type definition

  if( depth != 0 )
    return; // nothing to do, skip generation of lvl 0 parts

  stringstream types, data, fct;

  // start: type definitions --------------------------------------------------
  bool res_is_undef = !has_result_type(*loc);
  bool res_is_ref   = false;
  if (!res_is_undef) {
    ACM_Type *rtype = get_result_type (*loc);
    MatchSignature &match_sig = rtype->get_match_sig();
    if (match_sig.is_new())
      match_sig.parse(format_type (*rtype));
    if (match_sig.type ().is_reference())
      res_is_ref = true;
  }
  types << "  typedef TResult" << ( res_is_ref ? "*" : "" ) << ( forced_const_result() ? " const" : "" ) << " Result;" << endl;
  types << "  typedef TThat   That;" << endl;
  types << "  typedef TTarget Target;" << endl;
#ifdef FRONTEND_CLANG
  // this feature is only available in Clang and if enabled on cmdline
  if( _enable_entity ) {
    types << "  typedef TEntity Entity;" << endl;
    if( loc->type_val() & ( JPT_Access & ~ ( JPT_Call | JPT_Builtin ) ) ) {
      types << "  enum { DIMS = TDims::Count };" << endl;
      types << "  typedef typename TDims::template Array<TEntity>::Type Array;" << endl;
      types << "  template <unsigned int D> struct Dim : AC::DIL_Op<TDims, D> {};" << endl;
    }
    else
      types << "  enum { DIMS = 0 };" << endl;
    if( memberptr_defined( loc ) )
      types << "  typedef " << ( array_defined( loc ) ? "Array" : "TEntity" ) << " TTarget::* MemberPtr;" << endl;
    else // use void * for non member entities (function returns 0)
      types << "  typedef void * MemberPtr;" << endl;
  }
#endif

  // argument count and types
  unsigned arg_count = get_arg_count( *loc );
  types << "  enum { ARGS = TArgs::ARGS };" << endl;
  if( forced_const_args() ) {
    types << "  template <int I> struct Arg : AC::ArgCQ<TArgs, " << _force_const_args << ", I> {};" << endl;
  }
  else
    types << "  template <int I> struct Arg : AC::Arg<TArgs, I> {};" << endl;
    
  // join point id and type;
  // TODO: handle join-point ID in TJP
  if (id()) {
    int jpid = loc->get_jpid(); //(loc->assigned_id () == -1 ? loc->id () : loc->assigned_id ());
    types << "  static const int JPID = " << jpid << ";" << endl;
  }
  types << "  static const AC::JPType JPTYPE = (AC::JPType)";
  switch( loc->type_val() ) { // map to real ids if necessary
  case JPT_GetRef:
    types << JPT_Get; break;
  case JPT_SetRef:
    types << JPT_Set; break;
  default:
    types << loc->type_val(); break;
  }
  types << ";" << endl;
  
  // result type
  types << "  struct Res {" << endl;
  types << "    typedef " << (res_is_undef ? "void" : "TResult") << " "
        << (res_is_ref ? "&" : "") << "Type;" << endl;
  types << "    typedef " << (res_is_undef ? "void" : "TResult") << " ReferredType;" << endl;
  types << "  };" << endl;

  if( _need_wormhole )
    types << "  typedef TWORMHOLE __WORMHOLE;" << endl;
  
  // begin of static data -----------------------------------------------------
  if (signature ()) {
    fct << "  inline static const char *signature () { return \"";
    fct << ::signature (*loc) << "\";}" << endl;
  }
  if (filename ()) {
    fct << "  inline static const char *filename () { return \"";
    fct << ::filename (*loc) << "\";}" << endl;
  }
  if (line ()) {
    fct << "  inline static int line () { return ";
    fct << ::line(*loc) << ";}" << endl;
  }
  if (args ()) {
    fct << "  inline static const int args () { return ";
    fct << arg_count << ";";
    fct << " }" << endl;
  }
  if (type()) {
    fct << "  inline static  AC::Type type() { return ";
    fct << "\"";
    if (has_result_type(*loc))
      Naming::mangle (fct, get_result_type(*loc));
    list<ACM_Type*> tlist;
    get_arg_types (*loc, tlist);
    for (list<ACM_Type*>::iterator iter = tlist.begin(); iter != tlist.end();
        ++iter)
      Naming::mangle (fct, *iter);
    fct << "\"; }" << endl;
  }
  if (id()) {
    fct << "  inline static unsigned int id() { return JPID; }" << endl;
  }
  
  if (resulttype()) {
    fct << "  inline static AC::Type resulttype() {return ";
    if (has_result_type(*loc)) {
      fct << "\"";
      Naming::mangle (fct, get_result_type(*loc));
      fct << "\";";
    } else fct << "\"<unknown signature>\";";
    fct << "}" << endl;    
  }
  
  if (jptype()) {
    fct << "  inline static AC::JPType jptype() { return JPTYPE; }" << endl;
  }
  
  if (argtype()) {
    fct << "  inline static AC::Type const argtype(unsigned i) {" << endl;
    if (arg_count > 0) {
      fct << "    static AC::Type const type[" << arg_count << "] = {";
      list<ACM_Type*> tlist;
      get_arg_types (*loc, tlist);
      unsigned i = 0;
      for (list<ACM_Type*>::iterator iter = tlist.begin(); iter != tlist.end();
           ++iter, ++i) {
        if (i > 0) fct << ", ";
        fct << "\"";
        Naming::mangle (fct, *iter);
        fct << "\"";
      }
      fct << "};" << endl;
      fct << "    return type[i];" << endl;
      fct << "  }" << endl;
    } else {
      fct << "    return \"\";" << endl;
      fct << "  }" << endl;
    }
  }
   
  // begin of dynamic data ----------------------------------------------------
  if( arg_needed( loc ) ) {
    if (!useAction () && arg_count > 0)
      data << "  void *_args[ARGS];" << endl;
    fct <<  "  inline " << ( forced_const_args() ? "const " : "" ) << "void * arg (int n) {return "
        << (arg_count > 0 ? "_args[n]" : "0") << ";}" << endl;
    fct <<  "  template <int I> typename Arg<I>::ReferredType *arg () {"
        << endl;
    fct <<  "    return (typename Arg<I>::ReferredType*)arg (I);" << endl;
    fct <<  "  }" << endl;
  }

  if( result_needed( loc ) ) {
    if (!useAction ())
      data << "  Result *_result;" << endl;
    fct <<  "  inline Result *result() {return (Result*)_result;}" << endl;
  }

  if( target_needed( loc ) ) {
    if (!useAction ())
      data << "  Target *_target;" << endl;
    fct <<  "  inline Target *target() {return (Target*)_target;}" << endl;
  }

#ifdef FRONTEND_CLANG
  // this feature is only available in Clang and if enabled on cmdline
  if( entity_needed( loc ) ) {
    if( !useAction() )
      data << "  Entity *_entity;" << endl;
    fct << "  inline Entity *entity() {return (Entity *)_entity;}" << endl;
  }

  // member pointers cant be represented as void * so they are not available in action
  if( memberptr_needed( loc ) ) {
    if( memberptr_defined( loc ) ) {
      data << "  MemberPtr _memberptr;" << endl;
      fct << "  inline MemberPtr memberptr() {return _memberptr;}" << endl;
    }
    else // return void *0 for non member entities
      fct << "  inline MemberPtr memberptr() {return 0;}" << endl;
  }
  if( loc->type_val() & ( JPT_Access & ~ ( JPT_Call | JPT_Builtin ) ) ) {
    if( ! useAction() && array_defined( loc ) ) {
      if( array_needed( loc ) )
        data << "  Array *_array;" << endl;
      if( idx_needed( loc ) )
        data << "  void *_indices[DIMS];" << endl;
    }
    if( array_needed( loc ) )
      fct << "  inline Array *array() {return (Array*)" << ( array_defined( loc ) ? "_array" : "0" ) << ";}" << endl;
    if( idx_needed( loc ) ) {
      if( array_defined( loc ) ) {
        fct << "  template <unsigned int D> typename Dim<D>::Idx idx() {" << endl;
        fct << "    return *((typename Dim<D>::Idx*)_indices[D]);" << endl;
        fct << "  }" << endl;
      }
      else
        fct << "  template <unsigned int D> void idx() { return; }" << endl;
    }
  }
#endif

  if( that_needed( loc ) ) {
    if (!useAction ())
      data << "  That *_that;" << endl;
    fct <<  "  inline That *that() {return (That*)_that;}" << endl;
  }

  if (wrapper() || useAction ()) {
    if (!useAction ())
      data << "  void (*_wrapper)(AC::Action &);" << endl;
    fct <<  "  inline void (*wrapper ())(AC::Action &) {return _wrapper;}" << endl;
  }

  if( _need_wormhole )
    data << "  __WORMHOLE *__wormhole;" << endl;

  // terminate all the strings
  types << endl;
  data << endl;
  fct << endl;
  
  // add all types, attributes, and functions (on level 0)
  assert( depth == 0 ); // the functions returns early if not on level 0
  code << types.rdbuf();
  code << data.rdbuf();
  code << fct.rdbuf();
  
  // the closing bracket is *NOT* generated here -> for external extensions
}

ACM_Function *ThisJoinPoint::that_func (ACM_Code *loc) {
  ACM_Name *parent = get_explicit_parent( *loc );
  return ( parent && ( parent->type_val() == JPT_Function ) ) ? static_cast<ACM_Function *>( parent ) : 0;
}

void ThisJoinPoint::gen_tjp_init (ostream &code, ACM_Code *loc,
                                  BackEndProblems &bep, int depth, bool is_dep,
                                  vector<string> *arg_names, int wrapper_number ) const {
  const ThisJoinPoint &tjp = *this; // TODO: not necessary!
  JoinPointType jptype = loc->type_val ();
  
  if (tjp.pointer_needed ()) {

    code << "  __TJP ";
    Naming::tjp_instance(code, loc);
    code << ";" << endl;

#ifdef FRONTEND_CLANG
    TI_Builtin *builtin = 0;
    if( loc->type_val() == JPT_Builtin )
      builtin = TI_Builtin::of( static_cast<ACM_Builtin &>( *loc ) );
#endif

    int args = get_arg_count(*loc);
    if (arg_needed (loc)) {
      if (args) {
        if (tjp.useAction ()) {
          code << "  void *";
          Naming::tjp_args_array(code, loc);
          code << "[] = { ";
          for (int i = 0; i < args; i++) {
            if (i > 0)
              code << ", ";
            code << "(void*)";
#ifdef FRONTEND_CLANG
            if( builtin && builtin->arg_is_unavailable( i, wrapper_number ) ) {
              code << "0";
              continue;
            }
            else
#endif
              code << "&";
            if (arg_names)
              code << (*arg_names)[i];
            else
              code << "arg" << i;
          }
          code << " };" << endl;
        }
      }

      if (tjp.useAction ()) {
        code << "  ";
        Naming::tjp_instance(code, loc);
        code << "._args = ";
        if (args)
          Naming::tjp_args_array(code, loc);
        else
          code << "0";
        code << ";" << endl;
      }
      else {
        for (int i = 0; i < args; i++) {
          code << "  ";
          Naming::tjp_instance(code, loc);
          code << "._args[" << i << "] = (void*)";
#ifdef FRONTEND_CLANG
          if( builtin && builtin->arg_is_unavailable( i, wrapper_number ) ) {
            code << "0;" << endl;
            continue;
          }
          else
#endif
            code << "&";
          if (arg_names)
            code << (*arg_names)[i];
          else
            code << "arg" << i;
          code << ";" << endl;
        }
      }
    }
    
    if( result_needed( loc ) ) {
        code << "  ";
      Naming::tjp_instance(code, loc);
      code << "._result = ";
      if (!has_result_type(*loc) || get_result_type(*loc)->get_signature() == "void") {
        code << "0";
      } else {
        if (tjp.useAction ())
          code << "(void*)";
        code << "&(" << (is_dep ? "typename " : "") << "__TJP::Result&)" << ACResultBuffer::result_name();
      }
      code << ";" << endl;
    }

#ifdef FRONTEND_CLANG
    // this feature is only available in Clang and if enabled on cmdline
    if( entity_needed( loc ) ) {
      code << "  ";
      Naming::tjp_instance(code, loc);
      code << "._entity = ";
      if( loc->type_val() & ( JPT_Code & ~ JPT_Builtin ) ) {
        code << " (";
        if (tjp.useAction ())
          code << "void*)";
        else
          code << (is_dep ? "typename " : "") << "__TJP::Entity *)";
        if( has_entity_func( *loc ) && needs_this( *get_entity_func( *loc ) ) )
          code << "0"; // for member function we can't aquire a normal function pointer
        else
          code << "&ent";
      }
      else {
        code << " 0";
      }
      code << ";" << endl;
    }

    // this feature is only available in Clang and if enabled on cmdline
    if( memberptr_defined( loc ) && memberptr_needed( loc ) ) {
      code << "  ";
      Naming::tjp_instance(code, loc);
      code << "._memberptr = " << "(" << (is_dep ? "typename " : "") << "__TJP::MemberPtr)" << "member;" << endl;
    }

    if( loc->type_val() & ( JPT_Access & ~ ( JPT_Call | JPT_Builtin ) ) ) {
      if( array_needed( loc ) && array_defined( loc ) ) {
        code << "  ";
        Naming::tjp_instance( code, loc );
        code << "._array = (";
        if( tjp.useAction() )
          code << "void*)";
        else
          code << (is_dep ? "typename " : "") << "__TJP::Array*)";
        code << "&base;" << endl;
      }
      else if( useAction() ) { // useAction() => array_needed() , thus array_defined() == false here
        code << "  ";
        Naming::tjp_instance( code, loc );
        code << "._array = 0;" << endl;
      }

      const unsigned int idx_count = TI_Access::of( *static_cast<ACM_Access *>( loc ) )->entity_index_count();
      if( useAction() ) {
        code << "  void *__idx_array[] = { ";
        for( unsigned int i = 0; i < idx_count; i++ ) {
          if( i > 0 )
            code << ", ";
          code << "(void*)&idx" << i;
        }
        code << " };" << endl;

        code << "  ";
        Naming::tjp_instance( code, loc );
        code << "._indices = " << ( array_defined( loc ) ? "__idx_array" : "0" ) << ";" << endl;
      }
      else if( idx_needed( loc ) && array_defined( loc ) ) {
        for( unsigned int i = 0; i < idx_count; i++ ) {
          code << "  ";
          Naming::tjp_instance( code, loc );
          code << "._indices[" << i << "] = (void *)&idx" << i << ";" << endl;
        }
      }
    }
#endif
    
    if( target_needed( loc ) ) {
      code << "  ";
      Naming::tjp_instance(code, loc);
      code << "._target = ";
      if( has_entity( *loc ) && needs_this( *get_entity( *loc ) ) ) {
        assert(jptype != JPT_Builtin);
        code << " (";
        if (tjp.useAction ())
          code << "void*)";
        else {
          code << (is_dep ? "typename " : "") << "__TJP::Target*)";
        }
        switch (jptype) {
        // inner joinpoints
        case JPT_Construction:
        case JPT_Destruction:
        case JPT_Execution:
          code << "this";
          break;
        // outer joinpoints
        case JPT_Call:
        case JPT_Get:
        case JPT_Set:
        case JPT_Ref:
          code << "&dst";
          break;
        default:
          code << " 0";
        }
      } else {
        code << " 0";
      }
      code << ";" << endl;
    }

    if( that_needed( loc ) ) {
      code << "  ";
      Naming::tjp_instance(code, loc);
      code << "._that = ";
      if (that_func (loc) && needs_this (*that_func (loc))) {
        code << " (";
        if (tjp.useAction ())
          code << "void*)";
        else {
          code << (is_dep ? "typename " : "")<< "__TJP::That*)";
        }
        switch (jptype) {
        // inner joinpoints
        case JPT_Construction:
        case JPT_Destruction:
        case JPT_Execution:
          code << "this";
          break;
        // outer joinpoints
        case JPT_Call:
        case JPT_Builtin:
        case JPT_Get:
        case JPT_Set:
        case JPT_Ref:
        case JPT_GetRef:
        case JPT_SetRef:
          code << "srcthis";
          break;
        default:
            code << " 0";
        }
      } else {
        code << " 0";
      }
      code << ";" << endl;
    }
    
    if (tjp.useAction ()) {
      code << "  ";
      Naming::tjp_instance(code, loc);
      code << "._fptr = ";
      code << "0";
      code << ";" << endl;
    }
  }
}

void ThisJoinPoint::merge_flags (ACM_CodePlan &plan) {
  if (plan.has_next_level())
    merge_flags(*plan.get_next_level());
  typedef ACM_Container<ACM_CodeAdvice, true> Container;
  Container &before = plan.get_before();
  for (Container::iterator i = before.begin(); i != before.end(); ++i)
    TI_CodeAdvice::of(*(*i))->get_advice_info()->addTJPFlags(*this);
  if (plan.has_around()) {
    TI_CodeAdvice::of(*plan.get_around())->get_advice_info()->addTJPFlags(*this);
    if (*TI_CodeAdvice::of(*plan.get_around())->get_condition())
      conditional();
  }
  Container &after = plan.get_after();
  for (Container::iterator i = after.begin(); i != after.end(); ++i)
    TI_CodeAdvice::of(*(*i))->get_advice_info()->addTJPFlags(*this);
}

#ifdef FRONTEND_CLANG
// only required for features of the Clang variant
void ThisJoinPoint::merge_implicit( ACM_Code &loc ) {
  assert( loc.type_val() == JPT_Builtin ); // currently only builtin operator calls can have implicit joinpoints
  ACM_Builtin &node = static_cast<ACM_Builtin &>( loc );

  ThisJoinPoint collect;

  typedef ACM_Container<ACM_Access, true> Container;
  Container &implicit = node.get_implicit_access();
  for( Container::const_iterator it = implicit.begin(); it != implicit.end(); it++ ) {
    if( ( *it )->has_plan() )
      collect.merge_flags( *( ( *it )->get_plan() ) );
    if( TI_Code::of( **it )->has_implicit_joinpoints() )
      collect.merge_implicit( **it );
  }

  if( collect.useAction() ) { // if we use an action, make sure we have all we need
    collect._used |= THAT;
  }

  unsigned int mask = 0; // only forward a selection, as not all are shared
  mask |= THAT;

  // merge
  _used |= ( collect._used & mask );
}
#endif

bool ThisJoinPoint::arg_needed( ACM_Code *loc ) const {
  return arg() || useAction() || ( proceed() && proceed_needs_args( *loc ) );
}

bool ThisJoinPoint::that_needed( ACM_Code *loc ) const {
  return that() || useAction() || ( proceed() && proceed_needs_that( *loc ) );
}

bool ThisJoinPoint::target_needed( ACM_Code *loc ) const {
  return target() || useAction() || ( proceed() && proceed_needs_target( *loc ) );
}

bool ThisJoinPoint::result_needed( ACM_Code *loc ) const {
  return result() || useAction() || ( proceed() && proceed_needs_result( *loc ) );
}

bool ThisJoinPoint::entity_needed( ACM_Code *loc ) const {
  return _enable_entity && ( entity() || useAction() );
}

bool ThisJoinPoint::memberptr_needed( ACM_Code *loc ) const {
  return _enable_entity && memberptr();
}

bool ThisJoinPoint::memberptr_defined( ACM_Code *loc ) const {
  return has_entity( *loc ) && needs_this( *get_entity( *loc ) ) && ! ( loc->type_val() == JPT_Construction || loc->type_val() == JPT_Destruction );
}

bool ThisJoinPoint::array_needed( ACM_Code *loc ) const {
  return _enable_entity && ( array() || useAction() );
}

bool ThisJoinPoint::array_defined( ACM_Code *loc ) const {
#ifdef FRONTEND_CLANG
  // arrays are only supported in the clang variant
  if( ! ( loc->type_val() & ( JPT_Access & ~ ( JPT_Call | JPT_Builtin ) ) ) )
#endif
    return false;

#ifdef FRONTEND_CLANG
  TI_Access *ti = TI_Access::of( *static_cast<ACM_Access *>( loc ) );
  return ti->entity_index_count() > 0;
#endif
}

bool ThisJoinPoint::idx_needed( ACM_Code *loc ) const {
  return _enable_entity && ( idx() || useAction() );
}

void ThisJoinPoint::dump (ostream &out) const {
  out << "ThisJoinPoint (";
  out << "signature=" << (signature () ? "true" : "false");
  out << ", filename=" << (filename () ? "true" : "false");
  out << ", line=" << (line () ? "true" : "false");
  out << ", args=" << (args () ? "true" : "false");
  out << ", arg=" << (arg() ? "true" : "false");
  out << ", argtype=" << (argtype() ? "true" : "false");
  out << ", type=" << (type() ? "true" : "false");
  out << ", id=" << (id() ? "true" : "false");
  out << ", resulttype=" << (resulttype() ? "true" : "false");
  out << ", that=" << (that() ? "true" : "false");
  out << ", target=" << (target() ? "true" : "false");
  out << ", result=" << (result() ? "true" : "false");
  out << ", entity=" << (entity() ? "true" : "false");
  out << ", memberptr=" << (memberptr() ? "true" : "false");
  out << ", array=" << (array() ? "true" : "false");
  out << ", idx=" << (idx() ? "true" : "false");
  out << ", jptype=" << (jptype() ? "true" : "false");
  out << ", action=" << (action() ? "true" : "false");
  out << ", proceed=" << (proceed() ? "true" : "false");
  out << ")" << endl;
}
