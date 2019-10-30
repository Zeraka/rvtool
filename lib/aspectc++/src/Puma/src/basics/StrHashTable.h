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

#ifndef PUMA_StrHashTable_H
#define PUMA_StrHashTable_H

/** \file
 * String hash table implementation. */

#include "Puma/StrHashKey.h"
#include <assert.h>

namespace Puma {

/** \class StrHashTable StrHashTable.h Puma/StrHashTable.h
 * String hash table backed by a fixed size array of
 * single-linked chains.
 * \ingroup basics */
class StrHashTable {
  enum {
    /** Size of hash table array. */
    STRHASHTABLE_SIZE = 0xffff
  };

  StrHashKey *m_emptyKey;
  Chain m_table[STRHASHTABLE_SIZE];

  StrHashKey *find(unsigned long magic, const char *str, unsigned long len);
  unsigned long hash(const char *str, unsigned long &len);

public:
  /** Construct a new empty hash table. */
  StrHashTable() {
    m_emptyKey = insert("");
    assert(m_emptyKey);
  }
  /** Get the empty hash table item used for comparison purposes.
   * \return The empty hash table item. */
  StrHashKey *empty() const {
    assert(m_emptyKey);
    return m_emptyKey;
  }
  /** Add a string to the hash table. Does not add strings
   * twice. Added strings are copied.
   * \param str The string to add.
   * \return The hash table item for the string. */
  StrHashKey *insert(const char *str);
  /** Clear the hash table and free all copied strings. */
  void clear();
};

} // namespace Puma

#endif /* PUMA_StrHashTable_H */
