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

#ifndef __PumaConfigFile_h__
#define __PumaConfigFile_h__

// Puma includes
#include "Puma/RegComp.h"
using namespace Puma;

// stdc++ includes
#include <string>
#include <map>

/* this class encapsulates a container for regular expressions */

namespace regex
{

  class RegExCont
  {
  private:
    // a map for options and its values
    std::map<std::string, RegComp*> _re_map;

    // type of option-value pair 
    typedef std::pair<std::string, RegComp*> regex_pair;
  public:

    // lookup for a regular expression
    RegComp*
    lookup(std::string expr);

    // add a new regular expression to the container
    void
    add(std::string expr, RegComp* regcomp);

    ~RegExCont();

  };

} // namespace regex

#endif //RegCompCont
