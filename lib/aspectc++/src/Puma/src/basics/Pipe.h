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

#ifndef PUMA_Pipe_H
#define PUMA_Pipe_H

/** \file
 * Input/output pipe implementation. */

#include "Puma/Source.h"
#include "Puma/Sink.h"

namespace Puma {

/** \class Pipe Pipe.h Puma/Pipe.h
 * Pipe between an input source and an output sink.
 * \warning This class is not used and may disappear without notice. 
 * \ingroup basics */
class Pipe : public Source, public Sink {
  char *m_buffer;
  char *m_rpos;
  char *m_wpos;
  int m_size;
  bool m_empty;

public:
  /** Constructor.
   * \param buffer_size Source buffer size. */
  Pipe(int buffer_size);
  /** Destructor. */
  ~Pipe();
  /** Read n bytes from the internal buffer into the given string buffer.
   * \param buffer The string used as buffer for the bytes read.
   * \param n The number of bytes to read.
   * \return The number of bytes read, or -1 in case of errors. */
  int read(char *buffer, int n);
  /** Write n bytes from the given buffer into the internal source buffer.
   * \param buffer The bytes to write.
   * \param n The number of bytes to write.
   * \return The number of bytes written. */
  int write(const char *buffer, int n);
  /** Get the number of bytes left in the internal buffer.
   * \return The free space in the internal buffer. */
  int free();
  /** Check if the internal buffer is full.
   * \return True if no space left in the internal buffer. */
  bool full() {
    return free() == 0;
  }
};

} // namespace Puma

#endif /* PUMA_Pipe_H */
