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

#ifndef __CC1XTree_h__
#define __CC1XTree_h__

#include "Puma/CTree.h"

/** \file
 *  AspectC++ specific syntax tree classes. */

namespace Puma {


/** \class CT_StaticAssert CC1XTree.h Puma/CC1XTree.h
 *  Tree node representing a C++1X static_assert declaration. 
 *
 *  Example: 
 *
 *  \code 
 * static_assert(sizeof(long) > 4, "long is too long, haha")
 *  \endcode */
class CT_StaticAssert : public CTree {
  CTree *sons[6]; // static_assert, '(', const_expr, ',', cmpd_str, ')'

public:
  /** Constructor.
   *  \param sa The keyword 'static_assert'.
   *  \param op The opening bracket token.
   *  \param ce The constant expression.
   *  \param co The comma token.
   *  \param msg The error message.
   *  \param cl The closing bracket token. */
  CT_StaticAssert (CTree *sa, CTree *op, CTree *ce, CTree *co, CTree *msg,
      CTree *cl) {
    AddSon (sons[0], sa); AddSon (sons[1], op);
    AddSon (sons[2], ce); AddSon (sons[3], co);
    AddSon (sons[4], msg); AddSon (sons[5], cl);
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 6; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 6, n); }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 6, old_son, new_son);
  }
  /** Get the constant expression. */
  CT_Expression *Expr () const { return (CT_Expression*)sons[2]; }
  /** Get the error message node. */
  CT_String *ErrorMessage () const { return (CT_String*) sons[4]; }
};

} // namespace Puma

#endif /* __CC1XTree_h__ */
