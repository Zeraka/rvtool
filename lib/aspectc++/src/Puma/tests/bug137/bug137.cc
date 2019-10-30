struct X {
  int l;

  template <class T>
  int foo(T) {return l;}

  int foo(int) {return l;}
};

X x;
int i = x.foo(1);  // X::foo(int)
