#include <stdio.h>

class Container {
  Container* thread_list;

public:
  Container() : thread_list(this) {}

  Container* get_list_head() {
    return thread_list?thread_list->thread_list:0;
  }
};

int main() {
  Container c;
  c.get_list_head();
  return 0;
}


aspect ConditionalGet {
  advice get("% ...::%") : before() {
    printf("GET: %s\n", JoinPoint::signature());
  }
};

