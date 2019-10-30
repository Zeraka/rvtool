/*********************************************
 ** namespace dependent generation of class **
 ** template instance argument code         **
 *********************************************/
 
namespace N1 {
  template <class T>
  class X {
  };
}

namespace N2 {
  template <class T>
  class Y {
    typedef N1::X< Y<T> > TYPE;
  };
};

N2::Y<int> y;
