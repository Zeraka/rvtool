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

/* $Id: JoinPoint.h,v 1.10 2004/04/19 10:13:57 matthias.urban Exp $ */

#ifndef __JoinPoint_h__
#define __JoinPoint_h__

#include <iostream>
using namespace std;

#include "Puma/ListElement.h"
#include "ACModel/Elements.h"
#include "Condition.h"

class JoinPoint : public Puma::ListElement {
  ACM_Any* _location;
  Condition _cond;
public:

  JoinPoint (ACM_Any *location, Condition &cond) : _location (location) {
    _cond.assign (cond);
  }
  ACM_Any* location() const { return _location; }
  
//  join_point_type type() const { return location()->type (); }
  const Condition &condition () const { return _cond; }
  
  virtual Puma::ListElement* duplicate();
  
  friend ostream& operator<< (ostream&, JoinPoint&);
};

#endif
