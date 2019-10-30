// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2004  The 'ac++' developers (see aspectc.org)
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

#include "regex.h"

// Puma includes
#include "Puma/RegComp.h"
using namespace Puma;

// stdc++ includes
#include <fstream>
#include <iostream>
using namespace std;

// static instance of RegExCont(ainer)
static regex::RegExCont regex_cont;

bool
regex::regExMatch(const string& expr, const char* str)
{
  bool ret = false;
  RegComp* regcomp = 0;

  // lookup for already compiled regular expressions
  regcomp = regex_cont.lookup(expr);
  if (regcomp == NULL)
  {
    regcomp = new RegComp(expr.c_str());
    regex_cont.add(expr, regcomp);
  }

  if (regcomp)
  {
    if (regcomp->match(str))
    {
      //cout << "match for " <<" '"<< expr <<"' in '" << str << "'" <<endl;
      ret = true;
    }
    //}else{
    //	//cout << "NO match for "<<" '"<< expr <<"' "<<endl;
    //	regcomp->comperror(cerr);
    //}

  }
  else
  {
    cerr << "no reg ex" << endl;

  }

  return ret;
}
