#include <iostream>

using namespace std;

int a = 42;
int b = 3;

class T {
public:
  int a;
  static int b;
} t;
int T::b;

namespace Q {
  class R {
  public:
    int a;
    static int b;
  } r;
  int a;
}
int Q::R::b;
Q::R r;

int main() {
  {int x = a;}
  {int x = b;}
  {int x = t.a;}
  {int x = T::b;}
  {int x = Q::r.a;}
  {int x = r.a;}
  {int x = Q::R::b;}
  {int x = Q::a;}

  return 0;
}
