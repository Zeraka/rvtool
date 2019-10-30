#include <stdio.h>

struct Basic_list_policy_Timeout
{
  typedef Basic_list_policy_Timeout **Type;
};

class Basic_list_Timeout
{

public:
  class Iterator
  {
  private:
    typedef Basic_list_policy_Timeout::Type Internal_type;

  public:


    Internal_type get_c_const() const { return _c; }

    //template<typename T>
    //Internal_type ttest(T o) const { return _c; }

    //Internal_type arg_test(Internal_type o) const { return _c; }
    
  private:

    Internal_type _c;

  public:

    Internal_type _internal;
    
  };
  
public:
  static Basic_list_policy_Timeout::Type __get_internal(Iterator const &i) { return i.get_c_const(); }

  //static Basic_list_policy_Timeout::Type __get_ttest(Iterator const &i) { return i.ttest(i.get_c_const()); }

  //static Basic_list_policy_Timeout::Type __get_arg_test(Iterator const &i) { return i.arg_test(i.get_c_const()); }
};

int main() {
  printf ("Bug589: Entity type uses private types instead of deduced types\n");
  printf ("===============================================================\n");
  Basic_list_Timeout::Iterator i;
  Basic_list_policy_Timeout::Type v1 = Basic_list_Timeout::__get_internal(i);
  Basic_list_policy_Timeout::Type v2 = i._internal;
  printf ("Fine if it compiles without errors\n");
  printf ("===============================================================\n");
}

aspect Buggy {
  // This advice doesn't work yet
  // advice get("% ...::%") : before() {
  //   tjp->entity();
  // }
  advice call("% ...::%(...)") : before() {
    tjp->target();
  }
};

