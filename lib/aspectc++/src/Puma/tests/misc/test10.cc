/***********************************************
 ** problem with type generation for instance **
 ** of fully specialized class template       **
 ***********************************************/


template <int I> 
struct fac {
  enum { VAL = 2 };
};

template <> 
struct fac<1> {      // <= fully specialized for 1
  enum { VAL = 1 };
};

template <typename T> 
class X {};

X< fac<1> > x;
