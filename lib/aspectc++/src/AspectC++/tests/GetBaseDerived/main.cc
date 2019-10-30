#include <stdio.h>

struct A {
  int a;
  A() { a = 1; }
};

struct B : public A {
  int b;
  B() { b = 2; }
};

struct C : public B {
  int c;
  C() { c = 3; }
};

struct D : public C {
  int d;
  int b;
  D() { d = 4; b = 5; }
};

int main () {
  D obj;
  int v;
  v = obj.a;
  v = obj.B::b;
  v = obj.c;
  v = obj.d;
  v = obj.b;
}

aspect Test {
  advice get("%") : before () {
    printf ("get: %s = %i\n", JoinPoint::signature (), *tjp->entity());
  }
  advice get(derived("B")) : before () {
    printf ("derived from \"B\": %s = %i\n", JoinPoint::signature (), *tjp->entity());
  }
  advice get(derived("% B::b")) : before () {
    printf ("derived from \"B::b\": %s = %i\n", JoinPoint::signature (), *tjp->entity());
  }
};
