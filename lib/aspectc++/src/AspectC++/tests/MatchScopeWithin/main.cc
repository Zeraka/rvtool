#include <iostream>
using namespace std;

void foo() {}

namespace A {

namespace B {

namespace C {

class InC {
  public:
  void bar() { foo(); in.bar1(); }

  class Inner {
  public:
    void bar1() { foo(); } // within("A::B::C::InC") should not match
  } in;
};

} // C

} // B

} // A

static const int expected_matches = 6;
int matches = 0;

int main() {
  A::B::C::InC c;
  c.bar();

  if(matches != expected_matches){
    cout << "ERROR: there should have been " << expected_matches << " matches instead of " << matches << endl;
  }
};

aspect MatchScopeWithin {

  advice call("void foo()") && within("A") : before() { // 2 matches
    cout << "within(\"A\"): " << JoinPoint::signature() << " at line " << tjp->line() << endl;
    ++matches;
  }

  advice call("void foo()") && within("A::B") : before() { // 2 matches
    cout << "within(\"A::B\"): " << JoinPoint::signature() << " at line " << tjp->line() << endl;
    ++matches;
  }

  advice call("void foo()") && within("A::B::C::InC") : before() { // should only match at line 14 (not 18) -> 1 match
    cout << "within(\"A::B::C::InC\"): " << JoinPoint::signature() << " at line " << tjp->line() << endl;
    ++matches;
  }

  advice call("void foo()") && within("A::B::C::InC::Inner") : before() { // 1 match
    cout << "within(\"A::B::C::InC::Inner\"): " << JoinPoint::signature() << " at line " << tjp->line() << endl;
    ++matches;
  }

};
