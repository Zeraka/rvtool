#include <stdio.h>

void func() {}
void func(int) {}

pointcut pGlobalParam(int i) = execution("void func(...)") && args(i);

pointcut pGlobal() = execution("void func()");
pointcut pGlobal2() = pGlobal();

struct A {
  pointcut pA() = ::pGlobal();
};

struct B : public A {};

typedef B C;

template <int I> struct T {
  // forbidden: named pointcuts may not be defined in templates (new phase 1 parser!)
  // pointcut pT() = pGlobal ();
};

namespace N1 {

  pointcut pN1() = pGlobal2();

  namespace N2 {
    pointcut pN2() = pGlobal();
  }

}

// Does not work! For some unknown reason a using declaration does not work
// for named pointcuts:
// using N1::pN1;

namespace N3 {

  using namespace N1;

}

int main() {
  A a;
  printf ("NamePointcuts: test use of named pointcuts in different scopes\n");
  printf ("==============================================================\n");
  func();
  func(42);
  printf ("=============================================================\n");
}

aspect Test {
  pointcut pTestParam(int i) = execution("void func(...)") && args(i);

  advice execution("void func(...)") && args(i) : before (int i) {
    printf ("advice on pTestParam in advice\n");
  }
  advice pTestParam(j) : before (int j) {
    printf ("advice on pTestParam in aspect\n");
  }
  advice ::pGlobalParam(k) : before (int k) {
    printf ("advice on pGlobalParam\n");
  }

  advice pGlobal() : before () {
    printf ("advice on pGlobal\n");
  }
  advice pGlobal2() : before () {
    printf ("advice on pGlobal2\n");
  }
  advice A::pA() : before () {
    printf ("advice on A::pA\n");
  }
  advice B::pA() : before () {
    printf ("advice on B::pA\n");
  }
  advice C::pA() : before () {
    printf ("advice on C::pA\n");
  }
  // forbidden: named pointcuts may not be defined in templates (new phase 1 parser!)
  // advice T<1>::pT() : before () {
  //   printf ("advice on T<1>::pT\n");
  // }
  advice N1::pN1() : before () {
    printf ("advice on N1::pN1\n");
  }
  advice N1::N2::pN2() : before () {
    printf ("advice on N1::N2::pN2\n");
  }
  advice N3::pN1() : before () {
    printf ("advice on N3::pN1\n");
  }
};
