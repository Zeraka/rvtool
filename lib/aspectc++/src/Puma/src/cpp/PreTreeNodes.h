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

#ifndef __pre_syntax_tree_nodes__
#define __pre_syntax_tree_nodes__

/** \file 
 *  Preprocessor syntax tree classes. */

#include "Puma/Unit.h"
#include "Puma/PreVisitor.h"
#include "Puma/PreTreeToken.h"
#include "Puma/PreTreeComposite.h"

namespace Puma {


/** \class PreProgram PreTreeNodes.h Puma/PreTreeNodes.h
 *  The root node of the preprocessor syntax tree. */
class PreProgram : public PreTreeComposite {
public:
  /** Constructor.
   *  \param g The group of preprocessor directives. */
  PreProgram (PreTree* g) : PreTreeComposite (1, 0) { 
    add_son (g); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor &v) {
    v.visitPreProgram_Pre (this);
    v.iterateNodes (this);
    v.visitPreProgram_Post (this);
  }            
};      


/** \class PreDirectiveGroups PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing the directive groups in the program. */
class PreDirectiveGroups : public PreTreeComposite {
public:
  /** Constructor. */
  PreDirectiveGroups () : PreTreeComposite (-1, 0) {}

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreDirectiveGroups_Pre (this);
    v.iterateNodes (this);
    v.visitPreDirectiveGroups_Post (this);
  }            
};      


/** \class PreConditionalGroup PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing a group of conditional directives. 
 *  Example: \code #if ... #elif ... #else ... #endif \endcode */
class PreConditionalGroup : public PreTreeComposite {
public:
  /** Constructor. 
   *  \param i The \#if part.
   *  \param dg The directive group.
   *  \param ei The \#endif part. */
  PreConditionalGroup (PreTree* i, PreTree* dg, PreTree* ei) : PreTreeComposite (3, 0) { 
    add_son (i); add_son (dg); add_son (ei); 
  }
  /** Constructor. 
   *  \param i The \#if part.
   *  \param e The \#elif or \#else part.
   *  \param dg The directive group.
   *  \param ei The \#endif part. */
  PreConditionalGroup (PreTree* i, PreTree* e, PreTree* dg, PreTree* ei) : PreTreeComposite (4, 0) { 
    add_son (i); add_son (e); add_son (dg); add_son (ei); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreConditionalGroup_Pre (this);
    v.iterateNodes (this);
    v.visitPreConditionalGroup_Post (this);
  }
};      


/** \class PreElsePart PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing a group of directives 
 *  in the \#else part of an \#if conditional. */
class PreElsePart : public PreTreeComposite {
public:
  /** Constructor. 
   *  \param dg The directive group.
   *  \param el The \#else directive. */
  PreElsePart (PreTree* dg, PreTree* el) : PreTreeComposite (2, 0) { 
    add_son (dg); add_son (el); 
  }
  /** Constructor. 
   *  \param ei The preceding \#elif part.
   *  \param dg The directive group.
   *  \param el The \#else directive. */
  PreElsePart (PreTree* ei, PreTree* dg, PreTree* el) : PreTreeComposite (3, 0) { 
    add_son (ei); add_son (dg); add_son (el); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreElsePart_Pre (this);
    v.iterateNodes (this);
    v.visitPreElsePart_Post (this);
  }
};      


/** \class PreElifPart PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing a group of directives 
 *  in the \#elif part of an \#if conditional. */
class PreElifPart : public PreTreeComposite {
public:
  /** Constructor. */
  PreElifPart () : PreTreeComposite (-1, 0) {}
        
  /** Add two sons, a directive group and a \#elif directive.
   *  \param dg The directive group.
   *  \param el The \#elif directive. */
  void addSons (PreTree* dg, PreTree* el) { 
    add_son (dg); add_son (el); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreElifPart_Pre (this);
    v.iterateNodes (this);
    v.visitPreElifPart_Post (this);
  }
};      


/** \class PreIfDirective PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing an \#if directive. 
 *  Example: \code #if OSTYPE==Linux \endcode */
