#include <stdio.h>

#include "classes.h"

int main() {
  A a;
  N::B b;
  printf ("CallViaADN: Calls that require argument-dependent name lookup\n");
  printf ("=============================================================\n");
  ::declared_friend_1(a);
  declared_friend_2(a);
  defined_friend_1(a);
  ::defined_friend_2(a);
  printf ("-------------------------------------------------------------\n");
  declared_friend(b);
  defined_friend_1(b);
  N::defined_friend_2(b);
  printf ("-------------------------------------------------------------\n");
  QT::QString s1, s2;
  operator ==(s1, s2);
  Peer::TorrentPeer p1,p2;
  bool c = (p1==p2);
  printf ("=============================================================\n");
}
