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

#ifndef __gnu_c_tree_h__
#define __gnu_c_tree_h__

/** \file
 *  GNU C/C++ specific syntax tree classes. */

namespace Puma {

// Syntax tree node hierarchy:
class   CT_GnuAsmSpec;        // derived from CTree
class   CT_GnuAsmDef;         // derived from CT_AsmDef
class   CT_GnuAsmOperand;     // derived from CTree
class     CT_GnuAsmOperands;  // derived from CT_List
class     CT_GnuAsmClobbers;  // derived from CT_List
class   CT_GnuStatementExpr;  // derived from CT_Expression
class   CT_GnuTypeof;         // derived from CT_DeclSpec
class   CT_GnuAttribute;      // derived from CTree
class   CT_GnuLocalLabelStmt; // derived from CT_Statement and CT_List
class     CT_LabelStmt;       // derived from CT_Statement

} // namespace Puma

#include "Puma/CTree.h"

namespace Puma {


/** \class CT_GnuAsmSpec GnuCTree.h Puma/GnuCTree.h
 *  Tree node representing an extended inline assembly specifier. 
 *  Example: \code asm("r0") \endcode */
class CT_GnuAsmSpec : public CTree {
  CTree *sons[4]; // asm, open, expr, close

public:
  /** Constructor.
   *  \param a The keyword 'asm'.
   *  \param o Left parenthesis before the assembly.
   *  \param e The assembly instructions.
   *  \param c Right parenthesis behind the assembly. */
  CT_GnuAsmSpec (CTree *a, CTree *o, CTree *e, CTree *c) {
    AddSon (sons[0], a); AddSon (sons[1], o);
    AddSon (sons[2], e); AddSon (sons[3], c);
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); } 
  /** Get the number of sons. */
  int Sons () const { return 4; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 4, n); }
  /** Get the assembly instructions. */
  CT_Expression *Expr () const { return (CT_Expression*)sons[2]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) {
    CTree::ReplaceSon (sons, 4, old_son, new_son);
  }
};

/** \class CT_GnuAsmDef GnuCTree.h Puma/GnuCTree.h
 *  Tree node representing an extended inline assembly definition. 
 *  Example: \code asm("fsinx %1,%0" : "=f" (result) : "f" (angle)); \endcode */
