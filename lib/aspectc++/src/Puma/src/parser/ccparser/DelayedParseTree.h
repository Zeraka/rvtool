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

#ifndef __delayed_parse_tree_h__
#define __delayed_parse_tree_h__

/** \file
 *  Delayed parse syntax tree classes. */

namespace Puma {


// Syntax tree node hierarchy:
class CTree;
class   CT_DelayedParse;
class     CT_DelayedParseParamInit;
class     CT_DelayedParseFctTryBlock;
class     CT_DelayedParseCtorInit;
class     CT_DelayedParsePureSpec;
class     CT_DelayedParseFctBody;
class     CT_DelayedParseSkippedFctBody;
class     CT_DelayedParseConstInit;


} // namespace Puma

#include "Puma/CTree.h"
#include "Puma/TokenProvider.h"
#include "Puma/CRecord.h"
#include "Puma/CStructure.h"

namespace Puma {


/** \class CT_DelayedParse DelayedParseTree.h Puma/DelayedParseTree.h
 *  Tree node representing a piece of code to be parsed delayed. */
class CT_DelayedParse : public CTree {
  CRecord* _Class;
  CStructure* _Scope;
  TokenProvider::State _State;
  CTree* sons[2];

protected:
  /** Constructor.
   *  \param s Start token. */
  CT_DelayedParse (Token* s) : _Class(0), _Scope(0) {
    AddSon (sons[0], new CT_Token(s));
    AddSon (sons[1], 0);
  }
  /** Constructor.
   *  \param s Start token.
   *  \param e End token. */
  CT_DelayedParse (Token* s, Token* e) : _Class(0), _Scope(0) {
    AddSon (sons[0], new CT_Token(s));
    AddSon (sons[1], new CT_Token(e));
  }

public:
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get a pointer to this CT_DelayedParse.
   *  \return The CT_DelayedParse pointer. */
  CT_DelayedParse *IsDelayedParse () { return this; }
  /** Get the number of sons. */
  int Sons () const { return sons[1] ? 2 : 1; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 2, n); }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 2, old_son, new_son);
  }
  /** Get the outermost class for this delayed code fragment. */
  CRecord* Class () const { return (CRecord*)_Class; }
  /** Get the scope of the delayed code fragment. */
  CStructure* Scope () const { return (CStructure*)_Scope; }
  /** Get the token stream state of the delayed code fragment. */
  TokenProvider::State State() const { return (TokenProvider::State&)_State; }
  /** Set the outermost class for this delayed code fragment.
   *  \param record The outermost class. */
  void Class (CRecord* record) { _Class = record; }
  /** Set the scope of the delayed code fragment.
   *  \param scope The scope. */
  void Scope (CStructure* scope) { _Scope = scope; }
  /** Set the token stream state of the delayed code fragment.
   *  \param state The token stream state. */
  void State (TokenProvider::State state) { _State = state; }
};


/** \class CT_DelayedParseParamInit DelayedParseTree.h Puma/DelayedParseTree.h
 *  Tree node representing a delayed parameter initializer. */
class CT_DelayedParseParamInit : public CT_DelayedParse {
public:
  /** Constructor.
   *  \param s Start token. */
  CT_DelayedParseParamInit (Token* s) : CT_DelayedParse (s) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }  
};


/** \class CT_DelayedParseFctTryBlock DelayedParseTree.h Puma/DelayedParseTree.h
 *  Tree node representing a delayed function try-block. */
class CT_DelayedParseFctTryBlock : public CT_DelayedParse, CT_HandlerSeq {
public:
  /** Constructor.
   *  \param s Start token. */
  CT_DelayedParseFctTryBlock (Token* s) : CT_DelayedParse (s) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }  
};


/** \class CT_DelayedParseCtorInit DelayedParseTree.h Puma/DelayedParseTree.h
 *  Tree node representing a delayed constructor initializer. */
class CT_DelayedParseCtorInit : public CT_DelayedParse {
public:
  /** Constructor.
   *  \param s Start token. */
  CT_DelayedParseCtorInit (Token* s) : CT_DelayedParse (s) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }  
};


/** \class CT_DelayedParsePureSpec DelayedParseTree.h Puma/DelayedParseTree.h
 *  Tree node representing a delayed pure specifier. */
class CT_DelayedParsePureSpec : public CT_DelayedParse {
public:
  /** Constructor.
   *  \param s Start token. */
  CT_DelayedParsePureSpec (Token* s) : CT_DelayedParse (s) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }  
};


/** \class CT_DelayedParseFctBody DelayedParseTree.h Puma/DelayedParseTree.h
 *  Tree node representing a delayed function body. */
class CT_DelayedParseFctBody : public CT_DelayedParse, CT_CmpdStmt {
public:
  /** Constructor.
   *  \param s Start token.
   *  \param e End token. */
  CT_DelayedParseFctBody (Token* s, Token* e) : CT_DelayedParse (s, e) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }  
};


/** \class CT_DelayedParseSkippedFctBody DelayedParseTree.h Puma/DelayedParseTree.h
 *  Tree node representing a delayed skipped function body. */
class CT_DelayedParseSkippedFctBody : public CT_DelayedParse, CT_CmpdStmt {
public:
  /** Constructor.
   *  \param s Start token.
   *  \param e End token. */
  CT_DelayedParseSkippedFctBody (Token* s, Token* e) : CT_DelayedParse (s, e) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }  
};


/** \class CT_DelayedParseConstInit DelayedParseTree.h Puma/DelayedParseTree.h
 *  Tree node representing a delayed constant initializer. */
class CT_DelayedParseConstInit : public CT_DelayedParse, CT_ExprList {
public:
  /** Constructor.
   *  \param s Start token. */
  CT_DelayedParseConstInit (Token* s) : CT_DelayedParse (s) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }  
};


} // namespace Puma

#endif /* __delayed_parse_tree_h__ */
