class C;

template <typename T>
struct TC {
  T _t;
  typedef typename T::FOO BLA;
};

typedef TC<C> TCC; // Puma already instantiates here

struct C { typedef int FOO; };

void x (TCC::BLA bla) { // g++ instantiates here (C is complete!)
  bla += 42;
}
