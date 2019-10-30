#include <stdio.h>

#define TRAP_CTOR_SCOPE public

class TrapListener {
  TRAP_CTOR_SCOPE:
public:
  TrapListener() {}
};

#define CLASS_START class C2 {

CLASS_START
public:
  int i;
};

struct A { typedef struct {} B; struct X{} x; };

int main () {
  printf ("Bug575: Inject bypass class vs. macros\n");
  printf ("======================================\n");
  TrapListener tl;
  C2 c2;
  c2.i = 42;
  printf ("... fine if it compiles\n");
  printf ("======================================\n");
}

aspect X {
  advice "A" : slice struct : public TrapListener;
};