class PreIfDirective : public PreTreeComposite {
public:
  /** Constructor.
   *  \param i The \#if token.
   *  \param c The condition. */
  PreIfDirective (PreTree* i, PreTree* c) : PreTreeComposite (2, 1) { 
    add_son (i); add_son (c); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreIfDirective_Pre (this);
    v.iterateNodes (this);
    v.visitPreIfDirective_Post (this);
  }
};      


/** \class PreIfdefDirective PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing an \#ifdef directive. 
 *  Example: \code #ifdef Linux \endcode */
class PreIfdefDirective : public PreTreeComposite {
public:
  /** Constructor. 
   *  \param i The \#ifdef token.
   *  \param n The name of the macro. 
   *  \param tl The remaining tokens of the line. */
  PreIfdefDirective (PreTree* i, PreTree* n, PreTree* tl) : PreTreeComposite (3, 1) { 
    add_son (i); add_son (n); add_son (tl); 
  }
  /** Constructor. 
   *  \param i The \#ifdef token.
   *  \param tl The remaining tokens of the line. */
  PreIfdefDirective (PreTree* i, PreTree* tl) : PreTreeComposite (2, 0) { 
    add_son (i); add_son (tl); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreIfdefDirective_Pre (this);
    v.iterateNodes (this);
    v.visitPreIfdefDirective_Post (this);
  }
};      


/** \class PreIfndefDirective PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing an \#ifndef directive. 
 *  Example: \code #ifndef Linux \endcode */
class PreIfndefDirective : public PreTreeComposite {
public:
  /** Constructor. 
   *  \param i The \#ifndef token.
   *  \param n The name of the macro. 
   *  \param tl The remaining tokens of the line. */
  PreIfndefDirective (PreTree* i, PreTree* n, PreTree* tl) : PreTreeComposite (3, 1) { 
    add_son (i); add_son (n); add_son (tl); 
  }
  /** Constructor. 
   *  \param i The \#ifndef token.
   *  \param tl The remaining tokens of the line. */
  PreIfndefDirective (PreTree* i, PreTree* tl) : PreTreeComposite (2, 0) { 
    add_son (i); add_son (tl); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreIfndefDirective_Pre (this);
    v.iterateNodes (this);
    v.visitPreIfndefDirective_Post (this);
  }
};      


/** \class PreElifDirective PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing an \#elif directive. 
 *  Example: \code #elif OSTYPE==linux \endcode */
class PreElifDirective : public PreTreeComposite {
public:
  /** Constructor. 
   *  \param e The \#elif token.
   *  \param c The condition. */
  PreElifDirective (PreTree* e, PreTree* c) : PreTreeComposite (2, 1) { 
    add_son (e); add_son (c); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreElifDirective_Pre (this);
    v.iterateNodes (this);
    v.visitPreElifDirective_Post (this);
  }
};      


/** \class PreElseDirective PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing an \#else directive. 
 *  Example: \code #else \endcode */
class PreElseDirective : public PreTreeComposite {
public:
  /** Constructor. 
   *  \param e The \#else token.
   *  \param tl The remaining tokens of the line. */
  PreElseDirective (PreTree* e, PreTree* tl) : PreTreeComposite (2, 0) { 
    add_son (e); add_son (tl); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreElseDirective_Pre (this);
    v.iterateNodes (this);
    v.visitPreElseDirective_Post (this);
  }
};      


/** \class PreEndifDirective PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing an \#endif directive. 
 *  Example: \code #endif \endcode */
class PreEndifDirective : public PreTreeComposite {
public:
  /** Constructor. 
   *  \param e The \#endif token.
   *  \param tl The remaining tokens of the line. */
  PreEndifDirective (PreTree* e, PreTree* tl) : PreTreeComposite (2, 0) { 
    add_son (e); add_son (tl); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreEndifDirective_Pre (this);
    v.iterateNodes (this);
    v.visitPreEndifDirective_Post (this);
  }
};      


/** \class PreIncludeDirective PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing an \#include or \#include_next directive. 
 *  Example: \code #include <stdio.h> \endcode */
