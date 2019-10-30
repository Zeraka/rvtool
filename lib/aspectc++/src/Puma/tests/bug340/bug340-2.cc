//#include <iostream>

template<typename T1, typename T2> 
struct S {};

template<typename C, typename T>
void foo(S<C,T>&, C*) {
  //std::cout<<__PRETTY_FUNCTION__<<std::endl;
}

template<typename T>
void foo(S<char,T>, char*) {
  //std::cout<<__PRETTY_FUNCTION__<<std::endl;
}

int main() {
  S<char,int> s;
  char *p;
  foo(s,p);
  return 0;
}
