#include <stdio.h>

class A {
public:
  A operator++ () {
    printf("inside A A::operator ++()\n");
    return *this;
  }
  A operator++ (int) {
    printf("inside A A::operator ++(int)\n");
    return *this;
  }
};

aspect PlusPlus {
  advice call ("% A::operator ++()") : before () {
    printf ("before %s\n", JoinPoint::signature ());
  }
  advice call ("% A::operator ++(int)") : before () {
    printf ("before %s\n", JoinPoint::signature ());
  }
};

int main() {
  printf ("Bug239: distinguish call advice for operator ++\n");
  printf ("===============================================\n");
  A a;
  a++;
  ++a;
  printf ("===============================================\n");
}
