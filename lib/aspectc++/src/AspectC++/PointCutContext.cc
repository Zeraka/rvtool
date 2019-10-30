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

#include "PointCutContext.h"
#include "JoinPoint.h"
#include "IntroductionUnit.h"
#include "TransformInfo.h"
#include "ACModel/Utils.h"


bool is_derived_from (ACM_Class *base, ACM_Class *derived) {
  typedef ACM_Container<ACM_Class, false> Container;
  for (Container::iterator i = base->get_derived().begin ();
      i != base->get_derived().end (); ++i) {
    if (*i == derived || is_derived_from(*i, derived))
      return true;
  }
  return false;
}

PointCutExpr *PointCutContext::lookup_virtual_pct_func (ACM_Class *scope, ACM_Pointcut *pct_func) {
  // check whether the virtual pointcut is defined in the concrete aspect
  ACM_Name *result = map_lookup(*scope, pct_func->get_name ());
  if (result && result->type_val() == JPT_Pointcut)
    return (PointCutExpr*)((ACM_Pointcut*)result)->get_parsed_expr().get ();

  // get the scope of the virtual pointcut
  ACM_Any *fct_scope = (ACM_Any*)pct_func->get_parent();
  if (!fct_scope || !(fct_scope->type_val() & (JPT_Class|JPT_Aspect)))
    return 0;
  ACM_Class *fct_class = (ACM_Class*)fct_scope;

  // lookup the pointcut in the base classes/aspects
  typedef ACM_Container<ACM_Class, false> Container;
  for (Container::iterator i = scope->get_bases().begin ();
      i != scope->get_bases().end (); ++i) {
    if (*i != fct_class && !is_derived_from (fct_class, *i)) {
      continue;
    }
    PointCutExpr *pce = lookup_virtual_pct_func(*i, pct_func);
    if (pce) {
      return pce;
    }
  }
  return 0;
}
