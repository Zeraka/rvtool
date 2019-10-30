#include <stdio.h>

class A {
public:
  void a(int i, float b, const int &a, void *p = (void*)0x4711) {
  }
};

void no_args() {}

int main() {
  int val = 42;
  A a;
  printf ("Static Trace: arguments for each call\n");
  printf ("=============================================================\n");
  a.a(4711, 3.14f, val);
  printf ("-------------------------------------------------------------\n");
  a.a(4712, 3.15f, val, (void*)0x12345678);
  printf ("-------------------------------------------------------------\n");
  no_args ();
  printf ("=============================================================\n");
}
