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

#include "CFlow.h"
#include "AdviceInfo.h"
#include "Naming.h"

void CFlowList::gen_trigger_obj (ostream &out) const {
  int i = 0;
  for (const_iterator iter = begin (); iter != end (); ++iter) {
    out << "      ";
    Naming::cflow (out, (*iter).advice_info ()->aspect (), (*iter).index ());
    out << " trigger" << i << ";" << endl;
    i++;
  }
}

const string &CFlow::ac_namespace_contributions () {
  static const string cflow_template =
    "  template <class Aspect, int Index>\n"
    "  struct CFlow {\n"
    "    static int &instance () {\n"
    "      static int counter = 0;\n"
    "      return counter;\n"
    "    }\n"
    "    CFlow () { instance ()++; }\n"
    "    ~CFlow () { instance ()--; }\n"
    "    static bool active () { return instance () > 0; }\n"
    "  };\n";
  return cflow_template;
}


