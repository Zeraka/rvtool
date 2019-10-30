#include <stdio.h>

aspect Bug191 {
  advice execution ("% * volatile ...::%(...)") : around () {
    printf ("before function \"%s\"\n", JoinPoint::signature ());
    tjp->proceed ();
  }
};

typedef void * volatile VPTR;

VPTR f (VPTR a) { VPTR p = 0; return p; }

int main () {
  VPTR a = 0;
  VPTR p = f (a);
  return 0;
}
