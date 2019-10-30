#include <pthread.h>
class A {
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // crashes the parser
};
