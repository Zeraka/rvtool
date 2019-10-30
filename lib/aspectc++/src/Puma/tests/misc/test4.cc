/*****************************************
 ** default template argument specified **
 ** in a forward declaration instead of **
 ** at the base template declaration    **
 *****************************************/

template <class T, class S>
class X{};

template <class T, class S = int>
class X;

X<int> x;
