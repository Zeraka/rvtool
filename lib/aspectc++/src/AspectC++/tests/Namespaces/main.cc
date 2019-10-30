#include <stdio.h>

namespace n1 {
  namespace n2 {
//     class C1;
    class C2 {
      enum { VAL = 2 };
    public:
      void f () {}
    };
    template <class Item> class C3 { 
      enum { VAL = 3 }; 
    public:
      void f () {}
    };
  }
}

namespace n3 {
template <typename T> class C4;
}

template <typename T> class n3::C4 {
  enum { VAL = 4 };
public:
  void f () {}
};

// class n1::n2::C1 { enum { VAL = 3 }; void f () {} };

aspect BigBrother {
  advice execution ("%") : before () {
    printf ("%s: %d\n", tjp->signature (), tjp->that ()->VAL);
  }
  advice that ("n1::n2::C2") && execution ("%") : before () {
    printf ("that C2: %s\n", tjp->signature ());
  }
};

int main() {
  using namespace n1::n2;
//   C1 c1;
  C2 c2;
  C3<int> c3;
  n3::C4<double> c4;
  printf ("Namespace:\n");
  printf ("===============================================================\n");
//   c1.f ();
  c2.f ();
  c3.f ();
  c4.f ();
  printf ("---------------------------------------------------------------\n");
  return 0;
}
