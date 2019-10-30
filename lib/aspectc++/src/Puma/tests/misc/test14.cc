/***************************************************
 ** specialization for pointer to member function **
 ***************************************************/
 
struct Y {
  void foo() {}
  void bar() {}
};

template < void (Y::*T)() >
struct X {
  X() { Y y; (y.*T)(); }
};

template <>
struct X< &Y::foo > {
};

X< &Y::foo > x1;
X< &Y::bar > x2;
