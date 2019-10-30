#include <iostream>
using namespace std;

int main() {
  // If clang 3.7.1 is used, the register builtin operator member function 
  // is called two times for the same operator inside an array init expr.
  int i = 0;
  int* a[] = { &i };
  int a2[] = { +i };
  int a3[] = { i + i };
  int a4[] = { ++i };
  // ...
  return 0;
}

aspect TestExecution {
  advice  builtin("% ...::%(...)") : before () {
    cout << tjp->signature() << " at line " << tjp->line() << endl;
  }
};
