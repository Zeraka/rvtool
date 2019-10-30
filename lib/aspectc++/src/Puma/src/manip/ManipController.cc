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

#include "Puma/ManipController.h"
#include "Puma/Token.h"
#include "Puma/Unit.h"

namespace Puma {


// Reset the controller for the next run.
void ManipController::reset () {
  _excluded.reset ();
  _in_progress.reset ();
}

// All manipulated units are excluded to be manipulated again.
void ManipController::exclude () {
  for (long i = 0; i < _in_progress.length (); i++)
    _excluded.append (_in_progress[i]);
  _in_progress.reset ();
}

// Is the unit, the token belongs to, permitted to be manipulated?
bool ManipController::permitted (Token *token) {
  if (! token) return true;
  Unit *unit = (Unit*)token->belonging_to ();
  if (! unit) return true;
    
  for (long i = 0; i < _excluded.length (); i++)
    if (unit == _excluded[i])
      return false;

  // Do not add a unit twice.
  for (long i = 0; i < _in_progress.length (); i++)
    if (unit == _in_progress[i])
      return true;

  _in_progress.append (unit);
  return true;
}


} // namespace Puma
