class X {
protected:
  virtual void foo();
};
   
class Y : protected X {
public:
  X::foo;
};
