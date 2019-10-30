#include <stdio.h>

class C {
  int _priv_attr;
  void priv_fct () { printf ("  in private function\n"); _priv_attr++; }
public:
  C() : _priv_attr (0) {}
  void pub_fct () {}
};

union U {
  int u1;
  char u2;
};

slice class Slice {
  int _priv_slice_attr;
  class Inner {
    int _priv_inner_attr;
  public:
    Inner () : _priv_inner_attr (0) {}
  } _inner;
  union InnerUnion {
    int i;
    long j;
  } _inner_union;
};

template <typename X> class T { static void f() { printf ("  in private T::f()\n"); } };

int main() {
  C c;
  printf ("AspectPrivileges: Check whether aspects are friends of all classes\n");
  printf ("==================================================================\n");
  c.pub_fct ();
  printf ("==================================================================\n");
}

aspect A {
  U _u;

  advice "C" : slice Slice;

  advice execution("void C::pub_fct()") : before () {
    printf ("printing private attribute\n");
    printf ("  private attribute: %d\n", tjp->that ()->_priv_attr);
    printf ("increasing private attribute by calling private member function\n");
    tjp->that ()->priv_fct ();
    printf ("printing private attribute again\n");
    printf ("  private attribute: %d\n", tjp->that ()->_priv_attr);
    printf ("writing into private attribute of introduced class\n");
    tjp->that ()->_inner._priv_inner_attr++;
    printf ("writing into private attribute of a union\n");
    _u.u1 = 42;
    printf ("calling private member function of template\n");
    T<int>::f();
  }
};
