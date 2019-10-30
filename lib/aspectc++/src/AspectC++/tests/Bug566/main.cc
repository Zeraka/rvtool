#include <iostream>
using namespace std;
aspect TestAround {
   pointcut matchFunc() = "% A::matchFunc%(...)";
   advice call(matchFunc()) || execution(matchFunc()) || construction("A") : around() {
     cout << tjp->signature() << endl;
     tjp->proceed();
   }
};

#include <stdio.h>
class A {
public:
  A(const char p1[], unsigned int p2) {
    printf("0x%x, %u\n", *(int*)(&p1), p2);
  }
  A(const char* p1, unsigned int p2, bool b) {
    printf("0x%x, %u\n", *(int*)(&p1), p2);
  }
  void matchFunc1(const char p1[], unsigned int p2) {
    printf("0x%x, %u\n", *(int*)(&p1), p2);
  }
  void matchFunc2(const char* p1, unsigned int p2) {
    printf("0x%x, %u\n", *(int*)(&p1), p2);
  }
};

int main() {
   A a = A((char*) 0x1234, 4);
   a = A((char*) 0x1234, 4, true);
   a.matchFunc1((char*) 0x1234, 4);
   a.matchFunc2((char*) 0x1234, 4);
   return 0;
}
