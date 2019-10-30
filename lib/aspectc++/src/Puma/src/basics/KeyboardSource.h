// This file is part of PUMA.
// Copyright (C) 1999-2015  The PUMA developer team.
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

#ifndef PUMA_KeyboardSource_H
#define PUMA_KeyboardSource_H

/** \file
 * Keyboard input source implementation. */

#include "Puma/Source.h"

namespace Puma {

/** \class KeyboardSource KeyboardSource.h Puma/KeyboardSource.h
 * Keyboard input wrapper implementing the Source interface. 
 * \ingroup basics */
class KeyboardSource : public Source {
public:
  /** Read one byte from the keyboard into the given string.
   * \param dest The string used as buffer for the bytes read.
   * \param n The number of bytes to read.
   * \return The number of bytes read, or -1 in case of errors. */
  virtual int read(char *dest, int n);
};

} // namespace Puma

#endif /* PUMA_KeyboardSource_H */
