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

#include "Plan.h"
#include "AspectInfo.h"
#include "AdviceInfo.h"
#include "OrderInfo.h"
#include "IntroductionInfo.h"
#include "PointCutContext.h"
#include "PointCutExpr.h"
#include "ModelBuilder.h"
#include "ACModel/Utils.h"

using namespace Puma;

Plan::Plan (ACErrorStream &e, ProjectModel &jpm, ACConfig& config)
           : _conf(config), _err (e), _jpm (jpm) {
  
  // get all aspects from the join point model
  ProjectModel::Selection all_aspects;
  _jpm.select (JPT_Aspect, all_aspects);
  
  // sort them by inserting them into a set
  for (ProjectModel::Selection::iterator iter = all_aspects.begin ();
       iter != all_aspects.end (); ++iter) {
    ACM_Aspect &jpl_aspect = (ACM_Aspect&)**iter;
    _sorted_aspects.insert (&jpl_aspect);
  }

  for (set<ACM_Aspect*, LessAspectPtr>::iterator iter = _sorted_aspects.begin ();
      iter != _sorted_aspects.end (); ++iter) {
    ACM_Aspect &jpl_aspect = (ACM_Aspect&)**iter;

    if (jpl_aspect.get_derived().size () > 0)
//    if (is_abstract(jpl_aspect))
      continue;

    AspectInfo *aspect_info = addAspect (jpl_aspect);

    // collect and register all introductions, advice code, and order advice
    // triggered by a concrete aspect or any of its base aspects
    collect_advice (aspect_info, jpl_aspect);
  }
}

Plan::~Plan () {
  for (int i = 0; i < (int)_advice_infos.length (); i++)
    delete _advice_infos[i];
  for (list<IntroductionInfo*>::iterator i = _introduction_infos.begin ();
       i != _introduction_infos.end (); ++i)
    delete *i;
  for (list<OrderInfo*>::iterator i = _order_infos.begin ();
       i != _order_infos.end (); ++i)
    delete *i;
  for (AdviceMap::iterator i = _advice_map.begin (); i != _advice_map.end(); ++i)
    delete i->second;
  for (IntroMap::iterator i = _intro_map.begin (); i != _intro_map.end(); ++i)
    delete i->second;
}

// collect and register all introductions, advice code, and order advice
// triggered by a concrete aspect or any of its base aspects
void Plan::collect_advice (AspectInfo *aspect_info, ACM_Aspect &checked_aspect) {
  ACM_Aspect &concrete_aspect = aspect_info->loc();
  typedef ACM_Container<ACM_Introduction, true> IContainer;
  IContainer &intros = checked_aspect.get_intros();
  for (IContainer::const_iterator i = intros.begin ();i != intros.end (); ++i) {
    IntroductionInfo *intro_info = addIntroduction (concrete_aspect, **i);
    intro_info->pointcut_expr() = (*i)->get_pointcut ()->get_parsed_expr ();
  }

  typedef ACM_Container<ACM_AdviceCode, true> AContainer;
  AContainer &advices = checked_aspect.get_advices();
  for (AContainer::const_iterator i = advices.begin ();i != advices.end (); ++i) {
    AdviceInfo *ai = addAdvice(*aspect_info, **i);
    aspect_info->advice_infos().push_back (ai);
  }

  typedef ACM_Container<ACM_Order, true> OContainer;
  OContainer &orders = checked_aspect.get_orders();
  for (OContainer::const_iterator i = orders.begin ();i != orders.end (); ++i) {
    addOrder(concrete_aspect, **i);
  }

  typedef ACM_Container<ACM_Class, false> BaseContainer;
  BaseContainer &bases = checked_aspect.get_bases();
  for (BaseContainer::const_iterator i = bases.begin (); i != bases.end (); ++i) {
    ACM_Class *cls = (ACM_Class*)(*i);
    if (cls->type_val () == JPT_Aspect)
      collect_advice (aspect_info, *(ACM_Aspect*)cls);
  }
}

