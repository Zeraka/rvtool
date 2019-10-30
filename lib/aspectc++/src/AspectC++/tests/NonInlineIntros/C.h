#ifndef __C_h__
#define __C_h__

class C {
public:
  template <int I> void a_template_function ();

  void f (); // link-once code element
};

// this should no be considered as the link-once code element
template <int I>
void C::a_template_function () {}

#endif // __C_h__
