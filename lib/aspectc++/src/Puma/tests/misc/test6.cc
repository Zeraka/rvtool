/******************************************
 ** simple class template specialization **
 ******************************************/
 
template <class T = int> 
struct X {
};

template <> 
struct X<int> {
  typedef int TYPE;
};

X<int>::TYPE x;
