#include <stdio.h>

class C {
public:
  void f () {}
  int g (int) { return 0; }
  void h () {}
};

aspect A {
  advice execution ("void C::f()") : around () {
    printf ("replaced %s by advice!\n", JoinPoint::signature ());
  }
  advice execution ("% C::g(...)") &&
         cflow(execution("% main(...)")): around () {
    printf ("replaced %s by advice!\n", JoinPoint::signature ());
  }
  advice execution ("void C::h()") : around () {
    printf ("replaced %s by advice!\n", JoinPoint::signature ());
    tjp->action ().trigger ();
  }
};

int main () {
  printf ("Bug267: around advice without proceed\n");
  printf ("=====================================\n");
  C c;
  c.f ();
  c.g (42);
  c.h ();
  printf ("=====================================\n");
  return 0;
}
