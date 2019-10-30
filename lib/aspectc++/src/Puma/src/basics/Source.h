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

#ifndef PUMA_Source_H
#define PUMA_Source_H

/** \file
 * Input source interface. */

namespace Puma {

/** \class Source Source.h Puma/Source.h
 * Interface for all input sources. 
 * \ingroup basics */
class Source {
public:
  /** Constructs an empty source object. */
  Source() {
  }
  /** Destroys the source object. */
  virtual ~Source() {
  }
  /** Read n bytes from the source into the given string buffer.
   * \param buffer The string used as buffer for the bytes read.
   * \param n The number of bytes to read.
   * \return The number of bytes read, or -1 in case of errors. */
  virtual int read(char *buffer, int n) = 0;
  /** Get the size of the source. To be implemented if a specific
   * source supports asking for its size.
   * \return This default implementation always returns -1 to indicate that the size is unknown. */
  virtual int size() {
    return -1;
  }
};

} // namespace Puma

#endif /* PUMA_Source_H */
