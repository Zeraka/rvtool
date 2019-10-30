#include "singleton.h"
#include "falsefriend.h"

int main () {
 
  Singleton& s1 = Singleton::getInstance();

  FalseFriend f;
  
  return 0;
}
