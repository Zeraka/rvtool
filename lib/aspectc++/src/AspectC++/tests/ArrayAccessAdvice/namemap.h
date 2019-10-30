#ifndef __NAMEMAP_H__
#define __NAMEMAP_H__

#include <map>

namespace NameMap {
  typedef std::pair<const char *, size_t> Info;
  typedef std::map<void *, Info > Map;
  extern Map objs;
  extern Map arrays;

  extern void mangle_arraypart( void *addr, size_t s );
  template<typename T> inline void mangle_arraypart( T* addr ) {
    mangle_arraypart( addr, sizeof( *addr ) );
  }
}

#endif
