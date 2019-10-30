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

#ifndef __AspectInfo_h__
#define __AspectInfo_h__

#include <list>
using std::list;
#include <set>
using std::set;
#include <string>
using std::string;

#include "ACModel/Utils.h"
#include "ThisJoinPoint.h"

struct BackEndProblems;
class AdviceInfo;

class AspectInfo {
  ACM_Aspect &_aspect;
  ThisJoinPoint _aspectof_tjp;
  list<AdviceInfo*> _advice_infos;
    
 public:

  AspectInfo (ACM_Aspect &a) : _aspect (a) {}
  bool operator < (const AspectInfo &right) const {
    return name() < right.name();
  }
  ACM_Aspect &loc () const { return _aspect; }
  string name () const { return signature (_aspect); }
  const ThisJoinPoint &aspectof_this_join_point () const { return _aspectof_tjp; }
  ThisJoinPoint &aspectof_this_join_point () { return _aspectof_tjp; }

  // the list of associated code advice nodes
  list<AdviceInfo*> &advice_infos () { return _advice_infos; }

  // generate the invocation function forward declarations
  string ifct_decls (const BackEndProblems &) const;

  // generate the invocation function definitions
  string ifct_defs (const BackEndProblems &) const;
};

#endif // __AspectInfo_h__
