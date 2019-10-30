/*********************************************
 ** partial class template specialization   **
 ** and argument deduction against partial  **
 ** class template specialization arguments **
 *********************************************/
 
template <int I>
struct Y {
};

template <class T>
struct X {
};

template <int I>
struct X< Y<I> > {     // <= I has to be deduced to 0
  typedef Y<I> TYPE;   // <= Y<0> has to be instantiated
};

X< Y<0> > x;
