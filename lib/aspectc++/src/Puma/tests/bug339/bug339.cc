template<template<int I> class T> 
class Host {
  typedef T<42> T42;
};

template<int I> 
class H {
};

template<int I> 
class P : H<I> {
};

Host<P> var1; // <== not instantiated!!!
