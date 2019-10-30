template <template<class T> class X> 
struct A {};

template<class T> 
struct B {};

template<class T,class S> 
struct C {};

template <template<class T> class X> 
void f(A<X>) {}

template<template<class T1,class T2> class T> 
void l(T<int,int>) {}  

template<template<class T1,class T2> class T, class S> 
void l(T<int,S>) {}  

int main() {
  A<B> ab;
  f(ab);         // calls f(A<B>)

  C<int,int> z;
  l(z);          // calls l(C<int,int>)
}
