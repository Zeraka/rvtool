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

/* $Id: JoinPoint.cc,v 1.8 2006/01/09 17:05:34 reinhard tartler Exp $ */

#include "JoinPoint.h"

Puma::ListElement* JoinPoint::duplicate() {
  return new JoinPoint(_location, _cond);
}

ostream& operator<< (ostream& o, JoinPoint& p) {
//  o << p.type() << ' ' << p.signature () << ' ';
//  o << hex << (unsigned long)p._location << ' ';  <-- problematic on 64-bit platforms with sizeof(long)!=sizeof(void*)
  o << (void *)p._location << ' '; //
  o << p._cond;
  o << endl;
  return o;
}
