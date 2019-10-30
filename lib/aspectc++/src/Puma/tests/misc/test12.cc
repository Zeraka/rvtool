/*******************************************
 ** partial class template specialization **
 *******************************************/
 
template <class T>
struct X {
};

template <class T>
struct X<T*> {
};

template <class T>
struct X<const T*> {
  typedef T TYPE;
};

X<const int*>::TYPE x;
