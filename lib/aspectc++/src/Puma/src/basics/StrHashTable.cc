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

#include "Puma/StrHashTable.h"
#include <string.h>
#include <iostream>

namespace Puma {

unsigned long StrHashTable::hash(const char *str, unsigned long& len) {
  unsigned long h = 0;
  const char* p = str;
  while (*p) {
    h = *p + (h << 6) + (h << 16) - h;
    p++;
  }
  len = p - str;
  return h % STRHASHTABLE_SIZE;
}

StrHashKey *StrHashTable::find(unsigned long magic, const char *str, unsigned long len) {
  StrHashKey *key = (StrHashKey*) m_table[magic].select();

  while (key) {
    if (key->length() == len && memcmp(key->c_str(), str, len) == 0) {
      return key;
    }
    key = (StrHashKey*) key->select();
  }
  return 0;
}

void StrHashTable::clear() {
  // delete all dynamically created hash keys
  for (long i = 0; i < STRHASHTABLE_SIZE; i++) {
    StrHashKey *key = (StrHashKey*) m_table[i].select();
    while (key) {
      StrHashKey *curr = key;
      key = (StrHashKey*) key->select();
      // don't need to unlink since the whole chain 
      // is deleted and unlinked below
      delete curr;
    }
    // unlink the whole chain
    m_table[i].select(0);
  }
  // restore valid state
  m_emptyKey = insert("");
  assert(m_emptyKey);
}

StrHashKey *StrHashTable::insert(const char *str) {
  unsigned long len;
  unsigned long magic = hash(str, len);
  StrHashKey *key = find(magic, str, len);
  if (key == 0) {
    key = new StrHashKey(magic, str, len);
    m_table[magic].insert(key);
  }
  return key;
}

} // namespace Puma
