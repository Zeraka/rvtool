class A {
  public: void bar(int x) {}
};

// the function itself
int foo(int f) { return f; }

int main() {
  // function pointer
  int (*foo2)(int) = &foo;
  foo2 = &foo;
  foo2 = foo;

  // function reference
  int(& func)(int) = foo;
  func(5);

  // function-pointer call
  foo2(42);
  (*foo2)(4711);

  // member-function pointer:
  A a;
  void (A::*fptr) (int) = &A::bar;
  (a.*fptr)(3);
}

