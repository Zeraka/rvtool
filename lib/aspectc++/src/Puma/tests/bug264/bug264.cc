class X {};

void foo(X* x) {
  foo(&X());    // taking address of temporary, should be only a warning
}
