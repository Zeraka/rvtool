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

#ifndef PUMA_Heap_H
#define PUMA_Heap_H

/** \file
 * Heap implementation. */

#include <stdlib.h>

// use pt_malloc only for the cross-compiled mingw version!
#if defined(WIN32) && !defined(_MSC_VER) 

extern "C" void *pt_malloc(size_t);
extern "C" void pt_free(void *);

namespace Puma {

/** \class Heap Heap.h Puma/Heap.h
 * Heap implementation using pt_malloc and pt_free.
 * \warning This class is not used and may disappear without notice. 
 * \ingroup basics */
class Heap {
public:
  /** Constructor. */
  Heap() {
  }
  /** Destructor. */
  ~Heap() {
  }
  /** Allocate n bytes of memory.
   * \param n The number of bytes to allocate.
   * \return A pointer to memory allocated. */
  void *malloc(size_t s) {
    return pt_malloc(s);
  }
  /** Free the given memory.
   * \param p A pointer to the memory to free. */
  void free(void *p) {
    pt_free(p);
  }
};

} // namespace Puma

#else /* defined(WIN32) && !defined(_MSC_VER) */

namespace Puma {

/** \class Heap Heap.h Puma/Heap.h
 * Heap implementation using standard malloc and free.
 * \warning This class is not used and may disappear without notice. 
 * \ingroup basics */
class Heap {
public:
  /** Constructor. */
  Heap() {
  }
  /** Destructor. */
  ~Heap() {
  }
  /** Allocate n bytes of memory.
   * \param n The number of bytes to allocate.
   * \return A pointer to memory allocated. */
  void *malloc(size_t n) {
    return ::malloc(n);
  }
  /** Free the given memory.
   * \param p A pointer to the memory to free. */
  void free(void *p) {
    ::free(p);
  }
};

} // namespace Puma

#endif /* defined(WIN32) && !defined(_MSC_VER) */

#endif /* PUMA_Heap_H */
