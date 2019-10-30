#include <stdio.h>

void func () {
}

class Test {
public:
  Test () {
    func ();
  }
  ~Test () {
    func ();
  }
  void *operator -> () {
    func ();
    return 0;
  }
  void foo (int) const /* volatile */ {
    func ();
  }
  void foo (int) {
    func ();
  }
};

namespace Name {
  void space () {
    func ();
  }
}

int main () {
  printf ("Name Mangling: func() should be invoked several times:\n");
  printf ("======================================================\n");
  Test obj;
  obj.operator -> ();
  obj.foo (4711);
  Name::space ();
  return 0;
}

