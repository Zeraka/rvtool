#include "Forward.h"

class Backward : public Forward {
public:
  void run () {
    Forward::introduced ();
    introduced ();
  }
};

int main () {
  Backward b;
  b.run ();
  return 0;
}
