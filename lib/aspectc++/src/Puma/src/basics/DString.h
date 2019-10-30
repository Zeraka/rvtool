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

#ifndef PUMA_DString_H
#define PUMA_DString_H

/** \file
 *  Global hash table based string container. */

#include "StrHashTable.h"
#include <assert.h>

namespace Puma {

/** \class DString DString.h Puma/DString.h
 * Global dictionary based string container. 
 * \ingroup basics */
class DString {
  static StrHashTable global_dictionary;
  StrHashKey *m_Key;

public:
  /** Construct an empty string container. Refers to the empty string. */
  DString() {
    m_Key = global_dictionary.empty();
    assert(m_Key);
  }
  /** Construct a new string container for the given string.
   *  If the string is not null, then it is inserted into the dictionary.
   *  \param str The string. */
  DString(const char *str) {
    *this = str;
  }
  /** Copy a string container.
   *  \param copy The string container to copy. */
  DString(const DString &copy)
      : m_Key(copy.m_Key) {
    assert(copy.m_Key);
  }
  /** Destroy the container. Does not delete the string. */
  ~DString() {
    m_Key = 0;
  }
  /** Copy a string container.
   *  \param copy The string container to copy. */
  DString &operator=(const DString &copy) {
    assert(copy.m_Key);
    m_Key = copy.m_Key;
    return *this;
  }
  /** Assign a string to this string container.
   *  \param str The string. */
  DString &operator=(const char *str) {
    if (str == 0) {
      m_Key = global_dictionary.empty();
    } else {
      m_Key = global_dictionary.insert(str);
    }
    assert(m_Key);
    return *this;
  }
  /** Check if the given string equals this string.
   *  \param str The string to compare with. */
  bool operator==(const DString &str) const {
    assert(m_Key && str.m_Key);
    return m_Key == str.m_Key;
  }
  /** Check if the given string not equals this string.
   *  \param str The string to compare with. */
  bool operator!=(const DString &str) const {
    assert(m_Key && str.m_Key);
    return m_Key != str.m_Key;
  }
  /** Get the contained string.
   *  \return A pointer to the contained string. */
  const char *c_str() const {
    assert(m_Key);
    return m_Key->c_str();
  }
  /** Get the length of the string.
   *  \return The length of the contained string. */
  unsigned int length() const {
    assert(m_Key);
    return m_Key->length();
  }
  /** Check if the string is empty. The string is empty if its length is zero.
   *  \return True if the string is empty. */
  bool empty() const {
    assert(m_Key);
    return m_Key->length() == 0;
  }
  /** Get the magic number (hash) of the string.
   *  \return The magic number. */
  unsigned int magic() const {
    assert(m_Key);
    return m_Key->getMagic();
  }
  /** Conversion operator returning the contained string.
   *  \return A pointer to the contained string. */
  operator const char *() const {
    assert(m_Key);
    return c_str();
  }
  /** Clear the global dictionary. */
  static void clearDict() {
    global_dictionary.clear();
  }
};

} // namespace Puma

#endif /* PUMA_DString_H */
