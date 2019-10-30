/*******************************************
 ** 1. inner class template instantiation **
 ** 2. segmentation fault (commented out) ** 
 *******************************************/

class X;

template<class T>
struct Y {
  template<class T1>
  struct R { 
    //R() {}            <= segmentation fault 
  };
};

Y<int>::R<int> type1;
Y<int>::R<X> type2;
