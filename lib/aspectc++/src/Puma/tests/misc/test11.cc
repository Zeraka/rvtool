/****************************************
 ** recursive template instantiation:  **
 ** the well-known factorial example   **
 ****************************************/

template <int I> 
struct fac {
  enum { VAL = I * fac< I - 1 >::VAL };
};

template <> 
struct fac<1> {
  enum { VAL = 1 };
};

int main() {
  int i = fac<10>::VAL;
}
