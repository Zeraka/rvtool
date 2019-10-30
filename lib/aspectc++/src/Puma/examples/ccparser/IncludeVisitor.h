#ifndef __INCLUDE_VISITOR__
#define __INCLUDE_VISITOR__

// Visitor to print the name of each file included.

#include "Puma/PreVisitor.h"
#include "Puma/PreTreeNodes.h"
#include "Puma/Array.h"
#include <iostream>
using namespace std;

using namespace Puma;


class IncludeVisitor : public PreVisitor {
  ostream *_os;
  long _length;

  // array of include file names to get the inclusion depth
  Array<const char *> _struct;
	
public:
  IncludeVisitor (ostream &os = cout) : _os (&os), _length (0) {}
  virtual ~IncludeVisitor ();

  void iterateNodes (PreTree *);

public:
  // called on visit of the include directive node 
  void visitPreIncludeDirective_Pre (PreIncludeDirective *);
};  	    

#endif /* __INCLUDE_VISITOR__ */
