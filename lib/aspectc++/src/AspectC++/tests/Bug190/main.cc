#include <stdio.h>

aspect Bug190 {
  advice execution ("% f(...)") : before () {
    printf ("before function \"%s\"\n", JoinPoint::signature ());
  }
};

void f (int) {}
void f (float) {}

int main () {
  f (2);
  f (3.14f);
  return 0;
}