AspectInfo *Plan::addAspect (ACM_Aspect &a) {
  pair<AspectContainer::iterator, bool> res = _aspect_infos.insert (AspectInfo (a));
  return (AspectInfo*)&*res.first;
}

AdviceInfo *Plan::addAdvice (AspectInfo &ai, ACM_AdviceCode &code) {
  AdviceInfo *result = new AdviceInfo (ai, code);
  _advice_infos.append (result);
  return result;
}

IntroductionInfo *Plan::addIntroduction (ACM_Aspect &jpl_aspect, ACM_Introduction &intro) {
  IntroductionInfo *result = new IntroductionInfo (jpl_aspect, intro);
  _introduction_infos.push_back (result);
  return result;
}

OrderInfo *Plan::addOrder (ACM_Aspect &a, ACM_Order &o) {
  OrderInfo *result = new OrderInfo (a, o);
  _order_infos.push_back (result);
  return result;
}

// consider a join point and advice in the plan
void Plan::consider( ACM_Code *jpl, const Condition &cond, AdviceInfo *ai ) {
  if (!is_pseudo(*jpl)) {
    bool has_plan = jpl->has_plan ();
    if (!has_plan)
      jpl->set_plan (_jpm.newCodePlan());
    if (!has_plan) {
      if( jpl->type_val () & JPT_Access )
        _access_jpls.append( jpl );
      else
        switch (jpl->type_val ()) {
        case JPT_Execution:    _exec_jpls.append (jpl); break;
        case JPT_Construction: _cons_jpls.append (jpl); break;
        case JPT_Destruction:  _dest_jpls.append (jpl); break;
        default:
          _err << sev_error
               << "internal problem, invalid join point type in plan"
               << endMessage;
          return;
        }
    }
    ACM_CodeAdvice *code_advice = _jpm.newCodeAdvice();
    ACM_AdviceCode* advice_code = &ai->code();
    code_advice->set_advice(advice_code);
    code_advice->set_conditional(cond);
    TI_CodeAdvice::of(*code_advice)->set_condition(&cond);
    TI_CodeAdvice::of(*code_advice)->set_advice_info(ai);

    // TODO:
    // It looks like the advice info object of an advice A is the same for some/all(?)
    // join points that are in the pointcut of A when weaving these join points for the
    // advice A. Therefore the ThisJoinPoint object is the same and e.g. arguments for
    // runtime checks are inserted at each of these join points even if an individual
    // join point does not need them (because it need no runtime check).
    // Thus a ThisJoinPoint object should be linked to the combination of advice info
    // and join point.
    // This problem yields unnecessary code (e.g. in case of "that") but apart from
    // that is no real problem.
    ThisJoinPoint& tjp = TI_AdviceCode::of(*advice_code)->this_join_point ();
    tjp.check_condition(cond);
    if(code_advice->get_conditional() && advice_code->get_kind() == ACT_AROUND) {
      tjp.conditional();
    }

    AdviceMap::iterator i = _advice_map.find( jpl );
    vector<ACM_CodeAdvice*> *jp_advice = 0;
    if (i == _advice_map.end ()) {
      jp_advice = new vector<ACM_CodeAdvice*>;
      _advice_map.insert( AdviceMap::value_type( jpl, jp_advice ) );
    }
    else
      jp_advice = i->second;
    // remember this advice; unsorted for now
    jp_advice->push_back(code_advice);
  }

  if (cond) {
    ACM_Function *that_func = 0;
    if (jpl->get_parent () && ((ACM_Any*)jpl->get_parent())->type_val() == JPT_Function)
      that_func = (ACM_Function*)jpl->get_parent();
    ACM_Name *target_name = 0;
    if (jpl->type_val () == JPT_Call)
      target_name = ((ACM_Call*)jpl)->get_target();
    else if (jpl->type_val () == JPT_Builtin)
      target_name = ((ACM_Builtin*)jpl)->get_target();
    else if (jpl->type_val () == JPT_Get)
      target_name = ((ACM_Get*)jpl)->get_variable();
    else if (jpl->type_val () == JPT_Set)
      target_name = ((ACM_Set*)jpl)->get_variable();
    else if (jpl->type_val () == JPT_Ref)
      target_name = ((ACM_Ref*)jpl)->get_variable();
    cond.checks_for_that (_type_checks_true);
    cond.checks_for_target (_type_checks_true);

    StringSet check_names_that, check_names_target;

    if (that_func) {
      cond.names_for_that (check_names_that);
      for (StringSet::iterator iter = check_names_that.begin ();
          iter != check_names_that.end (); ++iter) {
        if (((ACM_Any*)that_func->get_parent())->type_val() & (JPT_Class|JPT_Aspect))
          _type_checks_false.insert (TypeCheck ((ACM_Class*)that_func->get_parent(),
              *iter));
      }
    }

    if (target_name) {
      cond.names_for_target (check_names_target);
      for (StringSet::iterator iter = check_names_target.begin ();
          iter != check_names_target.end (); ++iter) {
        if (((ACM_Any*)target_name->get_parent())->type_val() & (JPT_Class|JPT_Aspect))
          _type_checks_false.insert (TypeCheck ((ACM_Class*)target_name->get_parent(),
              *iter));
      }
    }
  }
}

