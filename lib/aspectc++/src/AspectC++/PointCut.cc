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

/* $Id: PointCut.cc,v 1.6 2004/01/15 13:20:04 olaf.spinczyk Exp $ */

#include "PointCut.h"

using namespace Puma;

PointCut::PointCut() {
  _type = PCT_UNKNOWN;
//  _contents = (pccontents)0;
}

PointCut::~PointCut () {
  for (list<PointCut*>::iterator iter = _cflow_triggers.begin ();
       iter != _cflow_triggers.end (); ++iter)
    delete *iter;
}

void PointCut::cflow_triggers(PointCut &pc) {
  PointCut *triggers = new PointCut();
  *triggers = pc;
  _cflow_triggers.push_back (triggers);
}

const list<PointCut*> &PointCut::cflow_triggers() {
  return _cflow_triggers;
}

PointCut& PointCut::operator = (const PointCut& pc) {
  clear ();
  _type = pc.type ();
//  _contents = pc.contents ();
  return (PointCut&)(*this += pc);
}

PointCut::iterator PointCut::find (ACM_Any *jpl) {
  iterator iter = begin ();
  while (iter != end () && (*iter).location () != jpl)
    ++iter;
  return iter;
}

ostream& operator<< (ostream& o, PointCut& p) 
{
    for (const ListElement* e = p.first(); e; e = p.next(e)) {
	JoinPoint& jp = ((JoinPoint&)*e);
	o << jp;
    }
    return o;
}
