#include <typeinfo>

class A {};
class B : A {};
class C : A {};
class D : A {};

void foo(A& a) {
  if (typeid(a) == typeid(B)) {
    /* Object has type B */;
  } else if (typeid(a) == typeid(C)) {
    /* Object has type C */; 
  } else {
    /* Unexpected type */;
  }
}

int main() {
  B b; foo(b);
  C c; foo(c);
  D d; foo(d);
  return 0;
}
