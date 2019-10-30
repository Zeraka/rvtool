#include <iostream>
#include <map>

using namespace std;

namespace NameMap {
  typedef std::pair<const char *, size_t> Info;
  typedef map<void *, Info > Map;
  Map objs;
  Map arrays;

  void mangle_arraypart( void *addr, size_t s ) {
    void *base = 0;
    const char * name = 0;
    size_t diff = 0;
    Map::iterator it = arrays.upper_bound( addr );
    if( it != arrays.begin() ) {
      --it;
      base = it->first;
      name = it->second.first;
      diff = ( reinterpret_cast<size_t>( addr ) - reinterpret_cast<size_t>( base ) );

      if( diff >= it->second.second )
        name = 0;

      diff /= s;
    }

    if( name ) {
      cout << name << "+" << diff;
    }
    else
      cout << "<unknown>";
  }
}
