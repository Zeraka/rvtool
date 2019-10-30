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

#ifndef PUMA_MemPool_H
#define PUMA_MemPool_H

/** \file
 * Memory pool implementation. */

#include "Puma/Heap.h"

namespace Puma {

/** \class MemPool MemPool.h Puma/MemPool.h
 * Memory pool implementation based on a linked list of memory blocks.
 * Allocates large blocks of memory to reduce the overhead of memory
 * allocation when a lot of small objects need to be allocated. Does not
 * free any allocated memory until the pool is destroyed.
 * \warning This class is not used and may disappear without notice. 
 * \ingroup basics */
class MemPool : protected Heap {
  static const unsigned long alignment = sizeof(long); // object alignment

  struct Link {
    struct Link *next;
  };

  Link *m_block;
  unsigned long m_size;
  unsigned long m_used;
  unsigned long m_defsize;

  void grow(unsigned long blocksize);
  unsigned long align(unsigned long size);

public:
  /** Construct a new memory pool.
   * \param blocksize Size of a memory block. Defaults to 128 KB.*/
  MemPool(unsigned long blocksize = 128L * 1024L);
  /** Destroy the pool. Frees the allocated memory. */
  ~MemPool();
  /** Allocate memory for a new object of a given size.
   * Usually no new memory needs to be allocated, but a chunk of
   * previously allocated memory is returned.
   * \param objectsize The size of the object in bytes.
   * \return The allocated memory. */
  void *alloc(unsigned long objectsize);
  /** De-allocate memory for a new object. Since no memory is
   * freed until the pool is destroyed, this method does nothing.
   * \param pointer Pointer to the chunk of memory to de-allocate. */
  void dealloc(void *pointer);
  /** Free all the allocated memory in this pool. */
  void free();
};

} // namespace Puma

#endif /* PUMA_MemPool_H */