class PreIncludeDirective : public PreTreeComposite {
  int _depth; // depth of nested includes

public:
  /** Constructor. 
   *  \param i The \#include or \#include_next token. 
   *  \param tl The remaining tokens of the line containing the file to include. */
  PreIncludeDirective (PreTree* i, PreTree* tl) : PreTreeComposite (2, 1) { 
    add_son (i); add_son (tl); 
    _depth = -1; 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreIncludeDirective_Pre (this);
    v.iterateNodes (this);
    v.visitPreIncludeDirective_Post (this);
  }
        
  /** Get the depth of nested inclusion. 
   *  \return The depth or -1 for a top-level include. */
  int depth () const {
    return _depth;
  }
  /** Set the depth of nested inclusion. 
   *  \param d The depth of inclusion. */
  void depth (int d) {
    _depth = d;
  }
        
  /** Check if this is a forced include (given by command line). */
  bool is_forced () const {
    return !((Unit*)startToken ()->belonging_to ())->isFile ();
  }
};      


/** \class PreAssertDirective PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing an \#assert directive. 
 *  Example: \code #assert OSTYPE (linux) \endcode */
class PreAssertDirective : public PreTreeComposite {
public:
  /** Constructor. 
   *  \param a The \#assert token.
   *  \param p The predicate name.
   *  \param an The answer to the predicate. */
  PreAssertDirective (PreTree* a, PreTree* p, PreTree* an) : PreTreeComposite (3, 0) { 
    add_son (a); add_son (p); add_son (an); 
  }
  /** Constructor. 
   *  \param a The \#assert token.
   *  \param tl The remaining tokens of the line. */
  PreAssertDirective (PreTree* a, PreTree* tl) : PreTreeComposite (2, 0) { 
    add_son (a); add_son (tl); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreAssertDirective_Pre (this);
    v.iterateNodes (this);
    v.visitPreAssertDirective_Post (this);
  }
};      


/** \class PreUnassertDirective PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing an \#unassert directive. 
 *  Example: \code #unassert OSTYPE \endcode */
class PreUnassertDirective : public PreTreeComposite {
public:
  /** Constructor. 
   *  \param ua The \#unassert token. 
   *  \param n The name of the predicate.
   *  \param tl The remaining tokens of the line. */
  PreUnassertDirective (PreTree* ua, PreTree* n, PreTree* tl) : PreTreeComposite (3, 0) { 
    add_son (ua); add_son (n); add_son (tl); 
  }
  /** Constructor. 
   *  \param ua The \#unassert token. 
   *  \param tl The remaining tokens of the line. */
  PreUnassertDirective (PreTree* ua, PreTree* tl) : PreTreeComposite (2, 0) { 
    add_son (ua); add_son (tl); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreUnassertDirective_Pre (this);
    v.iterateNodes (this);
    v.visitPreUnassertDirective_Post (this);
  }
};      


/** \class PreDefineFunctionDirective PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing a \#define directive for function-like macros. 
 *  Example: \code #define MUL(a,b) (a * b) \endcode */
class PreDefineFunctionDirective : public PreTreeComposite {
public:
  /** Constructor. 
   *  \param a The \#define token.
   *  \param b The macro name.
   *  \param c Left parenthesis before the parameter list.
   *  \param d The macro parameter list. 
   *  \param e Comma before the last parameter.
   *  \param f The token '...'.
   *  \param g Right parenthesis behind the parameter list.
   *  \param h The macro body. */
  PreDefineFunctionDirective (PreTree* a, PreTree* b, PreTree* c, PreTree* d, 
   PreTree* e, PreTree* f, PreTree* g, PreTree* h) : PreTreeComposite (8, 0) { 
    add_son (a); add_son (b); add_son (c); add_son (d);
    add_son (e); add_son (f); add_son (g); add_son (h); 
  }

  /** Constructor. 
   *  \param a The \#define token.
   *  \param b The macro name.
   *  \param c Left parenthesis before the parameter list.
   *  \param d The macro parameter list. 
   *  \param e The token '...'.
   *  \param f Right parenthesis behind the parameter list.
   *  \param g The macro body. */
  PreDefineFunctionDirective (PreTree* a, PreTree* b, PreTree* c, PreTree* d, 
   PreTree* e, PreTree* f, PreTree* g) : PreTreeComposite (7, 0) { 
    add_son (a); add_son (b); add_son (c); add_son (d);
    add_son (e); add_son (f); add_son (g); 
  }

