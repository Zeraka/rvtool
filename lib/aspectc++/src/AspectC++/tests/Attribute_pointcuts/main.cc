//#include <string>
#include <iostream>
#include<vector>

#define ATTR [[ac::typeDefAttr]]
#define TESTNN [[Test]]

template<typename A, int B> struct List : std::vector<A> { };

void func(int ab) {

}

int attrParam;

class Core2 {};

class Core {};

Core co;

class [[acc::aha(int, List)]] myClass {
public:
[[acc::myNNAttr(Core, co), acc::wholeFunc(4)]] int hi [[acc::myNoreturn, acc::aha(char, List), acc::hi(3, "alo" "a"), acc::myAttr(8, func)]] [[acc::myAttr(4, func)]] (const int a, const char b[], int c) {
 return c;
}
};

int main() {
  myClass mC;
  mC.hi(5, "as", 0);
  myClass mC2;
  return 0;
}
