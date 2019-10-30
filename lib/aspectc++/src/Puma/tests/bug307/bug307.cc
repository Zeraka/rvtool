namespace {
  void f() {}
}
namespace {
  void f() {} // should be an error: redefinition of f()!
}
    