  /** Constructor. 
   *  \param a The \#define token.
   *  \param b The macro name.
   *  \param c Left parenthesis before the parameter list.
   *  \param d The macro parameter list. 
   *  \param e Right parenthesis behind the parameter list.
   *  \param f The macro body. */
  PreDefineFunctionDirective (PreTree* a, PreTree* b, PreTree* c, PreTree* d, 
   PreTree* e, PreTree* f) : PreTreeComposite (6, 0) { 
    add_son (a); add_son (b); add_son (c); 
    add_son (d); add_son (e); add_son (f); 
  }
        
  /** Constructor. 
   *  \param a The \#define token.
   *  \param b The macro name.
   *  \param c Left parenthesis before the parameter list.
   *  \param d Right parenthesis behind the parameter list.
   *  \param e The macro body. */
  PreDefineFunctionDirective (PreTree* a, PreTree* b, PreTree* c, 
   PreTree* d, PreTree* e) : PreTreeComposite (5, 0) { 
    add_son (a); add_son (b); add_son (c); add_son (d); add_son (e); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreDefineFunctionDirective_Pre (this);
    v.iterateNodes (this);
    v.visitPreDefineFunctionDirective_Post (this);
  }
};   


/** \class PreDefineConstantDirective PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing a \#define directive for constants. 
 *  Example: \code #define CONSTANT 1 \endcode */
class PreDefineConstantDirective : public PreTreeComposite {
public:
  /** Constructor. 
   *  \param d The \#define token.
   *  \param n The name of the constant.
   *  \param v The constant value. */
  PreDefineConstantDirective (PreTree* d, PreTree* n, PreTree* v) : PreTreeComposite (3, 0) { 
    add_son (d); add_son (n); add_son (v); 
  }
  /** Constructor. 
   *  \param d The \#define token.
   *  \param tl The remaining tokens of the line. */
  PreDefineConstantDirective (PreTree* d, PreTree* tl) : PreTreeComposite (2, 0) { 
    add_son (d); add_son (tl); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreDefineConstantDirective_Pre (this);
    v.iterateNodes (this);
    v.visitPreDefineConstantDirective_Post (this);
  }
};      


/** \class PreUndefDirective PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing an \#undef directive. 
 *  Example: \code #undef MACRO \endcode */
class PreUndefDirective : public PreTreeComposite {
public:
  /** Constructor. 
   *  \param u The \#undef token. 
   *  \param m The name of the macro to undefine. 
   *  \param tl The remaining tokens of the line. */
  PreUndefDirective (PreTree* u, PreTree* m, PreTree* tl) : PreTreeComposite (3, 0) { 
    add_son (u); add_son (m); add_son (tl); 
  }
  /** Constructor. 
   *  \param u The \#undef token.
   *  \param tl The remaining tokens of the line. */
  PreUndefDirective (PreTree* u, PreTree* tl) : PreTreeComposite (2, 0) { 
    add_son (u); add_son (tl); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreUndefDirective_Pre (this);
    v.iterateNodes (this);
    v.visitPreUndefDirective_Post (this);
  }
};      


/** \class PreWarningDirective PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing a \#warning directive. 
 *  Example: \code #warning This is a warning. \endcode */
class PreWarningDirective : public PreTreeComposite {
public:
  /** Constructor. 
   *  \param w The \#warning token. 
   *  \param m The warning message. */
  PreWarningDirective (PreTree* w, PreTree* m) : PreTreeComposite (2, 0) { 
    add_son (w); add_son (m); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreWarningDirective_Pre (this);
    v.iterateNodes (this);
    v.visitPreWarningDirective_Post (this);
  }
};      


/** \class PreErrorDirective PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing an \#error directive. 
 *  Example: \code #error This is an error. \endcode */
