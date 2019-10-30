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

#ifndef PUMA_Sink_H
#define PUMA_Sink_H

/** \file
 * Output sink interface. */

namespace Puma {

/** \class Sink Sink.h Puma/Sink.h
 * Output sink interface.
 * \warning This class is not used and may disappear without notice.
 * \ingroup basics */
class Sink {
public:
  /** Destructor. */
  virtual ~Sink() {}
  /** Write n bytes from the given buffer into the sink.
   * \param buffer The bytes to write.
   * \param n The number of bytes to write.
   * \return The number of bytes written. */
  virtual int write(const char *buffer, int n) = 0;
  /** Check if the output sink is full.
   * \return This default implementation always returns false. */
  virtual bool full() {
    return false;
  }
};

} // namespace Puma

#endif /* PUMA_Sink_H */
