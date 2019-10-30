/******************************************************
 ** friend declarations for class template instances **
 ******************************************************/

template <class T> 
class X {
};

template <class T>
class Y {
  friend class X<int>;
  X<int> x;
};

Y<int> y;