void Plan::consider (ACM_Code *jpl, const CFlow &cflow) {
  if (is_pseudo(*jpl))
    return;

  bool has_plan = jpl->has_plan ();
  if (!has_plan)
    jpl->set_plan (_jpm.newCodePlan());
  if (!has_plan) {
    switch (jpl->type_val ()) {
    case JPT_Execution:    _exec_jpls.append (jpl); break;
    case JPT_Call:
    case JPT_Builtin:      _access_jpls.append (jpl); break;
    case JPT_Construction: _cons_jpls.append (jpl); break;
    case JPT_Destruction:  _dest_jpls.append (jpl); break;
    default:
      _err << sev_error
           << "internal problem, invalid join point type in plan"
           << endMessage;
      return;
    }
  }
  TI_Code::of(*jpl)->consider (cflow);
}

void Plan::consider( ACM_Code *jpl ) {
  if( is_pseudo( *jpl ) )
    return;

  bool has_plan = jpl->has_plan();
  if( ! has_plan ) {
    jpl->set_plan( _jpm.newCodePlan() );

    if( jpl->type_val () & JPT_Access )
      _access_jpls.append( jpl );
    else
      switch( jpl->type_val() ) {
        case JPT_Execution:
	  _exec_jpls.append( jpl );
	  break;
        case JPT_Construction:
	  _cons_jpls.append( jpl );
	  break;
        case JPT_Destruction:
	  _dest_jpls.append( jpl );
	  break;
        default:
          _err << sev_error
               << "internal problem, invalid join point type in plan"
               << endMessage;
          return;
      }
  }
}

// consider a join point for an introduction in the plan
void Plan::consider (ACM_Class &jpl, ACM_Introduction *intro) {

  bool has_plan = jpl.has_plan();
  if (!has_plan) {
    jpl.set_plan (_jpm.newClassPlan ());
    _class_jpls.append (&jpl); // TODO: still needed? we have the intro_map
  }
  IntroMap::iterator i = _intro_map.find(&jpl);
  vector<ACM_Introduction*> *cls_intros = 0;
  if (i == _intro_map.end ()) {
    cls_intros = new vector<ACM_Introduction*>;
    _intro_map.insert(IntroMap::value_type(&jpl, cls_intros));
  }
  else
    cls_intros = i->second;
  // remember this intro; unsorted for now
  cls_intros->push_back(intro);
}
  
