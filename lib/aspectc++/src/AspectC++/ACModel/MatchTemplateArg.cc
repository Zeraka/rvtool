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

#include "MatchTemplateArg.h"

bool MatchTemplateArgList::matches (MatchTemplateArgList &sig_mtal) const {
  const_iterator iter = begin ();
  const_iterator sig_iter = sig_mtal.begin ();
  while (iter != end ()) {
    // check if the current template match arg is '...'
    if ((*iter)->is_ellipses ())
      return true;
    // check if we have more match arguments than template arguments
    if (sig_iter == sig_mtal.end ())
      return false;
    // check if the current match argument matches the template argument
    else if (!(*iter)->matches (*(*sig_iter)))
      return false;
    // go to the next argument
    ++iter;
    ++sig_iter;
  }
  // check if we have examined all deduced template arguments
  if (sig_iter != sig_mtal.end ())
    return false;
    
  // everything fine, the list matches!
  return true;
}

bool MTA_Type::matches (MatchTemplateArg &arg) const {
  return arg.is_type () && _match_type.matches (((MTA_Type&)arg)._match_type);
}

bool MTA_Value::matches (MatchTemplateArg &arg) const {
  return arg.is_value () && _val == ((MTA_Value&)arg)._val;
}

bool MTA_Addr::matches (MatchTemplateArg &arg) const {
  // TODO: const correctness -> don't cast _name here!!!
  return arg.is_addr() && ((MatchName&)_name).matches (((MTA_Addr&)arg)._name);
}
