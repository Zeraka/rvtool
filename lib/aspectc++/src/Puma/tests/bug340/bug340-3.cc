template<class T> struct A { A(){}; };
template<class T,class S> struct B { B(){}; };
template<class T> void f(T, char = 1) {}
template<class T> void f(T*, long = 2) {}
template<class T> void f(const T*, short = 3) {}
template<class T> void g(T) {}
template<class T> void g(T&) {}
template<class T> void h(const T&) {}
template<class T> void h(A<T>&) {}
template<class T> void l(B<T,T>&) {}
template<class T> void l(B<T,int>&) {}

int main() {
  const int *p; 
  f(p);                 // f(const T*) is more specialized than f(T) or f(T*)

  float x;  
  g(x);                 // Ambiguous: g(T) or g(T&)

  A<int> z; 
  h(z);                 // overload resolution selects h(A<T>&)

  const A<int> z2;
  h(z2);                // h(const T&) is called because h(A<T>&) is not callable

  B<int,int> z3;
  l(z3);                // Ambiguous: l(B<T,int>) or l(B<T,T>)
}
