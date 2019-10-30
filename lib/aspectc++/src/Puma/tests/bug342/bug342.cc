template<bool V> 
struct X {
};

X<true> x;  // instantiated as X<1> instead of X<true>
