#include <stdio.h>

aspect JPTypePrinter {

  void print_jp_type (AC::JPType jptype) {
    switch (jptype) {
    case AC::EXECUTION:    printf ("execution"); break;
    case AC::CALL:         printf ("call"); break;
    case AC::CONSTRUCTION: printf ("construction"); break;
    case AC::DESTRUCTION:  printf ("destruction"); break;
    default:               printf ("unknown");
    }
  }

  advice (execution ("% ...::%(...)") || call ("% ...::%(...)") ||
	  construction ("C") || destruction ("C")) &&
    !within ("JPTypePrinter") : before () {
    print_jp_type (JoinPoint::JPTYPE);
    printf (" join point %s\n", JoinPoint::signature ());
  }

};

class C {
public:
  void f () {}
};

int main () {
  printf ("JPType: checks the JPType number generation\n");
  printf ("===========================================\n");
  C c;
  c.f ();
  printf ("===========================================\n");
  return 0;
}
