#include <stdio.h>

aspect Bug192 {
  advice execution ("void f(...)") : before () {
    printf ("before JP '%s'\n", JoinPoint::signature ());
  }
};

typedef struct { int _i; } Foo;
typedef enum { A, B, C } ABC;
enum Named { D, E };
typedef Named DE;
void f (Foo &arg, ABC abc, DE de) {}

int main () {
  Foo foo;
  f (foo, A, D);
  return 0;
}
