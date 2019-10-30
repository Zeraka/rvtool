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

#ifndef PUMA_StringSource_H
#define PUMA_StringSource_H

/** \file
 * String input source implementation. */

#include "Puma/Source.h"
#include <string.h>

namespace Puma {

/** \class StringSource StringSource.h Puma/StringSource.h
 * String input source implementation implementing the Source interface. 
 * \ingroup basics */
class StringSource : public Source {
  const char *m_string;
  int m_delivered;
  int m_len;

public:
  /** Construct an empty string input source. */
  StringSource()
      : m_string(0), m_delivered(0), m_len(-1) {
  }
  /** Construct an input source for the given string.
   * \param string The string from which to read. */
  StringSource(const char *string)
      : m_string(string) {
    m_delivered = 0;
    m_len = strlen(string);
  }
  /** Read n bytes from the input string into the given string.
   * \param dest The string used as buffer for the bytes read.
   * \param n The number of bytes to read.
   * \return The number of bytes read, or -1 in case of errors. */
  virtual int read(char *dest, int n) {
    int to_copy;
    if (!m_string || m_delivered == m_len)
      return 0;
    to_copy = (n < m_len - m_delivered) ? n : m_len - m_delivered;
    memcpy(dest, m_string + m_delivered, to_copy);
    m_delivered += to_copy;
    return to_copy;
  }
  /** Set the input string to read from.
   * \param string The input string. */
  void string(const char *string) {
    m_string = string;
    m_delivered = 0;
    m_len = strlen(m_string);
  }
  /** Get the input string.
   * \return The input string, or null if no input string set. */
  const char *string() {
    return m_string;
  }
  /** Get the size of the input string.
   * \return The size of the input string, or -1 if no input string set. */
  int size() {
    return m_len;
  }
};

} // namespace Puma

#endif /* PUMA_StringSource_H */
