#include <iostream>
using namespace std;

class Cyg_Mqueue {
public:
    struct qentry {
        struct qentry *next;
    };

    struct qentry *q;
};

typedef double (*fnt)( int );

double test( int a ) {
  return a / 2.0;
}

fnt test2() {
  std::cout << "H";
  return &test;
}

int main() {
  Cyg_Mqueue mq;
  mq.q = 0;
  
  test2();
  return 0;
}

aspect Setter {
  advice set("% Cyg_Mqueue::%") : before() {
    cout << tjp->signature() << endl;
  }
};

aspect Caller {
  advice call("% Cyg_Mqueue::%(...)") : before() {
    cout << tjp->signature() << endl;
  }
};

aspect GenericCaller {
  advice call("% %(...)"): before() { std::cout << tjp->signature() << std::endl; (*tjp->entity())(); }
};

