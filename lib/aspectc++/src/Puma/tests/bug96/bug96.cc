template <class T>
class X {
public:
  void g(T);

  template<class S>
  void g(S);
};

// T => X<T> 
template <class T>
void X<T>::g(T) {}

// T => X<T> 
// S => g<S> 
template <class T>
template <class S>
void X<T>::g(S) {}
