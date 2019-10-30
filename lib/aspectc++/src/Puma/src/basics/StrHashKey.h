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

#ifndef PUMA_StrHashKey_H
#define PUMA_StrHashKey_H

/** \file
 * String hash table key implementation. */

#include "Puma/StrCol.h"
#include "Puma/Chain.h"

namespace Puma {

/** \class StrHashKey StrHashKey.h Puma/StrHashKey.h
 * String hash table key. 
 * \ingroup basics */
class StrHashKey : public Chain {
  unsigned int m_Magic;
  unsigned int m_Len;
  const char *m_Str;

public:
  /** Construct a new string hash table key.
   * The input string is copied.
   * \param magic The hash value.
   * \param str The string.
   * \param len The length of the string. */
  StrHashKey(int magic, const char *str, int len)
      : m_Magic(magic), m_Len(len), m_Str(StrCol::dup(str, len)) {
  }
  /** Destroy the hash table key. Frees the string copy. */
  ~StrHashKey() {
    if (m_Str)
      delete[] m_Str;
  }
  /** Get the hash value of the string.
   * \return The hash value. */
  unsigned int getMagic() const {
    return m_Magic;
  }
  /** Get the length of the string.
   * \return The string's length. */
  unsigned int length() const {
    return m_Len;
  }
  /** Get the string.
   * \return The string. */
  const char *c_str() const {
    return m_Str;
  }
};

} // namespace Puma

#endif /* PUMA_StrHashKey_H */
