#include "classes.h"

void declared_friend_1(const A&) {}
void declared_friend_2(const A&) {}

namespace N {
  void declared_friend(const N::B&) {}
}

namespace QT {
  bool operator==(const QString&, const QString&) { return true; }
}
