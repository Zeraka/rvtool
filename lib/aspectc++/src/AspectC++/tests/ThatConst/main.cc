#include <stdio.h>

// in this class 'this' is constant in all functions!
class C {
  int _id;
  void g () const {}
public:
  C (int id) : _id (id) {}
  void f () const { g (); }
};

aspect ObjTracker {

  void print_jp_type (AC::JPType jptype) {
    switch (jptype) {
    case AC::EXECUTION:    printf ("execution"); break;
    case AC::CALL:         printf ("call"); break;
    case AC::CONSTRUCTION: printf ("construction"); break;
    case AC::DESTRUCTION:  printf ("destruction"); break;
    default:               printf ("unknown");
    }
  }

  advice within ("C") && that (c) : around (C &c) {
    print_jp_type (JoinPoint::JPTYPE);
    printf (" %d\n", c._id);
    tjp->proceed ();
  }
};

int main () {
  C c1(1);
  const C c2 (2);
	   
  printf ("ThatConst: checks access to const objects with 'that'\n");
  printf ("=====================================================\n");
  printf ("c1 (non-const object):\n");
  c1.f ();
  printf ("c2 (const object):\n");
  c2.f ();
  printf ("=====================================================\n");
  return 0;
}
