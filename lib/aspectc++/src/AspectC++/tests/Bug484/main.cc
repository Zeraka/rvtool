#include <iostream>
using namespace std;

int func(int result) { return result; }
int func2(int __result) { return __result; }


int main(){
  cout << "Bug484: Execution advice for function with parameter 'result' broken\n";
  cout << "====================================================================\n";
  func(42);
  func2(4711);
  cout << "====================================================================\n";
  
  return 0;
}

aspect ShadowAspect {
  advice execution("% ...::%(...)" && !"% main(...)") : around() {
    tjp->proceed();
    cout << tjp->signature() << " -> " << *tjp->result() << endl;
  }
};
