#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUMTHREADS 3

pthread_key_t tls_aspects_key;

class Thread {
public:
  virtual void run () = 0;
};

class TestClass : public Thread {
  void f1 () { f2 (); }
  void f2 () { }
public:
  virtual void run () { f1(); }
};

void *thread_start(void*) {
  TestClass thread_object;
  thread_object.run ();
}

#include "ThreadSafeLogging.ah"

int main () {
  pthread_t thread[NUMTHREADS];

  // initialize the thread-safe logging aspect *before* forking threads
  ThreadSafeLogging::init ();
 
  printf("Create/start threads\n");
  for (int i = 0; i < NUMTHREADS; i++) { 
    if (pthread_create(&thread[i], NULL, thread_start, NULL)) {
      perror("thread_create failed");
      exit(1);
    }
  }
 
  for (int i = 0; i < NUMTHREADS; i++) {
    if (pthread_join(thread[i], NULL)) {
      perror("thread_join failed");
      exit(1);
    }
  }

  printf("done\n");
  return 0;
}
