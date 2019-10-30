#include <iostream>

using namespace std;

void test() {};

class T {
public:
  void test() {};
  static void testS() {};
} t;

namespace Q {
  class R {
  public:
    void test() {};
    static void testS() {};
  } r;
  void test() {};
}
Q::R r;

int main() {
  test();
  t.test();
  T::testS();
  Q::r.test();
  r.test();
  Q::R::testS();
  Q::test();

  return 0;
}
