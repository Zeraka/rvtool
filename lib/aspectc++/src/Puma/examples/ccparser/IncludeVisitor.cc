#include "IncludeVisitor.h"
#include "Puma/PreTreeToken.h"
#include "Puma/PreSonIterator.h"
#include <string.h>

using namespace Puma;


IncludeVisitor::~IncludeVisitor () {
  for (int i = 0; i < _struct.length (); i++)
    _struct[i] = (const char*)0;
}

void IncludeVisitor::iterateNodes (PreTree *node) {
  PreSonIterator i (node);
  for (i.first (); ! i.isDone (); i.next ())
    i.currentItem ()->accept (*this);
}

void IncludeVisitor::visitPreIncludeDirective_Pre (PreIncludeDirective *node) {
  
  // if there is no semantic node this include directive was never
  // executed (maybe a FALSE conditional branch)
  if (! node->daughters () || ! ((PreInclSemNode*)(node->daughter (0)))->unit ())
    return;

  // print "depth" spaces 
  for (int i = 0; i < node->depth (); i++) 
    *_os << "  ";

  // print the name of the file included (to find in the semantic node)
  *_os << ((PreInclSemNode*)(node->daughter (0)))->unit ()->name () 
       << endl;
}
