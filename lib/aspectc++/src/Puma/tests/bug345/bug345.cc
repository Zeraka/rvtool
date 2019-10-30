template <class T>
struct X {
  friend void dont_instantiate_twice() {}
};
  
X<int> x1;     // OK
//X<double> x2;  // redefinition of dont_instantiate_twice
