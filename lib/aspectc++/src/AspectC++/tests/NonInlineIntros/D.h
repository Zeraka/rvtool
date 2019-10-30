#ifndef __D_h__
#define __D_h__

class D {
  int i;
  static const int j = 3;
  static int k;
public:
  void f (); // link-once code element (functions first)
};

#endif // __D_h__
