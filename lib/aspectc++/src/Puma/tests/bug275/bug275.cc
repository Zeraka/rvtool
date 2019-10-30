class K1;
class K2;

namespace C {
  void g (K2 *);
};
  
class K {
public:
  bool operator==(const K&) { return true; }
};
  
class K1 {
public:
  operator K () { static K k; return k; }
};
    
class K2 {
public:
  //operator K1 () { static K1 k; return k; }
};
      
void C::g (K2 *p) {
  K k;
  k = (K1)*p;  // error, no conversion from K2 to K1 defined
}
