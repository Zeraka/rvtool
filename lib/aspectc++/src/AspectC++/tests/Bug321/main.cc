#include <iostream>
using namespace std;

#include <stdio.h>
#include <stdarg.h>


int add(int zahlen, ...);

int add(int zahlen, ...)
{
   va_list zeiger;
   int zahl;

   va_start(zeiger,zahlen);

   do{
         zahl = va_arg(zeiger,int);
         zahlen += zahl;
      }while(zahl != 0);

   va_end(zeiger);
   return zahlen;
}

int main(){
  cout << "Bug484: va_list improperly weaved\n";
  cout << "=================================\n";
  int erg;
  printf("%d\n",add(11,12,13,0));
  printf("%d\n",add(99,66,33,22,11,0));
  erg = add(10, 13, 11, 0) + add(9, 8, 11, 0);
  printf("%d\n",erg);
  cout << "=================================\n";
  return 0;
}

aspect Logger {
  advice execution("% ...::%(...)" && !"% Logger::%(...)") : around() {
    cout << "execution: " << tjp->signature() << endl;
    tjp->proceed();
  }

  advice call("% add(...)" && !"% Logger::%(...)") : around() {
    cout << "call: " << tjp->signature() << endl;
    tjp->proceed();
  }
};

