#ifndef __falsefriend_h__
#define __falsefriend_h__

#include "singleton.h"

// Example implementation of a "false friend".
// Being a friend of Singleton, FalseFriend has got access to the
// private methods of that class.
// Although objects of class Singleton should only be created
// by calling Singleton::getInstance(), FalseFriend misuses its
// privileges and directly calls the constructor, thus creating
// additional Singleton objects.

class FalseFriend {
 public:
  FalseFriend() { Singleton s1; Singleton s2(s1); }
};

#endif
