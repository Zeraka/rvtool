class X {};
template<class I> struct T { class TC; };
template<class I> struct T<I>::TC { typedef int INT; typedef typename I::BLA i;};
T<X>::TC *tcp; // Instantiation of T<X>
T<X>::TC tc;   // Instantiation of T<X>::TC, should produce an error due to unknown BLA
