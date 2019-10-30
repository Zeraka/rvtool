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

#include "Binding.h"

void Binding::clear () {
  _used = false;
  _this = 0;
  _target = 0;
  _args.reset ();
  _result = 0;
}

bool Binding::operator == (const Binding &other) const {
  if (!_used && !other._used)
    return true;
  if (_this != other._this || _target != other._target ||
      _args.length () != other._args.length () || _result != other._result)
    return false;
  for (int i = 0; i < _args.length (); i++) {
    if (_args.lookup (i) != other._args.lookup (i))
      return false;
  }
  return true;
}

int Binding::bound_to (ACM_Arg *arg) const {
  if (_used) {
    if (arg == _this)
      return BIND_THAT;
    else if (arg == _target)
      return BIND_TARGET;
    else if (arg == _result)
      return BIND_RESULT;
    else {
      for (int a = 0; a < _args.length (); a++)
        if (arg == _args.lookup (a))
          return a;
    }
  }
  return BIND_NOT_FOUND;
}