// calculate the order for a single join point
void Plan::order (ACM_Any *jpl) {
  // use an OrderPlanner for this purpose
  OrderPlanner<ACM_Aspect, LessAspectPtr> order_planner;

  PointCutContext context (_jpm, _conf);

  const list<OrderInfo*> &orders = order_infos ();
  for (list<OrderInfo*>::const_iterator i = orders.begin ();
       i != orders.end (); ++i) {
    OrderInfo *order = *i;

    // check if the current join point location is matched by the pointcut
    // expression of the order advice
    context.concrete_aspect (order->aspect ());
    Binding binding;     // binding and condition not used for intros
    Condition condition;
    PointCutExpr *jp_pce = (PointCutExpr*)order->jp_pce ().get ();
    if (!jp_pce->evaluate (*jpl, context, binding, condition))
      continue;
    // remember order relations in the plan for this join point
    const list<PointcutExpression> &pces = order->pces ();
    list<ACM_Aspect*> *hi = 0;
    list<ACM_Aspect*> *lo = 0;
    bool start = true;
    for (list<PointcutExpression>::const_iterator i2 = pces.begin ();
         i2 != pces.end (); ++i2) {
      PointCutExpr *pce = (PointCutExpr*)((*i2).get ());

      // match all relevant aspects with the current match expression
      list<ACM_Aspect*> *matched = new list<ACM_Aspect*>;
      // TODO: this is inefficient. Not all aspects are relevant for each jp
      // get all aspects from the join point model
      for (set<ACM_Aspect*, LessAspectPtr>::iterator iter = _sorted_aspects.begin ();
          iter != _sorted_aspects.end (); ++iter) {
        ACM_Aspect &jpl_aspect = (ACM_Aspect&)**iter;
        if (pce->evaluate (jpl_aspect, context, binding, condition))
          matched->push_back (&jpl_aspect);
      }
      if (matched->size () == 0) {
        delete matched;
      }
      else if (start) {
        lo    = matched; // will become high in the next loop
        start = false;
      }
      else {
        if (hi) delete hi;
        hi = lo;
        lo = matched;

        // now we can consider the ordering info in the plan
        for (list<ACM_Aspect*>::iterator ihi = hi->begin ();
             ihi != hi->end (); ++ihi) {
          for (list<ACM_Aspect*>::iterator ilo = lo->begin ();
                 ilo != lo->end (); ++ilo) {
//            cout << signature(**ihi) << " -> " << signature (**ilo) << endl;
            order_planner.precedence (**ihi, **ilo);
          }
        }
      }
    }
    if (lo) delete lo;
  }
  
  if (jpl->type_val() & JPT_Code)
    check (*(ACM_Code*)jpl, order_planner);
  else
    check (*(ACM_Class*)jpl, order_planner);
}

void Plan::sort_access_joinpoints() {
  // TODO use better sorting algorithm
  for( int i = 0; i < _access_jpls.length(); i++ ) {
    assert( _access_jpls[ i ]->type_val() & JPT_Access );
    ACM_Access *min_ptr = static_cast<ACM_Access *>( _access_jpls[ i ] );
    unsigned int min_id = min_ptr->get_lid();
    int min_index = i;

    for( int j = i + 1; j < _access_jpls.length(); j++ ) {
      assert( _access_jpls[ j ]->type_val() & JPT_Access );
      ACM_Access *cmp_ptr = static_cast<ACM_Access *>( _access_jpls[ j ] );
      unsigned int cmp_id = cmp_ptr->get_lid();

      if( cmp_id < min_id ) {
        min_index = j;
	min_ptr = cmp_ptr;
	min_id = cmp_id;
      }
    }

    if( min_index != i ) {
      _access_jpls[ min_index ] = _access_jpls[ i ];
      _access_jpls[ i ] = min_ptr;
    }
  }
}
  
// calculate the order for all join points
void Plan::order_code_joinpoints() {
  // recreate the order of access joinpoints (it gets destroyed by Transformer)
  sort_access_joinpoints();

  for (int i = 0; i < (int)_exec_jpls.length (); i++)
    order (_exec_jpls[i]);
  for (int i = 0; i < (int)_access_jpls.length (); i++)
    order (_access_jpls[i]);
  for (int i = 0; i < (int)_cons_jpls.length (); i++)
    order (_cons_jpls[i]);
  for (int i = 0; i < (int)_dest_jpls.length (); i++)
    order (_dest_jpls[i]);
}  

