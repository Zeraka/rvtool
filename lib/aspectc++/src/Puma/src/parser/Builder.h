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

#ifndef __Builder_h__
#define __Builder_h__

/** \file
 *  Basic syntax tree builder component. */

#include "Puma/ErrorCollector.h"
#include "Puma/ErrorSink.h"
#include "Puma/PtrStack.h"
#include "Puma/CTree.h"

namespace Puma {


class Token;


/** \class Builder Builder.h Puma/Builder.h
 *  %Syntax tree builder base class. Implements the basic infrastructure 
 *  for building CTree based syntax trees. 
 *
 *  Tree builders are used in the syntax analysis to create the nodes 
 *  of the syntax tree according to the accepted grammar (see class 
 *  Syntax). A syntax tree shall be destroyed using the tree builder 
 *  that has created it by calling its method Builder::destroy(CTree*) 
 *  with the root node of the syntax tree as its argument.
 *
 *  The builder is organized as a multi-level stack. If a grammar rule 
 *  is parsed then a new stack level is created. The sub-trees of the 
 *  syntax tree representing the parsed grammer rule are pushed on this 
 *  level of the stack. If the grammar rule is parsed successfully then 
 *  these sub-trees are used to build the syntax tree representing the 
 *  parsed grammar rule (and thus the corresponding source code). The 
 *  current stack level is then discarded and the created syntax tree 
 *  is pushed on the stack of the previous level (which is now the top
 *  level of the stack). If the grammar rule could not be parsed 
 *  successfully then the current stack level is discarded and all 
 *  the sub-trees pushed on it are destroyed. */
class Builder : public PtrStack<CTree> {
protected:
  /** The error collector object. */
  ErrorCollector ec;
  /** Counter for the created CT_Token object. */
  static unsigned long int token_counter;
  
public:
  /** Container class for collecting a set of 
   *  syntax tree nodes. */
  class Container : public CT_List {
  public:
    /** Get the identifier for this node type. Can be compared with NodeName(). */
    static const char *NodeId ();
    /** Get the name of the node. Can be compared with NodeId(). */
    const char *NodeName () const { return NodeId (); }
  };

protected:
  /** Constructor. */
  Builder () {}

public:
  /** Destroy the given syntax tree recursively. 
   *  \param tree The root node of the syntax tree.
   *  \param recursive Delete sub tree too. */
  static void destroy (CTree *tree, bool recursive = true);

  /** Destroy the given syntax tree node. Child nodes are not destroyed.
   *  \param node The node. */
  virtual void destroyNode (CTree *node);

  /** Print the collected error messages on the
   *  given error output stream. 
   *  \param e The error output stream. */
  void errors (ErrorSink &e);

  /** Get the collected errors. */
  ErrorSink &err () const;
  /** Save the current state. */
  void save_state ();
  /** Discard the saved state. */
  void forget_state ();
  /** Restore the saved state. */
  void restore_state ();

  /** Create a new CT_Token object for the given token.
   *  \param t The token. */
  CTree *token (Token *t);
  /** Create a new CT_Error object. */
  CTree *error ();

  /** Get the current number of nodes on the builder stack. */
  int nodes () const;
  /** Get the n-th node from the builder stack. 
   *  \param n The index of the node. */
  CTree *get_node (int n = 0) const;
  
  /** Reset the token counter. 
   *  \param v The new counter value. */
  void setTokenCounter (unsigned long v);
  /** Get the current token count. */
  unsigned long getTokenCounter () const;

protected:
  /** Destroy the top tree node of the builder stack. */
  void Delete ();

  /** Put all nodes on the builder stack into a Container object. */
  CTree *container () const;

  /** Add all nodes on the builder stack to the given list node.
   *  \param l The list node. */
  CTree *list (CT_List *l) const;
  /** Add all nodes of the given container to the given list node.
   *  \param l The list node. 
   *  \param c The node container. */
  CTree *copy_list (CT_List *l, Container *c) const;
};

inline int Builder::nodes () const
 { return (int)Length (); }
 
inline CTree *Builder::get_node (int i) const
 { return (CTree*)Get (i); }

inline CTree *Builder::container () const
 { return list (new Container); }

inline ErrorSink &Builder::err () const { return (ErrorSink&)ec; }
inline void Builder::errors (ErrorSink &e) { ec.shift (e); }

inline void Builder::save_state ()    { New (); }
inline void Builder::forget_state ()  { Reject (); }
inline void Builder::restore_state () { Destroy (); }

inline void Builder::setTokenCounter (unsigned long num) 
 { token_counter = num; }
inline unsigned long Builder::getTokenCounter () const 
 { return token_counter; }

} // namespace Puma

#endif /* __Builder_h__ */
