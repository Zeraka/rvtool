#include "stdio.h"

// aspects; that() used for each kind of object

aspect On {
  pointcut virtual kind() = 0;
  advice kind () && !within("On") : after () {
    printf ("%s\n", JoinPoint::signature ());
  }
};

aspect OnA : public On { pointcut kind () = that("A"); };
aspect OnB : public On { pointcut kind () = that("B"); };
aspect OnC : public On { pointcut kind () = that("C"); };
aspect OnD : public On { pointcut kind () = that("D"); };
aspect OnTargetCorD : public On {
  pointcut kind () = target("C") || target("D");
//   pointcut kind () = that(derived("C%") && !base("A"||"B")) || target("D");
};
aspect OnThatnotCandnotD : public On {
  pointcut kind () = !that("C") && !that("D") && execution("% %::%(...)");
};

// class hierarchy; each class has an inline, static and non-inline function
//
//     A (root)
//     |
//     B
//    / \
//   C   D (leafs)

class A {
public:
  void f () {}
  void g ();
  static void h () {}
};
void A::g () {}

class B : public A {
public:
  void f () {}
  void g ();
  static void h () {}
};
void B::g () {}

class C : public B {
public:
  void f () {}
  void g ();
  static void h () {}
};
void C::g () {}

class D : public B {
public:
  void f () {}
  void g ();
  static void h () {}
};
void D::g () {}

// ordinary functions
void a () {}

void b ();
void b () {}

// testcode
int main () {
  printf ("ordinary functions (there should be no advice)\n");
  a ();
  b ();
  printf ("on A:\n");
  A a;
  a.f ();
  a.g ();
  a.h ();
  printf ("on B:\n");
  B b;
  b.f ();
  b.g ();
  b.h ();
  printf ("on C:\n");
  C c;
  c.f ();
  c.g ();
  c.h ();
  printf ("on D:\n");
  D d;
  d.f ();
  d.g ();
  d.h ();
  return 0;
}
