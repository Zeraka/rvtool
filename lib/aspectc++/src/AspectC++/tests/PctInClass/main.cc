#include <stdio.h>

namespace N1 {
  class C {
  public:
    pointcut me() = "N1::C";
    void f () {}
  };
}

aspect A {
  advice execution ("% ...::%(...)") && within(N1::C::me()) : around () {
    printf ("before %s\n", JoinPoint::signature ());
    tjp->proceed ();
    printf ("after %s\n", JoinPoint::signature ());
  }
};

int main () {
  N1::C c;
  printf ("PctInClass: tests pointcut lookup\n");
  printf ("=================================\n");
  c.f ();
  printf ("=================================\n");
}

