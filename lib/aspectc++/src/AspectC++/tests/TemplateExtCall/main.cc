#include <stdio.h>
#include <stdlib.h>

aspect AllTracer {
  pointcut builtin_ops() = call("bool operator <(int,int)") || call("int operator -(int,int)") || call("int operator ++(int &,int)");
  pointcut the_calls () = call ("% ...::%(...)") && !call("% printf(...)") && ! builtin_ops();
  advice the_calls () : before () {
    printf ("\n--- \"%s\" ---\n", JoinPoint::signature ());
  }
};

aspect ArrayTracer {
  advice call ("% Array::%(...)") : before () {
    printf ("ArrayTracer for any Array: \"%s\"\n", JoinPoint::signature ());
  }
  advice call ("% Array<int,4>::%(...)") : before () {
    printf ("ArrayTracer for <int,4>: \"%s\"\n", JoinPoint::signature ());
  }
  advice call ("% Array<%,3>::%(...)") : before () {
    printf ("ArrayTracer for <%%,3>: \"%s\"\n", JoinPoint::signature ());
  }
  advice call ("% Array<long,%>::%(...)") : before () {
    printf ("ArrayTracer for <long,%%>: \"%s\"\n", JoinPoint::signature ());
  }
};

aspect MaxTracer {
  advice call ("% ...::Max(...)") : before () {
    printf ("MaxTracer for any Max(...): \"%s\"\n",
	    JoinPoint::signature ());
  }
  advice call ("% Max<int>(...)") : before () {
    printf ("MaxTracer for Max<int>(...): \"%s\"\n",
	    JoinPoint::signature ());
  }
};

template <typename T, int I> class Array {
  T _data[I];
public:
  T get (int i) const { return _data[i]; }
  void set (int i, const T &d) { _data[i] = d; }
};

template <typename T> T Max(const T& v1, const T& v2) {
  return v1 > v2 ? v1 : v2;
}

namespace HardMatch {
  template <typename T> T Max(const T& v1, const T& v2) {
    return v1 > v2 ? v1 : v2;
  }
}

template <typename T> T* alloc () { return new T; }
template <typename T> void dealloc (T *obj) { delete obj; }

template <int V> struct Sys {
	template <typename T> struct Mem {
		static T *alloc () { return new T; }
		static void dealloc (T* obj) { delete obj; }
	};
};

template<int S> class HeapInt {};

template<int S> class BestFit : public HeapInt<S> {
public:
  void* Alloc(int s) { return malloc (s); }
  void Dealloc(void *p) { free (p); }
};

template<int S, template<int> class P, int BS = 0> class Heap : public P<S>
{
	typedef P<S> Policy;
public:
	void* Alloc(int s) { return Policy::Alloc(s); }
	void Dealloc(void *p) { Policy::Dealloc(p); }
};


struct C {
	C () { printf ("C constructor\n"); }
	~C () { printf ("C destructor\n"); }
};

int main () {
  Array<int, 4> data;
  Array<long, 5> data_long;
  Array<float, 3> data_3;

  printf ("TemplateExtCall: tracing template arg dependant calls\n");
  printf ("=====================================================\n");
  printf ("methods of class template instances\n");
  for (int i = 0; i < 4; i++)
    data.set (i, 4 - i);
  data.get (3);
  data_long.set (2, 2L);
  data_3.set (0, 3.14f);
  data_3.get (0);
  int max = Max (1, 2);
  double harder_max = HardMatch::Max (2.3, 4.5);
  printf ("-----------------------------------------------------\n");
  printf ("static template functions/member functions\n");
  C *ptr1 = alloc<C> ();
  dealloc (ptr1);
  C *ptr2 = Sys<42>::Mem<C>::alloc ();
  Sys<42>::Mem<C>::dealloc (ptr2);
// does not work yet!!!
//   printf ("-----------------------------------------------------\n");
//   printf ("allocation with help of template template parameter\n");
//   Heap<10, BestFit> a;
//   void *mem = a.Alloc(10);
//   a.Dealloc (mem);
  printf ("=====================================================\n");
}
