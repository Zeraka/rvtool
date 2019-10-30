class X {
public:
  X(){}
  X(const X&){}
  X &operator=(const X& that) {
    if (this != &that) {
      this->X::~X();
      this->X::X(that);  // error, constructors have no name
    }
  }
};