class PreErrorDirective : public PreTreeComposite {
public:
  /** Constructor. 
   *  \param e The \#error token.
   *  \param m The error message. */
  PreErrorDirective (PreTree* e, PreTree* m) : PreTreeComposite (2, 0) { 
    add_son (e); add_son (m); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreErrorDirective_Pre (this);
    v.iterateNodes (this);
    v.visitPreErrorDirective_Post (this);
  }
};      


/** \class PreIdentifierList PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing the identifier list of a 
 *  function-like macro definition. 
 *  Example: \code a,b,c \endcode */
class PreIdentifierList : public PreTreeComposite {
public:
  /** Constructor. 
   *  \param id An identifier. */
  PreIdentifierList (PreTree* id) : PreTreeComposite (-1, 0) { 
    add_son (id); 
  }
        
  /** Add two sons, a comma and an identifier.
   *  \param c A comma.
   *  \param id An identifier. */
  void addSons (PreTree* c, PreTree* id) { 
    add_son (c); add_son (id); 
  }

  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreIdentifierList_Pre (this);
    v.iterateNodes (this);
    v.visitPreIdentifierList_Post (this);
  }
};


/** \class PreTokenList PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing the token list of a macro body. */
class PreTokenList : public PreTreeComposite {
public:
  /** Constructor. */
  PreTokenList () : PreTreeComposite (0, 0) {}
  /** Constructor. 
   *  \param tl The token list. 
   *  \param nl The newline token. */
  PreTokenList (PreTree* tl, PreTree* nl) : PreTreeComposite (2, 0) { 
    add_son (tl); add_son (nl); 
  }
  /** Constructor. 
   *  \param tl The token list. */
  PreTokenList (PreTree* tl) : PreTreeComposite (1, 0) { 
    add_son (tl); 
  }
        
  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreTokenList_Pre (this);
    v.iterateNodes (this);
    v.visitPreTokenList_Post (this);
  }
};      


/** \class PreTokenListPart PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing a part of the token list of a macro body. */
class PreTokenListPart : public PreTreeComposite {
public:
  /** Constructor. 
   *  \param tl The token list. */
  PreTokenListPart (PreTree* tl) : PreTreeComposite (-1, 0) { 
    add_son (tl); 
  }
        
  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreTokenListPart_Pre (this);
    v.iterateNodes (this);
    v.visitPreTokenListPart_Post (this);
  }
};      


/** \class PreCondSemNode PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor semantic tree node for conditions. */
class PreCondSemNode : public PreTree {
  // The calculated value of the condition.
  bool _value;

public:
  /** Constructor. 
   *  \param value The calculated value of the condition. */
  PreCondSemNode (bool value) : _value (value) {}
        
  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreCondSemNode (this);
  }
        
  /** Get the calculated value of the condition. */
  bool value () const { return _value; }
};      


/** \class PreInclSemNode PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor semantic tree node for the \#include directive
 *  containing the unit to include. */
class PreInclSemNode : public PreTree {
  // Pointer to the included file unit.
  Unit* _unit;
        
  // True if the inclusion was not done, because of an active include guard.
  bool _guarded;

public:
  /** Constructor. 
   *  \param unit The unit containing the tokens of the include file.
   *  \param guarded True if the inclusion was not done due to an include guard. */
  PreInclSemNode (Unit* unit, bool guarded) :
    _unit (unit), _guarded (guarded) {}
        
  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreInclSemNode (this);
  }
        
  /** Get the token unit of the included file. */
  Unit* unit () const { return _unit; }
        
  /** Check if the inclusion was not done due to an include guard. */
  bool guarded () const { return _guarded; }
};      


/** \class PreError PreTreeNodes.h Puma/PreTreeNodes.h
 *  Preprocessor tree node representing a parse error. */
class PreError : public PreTree {
public:
  /** Constructor. */
  PreError () {}
        
  /** Part of the tree visitor pattern. Calls the node
   *  visiting functions suitable for this node type. 
   *  \param v The visitor object on which to call the 
   *           visiting functions. */
  void accept (PreVisitor& v) {
    v.visitPreError (this);
  }
};      


} // namespace Puma

#endif /* __pre_syntax_tree_nodes__ */