class CT_GnuAsmDef : public CT_AsmDef {
  CTree *_cvqual;
  CTree *_operands0;
  CTree *_operands1;
  CTree *_clobbers;

public:
  /** Constructor.
   *  \param a The keyword 'asm'.
   *  \param cv Optional const/volatile qualifier sequence.
   *  \param o Left parenthesis before the assembly.
   *  \param s The assembly instructions.
   *  \param op0 First operand.
   *  \param op1 Second operand.
   *  \param cl Clobbers.
   *  \param c Right parenthesis behind the assembly.
   *  \param sc The trailing semi-colon. */
  CT_GnuAsmDef (CTree *a, CTree *cv, CTree *o, CTree *s, 
                CTree *op0, CTree *op1, CTree *cl, CTree *c, CTree *sc) :
    CT_AsmDef (a, o, s, c, sc) {
    AddSon (_cvqual, cv); AddSon (_operands0, op0);
    AddSon (_operands1, op1); AddSon (_clobbers, cl);
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  virtual int Sons () const {
    return 5 +
           (_cvqual ? 1 : 0) +
           (_operands0 ? 1 : 0) +
           (_operands1 ? 1 : 0) +
           (_clobbers ? 1 : 0);
  }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  virtual CTree *Son (int n) const {
    int have_cv = _cvqual ? 1 : 0;
    int gnu_sons = Sons () - have_cv - 5;
    int gnu_first = 3 + have_cv;
    if (n == 0) return CT_AsmDef::Son (0);
    else if (n == 1 && _cvqual) return _cvqual;
    else if ((n == 1 && !_cvqual) || (n == 2 && _cvqual))
       return CT_AsmDef::Son (1);
    else if ((n == 2 && !_cvqual) || (n == 3 && _cvqual))
       return CT_AsmDef::Son (2);
    else if (n == gnu_first && gnu_sons >= 1) return _operands0;
    else if (n == gnu_first + 1 && gnu_sons >= 2) return _operands1;
    else if (n == gnu_first + 2 && gnu_sons >= 3) return _clobbers;
    else if (n == Sons () - 2)
       return CT_AsmDef::Son (3);
    else if (n == Sons () - 1)
       return CT_AsmDef::Son (4);
    else return (CTree*)0;
  }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  virtual void ReplaceSon (CTree *old_son, CTree *new_son) {
    if (old_son == _cvqual) CTree::ReplaceSon (_cvqual, old_son, new_son);
    else if (old_son == _operands0) CTree::ReplaceSon (_operands0, old_son, new_son);
    else if (old_son == _operands1) CTree::ReplaceSon (_operands1, old_son, new_son);
    else if (old_son == _clobbers) CTree::ReplaceSon (_clobbers, old_son, new_son);
    else CT_AsmDef::ReplaceSon (old_son, new_son);
  }
};

/** \class CT_GnuAsmOperand GnuCTree.h Puma/GnuCTree.h
 *  Tree node representing an extended inline assembly operand. 
 *  Example: \code "=f" (result) \endcode */
class CT_GnuAsmOperand : public CTree {
  CTree *sons[7]; // [ open_square, symbol, close_square ], string, open, expr, close

public:
  /** Constructor.
   *  \param s The operand constraint string.
   *  \param o Left parenthesis before the operand.
   *  \param e The operand.
   *  \param c Right parenthesis behind the operand. */
  CT_GnuAsmOperand (CTree *s, CTree *o, CTree *e, CTree *c) {
    sons[0] = 0; sons[1] = 0; sons[2] = 0;
    AddSon (sons[3], s); AddSon (sons[4], o);
    AddSon (sons[5], e); AddSon (sons[6], c);
  }
  /** Constructor.
   *  \param no opening square backet of the symbolic operand name (optional)
   *  \param on symbolic operand name (optional)
   *  \param nc closing square backet of the symbolic operand name (optional)
   *  \param s The operand constraint string.
   *  \param o Left parenthesis before the operand.
   *  \param e The operand.
   *  \param c Right parenthesis behind the operand. */
  CT_GnuAsmOperand (CTree *no, CTree *on, CTree *nc,
      CTree *s, CTree *o, CTree *e, CTree *c) {
    AddSon (sons[0], no), AddSon (sons[1], on), AddSon (sons[2], nc),
    AddSon (sons[3], s); AddSon (sons[4], o);
    AddSon (sons[5], e); AddSon (sons[6], c);
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); } 
  /** Get the number of sons. */
  int Sons () const { return sons[0] ? 7 : 4; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 7, n); }
  /** Get the operand expression. */
  CT_Expression *Expr () const { return (CT_Expression*)sons[5]; }
  /** Get the operand constraint string. */
  CT_String *String () const { return (CT_String*)sons[3]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) {
    CTree::ReplaceSon (sons, 7, old_son, new_son);
  }
};

/** \class CT_GnuAsmOperands GnuCTree.h Puma/GnuCTree.h
 *  Tree node representing a list of extended inline assembly operands. 
 *  Example: \code : "=f" (result) : "f" (angle) \endcode */
class CT_GnuAsmOperands : public CT_List {
public:
  /** Constructor. */
  CT_GnuAsmOperands () { AddProperties (OPEN | SEPARATORS); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }  
};

/** \class CT_GnuAsmClobbers GnuCTree.h Puma/GnuCTree.h
 *  Tree node representing a list of extended inline assembly clobbers. 
 *  Example: \code : "r1", "r2", "r3", "r4", "r5" \endcode */
class CT_GnuAsmClobbers : public CT_List {
public:
  /** Constructor. */
  CT_GnuAsmClobbers () { AddProperties (OPEN | SEPARATORS); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }  
};

/** \class CT_GnuAttribute GnuCTree.h Puma/GnuCTree.h
 *  Tree node representing a attribute (gnu c syntax extension) 
 *  Example: \code __attribute__(interrupt) \endcode */
class CT_GnuAttribute : public CT_List {

public:
  /** Constructor. */
  CT_GnuAttribute () { AddProperties (OPEN_CLOSE | INTRO); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); } 
};

/*****************************************************************************/
/*                                                                           */
/*                              Expressions                                  */
/*                                                                           */
/*****************************************************************************/

/** \class CT_GnuStatementExpr GnuCTree.h Puma/GnuCTree.h
 *  Tree node representing a statement expression. 
 *  Example: \code ({ int i = 0; i++; }) \endcode */
