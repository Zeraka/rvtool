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

#if defined(WIN32) && !defined(_MSC_VER) 

#include "Puma/Heap.h"
#include "Puma/Chain.h"
#include <windows.h>
#include <new>
#include <cassert>

namespace Puma {


class HeapPool : public Heap {
  Chain m_Trash;
  int m_Cnt;

public:
  HeapPool (int) : m_Cnt (0xff) {}
   
  void *alloc (size_t s) { 
    return Heap::malloc (s); 
  }
  void dealloc (void *p) {
    if (lowmem ())
      clean ();
    m_Trash.insert ((Chain*)p);
  }
  void clean () {
    long long num=0;
    Chain *p;
    while ((p = m_Trash.unlink ()) != 0) {
      Heap::free (p);
      num++;
    }
  }
  bool lowmem () {
    if (m_Trash.select () != 0 && --m_Cnt == 0 ) {
      m_Cnt = 0xff;
      MEMORYSTATUS stat;
      GlobalMemoryStatus (&stat);
      return stat.dwAvailPhys < (stat.dwTotalPhys / 50);
    }
    return false;
  }
};


#if PROFILE_MEM

class __ProfileAlloc : public Heap {
  long long Cnt[100];
  long long total;
  long long totalMB;

public:
  __ProfileAlloc () { 
    total = totalMB = 0; 
    for (int x = 0; x < 100; x++) { 
      Cnt[x] = 0; 
    } 
  }
  ~__ProfileAlloc () { 
    //for (int x = 8; x < 81; x += 4) {
    //  std::cout << x << ": " << Cnt[x] << std::endl;
    //} 
    //std::cout << "Total new: " << total << std::endl;
    //std::cout << "Total MB : " << totalMB / 1024 / 1024 << std::endl;
  }
  void *malloc (size_t s) { 
    if (s < 100)
      Cnt[s]++;
    total++;
    totalMB += s;
    return Heap::malloc (s);
  }
  void free (void *p) {
    Heap::free (p);
  }
};

static __ProfileAlloc __pool;

#else /* PROFILE_MEM */

static Heap __pool;

#endif /* PROFILE_MEM */


} // namespace Puma


// replace the global new and delete operators

void *operator new(std::size_t s) throw (std::bad_alloc) {
  void *res = Puma::__pool.malloc(s);
  if (res == 0) {
    std::cerr << "FATAL: Out of memory! Aborting..." << std::endl;
    exit(0);
  }
  return res;
}
void *operator new(std::size_t s, const std::nothrow_t &) throw () {
  void *res = Puma::__pool.malloc(s);
  if (res == 0) {
    std::cerr << "FATAL: Out of memory! Aborting..." << std::endl;
    exit(0);
  }
  return res;
}
void *operator new[](std::size_t s) throw (std::bad_alloc) {
  void *res = Puma::__pool.malloc(s);
  if (res == 0) {
    std::cerr << "FATAL: Out of memory! Aborting..." << std::endl;
    exit(0);
  }
  return res;
}
void *operator new[](std::size_t s, const std::nothrow_t &) throw () {
  void *res = Puma::__pool.malloc(s);
  if (res == 0) {
    std::cerr << "FATAL: Out of memory! Aborting..." << std::endl;
    exit(0);
  }
  return res;
}
void operator delete(void *p) throw () {
  assert(p);
  Puma::__pool.free(p);
}
void operator delete(void *p, const std::nothrow_t &) throw () {
  assert(p);
  Puma::__pool.free(p);
}
void operator delete[](void *p) throw () {
  assert(p);
  Puma::__pool.free(p);
}
void operator delete[](void *p, const std::nothrow_t &) throw () {
  assert(p);
  Puma::__pool.free(p);
}

#endif /* defined(WIN32) && !defined(_MSC_VER)  */
