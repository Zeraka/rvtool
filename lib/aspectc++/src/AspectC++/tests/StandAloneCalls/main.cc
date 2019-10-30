#include <stdio.h>

// case 1: call and function in the same namespace
namespace N1 {

  int foo() {
    return 1;
  }

  const int var = foo();
}

// case 2: call outside the namespace, function inside
namespace N2 {

  int foo() {
    return 1;
  }
}

int var2_1 = N2::foo(), var2_2 = N2::foo ();

// case 3: call inside the namespace, function outside
int bar() { return 42; }

namespace N3 {
  int var = ::bar();
}

// case 4: static attribute initialization
class Test {
public:
  static int var;
};

int Test::var = bar ();

// case 5: static attribute initialization in a namespace
namespace N3 {
  class Test {
  public:
    static int var;
  };

  int Test::var = bar ();
}

// case 6: static attribute initialization of class in namespace
namespace N4 {
  class Test {
  public:
    static int var;
  };
}

int N4::Test::var = bar ();

// case 7: a call in an initializer list
class WithInitializer {
  int attr;
public:
  WithInitializer () : attr (bar ()) {}
  WithInitializer (int);
};

WithInitializer::WithInitializer (int v) : attr (v + bar ()) {}

int main() {
  printf ("StandAloneCalls:\n");
  printf ("===============================================================\n");
  printf ("here starts main, but we should have seen our advice already\n");
  printf ("global var 1 = %d\n", N1::var);
  printf ("global var 2-1 = %d, 2-2 = %d\n", var2_1, var2_2);
  printf ("global var 3 = %d\n", N3::var);
  printf ("---------------------------------------------------------------\n");
  printf ("Now the advice for calls in constructor init lists is checked\n");
  WithInitializer wi1;
  WithInitializer wi2 (3);
  printf ("---------------------------------------------------------------\n");
  return 0;
}