class CT_GnuStatementExpr : public CT_Expression {
  CTree *sons[3]; // open, statement, close

public:
  /** Constructor.
   *  \param o Left parenthesis before the statement.
   *  \param s The statement. 
   *  \param c Right parenthesis behind the statement. */
  CT_GnuStatementExpr (CTree *o, CTree *s, CTree *c) {
    AddSon (sons[0], o); AddSon (sons[1], s); AddSon (sons[2], c);
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
  void ReplaceSon (CTree *old_son, CTree *new_son) {
    CTree::ReplaceSon (sons, 3, old_son, new_son);
  }
  /** Get the statement. */
  CT_CmpdStmt *CmpdStmt () const { return (CT_CmpdStmt*)sons[1]; }
};

/*****************************************************************************/
/*                                                                           */
/*                         Declaration specifiers                            */
/*                                                                           */
/*****************************************************************************/

/** \class CT_GnuTypeof GnuCTree.h Puma/GnuCTree.h
 *  Tree node representing a typeof expression. 
 *  Example: \code typeof(a+b) \endcode */
class CT_GnuTypeof : public CT_DeclSpec, public CSemValue {
  CTree *sons[5]; // key, open, type, close, expr

public:
  /** Constructor.
   *  \param k The keyword 'typeof'.
   *  \param o Left parenthesis before the type.
   *  \param t The type from which to get the type string.
   *  \param c Right parenthesis behind the type. */
  CT_GnuTypeof (CTree *k, CTree *o, CTree *t, CTree *c) {
    AddSon (sons[0], k); AddSon (sons[1], o); AddSon (sons[2], t);
    AddSon (sons[3], c); AddSon (sons[4], 0);
  }
  /** Constructor.
   *  \param k The keyword 'typeof'.
   *  \param e The expression from which to get the type string. */
  CT_GnuTypeof (CTree *k, CTree *e) {
    AddSon (sons[0], k); AddSon (sons[1], 0); AddSon (sons[2], 0);
    AddSon (sons[3], 0); AddSon (sons[4], e);
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 5); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 5, n); }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 5, old_son, new_son);
  }
  /** Get the expression from which to get the type string. */
  CTree *Expr () const { return sons[4]; }
  /** Get the type from which to get the type string. */
  CT_NamedType *TypeName () const { return (CT_NamedType*)sons[2]; }
  /** Get the semantic value information. */
  CSemValue *SemValue () const { return (CSemValue*)this; }
  /** Get the resulting type. */
  CTypeInfo *Type () const { return type; }
};

/*****************************************************************************/
/*                                                                           */
/*                              Statements                                   */
/*                                                                           */
/*****************************************************************************/

/** \class CT_GnuLocalLabelStmt GnuCTree.h Puma/GnuCTree.h
 *  Tree node representing a list of local labels.
 *  Example: \code __label__ L1, L2; \endcode */
class CT_GnuLocalLabelStmt : public CT_Statement, public CT_List {
public:
  /** Constructor. */
  CT_GnuLocalLabelStmt () { AddProperties (OPEN | SEPARATORS | CLOSE); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CT_List::Sons (); }
};

/** \class CT_GnuCaseStmt GnuCTree.h Puma/GnuCTree.h
 *  Tree node representing a label statement with case range.
 *  Example: \code case 10 ... 20: a++; \endcode */
class CT_GnuCaseStmt : public CT_Statement {
  CTree *sons[6]; // case id ... id, colon, stmt

public:
  /** Constructor.
   *  \param kw The keyword 'case'.
   *  \param expr1 The constant expression specifying the first case value.
   *  \param dots The ellipsis token '...'
   *  \param expr2 The constant expression specifying the second case value.
   *  \param c The colon.
   *  \param stmt The statement of the case. */
  CT_GnuCaseStmt (CTree *kw, CTree *expr1, CTree *dots, CTree *expr2,
      CTree *c, CTree *stmt) {
    AddSon (sons[0], kw); AddSon (sons[1], expr1); AddSon (sons[2], dots);
    AddSon (sons[3], expr2); AddSon (sons[4], c); AddSon (sons[5], stmt);
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
  /** Get the statement. */
  CT_Statement *Statement () const { return (CT_Statement*)sons[5]; }
  /** Get the first const expression. */
  CT_SimpleName *Expr1 () const { return (CT_SimpleName*)sons[1]; }
  /** Get the second const expression. */
  CT_SimpleName *Expr2 () const { return (CT_SimpleName*)sons[3]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) {
    CTree::ReplaceSon (sons, 6, old_son, new_son);
  }
};

} // namespace Puma

#endif /* __gnu_c_tree_h__ */
