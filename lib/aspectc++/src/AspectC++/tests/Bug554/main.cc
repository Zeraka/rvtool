#include <iostream>
using namespace std;

// --- test for constness ---
template <typename T>
struct is_constant {
    enum { IS_CONSTANT = 0 };
};
template <typename T>
struct is_constant<const T> {
    enum { IS_CONSTANT = 1 };
};


class A {
  int foo();
public:
  A() {}
};

int A::foo() { __leave(); return 0; }

int main() {
  const A a;
  cout << "========== const call =========" << endl;
  a.__leave(); // const call

  A a1;
  cout << "======== non-const call =======" << endl;
  a1.__leave(); // non-const call
  return 0;
}


slice class MySlice {
public:
  __attribute__((always_inline)) inline void __leave() const {
    cout << "const leave" << endl;
  }
  void __leave();
};

slice void MySlice::__leave() { cout << "non-const leave" << endl; }

aspect Aspect1 {
  advice execution("% ...::__leave()") : around() {
    cout << "exec: " << tjp->signature() << endl;
    cout << "  JoinPoint::That is const: " << is_constant<JoinPoint::That>::IS_CONSTANT << endl; // 1 (this is correct)
    cout << "  JoinPoint::Target is const: " << is_constant<JoinPoint::Target>::IS_CONSTANT << endl; // puma: 0 (does this make sense?)
    tjp->proceed();
  }

  advice call("% ...::__leave()") : around() {
    cout << "call: " << tjp->signature() << endl;
    cout << "  JoinPoint::That is const: " << is_constant<JoinPoint::That>::IS_CONSTANT << endl; // 1 (this is correct)
    cout << "  JoinPoint::Target is const: " << is_constant<JoinPoint::Target>::IS_CONSTANT << endl; // puma: 0 (does this make sense?)
    tjp->proceed();
  }

  advice "A" : slice MySlice;
};

