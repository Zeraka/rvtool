#include <iostream>
using namespace std;

const char* JPType(int num) {
  switch (num) {
  case AC::GET:
    return "get   "; break;
  case AC::SET:
    return "set   "; break;
  case AC::REF:
    return "ref   "; break;
  case AC::CALL:
    return "call  "; break;
  case AC::EXECUTION:
    return "exec  "; break;
  case AC::CONSTRUCTION:
    return "constr"; break;
  case AC::DESTRUCTION:
    return "destr "; break;
  default:
    return "error ";
  }
}

namespace A {

class InA {
public:
  void foo() {}
  int x;
};

namespace B {

class InB {
public:
  void foo() {}
  int x;
};

namespace C {

class InC {
public:
  void foo() {}
  int x;

  class Inner {
  public:
    void foo() {}
    int x;

    class InnerInner {
    public:
      void foo() {}
      int x;
    };
  };
};

} // C

} // B

} // A


int main() {
  {
    A::InA a;
    a.foo();
    a.x++;
    int *x = &a.x;
  }

  {
    A::B::InB b;
    b.foo();
    b.x++;
    int *x = &b.x;
  }

  {
    A::B::C::InC c;
    c.foo();
    c.x++;
    int *x = &c.x;
  }

  {
    A::B::C::InC::Inner in;
    in.foo();
    in.x++;
    int *x = &in.x;
  }

  {
    A::B::C::InC::Inner::InnerInner inin;
    inin.foo();
    inin.x++;
    int *x = &inin.x;
  }

  cout << endl << "Slice test:" << endl;
  cout << "# members of A::InA: " << AC::TypeInfo<A::InA>::MEMBERS << " (should be 2)" << endl;
  cout << "# members of A::B::InB: " << AC::TypeInfo<A::B::InB>::MEMBERS << " (should be 2)" << endl;
  cout << "# members of A::B::C::InC: " << AC::TypeInfo<A::B::C::InC>::MEMBERS << " (should be 3)" << endl;
  cout << "# members of A::B::C::InC::Inner: " << AC::TypeInfo<A::B::C::InC::Inner>::MEMBERS << " (should be 5)" << endl;
  cout << "# members of A::B::C::InC::Inner::InnerInner: " << AC::TypeInfo<A::B::C::InC::Inner::InnerInner>::MEMBERS << " (should be 2)" << endl;
}

aspect MatchingTest {

  // information for the human reader
  advice construction(!"MatchingTest") : before() {
    cout << endl << "Creating instance of the class type: " << AC::TypeInfo<JoinPoint::That>::signature() << endl;
    cout << "===========================================================================" << endl;
  }

  // the actual pointcuts to be checked
  pointcut a() = "A";
  pointcut b() = "A::B";
  pointcut c() = "A::B::C";
  pointcut in_c() = "A::B::C::InC"; // only a class
  pointcut inner() = "A::B::C::InC::Inner"; // only a nested class

  // namespace matching
  advice call(a()) || execution(a()) || construction(a()) || destruction(a()) || get(a()) || set(a()) || ref(a()) : before() {
    cout << JPType(JoinPoint::JPTYPE) << "(\"A\"): " << JoinPoint::signature() << endl;
  }
  advice call(b()) || execution(b()) || construction(b()) || destruction(b()) || get(b()) || set(b()) || ref(b()) : before() {
    cout << JPType(JoinPoint::JPTYPE) << "(\"A::B\"): " << JoinPoint::signature() << endl;
  }
  advice call(c()) || execution(c()) || construction(c()) || destruction(c()) || get(c()) || set(c()) || ref(c()) : before() {
    cout << JPType(JoinPoint::JPTYPE) << "(\"A::B::C\"): " << JoinPoint::signature() << endl;
  }

  // class matching
  advice call(in_c()) || execution(in_c()) || construction(in_c()) || destruction(in_c()) || get(in_c()) || set(in_c()) || ref(in_c()) : before() {
    cout << JPType(JoinPoint::JPTYPE) << "(\"A::B::C::InC\"): " << JoinPoint::signature() << endl;
  }
  advice call(inner()) || execution(inner()) || construction(inner()) || destruction(inner()) || get(inner()) || set(inner()) || ref(inner()) : before() {
    cout << JPType(JoinPoint::JPTYPE) << "(\"A::B::C::InC::Inner\"): " << JoinPoint::signature() << endl;
  }

  // slice matching
  advice a() : slice class {
    static const int y = JoinPoint::MEMBERS;
  };

  advice in_c() : slice class {
    static const int z = JoinPoint::MEMBERS;
  };

  advice inner() : slice class {
    static const int a1 = JoinPoint::MEMBERS;
    static const int a2 = JoinPoint::MEMBERS;
    static const int a3 = JoinPoint::MEMBERS;
  };

};
