#include <iostream>
using namespace std;

template <class T>
class A {
public:
  T s;
};

typedef A<int> myA;

class B {
public:
  void m(myA a){ cout << "m( " << a.s << " )\n"; }
};

int main() {
  myA a;
  a.s = 5;
  B b;
  b.m(a);
  return 0;
}