// issue an advice in the plan for this code join point
void Plan::issue (ACM_Code &jpl, ACM_CodeAdvice *ca) {
  AdviceInfo *ai = TI_CodeAdvice::of(*ca)->get_advice_info();

  // find latest around advice level
  ACM_CodePlan *last_plan = jpl.get_plan();
  while (last_plan->has_next_level())
    last_plan = last_plan->get_next_level();

  if (last_plan->has_around()) {
    ACM_CodePlan *new_plan = _jpm.newCodePlan();
    last_plan->set_next_level(new_plan);
    last_plan = new_plan;
  }

  switch (ai->type()) {
  case ACT_BEFORE:
    last_plan->get_before().insert(ca);
    break;
  case ACT_AROUND:
    last_plan->set_around(ca);
    break;
  case ACT_AFTER:
    last_plan->get_after().push_front(ca);
    break;
  }
}


// check the plan for a specific join point
void Plan::check (ACM_Code &jpl, OrderPlanner<ACM_Aspect, LessAspectPtr> &order_planner) {
  if (!jpl.has_plan ())
    return;

  if (!order_planner.plan()) {
    _err << sev_error << "order directives not resolvable for "
         << jpl.type_str () << "(\"" << signature(jpl).c_str ()
         << "\")" << TransformInfo::location (jpl) << endMessage;
    return;
  }

  AdviceMap::iterator iter = _advice_map.find(&jpl);
  if (iter == _advice_map.end ())
    return; // maybe only cflow triggers
  vector<ACM_CodeAdvice*> &advices = *iter->second;

  for (int i = 0; i < order_planner.items (); i++) {
    ACM_Aspect* ai = &order_planner.item (i);
    for (unsigned int j = 0; j < advices.size(); j++) {
      ACM_CodeAdvice *code_advice = advices[j];
      if (code_advice &&
          &TI_CodeAdvice::of (*code_advice)->get_advice_info()->aspect() == ai) {
        issue(jpl, code_advice);
        advices[j] = 0;
      }
    }
  }
  for (unsigned int j = 0; j < advices.size(); j++)
    if (advices[j] != 0)
      issue(jpl, advices[j]);
}

// issue an introduction in the plan for this class join point
void Plan::issue (ACM_Class &jpl, ACM_Introduction *ii) {
  TU_ClassSlice *slice = (TU_ClassSlice*)get_slice(*ii);
  bool base, members;
  slice->analyze_tokens(base, members);
  if (base) {
    ACM_BaseIntro *intro_ref = _jpm.newBaseIntro();
    intro_ref->set_intro(ii);
    jpl.get_plan()->get_base_intros().insert (intro_ref);
  }
  if (members) {
    ACM_MemberIntro *intro_ref = _jpm.newMemberIntro();
    intro_ref->set_intro(ii);
    jpl.get_plan()->get_member_intros().insert (intro_ref);
  }
}

// check the plan for a specific join point
// TODO: a lot of code duplication with "check(ACM_Code&)"
void Plan::check (ACM_Class &jpl, OrderPlanner<ACM_Aspect, LessAspectPtr> &order_planner) {
  if (!jpl.has_plan ())
    return;

  if (!order_planner.plan ()) {
    _err << sev_error << "order directives not resolvable for "
         << jpl.type_str () << "(\"" << signature(jpl).c_str ()
         << "\")" << TransformInfo::location (jpl) << endMessage;
    return;
  }

  IntroMap::iterator iter = _intro_map.find(&jpl);
  assert (iter != _intro_map.end ());
  vector<ACM_Introduction*> &intros = *iter->second;

  for (int i = 0; i < order_planner.items (); i++) {
    ACM_Aspect* ai = &order_planner.item (i);
    for (unsigned int j = 0; j < intros.size(); j++) {
      if (intros[j] && ((ACM_Aspect*)intros[j]->get_parent() == ai)) {
        issue(jpl, intros[j]);
        intros[j] = 0;
      }
    }
  }
  for (unsigned int j = 0; j < intros.size(); j++)
    if (intros[j] != 0)
      issue(jpl, intros[j]);
}

