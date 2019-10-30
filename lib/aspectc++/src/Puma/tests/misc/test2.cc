/****************************************
 ** semantic analysis of the argument  **
 ** list of construction expressions   **
 ****************************************/

class X {
  X(int,int);
};

X x1(1,2);              // <= argument types and values must be
X x2 = X(1,2);          // <= analysed and shown in the syntax
X *x3 = new X(1,2);     // <= tree
