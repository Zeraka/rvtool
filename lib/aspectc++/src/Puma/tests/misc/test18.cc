// test if for data members used in const/volatile methods of the 
// same or a derived class or on a const/volatile class object 
// the corresponding expression (SimpleName) is resolved to get 
// a const/volatile type

int i, j = i;
void f() { j = i; }
struct X {
  int k;
  static int l;
  void g() { j = i; j = k; j = l; X o; j = o.k; j = o.l; const X co; j = co.k; j = co.l; }
  void h() const { j = i; j = k; j = l; X o; j = o.k; j = o.l; const X co; j = co.k; j = co.l; }
  void x() volatile { j = i; j = k; j = l; X o; j = o.k; j = o.l; const X co; j = co.k; j = co.l; }
  void y() const volatile { j = i; j = k; j = l; X o; j = o.k; j = o.l; const X co; j = co.k; j = co.l; }
  static void z() { j = i; j = k; j = l; X o; j = o.k; j = o.l; const X co; j = co.k; j = co.l; }
};
struct Y : X {
  void g() { j = i; j = k; j = l; Y o; j = o.k; j = o.l; const Y co; j = co.k; j = co.l; }
  void h() const { j = i; j = k; j = l; Y o; j = o.k; j = o.l; const Y co; j = co.k; j = co.l; }
  void x() volatile { j = i; j = k; j = l; Y o; j = o.k; j = o.l; const Y co; j = co.k; j = co.l; }
  void y() const volatile { j = i; j = k; j = l; Y o; j = o.k; j = o.l; const Y co; j = co.k; j = co.l; }
  static void z() { j = i; j = k; j = l; Y o; j = o.k; j = o.l; const Y co; j = co.k; j = co.l; }
};
