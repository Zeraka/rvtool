#include <cstdio>
using std::printf;
#include <set>
using std::set;

aspect SetTracer {
  advice call ("% ...::%(...)" && !"% printf(...)") : before () {
    printf ("calling %s\n", JoinPoint::signature ());
  }
};

set<int> sint;

int main () {
  printf ("STLSet: checks advice weaving for the STL set template\n");
  printf ("======================================================\n");
  sint.insert (4711);
  printf ("======================================================\n");
};
