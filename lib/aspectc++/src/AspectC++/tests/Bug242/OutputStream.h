#ifndef __OUTPUTSTREAM_H
#define __OUTPUTSTREAM_H

#include <stdio.h>

class OutputStream {
public:
  // uses the introduced attribute _fill
  void print_fill () { printf ("%c", _fill); }
};

#endif
