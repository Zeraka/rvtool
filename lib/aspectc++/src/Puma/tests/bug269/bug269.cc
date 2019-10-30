class X {
  void bar(X* x) const {}
  void foo() const {
    this->foo();    // OK
    bar(this);      // Error
  }
};
