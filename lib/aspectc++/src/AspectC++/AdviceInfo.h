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

#ifndef __AdviceInfo_h__
#define __AdviceInfo_h__

#include <iostream>
using namespace std;

#include "PointCut.h"
#include "Binding.h"
#include "BackEndProblems.h"
#include "PointCutExpr.h"
#include "ACModel/Elements.h"

class AspectInfo;
class ThisJoinPoint;

class AdviceInfo {
 
  AspectInfo &_aspect_info;
  ACM_Aspect &_aspect;
  PointCut _pointcut;
  Binding _binding;
  PointcutExpression _pointcut_expr;
  ACM_AdviceCode &_code;

public:
  AdviceInfo (AspectInfo &ai, ACM_AdviceCode &c);

  ACM_Aspect &aspect () const { return _aspect; }
  ACM_AdviceCode &code () const { return _code; }
  AspectInfo &aspect_info () const { return _aspect_info; }
  PointCut &pointcut () { return _pointcut; }
  Binding &binding () { return _binding; }
  PointcutExpression &pointcut_expr () { return _pointcut_expr; }

  void gen_invocation_func (ostream &out, bool def, const BackEndProblems &bep);
  void gen_invocation_func_call (ostream &stmt, const char* tjp_tp,
                                 const char *tjp_obj);

  void gen_binding_template (ostream &out, const char *jpname,
                             const BackEndProblems &bep);

  std::string scope_name() { return signature(*(ACM_Aspect*)_code.get_parent()); }
  std::string name ();
  AdviceCodeType type() { return _code.get_kind(); }

  // add the TJP-flags of this advice to the given tjp object
  void addTJPFlags(ThisJoinPoint &tjp);
};

#endif // __AdviceInfo_h__
