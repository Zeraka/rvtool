class C;

template <typename T>
struct TC {
  T _t;
  typedef T BLA;
};

typedef TC<C> TCC; // Puma instantiates here
TCC::BLA bla; // g++ notices here that C is incomplete
              // Puma has workaround to ignore the problem

class C {};

TCC tcc;
