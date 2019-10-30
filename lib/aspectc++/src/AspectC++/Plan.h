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

#ifndef __Plan_h__
#define __Plan_h__

#include <string>
#include <set>
#include <map>
#include <vector>
using namespace std;

#include "ACModel/Elements.h"
#include "OrderPlanner.h"
#include "Condition.h"
#include "ACErrorStream.h"
#include "Puma/Array.h"
#include "ACConfig.h"

class AdviceInfo;
class OrderInfo;
class IntroductionInfo;
class AspectInfo;
class CFlow;
class ProjectModel;

class Plan {
public:
  typedef set<AspectInfo> AspectContainer;

  ACConfig &_conf;
  ACErrorStream &_err;
  ProjectModel &_jpm;
  Puma::Array<AdviceInfo*> _advice_infos;
  list<IntroductionInfo*>  _introduction_infos;
  list<OrderInfo*>  _order_infos;
  set<AspectInfo> _aspect_infos;
  Puma::Array<ACM_Any*> _access_jpls;
  Puma::Array<ACM_Any*> _exec_jpls;
  Puma::Array<ACM_Any*> _cons_jpls;
  Puma::Array<ACM_Any*> _dest_jpls;
  Puma::Array<ACM_Any*> _class_jpls;
  TypeCheckSet _type_checks_false;
  TypeCheckSet _type_checks_true;

  typedef map<ACM_Class*, vector<ACM_Introduction*>*> IntroMap;
  IntroMap _intro_map; // unsorted intros per class
    
  typedef map<ACM_Code*, vector<ACM_CodeAdvice*>*> AdviceMap;
  AdviceMap _advice_map; // unsorted advice per code joinpoint

  struct LessAspectPtr : public std::binary_function<ACM_Aspect*,ACM_Aspect*,bool> {
    bool operator() (const ACM_Aspect * a1, const ACM_Aspect* a2) const { return *a1 < *a2; }
  };

  set<ACM_Aspect*, LessAspectPtr> _sorted_aspects;

  void issue (ACM_Code &jpl, ACM_CodeAdvice *ca);
  void issue (ACM_Class &jpl, ACM_Introduction *ii);

  // collect and register all introductions, advice code, and order advice
  // triggered by a concrete aspect or any of its base aspects
  void collect_advice (AspectInfo *aspect_info, ACM_Aspect &checked_aspect);

  // check the plan for a specific join point
  void check (ACM_Code &jpl, OrderPlanner<ACM_Aspect, LessAspectPtr> &order_planner);

  // check the plan for a specific join point
  void check (ACM_Class &jpl, OrderPlanner<ACM_Aspect, LessAspectPtr> &order_planner);

public:

  Plan (ACErrorStream &e, ProjectModel &jpm, ACConfig&);
  ~Plan ();

  // manage advice and aspect ressources
  AspectInfo *addAspect (ACM_Aspect &);
  AspectContainer &aspect_infos () { return _aspect_infos; }
  AdviceInfo *addAdvice (AspectInfo &ai, ACM_AdviceCode &code);
  IntroductionInfo *addIntroduction (ACM_Aspect &jpl_aspect, ACM_Introduction &intro);
  OrderInfo *addOrder (ACM_Aspect &a, ACM_Order &o);
  const list<OrderInfo*> &order_infos () const { return _order_infos; }
  const list<IntroductionInfo*> &introduction_infos () const { return _introduction_infos; }
  
  // consider a join point and advice/intro in the plan
  void consider( ACM_Code *jpl, const Condition &cond, AdviceInfo *ai );
  void consider (ACM_Class &jpl, ACM_Introduction *intro);
  void consider (ACM_Code *jpl, const CFlow& cflow);
  void consider( ACM_Code *jpl ); // simple version for including parents of implicit joinpoints

  // calculate the order for a single join points
  void order (ACM_Any *jpl);
  
  // calculate the order for all code join points
  void sort_access_joinpoints();
  void order_code_joinpoints ();
  
  // read the accumulated plans
  int access_jp_plans () const { return _access_jpls.length (); }
  ACM_Access &access_jp_loc (int i) const {
    return *(ACM_Access*)_access_jpls.lookup (i);
  }

  int exec_jp_plans () const { return _exec_jpls.length (); }
  ACM_Execution &exec_jp_loc (int i) const {
    return *(ACM_Execution*)_exec_jpls.lookup (i);
  }

  int cons_jp_plans () const { return _cons_jpls.length (); }
  ACM_Construction &cons_jp_loc (int i) const {
    return *(ACM_Construction*)_cons_jpls.lookup (i);
  }

  int dest_jp_plans () const { return _dest_jpls.length (); }
  ACM_Destruction &dest_jp_loc (int i) const {
    return *(ACM_Destruction*)_dest_jpls.lookup (i);
  }

  int class_jp_plans () const { return _class_jpls.length (); }
  ACM_Class &class_jp_loc (int i) const {
    return *(ACM_Class*)_class_jpls.lookup (i);
  }
  
  const TypeCheckSet &type_checks_false () const {
    return _type_checks_false;
  }

  const TypeCheckSet &type_checks_true () const {
    return _type_checks_true;
  }
};

#endif // __Plan_h__
