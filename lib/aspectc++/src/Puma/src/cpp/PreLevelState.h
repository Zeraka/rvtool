// This file is part of PUMA.
// Copyright (C) 1999-2003  The PUMA developer team.
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

#ifndef __pre_level_state__
#define __pre_level_state__

// Store the current state of nested conditionals. A nested conditional
// is a conditional group within a conditional group, and such a state 
// consists of informations about the depth of nesting (the level) and 
// the current value of the conditional group at this level. It is used 
// to controll which source code will be ignored.

#include "Puma/Array.h"

namespace Puma {


class PreLevelState {
  long _level;
        
  // The state structure.
  struct _state {
    bool state;                // The level value.
    bool passOn;        // Decide whether code will be not ignored.
  };
        
  // Array to store the state information of every level.
  Array<_state> _levels;        

public:
  PreLevelState () { reset (); }
                
  // Return to level 0, that means no nesting.
  void reset () { _level = 0; _levels[0].state  = true; 
                  _levels[0].passOn = true; }
        
  // Return from a subconditional.
  void higher () { if (--_level < 0) _level = 0; }
        
  // Enter a subconditional.
  void deeper () { _levels[++_level].state = true; 
                   _levels[_level].passOn  = true; }
        
  // Set the state of the current level.
  void setState (bool state)  { _levels[_level].state  = state; }
  void setPassOn (bool state) { _levels[_level].passOn = state; }
        
  // Get the state of the current level.
  bool state ()  { return _levels[_level].state; }
  bool passOn () { return _levels[_level].passOn; }
};


} // namespace Puma

#endif /* __pre_level_state__ */
