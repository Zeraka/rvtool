#include <stdio.h>

template<typename T> class Vector {
};

class B {
  void f1 () {}
};

class C : public B {
public:
  void static f1 (Vector<double> &);

  void f2 (Vector<int> &) {}
  static void f3 () {}
};

typedef C X;
void X::f1 (Vector<double> &) { Vector<int> v; C c; c.f2 (v); f3 (); }

aspect A {
  advice call ("void ...::f%(...)") : around () {
    printf ("called %s\n", JoinPoint::signature ());
    printf ("  that:   %s\n", (tjp->that () ? "not NULL" : "NULL"));
    printf ("  target: %s\n", (tjp->target () ? "not NULL" : "NULL"));
    tjp->proceed ();
  }
};

int main () {
  printf ("Bug268: this pointer used in advice for static member functions\n");
  printf ("===============================================================\n");
  C c;
  Vector<double> d;
  C::f1 (d);
  c.f1 (d);
  Vector<int> v;
  c.f2 (v);
  printf ("===============================================================\n");
  return 0;
}
