#include <iostream>
#include <stdlib.h>

using namespace std;

attribute noreturn();

namespace [[ out::namespaceAttr ]] N {
}

struct [[out::classAttr()]] myClass;

struct myClass {
    [[noreturn]][[out::funcAttr]] void func [[out::otherAttr]] () {exit(0);}

    [[out::varAttr]] int var [[out::otherAttr]];
};

namespace N {
  namespace [[out::innerNsAttr]] N2 {
    void func() { }
    struct [[out::classAttr]] C {
      static void func() {}
    };
  }
}

int main()
{
    myClass obj;
    obj.var = 0;
    N::N2::func();
    N::N2::C::func();
    obj.func();

    return 0;
}


aspect out {
    attribute classAttr();
    attribute funcAttr();
    attribute varAttr();
    attribute namespaceAttr();
    attribute innerNsAttr();

    attribute otherAttr();

    pointcut all() = classAttr() || funcAttr();

    advice execution(all()) || construction(all()) : before() {
        cout << "Execution or Construction" << endl;
    }

    advice construction(otherAttr()) : before() {
        cout << "Failed: This attribute should have no affect!" << endl;
    }

    advice execution(otherAttr()) : before() {
        cout << "Execution of other Attr" << endl;
    }

    advice execution(member(classAttr() && member(namespaceAttr()))) : before() {
        cout << "Execution of funny member" << endl;
    }

    advice execution(namespaceAttr()) : before() {
        cout << "Execution of namespaceAttr" << endl;
    }

    advice execution(namespaceAttr() && !innerNsAttr()) : before() {
        cout << "Failed: This execution should be filtered!" << endl;
    }

    advice set(varAttr()) : before() {
        cout << "var set!" << endl;
    }

    advice set(otherAttr()) : before() {
        cout << "var set - other Attr!" << endl;
    }

    advice execution(noreturn()) : before() {
        cout << "This function will not return!!" << endl;
    }
};
