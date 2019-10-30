class X {};

class Y {
  friend X;  // standard requires: friend class X;
};
