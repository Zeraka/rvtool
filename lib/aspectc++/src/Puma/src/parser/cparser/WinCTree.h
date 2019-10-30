// This file is part of PUMA.
// Copyright (C) 1999-2003  The PUMA developer team.
//                                                                
// This program is free software;  you can redistribute it and/or 
// modify it under the terms of the GNU General Public License as 
// published by the Free Software Foundation; either version 2 of 
// the License, or (at your option) any later version.            
//                                                                
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
// GNU General Public License for more details.                   
//                                                                
// You should have received a copy of the GNU General Public      
// License along with this program; if not, write to the Free     
// Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
// MA  02111-1307  USA                                            

#ifndef __win_c_tree_h__
#define __win_c_tree_h__

/** \file
 *  VisualC++ specific syntax tree classes. */

namespace Puma {


// Syntax tree node hierarchy:
class CTree;
class   CT_Statement;
class     CT_AsmBlock;


} // namespace Puma

#include "Puma/CTree.h"

namespace Puma {


/** \class CT_AsmBlock WinCTree.h Puma/WinCTree.h
 *  Tree node representing an inline assembly block.
 *  Example: \code asm { movl ecx eax } \endcode */
class CT_AsmBlock : public CT_Statement {
  CTree *sons[3]; // key, begin, end

public:
  /** Constructor.
   *  \param k The keyword 'asm'.
   *  \param b Left brace.
   *  \param e Right brace. */
  CT_AsmBlock (CTree *k, CTree *b, CTree *e) {
    AddSon (sons[0], k); AddSon (sons[1], b); AddSon (sons[2], e);
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }  
  /** Get the number of sons. */
  int Sons () const { return 3; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 3, n); }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  virtual void ReplaceSon (CTree *old_son, CTree *new_son) {
    CTree::ReplaceSon (sons, 3, old_son, new_son);
  }
};


} // namespace Puma

#endif /* __win_c_tree_h__ */
