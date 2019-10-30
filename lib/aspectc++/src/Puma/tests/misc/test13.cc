/********************************************
 ** specialization for pointer to function **
 ********************************************/
 
void foo() {}
void bar() {}

template < void (*T)() >
struct X {
  X() { T(); }
};

template <>
struct X< &foo > {
};

X< &foo > x1;
X< &bar > x2;
