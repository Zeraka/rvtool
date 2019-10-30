// Test overload resolution of function calls inside
// const function body.

struct C { 
  void incoming() {}
};

struct A {
  C &c;
  void f() const {
    c.incoming();
  }
};
