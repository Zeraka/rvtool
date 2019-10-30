#include <iostream>
using namespace std;

namespace A {
  namespace A {
    namespace B {
      class C {
        public:
        static C* m;
        static int n;
        void func(int) {
          cout << "1" << endl;
        }
      };
    }
    namespace A {
      namespace B {
        class C {
          public:
          static ::A::A::B::C* m;
          static int n;
          void func(int) {
            cout << "2" << endl;
          }
        };
      }
    }
  }
  namespace B {
    class C {
      public:
      static A::A::B::C* m;
      static int n;
      void func(int) {
        cout << "3" << endl;
      }
    };
  }
  void caller2(A::B::C c) {
    c.func(1);
  }
  void caller3(A::B::C& c) {
    c.func(1);
  }
  void caller4(A::B::C* c) {
    c->func(1);
  }
  void caller5(void (A::B::C::* c)(int)) {
    A::B::C obj;
    (obj.*c)(1);
    obj.n = obj.n + 1;
  }
  void caller6(void (::A::B::C::* c)(int)) {
    ::A::B::C obj;
    (obj.*c)(1);
    obj.n = obj.n + 2;
  }
  void caller7(void (A::A::B::C::* c)(int)) {
    A::A::B::C obj;
    (obj.*c)(1);
    obj.n = obj.n + 4;
  }
  void caller8(void (::A::A::A::B::C::* c)(int)) {
    ::A::A::A::B::C obj;
    (obj.*c)(1);
    obj.n = obj.n + 8;
  }
  void caller9(void (B::C::* c)(int)) {
    B::C obj;
    (obj.*c)(1);
    obj.n = obj.n + 16;
  }
  
  template <typename T, typename T2>
  void TemplTester (T2*& t2) {
    T t = T2();
    t.func(42);
  }
    
  void caller(int) {
    
    
    A::A::B::C c5;
    ::A::B::C::m = &c5;
    ::A::A::B::C::m = A::A::B::C::m = B::C::m->m = reinterpret_cast<A::B::C*>(4711);
    
    
    cout << "Should select 1: ";
    caller2(A::B::C());
    A::B::C c;
    cout << "Should select 1: ";
    caller3(c);
    cout << "Should select 1: ";
    caller4(&c);
    cout << "Should select 1: ";
    caller5(&A::B::C::func);
    cout << "Should select 3: ";
    caller6(&::A::B::C::func);
    cout << "Should select 2: ";
    caller7(&A::A::B::C::func);
    cout << "Should select 2: ";
    caller8(&::A::A::A::B::C::func);
    cout << "Should select 3: ";
    caller9(&B::C::func);
    
    ::A::A::B::C c2;
    c2 = A::B::C();
    A::B::C* c3;
    c3 = &c2;
    ::A::A::B::C*& c4 = c3;
    cout << "Should select 1: ";
    TemplTester<A::B::C>(c4);
    
  }
}

A::A::B::C* A::A::B::C::m = 0;
::A::A::B::C* A::A::A::B::C::m = 0;
A::A::A::B::C* ::A::B::C::m = 0;

int A::A::B::C::n = 10;
int A::A::A::B::C::n = 20;
int ::A::B::C::n = 30;

int main() {
  
  A::caller(0);
  
  cout << "Should be 4711: " << reinterpret_cast<unsigned long>(A::A::B::C::m) << endl;
  cout << "Should be 4711: " << reinterpret_cast<unsigned long>(A::A::A::B::C::m) << endl;
  cout << "Should be 4711: " << reinterpret_cast<unsigned long>(A::B::C::m->m) << endl;
  
  cout << "Should be 11: " << A::A::B::C::n << endl;
  cout << "Should be 32: " << A::A::A::B::C::n << endl;
  cout << "Should be 48: " << A::B::C::n << endl;
  return 0;
}

aspect Asp {
  advice call("% ...::%(...)") || builtin("% ...::%(...)") || set("% ...::%") : before() {
    tjp->arg<0>(); // All matchable functions have exactly one argument.
  }
  
  advice get("% ...::%") : around() {
    tjp->proceed();
  }
};
