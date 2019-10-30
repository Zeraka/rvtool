namespace DataFlowAnalysis {

//----------------------- Member Expression Test ------------------------------

class Foo {
public:
  Foo& ref;
  Foo* ptr;
  Foo* const const_ptr;
  Foo() : ref(*this), ptr(this), const_ptr(this) {}

  void f() {}
  void g(Foo *ptr) {}
} foo;

void member_expressions() {
  foo.f(); // ID: 2
  foo.f(); // ID: 2

  foo.ref.f(); // ID: 3
  foo.ref.f(); // ID: 3

  foo.ptr->f(); // ID: 0
  foo.ptr->f(); // ID: 0

  foo.const_ptr->f(); // ID: 4
  foo.const_ptr->f(); // ID: 4

  foo.ref.ref.f(); // ID: 5
  foo.ref.ref.f(); // ID: 5

  foo.ref.ptr->f(); // ID: 0
  foo.ref.ptr->f(); // ID: 0

  foo.ref.const_ptr->f(); // ID: 6
  foo.ref.const_ptr->f(); // ID: 6

  Foo* foo_ptr = new Foo();
  foo_ptr->f(); // ID: 7
  foo_ptr->f(); // ID: 7

  foo_ptr->ref.f(); // ID: 0
  foo_ptr->ref.f(); // ID: 0

  Foo* const foo_const_ptr = foo_ptr;
  foo_const_ptr->f(); // ID: 7
  foo_const_ptr->f(); // ID: 7

  foo_const_ptr->ref.f(); // ID: 8
  foo_const_ptr->ref.f(); // ID: 8

  foo_const_ptr->ptr->f(); // ID: 0
  foo_const_ptr->ptr->f(); // ID: 0

  foo_const_ptr->const_ptr->f(); // ID: 9
  foo_const_ptr->const_ptr->f(); // ID: 9

  Foo& foo_ref = *foo_const_ptr;
  foo_ref.f(); // ID: 7
  foo_ref.f(); // ID: 7

  foo_ref.ref.f(); // ID: 10
  foo_ref.ref.f(); // ID: 10

  foo_ref.ptr->f(); // ID: 0
  foo_ref.ptr->f(); // ID: 0

  foo_ref.const_ptr->f(); // ID: 11
  foo_ref.const_ptr->f(); // ID: 11
}

//--------------------------- For-Loop Test -----------------------------------

void for_loop_test(int _a, char** _b) {
  Foo b;
  Foo* ptr = &b;

  b.f(); // ID: 2
  ptr->f(); // ID: 2

  for(int i=0; i<10; i++) {
    ptr->f(); // ID: 5
    ptr->f(); // ID: 5
    ptr++;
  }

  b.f(); // ID: 2
  ptr->f(); // ID: 4
}

//------------------------- Do-While-Loop Test --------------------------------

void do_while_loop_test() {
  Foo b;
  Foo* ptr = &b;

  b.f(); // ID: 2
  ptr->f(); // ID: 2
  int i = 0;

  do {
    i++;
    ptr->f(); // ID: 4
    ptr->f(); // ID: 4
    ptr++;
  } while (i<10);

  b.f(); // ID: 2
  ptr->f(); // ID: 3
}

//--------------------------- While-Loop Test ---------------------------------

void while_loop_test(void* __ch_data, const char* __buf, int __len)
{
  Foo b;
  b.f(); // ID: 2

  while(__len-- > 0)
    b.f(); // ID: 2

  b.f(); // ID: 2
}

//-------------------------- Address-Operator-Test ----------------------------

void addr_op_test() {
  Foo b;
  Foo* ptr = &b;

  b.f(); // ID: 2
  ptr->f(); // ID: 2
  Foo *const * double_ptr = &ptr; // kills the pointer

  ptr->f(); // ID: 0
  (**double_ptr).f(); // ID: 0

  Foo* ptr2 = &b;
  ptr2->f(); // ID: 2
  ptr2->g( *(&ptr2) ); // ID: 2
  ptr2->f(); // ID: 0

  Foo* ptr3 = &b;
  (*(&ptr3))->f(); // ID: 2
  ptr3->f(); // ID: 0

  Foo *ptr_a, *ptr_b;
  ptr_a = &b;
  ptr_b = *(&ptr_a); // kill the pointer and transfer its id
  ptr_a->f(); // ID: 0
  ptr_b->f(); // ID: 2

  Foo* ptr_c = *(&ptr_b); // kill the pointer and transfer its id
  ptr_b->f(); // ID: 0
  ptr_c->f(); // ID: 2
}


//---------------------- Reference-to-Pointer Test ----------------------------

class RefToPtr {
public:
  int f(RefToPtr*& _ff) { return 0; }
  void g() {};
};

void ref_to_ptr_test() {
  RefToPtr foo;
  RefToPtr* ptr = &foo;

  foo.g(); // ID: 2
  ptr->g(); // ID: 2

  ptr->f(ptr); // ID: 2
  ptr->g(); // ID: 0

  RefToPtr* ptr2 = &foo;
  ptr2->g(); // ID: 2

  RefToPtr& ref = *ptr2;
  ref.g(); // ID: 2

  RefToPtr*& ref_to_ptr = ptr2; // kills the pointer
  ptr2->g(); // ID: 0
  ref_to_ptr->g(); // ID: 0
}

//----------------------- Nested-Call-Order Test ------------------------------

void nested_call_order() {
  Foo foo;
  Foo* ptr = &foo;

  ptr->g(                 // ID: 0
    ( ptr->f(),           // ID: 0 (or 2)
      ptr->g(*(&ptr)),    // ID: 0 (or 2)
      ptr->f(), &foo) );  // ID: 0
}

//----------------------- Conditional-Operator Test ---------------------------

void conditional_operator() {
  Foo foo, foo2;
  Foo* ptr = &foo;
  Foo* ptr2 = &foo2;
  (1 ?  ptr : ptr2)->f(); // ID: 2
  Foo* ptr3 = (2 ?  ptr : ptr2);
  ptr3->f(); // ID: 2
  ( (&conditional_operator > &nested_call_order) ?  ptr : ptr2)->f(); // ID: 0
  ptr3 = ( (&conditional_operator > &nested_call_order) ?  ptr : ptr2);
  ptr3->f(); // ID: 4
}

//--------------------------- Compound Test -----------------------------------

void compound_test(Foo* param) {
  param->f(); // ID: 2

  Foo foo, foo2;
  (foo.f(),    // ID: 3
    foo2).f(); // ID: 4

  Foo* ptr = &foo;
  (ptr = &foo2)->f(); // ID: 4
  ptr->f(); // ID: 4

  Foo *ptr2, *ptr3;
  ptr = ptr2 = ptr3 = &foo;
  ptr->f(); // ID: 3
  ptr2->f(); // ID: 3
  ptr3->f(); // ID: 3
}

//--------------------------- Increment/Decrement Test ------------------------

void increment_decrement_op() {
  Foo array[2];
  Foo* ptr_1 = &array[0];
  ptr_1->f(); // ID: 2
  (ptr_1++)->f(); // ID: 0
  ptr_1->f(); // ID: 3

  Foo* ptr_2 = &array[1];
  ptr_2->f(); // ID: 4
  (--ptr_2)->f(); // ID: 0
  ptr_2->f(); // ID: 5

  (ptr_2+1)->f(); // ID: 0
}

//-------------------------------- Global-Pointer Test ------------------------

Foo glob;
Foo& glob_ref = glob;
Foo* glob_ptr = &glob;
Foo* const glob_const_ptr = &glob;

class MyStaticContainer {
public:
  static Foo* ptr;
  static Foo* const const_ptr;
  static Foo& ref;
};

Foo* MyStaticContainer::ptr = &glob;
Foo* const MyStaticContainer::const_ptr = &glob;
Foo& MyStaticContainer::ref = glob;

void global_pointer() {
  glob.f(); // ID: 2
  glob_ref.f(); // ID: 3
  glob_ptr->f(); // ID: 0
  glob_const_ptr->f(); // ID: 4

  MyStaticContainer::ptr->f(); // ID: 0
  MyStaticContainer::const_ptr->f(); // ID: 5
  MyStaticContainer::ref.f(); // ID: 6
}

//----------------------------- If-Then-Else Test -----------------------------

void if_then_else_test() {
  Foo foo1;
  Foo foo2;
  Foo* ptr = 0;
  if (&increment_decrement_op > &global_pointer) {
    ptr = &foo1;
    ptr->f(); // ID: 2
  }
  else {
    ptr = &foo2;
    ptr->f(); // ID: 3
  }
  ptr->f(); // ID: 5

  for(int i=0; i<10; ++i) {
    ptr = 0;
    if (&increment_decrement_op < &global_pointer) {
      ptr = &foo1;
      ptr->f(); // ID: 2
    }
    else {
      ptr = &foo2;
      ptr->f(); // ID: 3
    }
    ptr->f(); // ID: 7
  }
  ptr->f(); // ID: 8

  ptr = &foo1;
  if (&increment_decrement_op > &global_pointer) {
    goto set;
  }
  else {
    goto exec;
  }
set:
  ptr = &foo2;
exec:
  ptr->f(); // ID: 10
}

} // namespace DataFlowAnalysis 


