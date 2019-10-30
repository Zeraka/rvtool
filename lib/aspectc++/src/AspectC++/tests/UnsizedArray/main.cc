#include <iostream>
using namespace std;

class MyClass {
public:
  int i;
  // non-static introductions should be inserted before the unsized array
  char data[];
  static int fourtytwo;
};

int MyClass::fourtytwo = 42;


aspect IntroAspect {
  advice "MyClass" : slice class { public: int letter; };

  advice construction("MyClass") : before() { tjp->that()->letter = 56; }
};


unsigned char mempool[100];

int main() {
  MyClass *m = (MyClass *)&mempool[0];
  new(&mempool[0]) MyClass; // placement new

  cout << "member offsets (should be distinct):" << endl
       << "====================================" << endl;
  if((AC::TypeInfo<MyClass>::Member<0>::spec & AC::SPEC_STATIC) == 0)
    cout << AC::TypeInfo<MyClass>::Member<0>::name() << ": " << ((int*)AC::TypeInfo<MyClass>::Member<0>::pointer(m) - (int*)m) << endl;
  if((AC::TypeInfo<MyClass>::Member<1>::spec & AC::SPEC_STATIC) == 0)
    cout << AC::TypeInfo<MyClass>::Member<1>::name() << ": " << ((int*)AC::TypeInfo<MyClass>::Member<1>::pointer(m) - (int*)m) << endl;
  if((AC::TypeInfo<MyClass>::Member<2>::spec & AC::SPEC_STATIC) == 0)
    cout << AC::TypeInfo<MyClass>::Member<2>::name() << ": " << ((int*)AC::TypeInfo<MyClass>::Member<2>::pointer(m) - (int*)m) << endl;
  if((AC::TypeInfo<MyClass>::Member<3>::spec & AC::SPEC_STATIC) == 0)
    cout << AC::TypeInfo<MyClass>::Member<3>::name() << ": " << ((int*)AC::TypeInfo<MyClass>::Member<3>::pointer(m) - (int*)m) << endl;

  cout << endl
       << "member values:" << endl
       << "==============" << endl;

  if((AC::TypeInfo<MyClass>::Member<0>::spec & AC::SPEC_STATIC) == 0)
    cout << AC::TypeInfo<MyClass>::Member<0>::name() << ": " << *AC::TypeInfo<MyClass>::Member<0>::pointer(m) << endl;
  if((AC::TypeInfo<MyClass>::Member<1>::spec & AC::SPEC_STATIC) == 0)
    cout << AC::TypeInfo<MyClass>::Member<1>::name() << ": " << *AC::TypeInfo<MyClass>::Member<1>::pointer(m) << endl;
  if((AC::TypeInfo<MyClass>::Member<2>::spec & AC::SPEC_STATIC) == 0)
    cout << AC::TypeInfo<MyClass>::Member<2>::name() << ": " << *AC::TypeInfo<MyClass>::Member<2>::pointer(m) << endl;
  if((AC::TypeInfo<MyClass>::Member<3>::spec & AC::SPEC_STATIC) == 0)
    cout << AC::TypeInfo<MyClass>::Member<3>::name() << ": " << *AC::TypeInfo<MyClass>::Member<3>::pointer(m) << endl;

  cout << endl;

  cout << "write to m->data[] and use copy ctor (m2's data should be zeros):" << endl
       << "=================================================================" << endl;
  for(int i=0; i<10; i++) {
    m->data[i] = '1';
  }
  m->data[10] = '\0';

  // test (default) copy ctor
  MyClass *m2 = (MyClass *)&mempool[50];
  *m2 = *m;

  // show that default copy ctor doesn't copy unsized arrays
  cout << "m->data[]: ";
  for(int i=0; i<10; i++) {
    cout << (short)m->data[i];
  }
  cout << endl;
  cout << "m2->data[]: ";
  for(int i=0; i<10; i++) {
    cout << (short)m2->data[i];
  }
  cout << endl;


  cout << endl
       << "member values (m2's letter should be 56):" << endl
       << "=========================================" << endl;

  if((AC::TypeInfo<MyClass>::Member<0>::spec & AC::SPEC_STATIC) == 0)
    cout << AC::TypeInfo<MyClass>::Member<0>::name() << ": " << *AC::TypeInfo<MyClass>::Member<0>::pointer(m2) << endl;
  if((AC::TypeInfo<MyClass>::Member<1>::spec & AC::SPEC_STATIC) == 0)
    cout << AC::TypeInfo<MyClass>::Member<1>::name() << ": " << *AC::TypeInfo<MyClass>::Member<1>::pointer(m2) << endl;
  if((AC::TypeInfo<MyClass>::Member<2>::spec & AC::SPEC_STATIC) == 0)
    cout << AC::TypeInfo<MyClass>::Member<2>::name() << ": " << *AC::TypeInfo<MyClass>::Member<2>::pointer(m2) << endl;
  if((AC::TypeInfo<MyClass>::Member<3>::spec & AC::SPEC_STATIC) == 0)
    cout << AC::TypeInfo<MyClass>::Member<3>::name() << ": " << *AC::TypeInfo<MyClass>::Member<3>::pointer(m2) << endl;

  return 0;
}
