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

#ifndef PUMA_ConstSizeMemPool_H
#define PUMA_ConstSizeMemPool_H

/** \file
 * Constant size memory block pool implementation. */

#include "Puma/Chain.h"
#include <assert.h>
#include <stdlib.h>

#ifdef PROFILE_MEM
# include <iostream>
#endif

namespace Puma {

/** \class ConstSizeMemPool ConstSizeMemPool.h Puma/ConstSizeMemPool.h
 * Constant size memory block pool. Memory blocks are not freed
 * before the pool is destroyed. Free blocks are inserted into
 * a free list for reuse.
 *
 * Following global functions are defined that use a constant size
 * memory pool:
 * - Puma::malloc
 * - Puma::free
 * - Puma::clean
 *
 * \tparam N Number of bytes for each memory block to allocate. 
 * \ingroup basics */
template<int N>
class ConstSizeMemPool {
  Chain m_FreeList;

#ifdef PROFILE_MEM
  long long m_use, m_reuse;
#endif

public:
  /** Construct an empty memory pool. */
  ConstSizeMemPool() {
#ifdef PROFILE_MEM
    m_use = 0;
    m_reuse = 0;
#endif
  }
  /** Destroy the memory pool. Frees all memory blocks allocated. */
  ~ConstSizeMemPool() {
#ifdef PROFILE_MEM
    std::cout << m_reuse << "(" << (100 * m_reuse / (m_use > 1 ? m_use : 1)) << "%)";
    std::cout << " of total " << m_use << " for size " << N <<" reused" << std::endl;
#endif
    clean();
  }
  /** Get the memory pool for this block size.
   * There is only one memory pool for each block size.
   * \return A reference to the memory pool. */
  static ConstSizeMemPool &inst() {
    static ConstSizeMemPool instance;
    return instance;
  }
  /** Get a block of memory. If no free block is available,
   * a new block of memory is allocated.
   * \return Pointer to the block of memory. */
  void *alloc();
  /** Insert a block of memory into the free list for reuse.
   * The memory block is freed when the pool is destroyed.
   * \param block Pointer to the memory block. */
  void free(void *block);
  /** Free all allocated memory blocks. */
  void clean();
};

template<int N>
inline void ConstSizeMemPool<N>::clean() {
  Chain *c = m_FreeList.unlink();
  while (c) {
    ::free(c);
    c = m_FreeList.unlink();
  }
}

template<int N>
inline void *ConstSizeMemPool<N>::alloc() {
#ifdef PROFILE_MEM
  m_use++;
#endif
  void *res = m_FreeList.unlink();
  if (res == 0) {
    res = ::malloc(N);
  }
#ifdef PROFILE_MEM
  else m_reuse++;
#endif
  return res;
}

template<int N>
inline void ConstSizeMemPool<N>::free(void *p) {
  m_FreeList.insert((Chain*) p);
}

/** Constant size memory block pool variant of malloc().
 * \tparam N The block size.
 * \return A pointer to the memory block. */
template<int N>
inline void *malloc() {
  return ConstSizeMemPool<N>::inst().alloc();
}

/** Constant size memory block pool variant of free().
 * \tparam N The block size.
 * \param block Pointer to the memory block to free. */
template<int N>
inline void free(void *block) {
  ConstSizeMemPool<N>::inst().free(block);
}

/** Short-cut function for calling method clean() on the corresponding
 * fixed size memory block pool instance.
 * \tparam N The block size. */
template<int N>
inline void clean() {
  ConstSizeMemPool<N>::inst().clean();
}

} // namespace Puma

#endif /* PUMA_ConstSizeMemPool_H */
