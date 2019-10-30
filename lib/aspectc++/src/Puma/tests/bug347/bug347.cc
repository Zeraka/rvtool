class X {};

template <class T> 
struct Templ {
  typedef X Y;    // early bind X to class X
};

void X() {}

Templ<int>::Y y;  // OK, Y is class X and not function X