//-------------------------- Print project model ------------------------------

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

int main() {
  std::ifstream input("repo.acp");
  std::string line;

  bool namespace_dflow = false;

  while ( std::getline( input, line ) ) {

    if ( (line.find("Namespace") != std::string::npos) &&
         (line.find("name=") != std::string::npos) ) {
      if ( line.find("\"DataFlowAnalysis\"") != std::string::npos )
        namespace_dflow = true;
      else
        namespace_dflow = false;
    }

    if (namespace_dflow) {
      // print function name
      if (line.find("<Function") != std::string::npos) {
        std::stringstream ss(line);
        std::string item;
        while (std::getline(ss, item, ' ')) { // split by whitespace
          if (item.find("name=") != std::string::npos) {
            std::stringstream name(item);
            std::getline( name, item, '>'); // remove '>'
            std::cout << "Function " << item << std::endl;
            break;
          }
        }
      }
      // print call target_object_lid
      else if (line.find("<Call") != std::string::npos) {
        std::stringstream ss(line);
        std::string item;
        bool target_object_lid = false;
        while (std::getline(ss, item, ' ')) { // split by whitespace
          if (item.find("target_object_lid=") != std::string::npos) {
            std::stringstream name(item);
            std::getline( name, item, '>'); // remove '>'
            std::cout << "  " << item << std::endl;
            target_object_lid = true;
            break;
          }
        }
        if (target_object_lid == false) {
          std::cout << "  target_object_lid=\"0\"" << std::endl;
        }
      }
    }

  }

  return 0;
}

