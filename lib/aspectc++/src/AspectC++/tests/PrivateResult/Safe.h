#ifndef __Safe_h__
#define __Safe_h__

#include <stdlib.h>

class SafeBase {};

class Safe : public SafeBase {
  friend Safe f ();
  int _id;
  Safe (int id) : _id (id) {}
  Safe &operator = (const Safe &obj) { _id = obj._id; return *this; }
  class Inner {}; // a private inner type!
public:
  Safe (const Safe &obj) { *this = obj; }
  Safe (const SafeBase &obj) { _id = -1; } // for confusion!
  ~Safe () {}
  // the following function is needed to make the overload resolution more
  // complicated
  void *operator new (size_t s) { return malloc (s); }
  void print (ostream &os) const { os << _id; }
  // the following code is needed to check how wrapper code can access private
  // inner types
  friend int main();
  inline Inner uses_private_inner1 ();
  inline const Inner &uses_private_inner2 ();
};

inline Safe::Inner Safe::uses_private_inner1 () { return Inner (); }

inline const Safe::Inner &Safe::uses_private_inner2 () {
  static Inner i;
  return i;
}

ostream &operator << (ostream &os, const Safe& safe) {
  safe.print (os);
  return os;
}

union AUnion {
  int i;
  int j;
};

#endif // __Safe_h__
