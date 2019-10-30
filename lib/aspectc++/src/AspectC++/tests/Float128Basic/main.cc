// Basic __float128 test. Types/Signatures differ between clang >= 3.9.1 
// and clang < 3.9.1 due to imitated __float128 type.

#include <iostream>
using namespace std;

class Test {
 public:
  __float128 member;
  
  __float128* mf(__float128& p) {
    return &p;
  }
  
  Test(__float128 p) : member(p) {
    
  }  
};

template <typename T>
class TestTemplate {
 public: 
  T template_member;
};

__float128 global_float = 242424.2424;

int main() {
  Test t(213.4124);
  t.member = 2323.42452;
  __float128* fp = t.mf(t.member);
  
  TestTemplate<__float128> tt;
  tt.template_member = 242424.2424;
  
  cout << sizeof(__float128) << endl;
  cout << sizeof(tt.template_member) << endl;
  cout << sizeof(*fp) << endl;
  
  __float128 result = tt.template_member + *fp;
  result = result - *fp;
  result = result * tt.template_member;
  result = result / *fp;
  result += *fp;
  result -= *fp;
  result *= *fp;
  result /= *fp;
}


aspect A {
  // TODO: Weaving does not work if " || builtin("% ...::%(...)")" is added
  advice (call("% ...::%(...)") && !call("% std::...::%(...)")) : before() {
    // TODO: Signature is not consistent over clang versions
    cout << "call" << endl;
  }
};

