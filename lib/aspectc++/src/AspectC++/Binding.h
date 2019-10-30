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

#ifndef __Binding_h__
#define __Binding_h__

#include "Puma/Array.h"

class ACM_Arg;

class Binding {
public:
  enum { BIND_THAT = -1, BIND_TARGET = -2, BIND_RESULT = -3,
         BIND_NOT_FOUND = -4 };
  bool _used;
  ACM_Arg* _this;
  ACM_Arg* _target;
  Puma::Array<ACM_Arg*> _args;
  ACM_Arg *_result;

  Binding () { clear (); }
  void clear ();
  bool operator == (const Binding &other) const;
  bool operator != (const Binding &other) const {
    return !(*this == other);
  }
  int bound_to (ACM_Arg*) const;
};

#endif // __Binding_h__
