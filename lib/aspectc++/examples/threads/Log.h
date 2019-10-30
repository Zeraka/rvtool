#ifndef __Log_h__
#define __Log_h__

#include <stdio.h>

class Log {
public:
  void print (int indent, bool enter, const char *text) {
    for (int i = 0; i < indent; i++) printf (" ");
    if (enter) printf ("> "); else printf ("< ");
    printf ("%s\n", text);
  }

  static Log &instance () {
    static Log log;
    return log;
  }
};

#endif //  __Log_h__
