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
  A operator-- () {
    printf("inside A A::operator --()\n");
    return *this;
  }
  A operator-- (int) {
    printf("inside A A::operator --(int)\n");
    return *this;
  }
  int operator [] (int) {
    printf("inside A A::operator [](int)\n");
    return 0;
  }
  void operator () (int, bool = false) {
    printf("inside A A::operator ()(int,bool=false)\n");
  }
};

aspect Postfix {
  advice call ("% A::operator ++()") : before () {
    printf ("before %s\n", JoinPoint::signature ());
  }
  advice call ("% A::operator ++(int)") : before () {
    printf ("before %s\n", JoinPoint::signature ());
  }
  advice call ("% A::operator --()") : before () {
    printf ("before %s\n", JoinPoint::signature ());
  }
  advice call ("% A::operator --(int)") : before () {
    printf ("before %s\n", JoinPoint::signature ());
  }
  advice call ("% A::operator [](...)") : before () {
    printf ("before %s\n", JoinPoint::signature ());
  }
  advice call ("% A::operator ()(...)") : before () {
    printf ("before %s\n", JoinPoint::signature ());
  }
};

int main() {
  printf ("CallPostfixOp: advice for calls to postfix operators\n");
  printf ("====================================================\n");
  A a;
  a++;
  ++a;
  a--;
  --a;
  a[4711];
  a (1);
  a (2, true);
  a.operator()(3);
  printf ("====================================================\n");
}
