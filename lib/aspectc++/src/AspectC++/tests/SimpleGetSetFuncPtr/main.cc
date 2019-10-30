#include <iostream>
using namespace std;

void my_int_func(int x) {
  cout << "called with x=" << x << endl;
}

class Container {
  void (*foo)(int);

public:
  Container() {
    foo = &my_int_func;
  }

  void bar(int x) {
    foo(x);
  }
};

int main() {
  Container c;
  c.bar(42);
}

aspect GetSetFuncPtr {
  // matching any member access
  advice get("% Container::%") : before() {
    cout << "GET: " << JoinPoint::signature() << " at line " << tjp->line() << endl;
  }

  advice set("% Container::%") : before() {
     cout << "SET: " << JoinPoint::signature() << " at line " << tjp->line() << endl;
  }

  // matching only the function pointer 'foo'
  advice get("void (*Container::foo)(int)") : before() {
    cout << "GET of foo at line " << tjp->line() << endl;
  }

  advice set("void (*Container::foo)(int)") : before() {
    cout << "SET of foo at line " << tjp->line() << endl;
  }

  // matching any function pointer
  advice get("% (*...::%)(...)") : before() {
    cout << "GET of any function pointer: " << JoinPoint::signature() << " at line " << tjp->line() << endl;
  }

  advice set("% (*...::%)(...)") : before() {
    cout << "SET of any function pointer: " << JoinPoint::signature() << " at line " << tjp->line() << endl;
  }
};

