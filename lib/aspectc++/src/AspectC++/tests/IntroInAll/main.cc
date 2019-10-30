// foo.h
#include <vector>
#include <stdio.h>

#define std_default_vector    std::vector

template <class T>
class FOO
{
public:
  typedef std_default_vector<T> vector;
  class MyVec;
  class BlaBla {};
};

template <class T>
class std_vector : public FOO<T>::vector { };

template<class T>
class MyStdVector : public std_default_vector<T>
{
};

template <class T>
class FOO<T>::MyVec : public std::vector<T>
{
};

#define std_vector MyStdVector

class GOO;

typedef std_vector<GOO*> GooList;

class GOO {
public:
  GOO(void) {};
  GOO(const GOO &, int = 0) {};
  virtual void m(void);
  static void m(GooList& glist);
  virtual int m2(void);
};

// foo.cpp

int main(int argc, char **argv)
{
  printf ("IntroInAll: tests intros into \"%%\"\n");
  printf ("=============================================================\n");
  printf ("it is OK if it compiles!\n");
GooList glist;
GOO::m(glist);
  printf ("=============================================================\n");
}

void GOO::m(GooList &glist)
{
GooList::const_iterator idx = glist.begin();
for (; idx != glist.end(); idx++)
  (*idx)->m();

int sorted = 1;
for (; sorted < (int)glist.size(); sorted++)
{
  GOO* tmp = glist[sorted];
  int index = sorted - 1;
  while (index >= 0)
  {
    GOO* current = glist[index];
    if (tmp->m2() > current->m2())
      break;

    glist[index+1] = glist[index];
    index--;
  }
  glist[index+1] = tmp;
}
}

void GOO::m(void)
{
}

int GOO::m2(void)
{
return 1;
}

// foo.ah
aspect Trace {
  pointcut all() = "%";

  advice all() : slice struct { void *fAttr; };
  advice all() : slice struct {
    void *getAttr(void) {
      return fAttr;
    }
  };
  advice all() : slice struct { void setAttr(void* attr) {
      fAttr = attr;
    }
  };
  advice construction (all ()) : after () {
    void *attr = 0;
// compute attr
    tjp->target()->setAttr(attr);
  }
  advice call ("virtual % ...::%(...)") : before () {
    void *selfAttr = tjp->target()->getAttr();
    void *myAttr = (void *)1;
    // compare selfAttr & myAttr
  }
};

