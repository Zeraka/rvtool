#include <stdio.h>

class A {
public:
  int a(int i, float b) {
    printf("inside A::a(%d, %f)\n", i, b);
    return i;
  }
  int c(int p, int q);
  friend void b(char c, char *str);
};

int A::c(int p, int q){
  printf("A::c(%d, %d)\n",p,q);
  return p;
}

void b(char c, const char *str);

void b(char c, const char *str) {
  printf("b(%c, %s)\n", c, str);
}

void d(char c, const char *str) {
  printf("d(%c, %s)\n", c, str);
}

void e(void) {
  printf ("e(void)\n");
}

void f(int i, int j);
void f(int i, int) {
  printf ("f(%d, anon int)\n", i);
}

int main() {
  A a;
  printf ("Advice: each function must be wrapped by call and exec advice\n");
  printf ("=============================================================\n");
  a.a(4711, 3.14f);
  printf ("-------------------------------------------------------------\n");
  b('H', "ello World");
  printf ("-------------------------------------------------------------\n");
  a.c(4,5);
  printf ("-------------------------------------------------------------\n");
  d('H', "ello World");
  printf ("-------------------------------------------------------------\n");
  e();
  printf ("-------------------------------------------------------------\n");
  f(1,0);
  printf ("=============================================================\n");
}
