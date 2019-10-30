#ifndef __C1_h__
#define __C1_h__

#include <stdio.h>

class M {
  int i;
public:
  M() { i = 815; };
  M(const M&c) { i = c.i; }
  int val () const { return i; }
};

template <typename T> class TemplateBase {
};

class Unique {
    int _id;
    void gen () {
	static int curr = 0;
	_id = curr++;
    }
 public:
    Unique () { gen (); }
    Unique (const Unique &obj) { gen (); }
    int id () const { return _id; }
};

// the TemplateBase is only handled correct with --real-instances
class C1 : TemplateBase<int>, Unique {
  // these elements should not be copied!
  friend class M;
  enum E { E1 = 10, E2 };
  static const int c = 13;
  // --
  M m;
  M ma[3];
  const int &r;
  const int val;
  double array[2];
  unsigned scalar;
  static char *static_ptr;
  int a1[3], s1;
  int s2, a2[6];
  typedef int INT[10];
  int s3, a3[4][6], s4;
  INT a4;
  union { int u1; float u2; };
  union { int u3; float u4; INT u5; };
public:
  C1 (int v) : val (v), r(val) { 
    array[0] = 3.14; array[1] = 2.71;
    scalar = 4711;
    dump ();
  }
  inline C1 (double);
  ~C1 () {}
  void dump () const {
    printf ("C1 obj %d (%d, [%d, %d, %d], %d, %d, %d, [%1.2lf, %1.2lf] %d)\n",
	    id (),
	    m.val (),
	    ma[0].val (), ma[1].val (), ma[2].val (),
	    r,
	    val,
	    c,
	    array[0], array[1],
	    scalar);
  }
};

char *C1::static_ptr = (char*)0; // this cast creates an anonymous attribute!

inline C1::C1 (double d) : val (-1), r (val) {
  array[0] = d + 1.0;
  array[1] = d + 2.0;
  scalar = 4712;
  dump ();
}

#endif // __C1_h__
