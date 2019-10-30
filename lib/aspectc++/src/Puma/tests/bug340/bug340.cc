template<class S>
void foo(S&) {}

void bar() {
  char c;
  foo(c);   // instantiated as foo(int&) instead of foo(char&)
}

template<class T> 
void g(const T*) {
}

template<class T> 
void f(const T*) {
}

template<class T> 
void f(T*) {
}

int *p1;
const int *p2;

void s() {
  f(p1);     // f<int>(int *)
  f(p2);     // f<int>(const int *)
  g(p1);     // g<int>(const int *)
}
