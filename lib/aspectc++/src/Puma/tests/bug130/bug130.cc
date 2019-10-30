template<bool V> 
struct X {
  X(){}
  ~X(){}
};

template<>
X<true>::X();    // <= fails

template<>
X<false>::~X();  // <= fails
