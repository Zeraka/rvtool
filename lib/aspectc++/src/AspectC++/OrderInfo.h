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

#ifndef __OrderInfo_h__
#define __OrderInfo_h__

#include "ACErrorStream.h"
#include "PointCutContext.h"
#include "PointCutExpr.h"
#include "TransformInfo.h"
#include "ACModel/Elements.h"

class OrderInfo {

  ACM_Aspect &_aspect;
  ACM_Order &_order;
  list<PointcutExpression> _pces;
  PointcutExpression _jp_pce;

  PointCutExpr *add_pce (PointcutExpression &pce) {
    _pces.push_back (pce);
    return (PointCutExpr*)_pces.back().get();
  }

 public:

   OrderInfo (ACM_Aspect &a, ACM_Order &o) : _aspect (a), _order (o) {}

   ACM_Aspect &aspect () const { return _aspect; }
   ACM_Order &order () const { return _order; }
  const list<PointcutExpression> &pces () const { return _pces; }
  PointcutExpression &jp_pce () { return _jp_pce; }
  void analyze_exprs (ACErrorStream &err, PointCutContext &context, int warn_compat) {
    _jp_pce = _order.get_pointcut()->get_parsed_expr();
    ((PointCutExpr*)_jp_pce.get())->semantics(err, context, warn_compat);
    typedef ACM_Container<ACM_Pointcut, true> Container;
    const Container &exprs = _order.get_aspect_exprs();
    for (Container::const_iterator i = exprs.begin (); i != exprs.end (); ++i) {
      PointCutExpr *pce = add_pce ((*i)->get_parsed_expr ());
      pce->semantics(err, context, warn_compat);
      if (!(pce->possible_types() & JPT_Class)) {
        err  << Puma::sev_warning
            << TI_Pointcut::of(*_order.get_pointcut ())->get_location()
            << "pointcut expression in order advice can't match aspect"
            << Puma::endMessage;
      }

    }
  }
};

#endif // __OrderInfo_h__
