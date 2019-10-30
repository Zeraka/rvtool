#include <iostream>
#include <map>
using namespace std;

class TestInterface {
public:
  virtual void print() = 0;
};

class MyTest : public TestInterface {
public:
  int x;

  MyTest() : x(0) {}

  virtual void print() {
    cout << "Accessing member of MyTest" << endl;
  }
};

class MyAnother : public TestInterface {
public:
  unsigned long int z;

  MyAnother() : z(0) {}

  virtual void print() {
    cout << "Accessing member of MyAnother" << endl;
  }
};

int main() {
  // MyTest
  MyTest obj;
  obj.x = 3;
  int* ptr = &obj.x;
  int y;
  y = *ptr; // get #1
  *ptr += 3; // get #2 and set #1

  int& myref = obj.x;
  myref = myref - 1;

  // MyAnother
  MyAnother other;
  other.z = 42;
  unsigned long int* oth_ptr = &other.z;
  *oth_ptr = 4711;

  // unkown:
  ptr = &y;
  *ptr = 0;
}


aspect DynGetSet {
  pointcut observe() = "% My%::%";

  map<void*, TestInterface*> var_table;

  advice ref(observe()) : before() {
    cout << "Taking reference of a member" << endl;
    cout << " REF \"" << JoinPoint::signature() << "\" : " << /*tjp->entity()*/ "<some address>" << endl << endl;
    if (tjp->target() != 0) {
      var_table[tjp->entity()] = tjp->target();
    }
  }

  advice get(alias(observe())) : before() {
    TestInterface* target = var_table[tjp->entity()];
    if (target != 0) {
      target->print();
      cout << " GET via ALIAS \"" << JoinPoint::signature() << "\" : " << *tjp->entity() << endl << endl;
    }
    else {
      cout << "UNKNOWN GET" << endl;
    }
  }

  advice set(alias(observe())) : before() {
    TestInterface* target = var_table[tjp->entity()];
    if (target != 0) {
      target->print();
      cout << " SET via ALIAS \"" << JoinPoint::signature() << "\" : " << *tjp->entity() << " -> " << *tjp->arg<0>() << endl << endl;
    }
    else {
      cout << "UNKNOWN SET" << endl;
    }
  }
};

