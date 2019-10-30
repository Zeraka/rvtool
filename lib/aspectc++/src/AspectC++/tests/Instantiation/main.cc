#include <cstdio>
using std::printf;

void resolved_func () {};

template <typename T, int I> class Array {
  T _data[I];
public:
  T get (int i) const { resolved_func (); return _data[i]; }
  void set (int i, const T &d) { _data[i] = d; }
};

aspect Bla {
  advice execution ("% ...::set(...)") : before () {
  }
};

namespace bla {
Array<double,10> darray;
}

int main () {
  printf ("Instantiation: advice for template instances\n");
  printf ("=============================================================\n");
  printf ("... not yet implement\n");
  Array<int, 4> iarray;
  iarray.set (0, 0);
  Array<double,10> darray_second_inst;
  darray_second_inst.set (0, 3.14);
  printf ("=============================================================\n");
  return 0;
}
