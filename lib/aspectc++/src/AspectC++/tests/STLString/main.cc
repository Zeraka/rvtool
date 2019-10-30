#include <cstdio>
using std::printf;
#include <string>

aspect StringTracer {
 public:
  advice call ("% foo(... :: basic_string<char,...>)" && !"% printf(...)") : before () {
    printf ("calling %s\n", JoinPoint::signature ());
  }
};

//void foo(std::string s) {
void foo(std::basic_string<char> s) {
       printf("in foo(%s)\n", s.c_str ());
}

int main () {
  printf ("StdString: checks advice weaving for the STL string class\n");
  printf ("=========================================================\n");
  foo ("this is a string");
  printf ("=========================================================\n");
}
