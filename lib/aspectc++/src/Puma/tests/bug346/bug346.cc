template <class T>
class X {
  friend class Y;
};
 
X<int> x;  // introduces Y into file scope
  
Y* p;      // OK
