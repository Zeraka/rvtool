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

#include "Puma/PreAnswer.h"
#include <string.h>

namespace Puma {


PreAnswer::PreAnswer (const char *answer) {
  if (answer) {
    _text = new char[strlen (answer) + 1];
    strcpy (_text, answer);
  } else
    _text = (char*) 0;
}


PreAnswer::~PreAnswer () { 
  if (_text) 
    delete[] _text; 
}


} // namespace Puma
