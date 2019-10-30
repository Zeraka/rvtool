#include <inttypes.h>

class Base1 {};

class Base2 {};

class Base3 : private Base2 {
  int32_t a;
  int64_t a2;
};

class Derived : public Base3, public Base1 {
  int32_t b[2];
  int8_t c;
  int64_t d;
  
  public:
  void bar() {};
  void foo();
};

int main() {
  Derived b;
  b.bar();

  return 0;
}
