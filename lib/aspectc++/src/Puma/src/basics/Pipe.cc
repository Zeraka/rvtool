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

#include "Puma/Pipe.h"

namespace Puma {

Pipe::Pipe(int buffer_size) {
  m_size = buffer_size;
  m_buffer = new char[m_size];
  m_wpos = m_buffer;
  m_rpos = m_buffer;
  m_empty = true;
}

Pipe::~Pipe() {
  delete[] m_buffer;
}

int Pipe::read(char *dest, int n) {
  int p;

  for (p = 0; p < n; p++) {
    if (!m_empty) {
      dest[p] = *m_rpos++;
      if (m_rpos == m_buffer + m_size)
        m_rpos = m_buffer;
      if (m_rpos == m_wpos)
        m_empty = true;
    } else
      break;
  }
  return p;
}

int Pipe::write(const char *dest, int n) {
  int p;

  for (p = 0; p < n; p++) {
    if (m_rpos != m_wpos || m_empty) {
      *m_wpos++ = dest[p];
      if (m_wpos == m_buffer + m_size)
        m_wpos = m_buffer;
      m_empty = false;
    } else
      break;
  }
  return p;
}

int Pipe::free() {
  if (m_wpos > m_rpos)
    return m_size - (m_wpos - m_buffer) + (m_rpos - m_buffer);
  else if (m_wpos == m_rpos)
    return m_empty ? m_size : 0;
  else
    return (m_rpos - m_buffer) - (m_wpos - m_buffer);
}

} // namespace Puma
