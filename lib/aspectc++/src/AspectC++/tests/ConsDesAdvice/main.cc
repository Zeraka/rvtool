#include <stdio.h>

#include "c1.h"

class UCBase { UCBase (); };
class Unconstructable : public UCBase {
};

class C2 : public C1 {
public:
  C2 () : C1 (4711) {}
  ~C2 ();
  void f () {}
};

C2::~C2 () {
}

class C3 : public C1 {
public:
  C3 (int i) : C1 ((2 * i) + 3 * i) {}
};

class C4 {};

class Bug530 {
  Bug530 xxx() { Bug530 a; return a; } // this is NO ctor
};

template <typename P>
class TXXX {
public:
  TXXX() {}
};

int main () {
  printf ("ConsDesAdvice: advice for object construction & destruction\n");
  printf ("===========================================================\n");
  printf ("--C2:\n");
  C2 c2;
  printf ("--C2-copy:\n");
  C2 copy (c2);
  printf ("--C3:\n");
  C3 c3 (10);
  printf ("--C4:\n");
  C4 c4;
  printf ("--correctly copied?\n");
  printf ("  "); c2.dump ();
  printf ("  "); copy.dump ();
  printf ("--Bug530: is that object constructed here?\n");
  Bug530 b1;
  Bug530 b2 = Bug530(b1);
  printf ("--Instantiating template TXXX<int>: no advice should match here\n");
  TXXX<int> ti;
  printf ("----------------------end-of-main--------------------------\n");
  return 0;
}

aspect AfterMainActions {
  advice execution ("% main (...)") : after () {
    printf ("===========================================================\n");
  }
};
