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

#include "Puma/MemPool.h"

namespace Puma {

MemPool::MemPool(unsigned long blocksize) {
  m_block = (Link*) 0;
  m_size = 0L;
  m_used = 0L;
  m_defsize = blocksize;
  grow(m_defsize);
}

MemPool::~MemPool() {
  free();
}

unsigned long MemPool::align(unsigned long size) {
  return (size + alignment - 1L) & ~(alignment - 1L);
}

void MemPool::grow(unsigned long blocksize) {
  Link *head = m_block;

  m_used = align((unsigned long) sizeof(Link));
  m_size = blocksize + m_used;
  m_block = (Link*) Heap::malloc(m_size);
  m_block->next = head;
}

void *MemPool::alloc(unsigned long objectsize) {
  void *objectptr;

  objectsize = align(objectsize);

  if (m_used + objectsize > m_size)
    grow(objectsize > m_defsize ? objectsize : m_defsize);

  objectptr = (void*) ((char*) m_block + m_used);
  m_used += objectsize;

  return objectptr;
}

void MemPool::dealloc(void *) {
}

void MemPool::free() {
  Link *curr = m_block;
  Link *next;

  while (curr) {
    next = curr->next;
    Heap::free(curr);
    curr = next;
  }
}

} // namespace Puma
