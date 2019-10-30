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

#include "RegExCont.h"
using namespace regex;
using namespace std;

void
RegExCont::add(string expr, RegComp* regcomp)
{
  _re_map.insert(regex_pair(expr, regcomp));
}

RegComp*
RegExCont::lookup(string expr)
{
  map<string, RegComp*>::iterator iter = _re_map.find(expr);
  if (iter != _re_map.end())
  {
    return iter->second;
  }
  else
  {
    return NULL;
  }
}

RegExCont::~RegExCont()
{
  //cout << "RegExCont stored: "<< _re_map.size() << " regular expressions" <<endl;
  for (map<string, RegComp*>::iterator iter = _re_map.begin();
      iter != _re_map.end(); iter++)
  {
    if (iter->second)
    {
      delete (iter->second);
    }
  }
  _re_map.clear();
}

