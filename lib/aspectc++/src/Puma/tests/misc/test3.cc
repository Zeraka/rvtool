/***********************************************
 ** template template arguments and default   **
 ** arguments of template template parameters **
 ***********************************************/
 
template <class T>
class Y {
};

template <template <class T> class TT, template <class T> class TTT = TT>
class Z {
  TTT<int> z;
};

Z<Y> z;
