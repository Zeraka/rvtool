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

#ifndef __CTree_h__
#define __CTree_h__

/** \file 
 *  C/C++ syntax tree classes. */

namespace Puma {


// Syntax tree node hierarchy:
class CTree;
class   CT_Statement;          
class     CT_LabelStmt;
class     CT_IfStmt;
class     CT_IfElseStmt;
class     CT_SwitchStmt;
class     CT_BreakStmt;
class     CT_ExprStmt;
class     CT_WhileStmt;
class     CT_DoStmt;
class     CT_ForStmt;
class     CT_ContinueStmt;
class     CT_ReturnStmt;
class     CT_GotoStmt;
class     CT_DeclStmt;
class     CT_CaseStmt;
class     CT_DefaultStmt;
class     CT_TryStmt;
class   CT_Expression;
class     CT_Call;
class       CT_CallExpr;
class       CT_ImplicitCall;
class     CT_ThrowExpr;
class     CT_NewExpr;
class     CT_DeleteExpr;
class     CT_ConstructExpr;
class     CT_Integer;
class     CT_Character;
class       CT_WideCharacter;
class     CT_Float;
class     CT_Bool;
class     CT_BracedExpr;
class     CT_BinaryExpr;
class       CT_MembPtrExpr;
class         CT_MembRefExpr;
class     CT_UnaryExpr;
class       CT_PostfixExpr;
class       CT_AddrExpr;
class       CT_DerefExpr;
class     CT_IfThenExpr;
class     CT_CmpdLiteral;
class     CT_IndexExpr;
class     CT_CastExpr;
class     CT_StaticCast;
class       CT_ConstCast;
class       CT_ReintCast;
class       CT_DynamicCast;
class     CT_TypeidExpr;
class     CT_SizeofExpr;
class     CT_AlignofExpr;
class     CT_TypeTraitExpr;
class     CT_OffsetofExpr;
class     CT_MembDesignator;
class     CT_IndexDesignator;
class     CT_ImplicitCast;
class     CT_MembInit;
class   CT_DeclSpec;
class     CT_PrimDeclSpec;
class     CT_NamedType;
class     CT_ClassSpec;
class       CT_UnionSpec;
class       CT_EnumSpec;
class     CT_ExceptionSpec;
class   CT_Declarator;
class     CT_InitDeclarator;
class     CT_BracedDeclarator;
class     CT_ArrayDeclarator;
class     CT_FctDeclarator;
class     CT_RefDeclarator;
class     CT_PtrDeclarator;
class     CT_MembPtrDeclarator;
class     CT_BitFieldDeclarator;
class   CT_Decl;
class     CT_ObjDecl;
class     CT_ArgDecl;
class     CT_AccessDecl;
class       CT_UsingDecl;
class     CT_FctDef;
class     CT_AsmDef;
class     CT_EnumDef;
class     CT_ClassDef;
class       CT_UnionDef;
class     CT_Enumerator;
class     CT_LinkageSpec;
class     CT_Handler;
class     CT_TemplateDecl;
class     CT_TemplateParamDecl;
class       CT_TypeParamDecl;
class       CT_NonTypeParamDecl;
class     CT_NamespaceDef;
class     CT_NamespaceAliasDef;
class     CT_UsingDirective;
class     CT_Condition;
class   CT_List;
class     CT_CmpdStmt;
class     CT_DeclSpecSeq;
class     CT_HandlerSeq;
class     CT_DesignatorSeq;
class     CT_DeclList;
class       CT_Program;
class       CT_ArgDeclList;
class         CT_ArgNameList;
class       CT_ArgDeclSeq;
class       CT_MembList;
class     CT_ExprList;
class     CT_DeclaratorList;
class     CT_BaseSpecList;
class     CT_MembInitList;
class     CT_SimpleName;
class       CT_SpecialName;
class         CT_PrivateName;
class         CT_OperatorName;
class         CT_DestructorName;
class         CT_ConversionName;
class         CT_TemplateName;
class       CT_QualName;
class         CT_RootQualName;
class     CT_String;
class       CT_WideString;
class     CT_TemplateParamList;
class     CT_TemplateArgList;
class     CT_ExtensionList;
class   CT_Token;
class   CT_Error;
class   CT_DelayedParse;
class   CT_BaseSpec;
class   CT_AccessSpec;
class   CT_ArrayDelimiter;
class   CT_Any;
class   CT_AnyList;
class   CT_AnyExtension;
class   CT_AnyCondition;

} // namespace Puma

#include "Puma/ErrorSeverity.h"
#include "Puma/CSemObject.h"
#include "Puma/CSemScope.h"
#include "Puma/CSemValue.h"
#include "Puma/CExprValue.h"
#include "Puma/CStrLiteral.h"
#include "Puma/CTypeInfo.h"
#include "Puma/Printable.h"
#include "Puma/CTokens.h"
#include "Puma/Token.h"

#include <iostream>
#include <map>
#include <string.h>

namespace Puma {


class ErrorStream;
class CObjectInfo;
class CStructure;

/*****************************************************************************/
/*                                                                           */
/*                    S y n t a x  t r e e  n o d e s                        */
/*                                                                           */
/*****************************************************************************/

/** \class CTree CTree.h Puma/CTree.h
 *  Base class for all C/C++ syntax tree classes. 
 *
 *  The syntax tree is the result of the syntactic analysis of the input source 
 *  code representing its syntactic structure according to the accepted grammar
 *  (see class Syntax). 
 *
 *  Objects of this class and classes derived from this class are created by 
 *  the tree builder component of %Puma during the parse process. A syntax tree 
 *  shall be destroyed using the tree builder that has created it by calling its 
 *  method Builder::destroy(CTree*) with the root node of the syntax tree as its 
 *  argument.
 *  
 *  The navigation in the syntax tree is done using the methods CTree::Parent(), 
 *  CTree::Sons(), and CTree::Son(int) const. In a syntax tree "sons" are 
 *  understood as the syntactic child nodes of a syntax tree node, whereas 
 *  "daughters" are understood are their semantic child nodes. 
 *
 *  Another way to traverse a syntax tree is to implement an own tree visitor 
 *  based on class Puma::CVisitor. This is recommended especially for larger 
 *  syntax trees.
 *
 *  A syntax tree node can be identified by comparing its node name with the node 
 *  identifier of the expected syntax tree node:
 *  \code if (node->NodeName() == Puma::CT_BinaryExpr::NodeId()) ... \endcode
 *  
 *  Based on the syntax tree further semantic analyses can be performed. Semantic 
 *  information, like scope, value, type, and object information, is linked into 
 *  the syntax tree. It can be accessed using the methods CTree::SemScope(), 
 *  CTree::SemValue(), and CTree::SemObject(). Some nodes provide short-cuts to
 *  the semantic type and value information by implementing the methods 
 *  CTree::Type() and CTree::Value().
 *
 *  The information of the syntax tree can be used to perform high-level 
 *  transformations of the source code (see class ManipCommander). */
class CTree {
  CTree * _parent;

public:
  /*DEBUG*/static int alloc;
  /*DEBUG*/static int release;

protected:
  /** Get the n-th son from given sons array. Skips empty (NULL) array items.
   *  \param sons The sons array.
   *  \param len Length of the sons array.
   *  \param n Index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (CTree * const *sons, int len, int n) const;
  /** Get the number of sons in the given sons array. Skips empty (NULL) array items.
   *  \param sons The sons array.
   *  \param len Length of the sons array. */
  int Sons (CTree * const *sons, int len) const;
  /** Replace a son.
   *  \param sons The sons array.
   *  \param len Length of the sons array.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree **sons, int len, CTree *old_son, CTree *new_son);
  /** Replace a son if it equals the given son.
   *  \param son The actual son.
   *  \param old_son The son to replace, must match the actual son.
   *  \param new_son The new son, overwrites the actual son. */
  void ReplaceSon (CTree *&son, CTree *old_son, CTree *new_son);
  /** Add a new son.
   *  \param son The actual son.
   *  \param new_son The new son, overwrites the actual son. */
  void AddSon (CTree *&son, CTree *new_son);
  /** Set the parent tree node.
   *  \param parent The new parent tree node. */
  void SetParent (const CTree *parent) { _parent = (CTree*)parent; }
  /** Set the parent tree node of the given tree node.
   *  \param node The tree node.
   *  \param parent The new parent. */
  void SetParent (CTree *node, const CTree *parent) { node->_parent = (CTree*)parent; }
  
protected:
  /** Default constructor. */
  CTree () : _parent(0) { /*DEBUG*/alloc++; }

public:
  /** Destructor. */
  virtual ~CTree () { /*DEBUG*/release++; }
  /** Get the number of sons. */
  virtual int Sons () const = 0;
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  virtual CTree *Son (int n) const { return (CTree*)0; }
  /** Get the node name (node identifier). */
  virtual const char *NodeName () const = 0;
  /** Get the first token of the syntactic construct represented by this sub-tree.
   *  \return The token or NULL. */
  virtual Token *token () const;
  /** Get the last token of the syntactic construct represented by this sub-tree.
   *  \return The token or NULL. */
  virtual Token *end_token () const;
  /** Get the CT_Token node of the first token of the syntactic construct represented by this sub-tree.
   *  \return The token node or NULL. */
  virtual CT_Token *token_node () const;
  /** Get the CT_Token node of the last token of the syntactic construct represented by this sub-tree.
   *  \return The token node or NULL. */
  virtual CT_Token *end_token_node () const;
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The son with which to replace. */
  virtual void ReplaceSon (CTree *old_son, CTree *new_son) {}
  /** Get the parent node.
   *  \return The parent node or NULL. */
  virtual CTree *Parent () const { return (CTree*)_parent; }

public: // semantic information
  /** Get the semantic type of the node.
   *  \return The type object or NULL. */
  virtual CTypeInfo *Type () const { return (CTypeInfo*)0; }
  /** Get the calculated value of the expression.
   *  \return The value object or NULL. */
  virtual CExprValue *Value () const { return (CExprValue*)0; }
  
  /** Get the scope opened by the node.
   *  \return The scope object or NULL. */
  virtual CSemScope *SemScope () const { return (CSemScope*)0; }
  /** Get the semantic value of the node.
   *  \return The value object or NULL. */
  virtual CSemValue *SemValue () const { return (CSemValue*)0; }
  /** Get the semantic information of the node.
   *  \return The semantic object or NULL. */
  virtual CSemObject *SemObject () const { return (CSemObject*)0; }
  
public: // node classification function
  /** Get a pointer to CT_SimpleName if the current node represents a name.
   *  \return The CT_SimpleName node or NULL. */
  virtual CT_SimpleName *IsSimpleName () { return 0; }
  /** Get a pointer to CT_String if the current node represents a string.
   *  \return The CT_String node or NULL. */
  virtual CT_String *IsString () { return 0; }
  /** Get a pointer to CT_Declarator if the current node represents a declarator.
   *  \return The CT_Declarator pointer or NULL. */
  virtual CT_Declarator *IsDeclarator () { return 0; }
  /** Get a pointer to CT_Statement if the current node represents a statement.
   *  \return The CT_Statement pointer or NULL. */
  virtual CT_Statement *IsStatement () { return 0; }
  /** Get a pointer to CT_Expression if the current node represents a expression.
   *  \return The CT_Expression pointer or NULL. */
  virtual CT_Expression *IsExpression () { return 0; }
  /** Get a pointer to CT_Decl if the current node represents a declaration.
   *  \return The CT_Decl pointer or NULL. */
  virtual CT_Decl *IsDeclaration () { return 0; }
  /** Get a pointer to CT_Call if the current node represents a call expression.
   *  \return The CT_Call pointer or NULL. */
  virtual CT_Call *IsCall () { return 0; }
  /** Get a pointer to CT_List if the current node represents a list.
   *  \return The CT_List pointer or NULL. */
  virtual CT_List *IsList () { return 0; }
  /** Get a pointer to CT_DelayedParse if the current node represents a delayed code fragment.
   *  \return The CT_DelayedParse pointer or NULL. */
  virtual CT_DelayedParse *IsDelayedParse () { return 0; }

public: // additional semantic information
  /** Return true if the tree has the constant value 0.
   *  \return True if constant value is 0. */
  bool HasValueNull () const;
};

/** \class CT_Error CTree.h Puma/CTree.h
 *  Error tree node that is inserted into the tree for syntactic constructs
 *  that could not be parsed. */
class CT_Error : public CTree {
public:
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 0; }
};

/** \class CT_Token CTree.h Puma/CTree.h
 *  Tree node representing a single token in the source code. */
class CT_Token : public CTree {
  Token *_token;
  unsigned long int _number;
  
public:
  /** Constructor. 
   *  \param token The represented token.
   *  \param number The token number (a consecutive number). */
  CT_Token (Token *token, unsigned long int number = 0) : 
    _token (token), _number (number) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 0; }
  /** Get the represented token. */
  Token *token () const { return _token; }
  /** Get the represented token. */
  Token *end_token () const { return _token; }
  /** Get this. */
  CT_Token *token_node () const { return (CT_Token*)this; }
  /** Get this. */
  CT_Token *end_token_node () const { return (CT_Token*)this; }
  /** Set the token number. 
   *  \param number The token number. */ 
  void Number (unsigned long int number) { _number = number; }
  /** Get the token number. Can be used to indentify this token. */
  unsigned long int Number () const { return _number; }
  
public:
  /** Own new operator reusing memory. */
  void *operator new (size_t);
  /** Own delete operator. */
  void operator delete (void *);
};

/*****************************************************************************/
/*                                                                           */
/*                              List nodes                                   */
/*                                                                           */
/*****************************************************************************/

/** \class CT_List CTree.h Puma/CTree.h
 *  Base class for tree nodes representing lists. */
class CT_List : public CTree {
  /** Son to index map type. */
  typedef std::map<CTree*,int> SonToIndexMap;

  Array<CTree*> _sons;
  int _properties;
  SonToIndexMap _son2idx;

protected:
  /** Constructor.
   *  \param size The initial list size.
   *  \param incr The initial increment count. 
   *  \param props The list properties (bit array). */
  CT_List(int size = 5, int incr = 5, int props = 0) : 
    _sons (size, incr), _properties (props) {}

public:
  /** %List properties. */
  enum {
    /** %List has a start token, like ':' in ":a(1),b(2)" */
    OPEN = 1,         
    /** %List has an end token */
    CLOSE = 2,        
    /** %List has opening and closing delimiters, like '(' and ')' */
    OPEN_CLOSE = 3,   
    /** %List has separators, like ',' */
    SEPARATORS = 4,   
    /** %List pretend to be empty, like for "(void)" */
    FORCE_EMPTY = 8,  
    /** %List has trailing separator, like "a,b,c," */
    END_SEP = 16,     
    /** %List has no separator before last element, like "(a,b...)" */
    NO_LAST_SEP = 32, 
    /** %List has an introduction chararacter, like "=" in "={a,b}" */
    INTRO = 64        
  };
 
  /** Get a pointer to this CT_List. */
  CT_List *IsList () { return this; }
  /** Get the number of list entries. */
  int Entries () const;
  /** Get the n-th list entry.
   *  \param n The index of the entry. 
   *  \return The list entry or NULL. */
  CTree *Entry (int n) const;
  /** Get the number of sons. */
  int Sons () const { return _sons.length (); }
  /** Get the n-th son.
   *  \param n The index of the son. 
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return _sons.lookup (n); }
  /** Get the index of the given son, or -1 if not found. */
  int Index (CTree *son);
  /** Get the list properties. */
  int GetProperties () const { return _properties; }
  /** Add a list property.
   *  \param p The property to add. */
  void AddProperties (int p) { _properties |= p; }
  /** Add a son.
   *  \param s The son to add. */
  void AddSon (CTree *s);
  /** Prepend a son.
   *  \param s The son to prepend. */
  void PrefixSon (CTree *s);
  /** Insert a son before another son.
   *  \param before The son to insert the new son before.
   *  \param son The son to insert. */
  void InsertSon (CTree *before, CTree *son); 
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son);
  /** Remove a son.
   *  \param son The son to remove. */
  void RemoveSon (CTree *son);
  /** Insert a son at the given index. 
   *  \param idx The index at which to insert.
   *  \param s The son to insert. */
  void InsertSon (int idx, CTree *s);
  /** Replace the son at the given index.
   *  \param idx The index of the son to replace.
   *  \param s The new son. */
  void ReplaceSon (int idx, CTree *s);
  /** Remove the son at the given index. 
   *  \param idx The index of the son to remove. */
  void RemoveSon (int idx);
};

/** \class CT_ExprList CTree.h Puma/CTree.h
 *  Tree node representing an expression list. */
class CT_ExprList : public CT_List, public CSemValue, public CSemObject {
public:
  /** Constructor. */
  CT_ExprList () { AddProperties (SEPARATORS); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }

  /** Get the type of the last expression in the expression list.
   *  \return The type or NULL. */
  CTypeInfo *Type () const { return type; }
  /** Get the value of the last expression in the expression list.
   *  \return The value of NULL. */
  CExprValue *Value () const { return value; }
  /** Get the semantic value of the node. */
  CSemValue *SemValue () const { return (CSemValue*)this; }
  /** Get the semantic information about the node. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
};

/** \class CT_DeclaratorList CTree.h Puma/CTree.h
 *  Tree node representing a list of declarators. */
class CT_DeclaratorList : public CT_List {
public:
  /** Constructor. */
  CT_DeclaratorList () { AddProperties (SEPARATORS); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_EnumeratorList CTree.h Puma/CTree.h
 *  Tree node representing a list of enumerator constants. */
class CT_EnumeratorList : public CT_List {
public:
  /** Constructor. */
  CT_EnumeratorList () { AddProperties (SEPARATORS | OPEN_CLOSE); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};
   
/** \class CT_DeclList CTree.h Puma/CTree.h
 *  Tree node representing a list of declarations. */
class CT_DeclList : public CT_List {
public:
  /** Constructor. 
   *  \param size The initial size of the list.
   *  \param incr The initial increment count of the list. */
  CT_DeclList (int size = 20, int incr = 20) : CT_List (size, incr) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Set the linkage specifiers to each declaration in the list.
   *  \param l The linkage specifiers node. */
  void Linkage (CT_LinkageSpec *l);
};

/** \class CT_DeclSpecSeq CTree.h Puma/CTree.h
 *  Tree node representing a sequence of declaration specifiers. */
class CT_DeclSpecSeq : public CT_List {
public:
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_CmpdStmt CTree.h Puma/CTree.h
 *  Tree node representing a compound statement. */
class CT_CmpdStmt : public CT_List, public CSemScope {
public:
  /* Constructor. */
  CT_CmpdStmt () { AddProperties (OPEN_CLOSE); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the scope opened by the compound statement. */
  CSemScope *SemScope () const { return (CSemScope*)this; }
};

/** \class CT_HandlerSeq CTree.h Puma/CTree.h
 *  Tree node representing an exception handler sequence. */
class CT_HandlerSeq : public CT_List {
public:
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_TemplateParamList CTree.h Puma/CTree.h
 *  Tree node representing a template parameter list. */
class CT_TemplateParamList : public CT_List, public CSemScope {
public:
  CT_TemplateParamList () { AddProperties (INTRO | SEPARATORS | OPEN_CLOSE); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the scope opened by the template parameter list. */
  CSemScope *SemScope () const { return (CSemScope*)this; }
};

/** \class CT_TemplateArgList CTree.h Puma/CTree.h
 *  Tree node representing a template argument list. */
class CT_TemplateArgList : public CT_List {
public:
  /** Constructor. */
  CT_TemplateArgList () { AddProperties (SEPARATORS | OPEN_CLOSE); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_ExtensionList CTree.h Puma/CTree.h
 *  Tree node representing a sequence of compiler specific extensions such
 *  as __attribute__((...)) nodes. */
class CT_ExtensionList : public CT_List {
public:
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

/** \class CT_Expression CTree.h Puma/CTree.h
 *  Base class for all expression tree nodes. */
class CT_Expression : public CTree, public CSemValue {
public:
  /** Constructor. */
  CT_Expression () {}

  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 0; }
  /** Get the type of the expression.
   *  \return The type information object or NULL. */
  CTypeInfo *Type () const { return type; }
  /** Get the value of the expression.
   *  \return The value object or NULL. */
  CExprValue *Value () const { return value; }
  /** Get the semantic value information of the expression.
   *  \return The value object or NULL. */
  CSemValue *SemValue () const { return (CSemValue*)this; }
  /** Get this. */
  virtual CT_Expression *IsExpression () { return this; }
};

/** \class CT_Call CTree.h Puma/CTree.h
 *  Tree node representing explicit or implicit function calls 
 *  including built-in or user-defined functions and overloaded
 *  operators. */
class CT_Call : public CT_Expression, public CSemObject {
protected:
  /** Constructor. */
  CT_Call () {}
  
public:
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the semantic information of the call. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
  /** Get this. */
  CT_Call *IsCall () { return this; }
};

/** \class CT_ImplicitCall CTree.h Puma/CTree.h
 *  Tree node representing implicit function calls detected by
 *  the semantic analysis. 
 *  Example:
 *  \code
 * class Number {
 *   int _n;
 * public:
 *   Number(int n) : _n(n) {}
 *   int operator+(const Number& n) { return n._n + _n; }
 * };
 *     
 * Number one(1), two(2);
 * one + two;  // implicitely calls one.operator+(two)
 *  \endcode */
class CT_ImplicitCall : public CT_Call {
  CTree *_arg;

public:
  /** Constructor.
   *  \param arg The call argument. */
  CT_ImplicitCall (CTree *arg) { AddSon (_arg, arg); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 1; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return (n == 0) ? _arg : (CTree*)0; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) 
   { CTree::ReplaceSon (_arg, old_son, new_son); }
};

/** \class CT_String CTree.h Puma/CTree.h
 *  Tree node representing a string literal. 
 *  Example: \code "abc" \endcode */
class CT_String : public CT_List, public CSemValue {
public:
  /** Constructor. 
   *  \param size The number of sub-strings. */
  CT_String (int size) : CT_List (size, 1) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }

  /** Get the type of the string. 
   *  \return The type or NULL. */
  CTypeInfo *Type () const { return type; }
  /** Get the string value.
   *  \return The value or NULL. */
  CExprValue *Value () const { return value; }
  /** Get the semantic value info object.
   *  \return The semantic value object or NULL. */
  CSemValue *SemValue () const { return (CSemValue*)this; }
  /** Get this. */
  virtual CT_String *IsString () { return this; }
};

/** \class CT_WideString CTree.h Puma/CTree.h
 *  Tree node representing a wide string literal. 
 *  Example: \code L"abc" \endcode */
class CT_WideString : public CT_String {
public:
  /** Constructor.
   *  \param size The number of sub-strings. */
  CT_WideString (int size) : CT_String (size) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_Integer CTree.h Puma/CTree.h
 *  Tree node representing an integer constant. 
 *  Example: \code 1234 \endcode */
class CT_Integer : public CT_Expression {
  CTree *_value;  // CT_Token

public:
  /** Constructor.
   *  \param token The token containing the integer value. */
  CT_Integer (CTree *token) { AddSon (_value, token); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return _value ? 1 : 0; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return (n == 0) ? _value : (CTree*)0; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) 
   { CTree::ReplaceSon (_value, old_son, new_son); }
};

/** \class CT_Character CTree.h Puma/CTree.h
 *  Tree node representing a single character constant. 
 *  Example: \code 'a' \endcode */
class CT_Character : public CT_Expression {
  CTree *_value;  // CT_Token

public:
  /** Constructor.
   *  \param token The token containing the character value. */
  CT_Character (CTree *token) { AddSon (_value, token); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 1; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return (n == 0) ? _value : (CTree*)0; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) 
   { CTree::ReplaceSon (_value, old_son, new_son); }
};

/** \class CT_WideCharacter CTree.h Puma/CTree.h
 *  Tree node representing a wide character constant. 
 *  Example: \code L'a' \endcode */
class CT_WideCharacter : public CT_Character {
public:
  /** Constructor.
   *  \param token The token containing the wide character value. */
  CT_WideCharacter (CTree *token) : CT_Character (token) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_Float CTree.h Puma/CTree.h
 *  Tree node representing a floating point constant. 
 *  Example: \code 12.34 \endcode */
class CT_Float : public CT_Expression {
  CTree *_value;  // CT_Token

public:
  /** Constructor.
   *  \param token The token containing the floating point value. */
  CT_Float (CTree *token) { AddSon (_value, token); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 1; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return (n == 0) ? _value : (CTree*)0; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) 
   { CTree::ReplaceSon (_value, old_son, new_son); }
};

/** \class CT_Bool CTree.h Puma/CTree.h
 *  Tree node representing a boolean literal. 
 *  Examples: 
 *  \code 
 * true
 * false
 *  \endcode */
class CT_Bool : public CT_Expression {
  CTree *_value;  // CT_Token

public:
  /** Constructor.
   *  \param token The token containing the boolean value. */
  CT_Bool (CTree *token) { AddSon (_value, token); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 1; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return (n == 0) ? _value : (CTree*)0; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) 
   { CTree::ReplaceSon (_value, old_son, new_son); }
};

/** \class CT_BracedExpr CTree.h Puma/CTree.h
 *  Tree node representing a braced expression.
 *  Example: \code (a+b) \endcode */
class CT_BracedExpr : public CT_Expression {
  CTree *sons[3]; // open, expr, close

public:
  /** Constructor.
   *  \param o The opening brace.
   *  \param e The enclosed expression.
   *  \param c The closing brace. */
  CT_BracedExpr (CTree *o, CTree *e, CTree *c) { 
    AddSon (sons[0], o); AddSon (sons[1], e); AddSon (sons[2], c); 
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
  /** Get the enclosed expression. */
  CTree *Expr () const { return sons[1]; }
  /** Get the semantic value of the expression. */
  CSemValue *SemValue () const { return (CSemValue*)this; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 3, old_son, new_son);
  }
};

/** \class CT_SimpleName CTree.h Puma/CTree.h
 *  Base class for all tree nodes representing a name. 
 *  Example: \code a \endcode */
class CT_SimpleName : public CT_List, public Printable, 
                      public CSemValue, public CSemObject {
protected:
  /** Constructor.
   *  \param size The number of sub-names (for qualified names). */
  CT_SimpleName (int size) : CT_List (size, 1) {}
  /** Constructor.
   *  \param size The number of sub-names (for qualified names). 
   *  \param properties Additional name list properties (for root qualified names). */
  CT_SimpleName (int size, int properties) : 
    CT_List (size, 2, properties) {}
  
public:
  /** Constructor.
   *  \param n The sub-tree containing the name. */
  CT_SimpleName (CTree *n) : CT_List (1, 1) { AddSon (n); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the string containing the name. */
  virtual const char *Text () const 
   { return Son (Sons ()-1)->token ()->text (); }
  /** Print the name on the given stream. 
   *  \param os The output stream. */
  virtual void print (std::ostream &os) const { os << Text (); }
  /** Get this. */
  virtual CT_SimpleName *Name () const { return (CT_SimpleName*)this; }
  /** Get the type of the entity represented by the name. */
  CTypeInfo *Type () const { return type; }
  /** Get the value of the entity represented by the name. */ 
  CExprValue *Value () const { return value; }
  /** Get the sematic value information of the name. */
  CSemValue *SemValue () const { return (CSemValue*)this; }
  /** Get the sematic information about the name. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
  /** Get this. */
  virtual CT_SimpleName *IsSimpleName () { return this; }  

public:
  /** Own new operator reusing memory. */
  void *operator new (size_t);
  /** Own delete operator. */
  void operator delete (void *);
};

/** \class CT_SpecialName CTree.h Puma/CTree.h
 *  Base class for tree nodes representing a special name, like destructor names. */
class CT_SpecialName : public CT_SimpleName {
  char *_name;
  
protected:
  /** Constructor.
   *  \param size The number of sub-names (for qualified names). */
  CT_SpecialName (int size = 1) : CT_SimpleName (size), _name (0) {}
  
public:
  /** Destructor. Deletes the name string. */
  ~CT_SpecialName () { if (_name) delete[] _name; }
  /** Get the string containing the name. */
  const char *Text () const { return _name; }
  /** Set the name. The name is copied.
   *  \param n The name. */
  void Name (const char *n) { 
    if (n) { 
      _name = new char[strlen(n) + 1];
      strcpy (_name,n);
    }
  }

public:
  /** Own new operator reusing memory. */
  void *operator new (size_t);
  /** Own delete operator. */
  void operator delete (void *);
};

/** \class CT_PrivateName CTree.h Puma/CTree.h
 *  Tree node representing a private name. Private names 
 *  are generated names for instance for abstract declarators.
 *  Example: 
 *  \code 
 * void foo(int*);  // first parameter of foo has private name
 *  \endcode */
class CT_PrivateName : public CT_SpecialName {
public:
  /** Constructor.
   *  \param n The private (generated) name. */
  CT_PrivateName (const char *n) { Name (n); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 0; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return (CTree*)0; }

public:
  /** Own new operator reusing memory. */
  void *operator new (size_t);
  /** Own delete operator. */
  void operator delete (void *);
};

/** \class CT_DestructorName CTree.h Puma/CTree.h
 *  Tree node representing a destructor name.
 *  Example: \code ~X \endcode */
class CT_DestructorName : public CT_SpecialName {
public:
  /** Constructor.
   *  \param t The tilde operator.
   *  \param n The class name. */
  CT_DestructorName (CTree *t, CTree *n);
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }

public:
  /** Own new operator reusing memory. */
  void *operator new (size_t);
  /** Own delete operator. */
  void operator delete (void *);
};

/** \class CT_TemplateName CTree.h Puma/CTree.h
 *  Tree node representing a template name.
 *  Example: \code X<T> \endcode */
class CT_TemplateName : public CT_SpecialName {
public:
  /** Constructor.
   *  \param n The template class or function name.
   *  \param a The template argument list. */
  CT_TemplateName (CTree *n, CTree *a) 
   { AddSon (n); AddSon (a); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the template argument list. */
  CT_TemplateArgList *Arguments () const 
   { return (CT_TemplateArgList*)Son (Sons ()-1); }
  /** Get the template class or function name. */
  CT_SimpleName *TemplateName () const 
   { return (CT_SimpleName*)Son (Sons ()-2); }
  // may change in the future
  const char *Text () const { return TemplateName ()->Text (); }

public:
  /** Own new operator reusing memory. */
  void *operator new (size_t);
  /** Own delete operator. */
  void operator delete (void *);
};

/** \class CT_OperatorName CTree.h Puma/CTree.h
 *  Tree node representing the name of an overloaded operator. 
 *  Example: \code operator== \endcode */
class CT_OperatorName : public CT_SpecialName {
  int _oper;

public:
  /** Complex operator types. */
  enum { 
    FCT_CALL = -100,  /** Function call operator, i.e. (). */
    SUBSCRIPT,        /** Array subscript operator, i.e. []. */
    NEW_ARRAY,        /** New array operator, i.e. new[]. */
    DEL_ARRAY         /** Delete array operator, i.e. delete[]. */
  };
 
public:
  /** Constructor.
   *  \param op The token containing the operator. */
  CT_OperatorName (CTree *op);
  /** Constructor.
   *  \param f The operator function keyword 'operator'.
   *  \param op The token containing the operator. 
   *  \param o The token of '[' or '('.
   *  \param c The token of ']' or ')'. */
  CT_OperatorName (CTree *f, CTree *op, CTree *o, CTree *c);
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the operator type (either the token type or one of 
   *  the complex operator types). */
  int Operator () const { return _oper; }

public:
  /** Own new operator reusing memory. */
  void *operator new (size_t);
  /** Own delete operator. */
  void operator delete (void *);
};

/** \class CT_ConversionName CTree.h Puma/CTree.h
 *  Tree node representing the name of a conversion function.
 *  Example: \code operator int* \endcode */
class CT_ConversionName : public CT_SpecialName {
public:
  /** Constructor.
   *  \param f The operator function keyword 'operator'.
   *  \param t The sub-tree containing the conversion type. */
  CT_ConversionName (CTree *f, CTree *t);
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the conversion type. */
  CT_NamedType *TypeName () const { return (CT_NamedType*)Son (Sons ()-1); }

public:
  /** Own new operator reusing memory. */
  void *operator new (size_t);
  /** Own delete operator. */
  void operator delete (void *);
};

/** \class CT_QualName CTree.h Puma/CTree.h
 *  Tree node representing a qualified name.
 *  Example: \code X::Y::Z \endcode */
class CT_QualName : public CT_SimpleName {
public:
  /** Constructor.
   *  \param size The initial number sub-names plus separators. */
  CT_QualName (int size = 3) : 
    CT_SimpleName (size, CT_List::SEPARATORS) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Print the qualified name on the given stream. 
   *  \param os The output stream. */
  void print (std::ostream &os) const;
  /** Get the last name of the qualified name, e.g. Z of qualified name X::Y::Z. */
  CT_SimpleName *Name () const { return (CT_SimpleName*)Son (Sons ()-1); }
  /** Get the string containing the last name of the qualified name. */
  const char *Text () const { return Name ()->Text (); }
  /** Get the type of the last name. */
  CTypeInfo *Type () const { return Name ()->Type (); }
  /** Get the value of the last name. */
  CExprValue *Value () const { return Name ()->Value (); }
  /** Get the semantic value object of the last name. */
  CSemValue *SemValue () const { return Name ()->SemValue (); }
  /** Get the semantic information of the last name. */
  CSemObject *SemObject () const { return Name ()->SemObject (); }

public:
  /** Own new operator reusing memory. */
  void *operator new (size_t);
  /** Own delete operator. */
  void operator delete (void *);
};

/** \class CT_RootQualName CTree.h Puma/CTree.h
 *  Tree node representing a qualified name with introducing name separator.
 *  Example: \code ::X::Y::Z \endcode */
class CT_RootQualName : public CT_QualName {
public:
  /** Constructor.
   *  \param size Initial number of sub-name plus separator. */
  CT_RootQualName (int size = 2) : 
    CT_QualName (size) { AddProperties (INTRO); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }

public:
  /** Own new operator reusing memory. */
  void *operator new (size_t);
  /** Own delete operator. */
  void operator delete (void *);
};

/** \class CT_BinaryExpr CTree.h Puma/CTree.h
 *  Tree node representing a binary expression.
 *  Example: \code a+b \endcode */
class CT_BinaryExpr : public CT_Call {
  CTree *sons[3]; // expr, oper, expr

public:
  /** Constructor. 
   *  \param l Left hand side of the expression. 
   *  \param o The operator token. 
   *  \param r Right hand side of the expression. */
  CT_BinaryExpr (CTree *l, CTree *o, CTree *r) {
    AddSon (sons[0], l); AddSon (sons[1], o); AddSon (sons[2], r);
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
};

/** \class CT_MembPtrExpr CTree.h Puma/CTree.h
 *  Tree node representing a member pointer expression.
 *  Example: \code a->b \endcode */
class CT_MembPtrExpr : public CT_Expression, public CSemObject {
  CTree *sons[3]; // expr, oper, expr
  
public:
  /** Constructor.
   *  \param e Expression on which to call the member.
   *  \param o The arrow operator token.
   *  \param i The member name. */
  CT_MembPtrExpr (CTree *e, CTree *o, CTree *i) {
    AddSon (sons[0], e); AddSon (sons[1], o); AddSon (sons[2], i);
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
};

/** \class CT_MembRefExpr CTree.h Puma/CTree.h
 *  Tree node representing a member reference expression.
 *  Example: \code a.b \endcode */
class CT_MembRefExpr : public CT_MembPtrExpr {
public:
  /** Constructor.
   *  \param e Expression on which to call the member.
   *  \param o The dot operator.
   *  \param i The member name. */
  CT_MembRefExpr (CTree *e, CTree *o, CTree *i) :
    CT_MembPtrExpr (e, o, i) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_UnaryExpr CTree.h Puma/CTree.h
 *  Base class for tree nodes representing unary expressions. 
 *  Example: \code !a \endcode */
class CT_UnaryExpr : public CT_Call {
  CTree *sons[2]; // oper, expr

public:
  /** Constructor.
   *  \param o The unary operator.
   *  \param e The expression on which the operator is invoked. */
  CT_UnaryExpr (CTree *o, CTree *e) { AddSon (sons[0], o); AddSon (sons[1], e); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 2; }
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
  /** Get the expression node. */
  CTree *Expr () const { return sons[1]; }
};

/** \class CT_PostfixExpr CTree.h Puma/CTree.h
 *  Tree node representing a postfix expression.
 *  Example: \code a++ \endcode */
class CT_PostfixExpr : public CT_UnaryExpr {
public:
  /** Constructor.
   *  \param e The expression on which to invoke the operator. 
   *  \param o The postfix operator. */
  CT_PostfixExpr (CTree *e, CTree *o) :
    CT_UnaryExpr (e, o) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_AddrExpr CTree.h Puma/CTree.h
 *  Tree node representing an address expression.
 *  Example: \code &a \endcode */
class CT_AddrExpr : public CT_UnaryExpr {
public:
  /** Constructor.
   *  \param o The address operator, i.e. '&'.
   *  \param e The expression from which to take the address. */
  CT_AddrExpr (CTree *o, CTree *e) :
    CT_UnaryExpr (o, e) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_DerefExpr CTree.h Puma/CTree.h
 *  Tree node representing a pointer dereferencing expression.
 *  Example: \code *a \endcode */
class CT_DerefExpr : public CT_UnaryExpr {
public:
  /** Constructor.
   *  \param o The dereferencing operator, i.e. '*'.
   *  \param e The expression to dereference. */
  CT_DerefExpr (CTree *o, CTree *e) :
    CT_UnaryExpr (o, e) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_DeleteExpr CTree.h Puma/CTree.h
 *  Tree node representing a delete expression.
 *  Example: \code delete a \endcode */
class CT_DeleteExpr : public CT_Expression, public CSemObject {
  CTree *sons[2]; // oper, expr

public:
  /** Constructor.
   *  \param op The delete operator.
   *  \param e The expression representing the object to delete. */
  CT_DeleteExpr (CTree *op, CTree *e) { AddSon (sons[0], op); AddSon (sons[1], e); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 2; }
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
  /** Get the expression. */
  CTree *Expr () const { return sons[1]; }
  /** Get the operator name, i.e. 'delete' or 'delete[]'. */
  CT_SimpleName *OperName () const { return (CT_SimpleName*)sons[0]; }
  /** Get the semantic information. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
};

/** \class CT_NewExpr CTree.h Puma/CTree.h
 *  Tree node representing a new expression.
 *  Example: \code new A() \endcode */
class CT_NewExpr : public CT_Expression, public CSemObject {
  CTree *sons[6]; // oper, placement, open, type, close, init

public:
  /** Constructor.
   *  \param op The new operator.
   *  \param p The optional placement expression.
   *  \param o The optional left parenthesis around the type identifier.
   *  \param t The type identifier specifying the type of the object to create.
   *  \param c The optional right parenthesis around the type identifier.
   *  \param i The optional initializer. */
  CT_NewExpr (CTree *op, CTree *p, CTree *o, CTree *t, CTree *c, CTree *i) {
    AddSon (sons[0], op); AddSon (sons[1], p); AddSon (sons[2], o); 
    AddSon (sons[3], t); AddSon (sons[4], c); AddSon (sons[5], i); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 6); }
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
  /** Get the operator name. */
  CT_SimpleName *OperName () const { return (CT_SimpleName*)sons[0]; }
  /** Get the placement expression. */
  CT_ExprList *Placement () const { return (CT_ExprList*)sons[1];; }
  /** Get the initializer. */
  CT_ExprList *Initializer () const { return (CT_ExprList*)sons[5]; }
  /** Get the type of the object to create. */
  CT_NamedType *TypeName () const { return (CT_NamedType*)sons[3]; }
  /** Get the semantic information. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
};

/** \class CT_IfThenExpr CTree.h Puma/CTree.h
 *  Tree node representing an if-then expression.
 *  Example: \code a>0?a:b \endcode or \code a?:b \endcode */
class CT_IfThenExpr : public CT_Expression {
  CTree *sons[5]; // cond, oper, left, colon, right

public:
  /** Constructor.
   *  \param c1 The condition expression.
   *  \param o The question mark operator. 
   *  \param l The expression to the left of the colon.
   *  \param c2 The colon operator.
   *  \param r The expression to the right of the colon. */ 
  CT_IfThenExpr (CTree *c1, CTree *o, CTree *l, CTree *c2, CTree *r) {
    AddSon (sons[0], c1); AddSon (sons[1], o); AddSon (sons[2], l); 
    AddSon (sons[3], c2); AddSon (sons[4], r);
  }
  /** Constructor.
   *  \param c1 The condition expression.
   *  \param o The question mark operator. 
   *  \param c2 The colon operator.
   *  \param r The expression to the right of the colon. */ 
  CT_IfThenExpr (CTree *c1, CTree *o, CTree *c2, CTree *r) {
    AddSon (sons[0], c1); AddSon (sons[1], o); AddSon (sons[2], 0); 
    AddSon (sons[3], c2); AddSon (sons[4], r);
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
  /** Get the condition expression. */
  CTree *Condition () const { return sons[0]; }
  /** Get the left expression (condition=true). */
  CTree *LeftOperand () const { return sons[2]; }
  /** Get the right expression (condition=false). */
  CTree *RightOperand () const { return sons[4]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 5, old_son, new_son);
  }
};

/** \class CT_CmpdLiteral CTree.h Puma/CTree.h
 *  Tree node representing a compound literal.
 *  Example: \code (int[]){1,2,3) \endcode */
class CT_CmpdLiteral : public CT_Expression, public CSemObject {
  CTree *sons[4]; // open, type, close, init

public:
  /** Constructor.
   *  \param r Left parenthesis of the type name.
   *  \param t The type name.
   *  \param cr Right parenthesis of the type name.
   *  \param i The initializer list. */
  CT_CmpdLiteral (CTree *r, CTree *t, CTree *cr, CTree *i) {
    AddSon (sons[0], r); AddSon (sons[1], t); 
    AddSon (sons[2], cr); AddSon (sons[3], i);
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
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 4, old_son, new_son);
  }
  /** Get the type name. */
  CT_NamedType *TypeName () const { return (CT_NamedType*)sons[1]; }
  /** Get the initializer list. */
  CT_ExprList *Initializer () const { return (CT_ExprList*)sons[3]; }
  /** Get the semantic information about the created object. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
};

/** \class CT_ConstructExpr CTree.h Puma/CTree.h
 *  Tree node representing a construct expression.
 *  Example: \code std::string("abc") \endcode */
class CT_ConstructExpr : public CT_Expression, public CSemObject {
  CTree *sons[2]; // type, init

public:
  /** Constructor.
   *  \param t The type name.
   *  \param i The initializer list. */
  CT_ConstructExpr (CTree *t, CTree *i) { AddSon (sons[0], t); AddSon (sons[1], i); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 2; }
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
  /** Get the type name. */
  CT_NamedType *TypeName () const { return (CT_NamedType*)sons[0]; }
  /** Get the initializer. */
  CT_ExprList *Initializer () const { return (CT_ExprList*)sons[1]; }
  /** Get the semantic information about the created object. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
};

/** \class CT_ThrowExpr CTree.h Puma/CTree.h
 *  Tree node representing a throw expression.
 *  Example: \code throw std::exception() \endcode */
class CT_ThrowExpr : public CT_Expression {
  CTree *sons[2]; // throw, expr

public:
  /** Constructor.
   *  \param t The 'throw' keyword.
   *  \param e The expression. */
  CT_ThrowExpr (CTree *t, CTree *e = (CTree*)0) { AddSon (sons[0], t); AddSon (sons[1], e); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 2); }
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
  /** Get the expression. */
  CTree *Expr () const { return sons[1]; }
};

/** \class CT_IndexExpr CTree.h Puma/CTree.h
 *  Tree node representing an index expression. 
 *  Example: \code a[1] \endcode */
class CT_IndexExpr : public CT_Call {
  CTree *sons[4]; // expr, open, index, close

public:
  /** Constructor.
   *  \param e The expression on which to invoke the index operator.
   *  \param o Left parenthesis of the index expression.
   *  \param i The index expression. 
   *  \param c Right parenthesis of the index expression. */
  CT_IndexExpr (CTree *e, CTree *o, CTree *i, CTree *c) {
    AddSon (sons[0], e); AddSon (sons[1], o); 
    AddSon (sons[2], i); AddSon (sons[3], c);
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
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 4, old_son, new_son);
  }
};

/** \class CT_CallExpr CTree.h Puma/CTree.h
 *  Tree node representing a function call expression.
 *  Example: \code f(i) \endcode */
class CT_CallExpr : public CT_Call {
  CTree *sons[2]; // expr, args

public:
  /** Constructor.
   *  \param e The expression on which the call is invoked. */
  CT_CallExpr (CTree *e) { AddSon (sons[0], e); AddSon (sons[1], 0); }
  /** Constructor.
   *  \param e The expression on which the call is invoked.
   *  \param l The argument list of the call. */
  CT_CallExpr (CTree *e, CTree *l) { AddSon (sons[0], e); AddSon (sons[1], l); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 2); }
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
  CTree *Expr () const { return sons[0]; }
  CT_ExprList *Arguments () const { return (CT_ExprList*)sons[1]; }
};

/** \class CT_CastExpr CTree.h Puma/CTree.h
 *  Tree node representing a cast expression.
 *  Example: \code (int)a \endcode */
class CT_CastExpr : public CT_Expression {
  CTree *sons[4]; // open, type, close, expr

public:
  /** Constructor.
   *  \param o Left parenthesis of the type name.
   *  \param t The type to cast to.
   *  \param c Right parenthesis of the type name. 
   *  \param e The expression to cast. */
  CT_CastExpr (CTree *o, CTree *t, CTree *c, CTree *e) {
    AddSon (sons[0], o); AddSon (sons[1], t); 
    AddSon (sons[2], c); AddSon (sons[3], e);
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
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 4, old_son, new_son);
  }
  /** Get the casted expression. */
  CTree *Expr () const { return sons[3]; }
  /** Get the type to cast to. */
  CT_NamedType *TypeName () const { return (CT_NamedType*)sons[1]; }
};

/** \class CT_StaticCast CTree.h Puma/CTree.h
 *  Tree node representing a static cast.
 *  Example: \code static_cast<int>(a) \endcode */
class CT_StaticCast : public CT_Expression {
  CTree *sons[5]; // cast, open, type, close, expr

public:
  /** Constructor.
   *  \param cst The cast operator, i.e. 'static_cast'.
   *  \param o Left arrow bracket of the type name.
   *  \param t The type to cast to.
   *  \param c Right array bracket of the type name.
   *  \param e The expression to cast. */
  CT_StaticCast (CTree *cst, CTree *o, CTree *t, CTree *c, CTree *e) {
    AddSon (sons[0], cst); AddSon (sons[1], o); AddSon (sons[2], t); 
    AddSon (sons[3], c); AddSon (sons[4], e);
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 5; }
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
  /** Get the casted expression. */
  CTree *Expr () const { return sons[4]; }
  /** Get the type to cast to. */
  CT_NamedType *TypeName () const { return (CT_NamedType*)sons[2]; }
};

/** \class CT_ConstCast CTree.h Puma/CTree.h
 *  Tree node representing a const cast.
 *  Example: \code const_cast<int>(a) \endcode */
class CT_ConstCast : public CT_StaticCast {
public:
  /** Constructor.
   *  \param cst The cast operator, i.e. 'const_cast'.
   *  \param o Left arrow bracket of the type name.
   *  \param t The type to cast to.
   *  \param c Right array bracket of the type name.
   *  \param e The expression to cast. */
  CT_ConstCast (CTree *cst, CTree *o, CTree *t, CTree *c, CTree *e) :
    CT_StaticCast (cst, o, t, c, e) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_ReintCast CTree.h Puma/CTree.h
 *  Tree node representing a reinterpret cast.
 *  Example: \code reinterpret_cast<int>(a) \endcode */
class CT_ReintCast : public CT_StaticCast {
public:
  /** Constructor.
   *  \param cst The cast operator, i.e. 'reinterpret_cast'.
   *  \param o Left arrow bracket of the type name.
   *  \param t The type to cast to.
   *  \param c Right array bracket of the type name.
   *  \param e The expression to cast. */
  CT_ReintCast (CTree *cst, CTree *o, CTree *t, CTree *c, CTree *e) :
    CT_StaticCast (cst, o, t, c, e) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_DynamicCast CTree.h Puma/CTree.h
 *  Tree node representing a dynamic cast.
 *  Example: \code dynamic_cast<int>(a) \endcode */
class CT_DynamicCast : public CT_StaticCast {
public:
  /** Constructor.
   *  \param cst The cast operator, i.e. 'dynamic_cast'.
   *  \param o Left arrow bracket of the type name.
   *  \param t The type to cast to.
   *  \param c Right array bracket of the type name.
   *  \param e The expression to cast. */
  CT_DynamicCast (CTree *cst, CTree *o, CTree *t, CTree *c, CTree *e) :
    CT_StaticCast (cst, o, t, c, e) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_ImplicitCast CTree.h Puma/CTree.h
 *  Tree node representing an implicit cast.
 *  Example: 
 *  \code 
 * int i = 1.2;  // implicit cast from float to int 
 *  \endcode */
class CT_ImplicitCast : public CT_Expression {
  CTree *_expr; // casted expression

public:
  /** Constructor.
   *  \param e The expression that is implicitely casted. */
  CT_ImplicitCast (CTree *e) { AddSon (_expr, e); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 1; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return n == 0 ? _expr : (CTree*)0; }
  /** Get the casted expression. */
  CTree *Expr () const { return _expr; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) 
   { CTree::ReplaceSon (_expr, old_son, new_son); }
};

/** \class CT_TypeidExpr CTree.h Puma/CTree.h
 *  Tree node representing a typeid expression.
 *  Example: \code typeid(X) \endcode */
class CT_TypeidExpr : public CT_Expression {
  CTree *sons[4]; // typeid, open, type_id/expr, close

public:
  /** Constructor.
   *  \param tid The 'typeid' operator.
   *  \param o The left parenthesis of the type name or expression.
   *  \param e The expression or type name for which to get the type identifier.
   *  \param c The right parenthesis of the type name or expression. */
  CT_TypeidExpr (CTree *tid, CTree *o, CTree *e, CTree *c) {
    AddSon (sons[0], tid); AddSon (sons[1], o); 
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
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 4, old_son, new_son);
  }
  /** Get the typeid argument, i.e. the expression or type name for
   *  which to get the type identifier. */
  CTree *Arg () const { return sons[2]; }
};

/** \class CT_SizeofExpr CTree.h Puma/CTree.h
 *  Tree node representing a sizeof expression.
 *  Example: \code sizeof(int*) \endcode */
class CT_SizeofExpr : public CT_Expression {
  CTree *sons[5]; // key, open, type, close, expr

public:
  /** Constructor.
   *  \param k The 'sizeof' keyword.
   *  \param o Left parenthesis around the type name.
   *  \param t The type from which to get the size.
   *  \param c Right parenthesis around the type name. */
  CT_SizeofExpr (CTree *k, CTree *o, CTree *t, CTree *c) {
    AddSon (sons[0], k); AddSon (sons[1], o); AddSon (sons[2], t); 
    AddSon (sons[3], c); AddSon (sons[4], 0);
  }
  /** Constructor.
   *  \param k The 'sizeof' keyword.
   *  \param e The expression from which to get the size. */
  CT_SizeofExpr (CTree *k, CTree *e) {
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
  /** Get the expression. */
  CTree *Expr () const { return sons[4]; }
  /** Get the type name. */
  CT_NamedType *TypeName () const { return (CT_NamedType*)sons[2]; }
};

/** \class CT_AlignofExpr CTree.h Puma/CTree.h
 *  Tree node representing an alignof expression.
 *  Example: \code __alignof(int) \endcode */
class CT_AlignofExpr : public CT_Expression {
  CTree *sons[5]; // key, open, type, close, expr

public:
  /** Constructor.
   *  \param k The 'alignof' keyword.
   *  \param o Left parenthesis around the type name.
   *  \param t The type from which to get the alignment.
   *  \param c Right parenthesis around the type name. */
  CT_AlignofExpr (CTree *k, CTree *o, CTree *t, CTree *c) {
    AddSon (sons[0], k); AddSon (sons[1], o); AddSon (sons[2], t); 
    AddSon (sons[3], c); AddSon (sons[4], 0);
  }
  /** Constructor.
   *  \param k The 'alignof' keyword.
   *  \param e The expression from which to get the alignment. */
  CT_AlignofExpr (CTree *k, CTree *e) {
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
  /** Get the expression. */
  CTree *Expr () const { return sons[4]; }
  /** Get the type name. */
  CT_NamedType *TypeName () const { return (CT_NamedType*)sons[2]; }
};

/** \class CT_TypeTraitExpr CTree.h Puma/CTree.h
 *  Tree node representing an type trait expression.
 *  Example: \code __is_enum(E) \endcode */
class CT_TypeTraitExpr : public CT_Expression {
  CTree *sons[6]; // key, open, type, comma, type, close

public:
  /** Constructor.
   *  \param k The type trait keyword.
   *  \param o Left parenthesis around the type name.
   *  \param t The type from which to get the trait.
   *  \param c Right parenthesis around the type name. */
  CT_TypeTraitExpr (CTree *k, CTree *o, CTree *t, CTree *c) {
    AddSon (sons[0], k); AddSon (sons[1], o); AddSon (sons[2], t);
    AddSon (sons[3], 0); AddSon (sons[4], 0); AddSon (sons[5], c);
  }
  /** Constructor.
   *  \param k The type trait keyword.
   *  \param o Left parenthesis around the type name.
   *  \param t1 The first type from which to get the trait.
   *  \param cc The comma between the types.
   *  \param t2 The second type from which to get the trait.
   *  \param c Right parenthesis around the type name. */
  CT_TypeTraitExpr (CTree *k, CTree *o, CTree *t1, CTree *cc, CTree *t2, CTree *c) {
    AddSon (sons[0], k); AddSon (sons[1], o); AddSon (sons[2], t1);
    AddSon (sons[3], cc); AddSon (sons[4], t2); AddSon (sons[5], c);
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 6); }
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
  /** Get the type trait operator. */
  int Operator () const { return sons[0]->token ()->type (); }
  /** Get the first type. */
  CT_NamedType *FirstType () const { return (CT_NamedType*)sons[2]; }
  /** Get the second type. */
  CT_NamedType *SecondType () const { return (CT_NamedType*)sons[4]; }
};

/** \class CT_OffsetofExpr CTree.h Puma/CTree.h
 *  Tree node representing an offsetof expression.
 *  Example: \code offsetof(Circle,radius) \endcode */
class CT_OffsetofExpr : public CT_Expression {
  CTree *sons[6]; // key, open, type, comma, member, close

public:
  /** Constructor.
   *  \param k The 'offsetof' keyword.
   *  \param o Left parenthesis around the parameters.
   *  \param t The type containing the member.
   *  \param co The comma between type and member.
   *  \param m The member for which to get the offset.
   *  \param c Right parenthesis around the parameters. */
  CT_OffsetofExpr (CTree *k, CTree *o, CTree *t, CTree *co, CTree *m, CTree *c) {
    AddSon (sons[0], k); AddSon (sons[1], o); AddSon (sons[2], t); 
    AddSon (sons[3], co); AddSon (sons[4], m); AddSon (sons[5], c);
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
  /** Get the typename. */
  CTree *TypeName () const { return sons[2]; }
  /** Get the member designator. */
  CT_DesignatorSeq *MemberDesignator () const { return (CT_DesignatorSeq*)sons[4]; }
};

/** \class CT_IndexDesignator CTree.h Puma/CTree.h
 *  Tree node representing an index designator.
 *  Example: \code [1] \endcode */
class CT_IndexDesignator : public CT_Expression {
  CTree *sons[3]; // open, index, close

public:
  /** Constructor.
   *  \param o Left bracket of the index designator.
   *  \param i The index expression.
   *  \param c Right bracket of the index designator. */
  CT_IndexDesignator (CTree *o, CTree *i, CTree *c) {
    AddSon (sons[0], o); AddSon (sons[1], i); AddSon (sons[2], c);
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
};

/** \class CT_MembDesignator CTree.h Puma/CTree.h
 *  Tree node representing a member designator.
 *  Example: \code .a \endcode */
class CT_MembDesignator : public CT_Expression {
  CTree *sons[2]; // dot, member

public:
  /** Constructor.
   *  \param d The dot before the member name.
   *  \param m The member name. */
  CT_MembDesignator (CTree *d, CTree *m) { AddSon (sons[0], d); AddSon (sons[1], m); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 2; }
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
};

/** \class CT_DesignatorSeq CTree.h Puma/CTree.h
 *  Tree node representing a designator sequence.
 *  Example: \code .a.b.c \endcode */
class CT_DesignatorSeq : public CT_List, public CSemValue {
public:
  /** Constructor.
   *  \param size Initial number of designators. */
  CT_DesignatorSeq (int size = 1) : CT_List (size, 2) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }

  /** Get the type of the entity to initialize. */
  CTypeInfo *Type () const { return type; }
  /** Get the value of the entity to initialize. */
  CExprValue *Value () const { return value; }
  /** Get the semantic value object. */
  CSemValue *SemValue () const { return (CSemValue*)this; }
};

/*****************************************************************************/
/*                                                                           */
/*                         Declaration specifiers                            */
/*                                                                           */
/*****************************************************************************/

/** \class CT_DeclSpec CTree.h Puma/CTree.h
 *  Base class for all tree nodes representing declaration specifiers. */
class CT_DeclSpec : public CTree {
protected:
  /** Constructor. */
  CT_DeclSpec () {}
};

/** \class CT_PrimDeclSpec CTree.h Puma/CTree.h
 *  Tree node representing a primitive declaration specifier. */
class CT_PrimDeclSpec : public CT_DeclSpec {
public:
  /** Declaration specifier types. */
  enum Type { 
    PDS_FRIEND,    /** friend */
    PDS_TYPEDEF,   /** typedef */
    PDS_AUTO,      /** auto */
    PDS_REGISTER,  /** register */
    PDS_STATIC,    /** static */
    PDS_EXTERN,    /** extern */
    PDS_MUTABLE,   /** mutable */
    PDS_INLINE,    /** inline */
    PDS_VIRTUAL,   /** virtual */
    PDS_EXPLICIT,  /** explicit */
    PDS_CONST,     /** const */
    PDS_VOLATILE,  /** volatile */
    PDS_RESTRICT,  /** restrict */
    PDS_CHAR,      /** char */
    PDS_WCHAR_T,   /** wchar_t */
    PDS_BOOL,      /** bool */
    PDS_C_BOOL,    /** _Bool */
    PDS_SHORT,     /** short */
    PDS_INT,       /** int */
    PDS_LONG,      /** long */
    PDS_SIGNED,    /** signed */
    PDS_UNSIGNED,  /** unsigned */
    PDS_FLOAT,     /** float */
    PDS_DOUBLE,    /** double */
    PDS_VOID,      /** void */
    // GNU C specific type specifier
    PDS_INT128,    /** __int128 */
    // GNU C++ specific storage specifier
    PDS_THREAD,    /** __thread */
    // AspectC++ specific type specifier
    PDS_UNKNOWN_T, /** unknown_t */
    // Win specific declaration specifiers
    PDS_CDECL,     /** __cdecl */
    PDS_STDCALL,   /** __stdcall */
    PDS_FASTCALL,  /** __fastcall */
    PDS_INT64,     /** __int64 */
    PDS_UNKNOWN,   /** Unknown declaration specifier. */
    PDS_NUM        /** Number of declaration specifier types. */
  };

private:
  Type _type;
  CTree *_token; // has to be a CT_Token

  void determine_type ();

public:
  /** Constructor.
   *  \param t The token containing the declaration specifier. */
  CT_PrimDeclSpec (CT_Token *t) { AddSon (_token, (CTree*)t); determine_type (); }
  /** Constructor.
   *  \param t The declaration specifier type. */
  CT_PrimDeclSpec (Type t) : _token (0) { _type = t; }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return _token ? 1 : 0; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const 
   { return (n == 0) ? _token : (CTree*)0; }
  /** Get the textual representation of the declaration specifier.
   *  \return The string representation or " ". */
  const char *SpecText () const 
   { return _token ? _token->token ()->text () : " "; }
  /** Get the declaration specifier type. */
  Type SpecType () const { return _type; }
  /** Number of declaration specifier types. */
  static const int NumTypes = PDS_NUM;
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (_token, (CTree*)old_son, (CTree*)new_son);
    determine_type ();
  }
};

/** \class CT_NamedType CTree.h Puma/CTree.h
 *  Tree node representing a named type.
 *  Example: \code (int*)a \endcode where int* is a 
 *  type with a generated name. */
class CT_NamedType : public CT_DeclSpec, public CSemObject {
  CTree *sons[2]; // declspecs, declarator

public:
  /** Constructor.
   *  \param dss The declaration specifier sequence of the type.
   *  \param d The type declarator. */
  CT_NamedType (CTree *dss, CTree *d) { AddSon (sons[0], dss); AddSon (sons[1], d); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 2); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 2, n); }
  /** Get the declarator. */
  CTree *Declarator () const { return sons[1]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 2, old_son, new_son);
  }
  /** Get the semantic information about the created temporary object. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
};
      
/** \class CT_ClassSpec CTree.h Puma/CTree.h
 *  Tree node representing a class specifier.
 *  Example: \code class X \endcode */
class CT_ClassSpec : public CT_DeclSpec, public CSemObject {
  CTree *sons[2]; // key, name
  
public:
  /** Constructor.
   *  \param k The 'class' or 'struct' keyword.
   *  \param n The class name. */
  CT_ClassSpec (CTree *k, CTree *n) { AddSon (sons[0], k); AddSon (sons[1], n); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 2; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 2, n); } 
  /** Get the class name. */
  CT_SimpleName *Name () const { return (CT_SimpleName*)sons[1]; }
  /** Get the semantic information about the class. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 2, old_son, new_son);
  }
};

/** \class CT_UnionSpec CTree.h Puma/CTree.h
 *  Tree node representing a union specifier.
 *  Example: \code union X \endcode */
class CT_UnionSpec : public CT_ClassSpec {
public:
  /** Constructor.
   *  \param k The 'union' keyword.
   *  \param n The name of the union. */
  CT_UnionSpec (CTree *k, CTree *n) : CT_ClassSpec (k, n) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_EnumSpec CTree.h Puma/CTree.h
 *  Tree node representing an enumeration specifier.
 *  Example: \code enum X \endcode */
class CT_EnumSpec : public CT_ClassSpec {
public:
  /** Constructor.
   *  \param k The 'enum' keyword. 
   *  \param n The name of the enumeration. */
  CT_EnumSpec (CTree *k, CTree *n) : CT_ClassSpec (k, n) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_ExceptionSpec CTree.h Puma/CTree.h
 *  Tree node representing an exception specifier.
 *  Example: \code throw(std::exception) \endcode */
class CT_ExceptionSpec : public CT_DeclSpec {
  CTree *sons[2]; // throw, type_id_list
  
public:
  /** Constructor.
   *  \param k The 'throw' keyword.
   *  \param l The type list for the exception type to throw. */
  CT_ExceptionSpec (CTree *k, CTree *l) { AddSon (sons[0], k); AddSon (sons[1], l); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 2; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 2, n); }
  /** Get the exception type list. */
  CT_ArgDeclList *Arguments () const { return (CT_ArgDeclList*)sons[1]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 2, old_son, new_son);
  }
};

/*****************************************************************************/
/*                                                                           */
/*                              Declarations                                 */
/*                                                                           */
/*****************************************************************************/

/** \class CT_Decl CTree.h Puma/CTree.h
 *  Base class for all tree nodes representing declarations. */
class CT_Decl : public CTree { 
  CT_LinkageSpec *_linkage;
  
protected:
  /** Constructor. */
  CT_Decl () : _linkage (0) {}
  
public:
  /** Set the linkage of the declared entity.
   *  \param l The linkage specifiers. */
  void Linkage (CT_LinkageSpec *l) { _linkage = l; }
  /** Get the linkage specifiers. */
  CT_LinkageSpec *Linkage () const { return _linkage; }
  /** Get this. */
  virtual CT_Decl *IsDeclaration () { return this; }
};

/** \class CT_Program CTree.h Puma/CTree.h
 *  Root node of C/C++ syntax trees. */
class CT_Program : public CT_DeclList, public CSemScope {
public:
  /** Constructor.
   *  \param size The initial number of declarations in the program.
   *  \param incr The initial increment count. */
  CT_Program (int size = 20, int incr = 20) : CT_DeclList (size, incr) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the top scope. */
  CSemScope *SemScope () const { return (CSemScope*)this; }
};
   
/** \class CT_ObjDecl CTree.h Puma/CTree.h
 *  Tree node representing an object declaration.
 *  Example: \code int *i \endcode */
class CT_ObjDecl : public CT_Decl {
  CTree *sons[3]; // declspecs, declarators, colon

public:
  /** Constructor.
   *  \param dsl The declaration specifier sequence.
   *  \param dl The declarator list.
   *  \param c Optional colon. */
  CT_ObjDecl (CTree *dsl, CTree *dl, CTree *c) {
    AddSon (sons[0], dsl); AddSon (sons[1], dl); AddSon (sons[2], c);
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
  /** Get the declaration specifier sequence. */
  CT_DeclSpecSeq *DeclSpecs () const { return (CT_DeclSpecSeq*)sons[0]; }
  /** Get the declarator list. */
  CT_DeclaratorList *Declarators () const { return (CT_DeclaratorList*)sons[1]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 3, old_son, new_son);
  }
};

/** \class CT_TemplateDecl CTree.h Puma/CTree.h
 *  Tree node representing a template declaration. */
class CT_TemplateDecl : public CT_Decl, public CSemScope {
  CTree *sons[3]; // export, param_list, decl

public:
  /** Constructor.
   *  \param e Optional 'export' keyword. 
   *  \param p The template parameter list.
   *  \param d The class or function declaration. */
  CT_TemplateDecl (CTree *e, CTree *p, CTree *d) {
    AddSon (sons[0], e); AddSon (sons[1], p); AddSon (sons[2], d);
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 3); }
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
  /** Get the 'export' keyword. */
  CTree *Export () const { return sons[0]; }
  /** Get the template parameter list. */
  CT_TemplateParamList *Parameters () const { 
    return (CT_TemplateParamList*)sons[1]; 
  }
  /** Get the class or function declaration. */
  CTree *Declaration () const { return sons[2]; }
  /** Get the scope opened by the template declaration. */
  CSemScope *SemScope () const { return (CSemScope*)this; }
};

/** \class CT_TemplateParamDecl CTree.h Puma/CTree.h
 *  Base class for all tree nodesrepresenting a template parameter declaration. */
class CT_TemplateParamDecl : public CT_Decl, public CSemObject {
protected:
  /** Constructor. */
  CT_TemplateParamDecl () {}
  
public:
  /** Get the template default argument. */
  virtual CT_ExprList *DefaultArgument () const = 0;
  /** Get the semantic information about the template parameter. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
};

/** \class CT_NonTypeParamDecl CTree.h Puma/CTree.h
 *  Tree node representing a template non-type parameter declaration. */
class CT_NonTypeParamDecl : public CT_TemplateParamDecl {
  CTree *sons[3]; // declspecs, declarator, init

public:
  /** Constructor.
   *  \param dsl The declaration specifier sequence.
   *  \param d The parameter declarator.
   *  \param i The default template argument. */
  CT_NonTypeParamDecl (CTree *dsl, CTree *d, CTree *i = (CTree*)0) {
    AddSon (sons[0], dsl); AddSon (sons[1], d); AddSon (sons[2], i);
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 3); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 3, n); }
  /** Get the declaration specifier sequence. */
  CT_DeclSpecSeq *DeclSpecs () const { return (CT_DeclSpecSeq*)sons[0]; }
  /** Get the parameter declarator. */
  CTree *Declarator () const { return sons[1]; }
  /** Get the default template argument. */
  CT_ExprList *DefaultArgument () const { return (CT_ExprList*)sons[2]; }
  /** Get the semantic information about the template parameter. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
  /** Set the default template argument. 
   *  \param i The default argument. */
  void Initializer (CTree *i) { AddSon (sons[2], i); }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 3, old_son, new_son);
  }
};

/** \class CT_TypeParamDecl CTree.h Puma/CTree.h
 *  Tree node representing a template type parameter declaration. */
class CT_TypeParamDecl : public CT_TemplateParamDecl {
  CTree *sons[4]; // params, key, id, init

public:
  /** Constructor.
   *  \param pl The template parameter list of an template template parameter.
   *  \param k The type keyword, i.e. 'class' or 'typename'.
   *  \param id The parameter identifier.
   *  \param i The default template argument. */
  CT_TypeParamDecl (CTree *pl, CTree *k, CTree *id, CTree *i = (CTree*)0) { 
    AddSon (sons[0], pl); AddSon (sons[1], k); 
    AddSon (sons[2], id); AddSon (sons[3], i);
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 4); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 4, n); }
  /** Get the template parameter list of a template template parameter. */
  CT_TemplateParamList *Parameters () const { 
    return (CT_TemplateParamList*)sons[0]; 
  }
  /** Get the templare parameter name. */
  CT_SimpleName *Name () const { return (CT_SimpleName*)sons[2]; }
  /** Get the template default argument. */
  CT_ExprList *DefaultArgument () const { return (CT_ExprList*)sons[3]; }
  /** Set the template default argument.
   *  \param i The default argument. */
  void Initializer (CTree *i) { AddSon (sons[3], i); }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 4, old_son, new_son);
  }
};

/** \class CT_EnumDef CTree.h Puma/CTree.h
 *  Tree node representing the definition of an enumeration. 
 *  Example: \code enum E { A, B, C } \endcode */
class CT_EnumDef : public CT_Decl, public CSemObject {
  CTree *sons[3]; // key, name, enumerators

public:
  /** Constructor.
   *  \param k The keyword 'enum'.
   *  \param n The name of the enumeration. */
  CT_EnumDef (CTree *k, CTree *n) {
    AddSon (sons[0], k); AddSon (sons[1], n); AddSon (sons[2], 0); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 3); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 3, n); }
  /** Get the name of the enumeration. */
  CT_SimpleName *Name () const { return (CT_SimpleName*)sons[1]; }
  /** Set the list of enumeration constants.
   *  \param el The enumerator list. */
  void Enumerators (CTree *el) { AddSon (sons[2], el); }
  /** Get the list of enumeration constants. */
  CT_EnumeratorList *Enumerators () const { return (CT_EnumeratorList*)sons[2]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 3, old_son, new_son);
  }
  /** Get the semantic information about the enumeration. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
};

/** \class CT_Enumerator CTree.h Puma/CTree.h
 *  Tree node representing a single enumeration constant. */
class CT_Enumerator : public CT_Decl, public CSemObject {
  CTree *sons[2]; // name, init

public:
  /** Constructor.
   *  \param n The name of the enumerator. */
  CT_Enumerator (CTree *n) { AddSon (sons[0], n); AddSon (sons[1], 0); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 2); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 2, n); }
  /** Get the name of the enumerator. */
  CT_SimpleName *Name () const { return (CT_SimpleName*)sons[0]; }
  /** Set the initializer expression of the enumerator. */
  void Initializer (CTree *i) { AddSon (sons[1], i); }
  /** Get the initializer expression of the enumerator. */
  CT_ExprList *Initializer () const { return (CT_ExprList*)sons[1]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 2, old_son, new_son); 
  }
  /** Get the semantic information about the enumerator. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
};

/** \class CT_FctDef CTree.h Puma/CTree.h
 *  Tree node representing a function definition. 
 *  Example:
 *  \code
 * int mul(int x, int y) {
 *   return x*y;
 * }
 *  \endcode */
class CT_FctDef : public CT_Decl, public CSemObject {
  CTree *sons[7]; // declspecs, declarator, try, ctor_init, args, body, handlers

public:
  /** Constructor.
   *  \param dss The declaration specifier sequence. 
   *  \param d The function declarator.
   *  \param t Optional keyword 'try' for a function try-block.
   *  \param ci Optional constructor initializer list.
   *  \param as Optional K&R argument declaration list.
   *  \param b The function body.
   *  \param hs Exception handler sequence for a function try-block. */
  CT_FctDef (CTree *dss, CTree *d, CTree *t, CTree *ci, CTree *as, 
             CTree *b, CTree *hs) {
    AddSon (sons[0], dss); AddSon (sons[1], d); AddSon (sons[2], t); 
    AddSon (sons[3], ci); AddSon (sons[4], as); AddSon (sons[5], b); 
    AddSon (sons[6], hs); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 7); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 7, n); }
  /** Get the declaration specifier sequence. */
  CT_DeclSpecSeq *DeclSpecs () const { return (CT_DeclSpecSeq*)sons[0]; }
  /** Get the function declarator. */
  CTree *Declarator () const { return sons[1]; }
  /** Get the 'try' keyword of the function try-block. */
  CT_Token *TryKey () const { return (CT_Token*)sons[2]; }
  /** Get the constructor initializer list. */
  CTree *CtorInit () const { return sons[3]; }
  /** Get the K&R argument declaration sequence. */
  CT_ArgDeclSeq *ArgDeclSeq () const { return (CT_ArgDeclSeq*)sons[4]; }
  /** Get the function body. */
  CT_CmpdStmt *Body () const { return (CT_CmpdStmt*)sons[5]; }
  /** Get the exception handler sequence of a function try-block. */
  CT_HandlerSeq *Handlers () const { return (CT_HandlerSeq*)sons[6]; }
  /** Get the semantic information about the function. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
  /** Set the constructor initializer list. 
   *  \param i The initializer list. */
  void CtorInit (CTree *i) { AddSon (sons[3], i); }
  /** Set the function body.
   *  \param b The function body. */
  void Body (CTree *b) { AddSon (sons[5], b); }
  /** Set the exception handler sequence of a function try-block.
   *  \param h The handlers. */
  void Handlers (CTree *h) { AddSon (sons[6], h); }
  /** Set the function try-block.
   *  \param t The keyword 'try'.
   *  \param c Optional constructor initializer list.
   *  \param b The function body.
   *  \param h The exception handler sequence. */
  void FctTryBlock (CTree *t, CTree *c, CTree *b, CTree *h) { 
    AddSon (sons[2], t); AddSon (sons[3], c); 
    AddSon (sons[5], b); AddSon (sons[6], h);
  }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 7, old_son, new_son);
  }
};

/** \class CT_AsmDef CTree.h Puma/CTree.h
 *  Tree node representing an inline assembly definition. 
 *  Example: \code asm("movl %ecx %eax"); \endcode */
class CT_AsmDef : public CT_Decl {
  CTree *sons[5]; // asm, open, str, close, semi_colon

public:
  /** Constructor.
   *  \param a The keyword 'asm'.
   *  \param o Left parenthesis around the assembler code string. 
   *  \param s The assembler code.
   *  \param c Right parenthesis around the assembler code string.
   *  \param sc Trailing semi-colon. */
  CT_AsmDef (CTree *a, CTree *o, CTree *s, CTree *c, CTree *sc) {
    AddSon (sons[0], a); AddSon (sons[1], o); AddSon (sons[2], s); 
    AddSon (sons[3], c); AddSon (sons[4], sc); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 5; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 5, n); }
  /** Get the assembler code. */
  CT_String *Instructions () const { return (CT_String*)sons[2]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 5, old_son, new_son);
  }
};

/** \class CT_Handler CTree.h Puma/CTree.h
 *  Tree node representing an exception handler. */
class CT_Handler : public CT_Decl, public CSemScope {
  CTree *sons[3]; // catch, exception_decl, stmt

public:
  /** Constructor.
   *  \param c The keyword 'catch'.
   *  \param e The exception object declaration.
   *  \param s The exception handling statement. */
  CT_Handler (CTree *c, CTree *e, CTree *s) {
    AddSon (sons[0], c); AddSon (sons[1], e); AddSon (sons[2], s);
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
  /** Get the exception object declaration. */
  CT_ArgDeclList *Arguments () const { return (CT_ArgDeclList*)sons[1]; }
  /** Get the exception handling statement. */
  CT_Statement *Statement () const { return (CT_Statement*)sons[2]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 3, old_son, new_son);
  }
  /** Get the scope opened by the handler. */
  CSemScope *SemScope () const { return (CSemScope*)this; }
};

/** \class CT_LinkageSpec CTree.h Puma/CTree.h
 *  Tree node representing a list of declaration with a specific linkage. */
class CT_LinkageSpec : public CT_Decl {
  CTree *sons[4]; // linkage specifiers, open, decls, close

public:
  /** Constructor.
   *  \param dss The linkage specifiers, e.g. extern and "C".
   *  \param o Left parenthesis around the declaration list.
   *  \param d The list of declarations.
   *  \param c Right parenthesis around the declaration list. */
  CT_LinkageSpec (CTree *dss, CTree *o, CTree *d, CTree *c) {
    AddSon (sons[0], dss); AddSon (sons[1], o);
    AddSon (sons[2], d); AddSon (sons[3], c);
    if (isList ())
      ((CT_DeclList*)Decls ())->Linkage (this);
    else
      ((CT_Decl*)Decls ())->Linkage (this);
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 4); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 4, n); }
  /** Get the linkage specifier list. */
  CT_DeclSpecSeq *LinkageSpecifiers () const { return (CT_DeclSpecSeq*)sons[0]; }
  /** Get the list declarations. */
  CTree *Decls () const { return sons[2]; }
  /** Check if there is more than one enclosed declaration. 
   *  In this case the node returned by Decls() is a CT_DeclList
   *  node. */
  bool isList () const {
    return Decls ()->NodeName () == CT_DeclList::NodeId ();
  }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 4, old_son, new_son);
  }
};

/** \class CT_ArgDecl CTree.h Puma/CTree.h
 *  Tree node representing the declaration of a function parameter. */
class CT_ArgDecl : public CT_Decl, public CSemObject, public CSemValue {
  CTree *sons[4]; // declspecs, declarator, init, ellipsis

public:
  /** Constructor.
   *  \param dsl The declaration specifier sequence.
   *  \param d The parameter declarator. */
  CT_ArgDecl (CTree *dsl, CTree *d) {
    AddSon (sons[0], dsl); AddSon (sons[1], d); 
    AddSon (sons[2], 0); AddSon (sons[3], 0); 
  }
  /** Constructor.
   *  \param ellipsis The variable argument list operator "...". */
  CT_ArgDecl (CTree *ellipsis) {
    AddSon (sons[0], 0); AddSon (sons[1], 0); 
    AddSon (sons[2], 0); AddSon (sons[3], ellipsis); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 4); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 4, n); }
  /** Get the declaration specifier sequence. */
  CT_DeclSpecSeq *DeclSpecs () const { return (CT_DeclSpecSeq*)sons[0]; }
  /** Get the function parameter declarator. */
  CTree *Declarator () const { return sons[1]; }
  /** Get the default argument. */
  CT_ExprList *Initializer () const {
    return (sons[2] && sons[2]->IsDelayedParse ()) ? 0 : (CT_ExprList*)sons[2];
  }
  /** Get the variable argument list operator. */
  CT_Token *Ellipsis () const { return (CT_Token*)sons[3]; }
  /** Get the semantic information about the function parameter. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
  /** Get the type of the function parameter.
   *  \return The type information object or NULL. */
  CTypeInfo *Type () const { return type; }
  /** Get the value of the function parameter.
   *  \return The value object or NULL. */
  CExprValue *Value () const { return value; }
  /** Get the semantic value information of the function parameter.
   *  \return The value object or NULL. */
  CSemValue *SemValue () const { return (CSemValue*)this; }
  /** Set the default argument. */
  void Initializer (CTree *i) { AddSon (sons[2], i); }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 4, old_son, new_son);
  }
};

/** \class CT_ArgDeclList CTree.h Puma/CTree.h
 *  Tree node representing a function parameter list. */
class CT_ArgDeclList : public CT_DeclList, public CSemScope {
public:
  /** Constructor.
   *  \param size The initial size of the list.
   *  \param props The list properties. */
  CT_ArgDeclList (int size = 2, int props = SEPARATORS | OPEN_CLOSE) : 
   CT_DeclList (size, 2) { AddProperties (props); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the scope opened by the parameter list. */
  CSemScope *SemScope () const { return (CSemScope*)this; }
};

/** \class CT_ArgDeclSeq CTree.h Puma/CTree.h
 *  Tree node representing a K&R function parameter declarations list. */
class CT_ArgDeclSeq : public CT_DeclList, public CSemScope {
public:
  /** Constructor.
   *  \param size The initial size of the list. */
  CT_ArgDeclSeq (int size = 2) : CT_DeclList (size, 2) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the scope opened by the parameter declarations list. */
  CSemScope *SemScope () const { return (CSemScope*)this; }
};

/** \class CT_ArgNameList CTree.h Puma/CTree.h
 *  Tree node representing a K&R function parameter name list. */
class CT_ArgNameList : public CT_ArgDeclList {
public:
  /** Constructor. */
  CT_ArgNameList () : CT_ArgDeclList () {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_NamespaceDef CTree.h Puma/CTree.h
 *  Tree node representing a namespace definition.
 *  Example: \code namespace a {} \endcode */
class CT_NamespaceDef : public CT_Decl, public CSemObject {
  CTree *sons[4]; // inline, ns, name, members

public:
  /** Constructor.
   *  \param i The keyword 'inline'.
   *  \param n The keyword 'namespace'.
   *  \param nm The name of the namespace. */
  CT_NamespaceDef (CTree *i, CTree *n, CTree *nm) {
    AddSon (sons[0], i); AddSon (sons[1], n); AddSon (sons[2], nm); AddSon (sons[3], 0); 
  }
  /** Constructor.
   *  \param i The keyword 'inline'.
   *  \param n The keyword 'namespace'.
   *  \param nm The name of the namespace. 
   *  \param m The namespace member declarations list. */
  CT_NamespaceDef (CTree *i, CTree *n, CTree *nm, CTree *m) {
    AddSon (sons[0], i); AddSon (sons[1], n); AddSon (sons[2], nm); AddSon (sons[3], m); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 4); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 4, n); }
  /** Set the namespace member declarations list. */
  void Members (CTree *m) { AddSon (sons[3], m); }
  /** Get the namespace member declarations list. */
  CT_MembList *Members () const { return (CT_MembList*)sons[3]; }
  /** Get the name of the namespace. */
  CT_SimpleName *Name () const { return (CT_SimpleName*)sons[2]; }
  /** Get the semantic information about the namespace. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 4, old_son, new_son);
  }
  /** Return true if this is an inline namespace. */
  bool isInline () const { return sons[0] != (CTree*)0; }
};

/** \class CT_NamespaceAliasDef CTree.h Puma/CTree.h
 *  Tree node representing a namespace alias definition.
 *  Example: \code namespace b = a; \endcode */
class CT_NamespaceAliasDef : public CT_Decl, public CSemObject {
  CTree *sons[5]; // ns, alias, assign, name, semi_colon

public:
  /** Constructor.
   *  \param n The keyword 'namespace'.
   *  \param a The name of the namespace alias.
   *  \param as The assignment operator '='.
   *  \param nm The name of the original namespace.
   *  \param s The trailing semi-colon. */
  CT_NamespaceAliasDef (CTree *n, CTree *a, CTree *as, CTree *nm, CTree *s) {
    AddSon (sons[0], n); AddSon (sons[1], a); AddSon (sons[2], as); 
    AddSon (sons[3], nm); AddSon (sons[4], s); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 5; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 5, n); }
  /** Get the name of the original namespace. */
  CT_SimpleName *Name () const { return (CT_SimpleName*)sons[3]; }
  /** Get the name of the namespace alias. */
  CT_SimpleName *Alias () const { return (CT_SimpleName*)sons[1]; }
  /** Get the semantic information about the namespace alias. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 5, old_son, new_son);
  }
};

/** \class CT_UsingDirective CTree.h Puma/CTree.h
 *  Tree node representing a namespace using directive.
 *  Example: \code using namespace std; \endcode */
class CT_UsingDirective : public CT_Decl {
  CTree *sons[4]; // using, ns, name, semi_colon

public:
  /** Constructor. 
   *  \param u The keyword 'using'.
   *  \param ns The keyword 'namespace'. 
   *  \param n The name of the namespace.
   *  \param s The trailing semi-colon. */
  CT_UsingDirective (CTree *u, CTree *ns, CTree *n, CTree *s) {
    AddSon (sons[0], u); AddSon (sons[1], ns); AddSon (sons[2], n); 
    AddSon (sons[3], s); 
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
  /** Get the name of the namespace. */
  CT_SimpleName *Name () const { return (CT_SimpleName*)sons[2]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 4, old_son, new_son);
  }
};

/*****************************************************************************/
/*                                                                           */
/*                              Declarators                                  */
/*                                                                           */
/*****************************************************************************/

/** \class CT_Declarator CTree.h Puma/CTree.h
 *  Base class for all tree nodes representing declarators. */
class CT_Declarator : public CTree {
protected:
  /** Constructor. */
  CT_Declarator () {}

public:
  /** Get the declarator. */
  virtual CTree *Declarator () const = 0;
  /** Get this. */
  virtual CT_Declarator *IsDeclarator () { return this; }
  /** Get the declared name. */
  CT_SimpleName *Name ();
  /** Get the declared name and set last_declarator to 
   *  the declarator containing the name. 
   *  \param last_declarator To be set to the declarator containing the name. */
  CT_SimpleName *Name (CT_Declarator *&last_declarator);
};

/** \class CT_InitDeclarator CTree.h Puma/CTree.h
 *  Tree node representing a declarator with initializer.
 *  Example: \code int *i = 0; \endcode */
class CT_InitDeclarator : public CT_Declarator, public CSemObject {
  CTree *sons[2]; // declarator, init
  CTree *obj_decl;

public:
  /** Constructor.
   *  \param d The declarator.
   *  \param i The initializer. */
  CT_InitDeclarator (CTree *d, CTree *i = 0) {
    AddSon (sons[0], d); AddSon (sons[1], i);
    AddSon (obj_decl, 0); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 2); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 2, n); }
  /** Get the declarator. */
  CTree *Declarator () const { return sons[0]; }
  /** Get the initializer. */
  CT_ExprList *Initializer () const { return (CT_ExprList*)sons[1]; }
  /** Get the semantic information about the declared object. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
  /** Get the object declaration node containing the declarator. */
  CT_ObjDecl *ObjDecl () const { return (CT_ObjDecl*)obj_decl; }
  /** Set the initializer. */
  void Initializer (CTree* i) { AddSon (sons[1], i); }
  /** Set the object declaration node containing the declarator. 
   *  \param od The object declaration node. */
  void ObjDecl (CTree *od) { AddSon (obj_decl, od); }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 2, old_son, new_son);
  }
};

/** \class CT_BracedDeclarator CTree.h Puma/CTree.h
 *  Tree node representing a braced declarator.
 *  Example: \code int (i); \endcode */
class CT_BracedDeclarator : public CT_Declarator {
  CTree *sons[4]; // open, win_specs, declarator, close

public:
  /** Constructor.
   *  \param o Left parenthesis around the declarator.
   *  \param d The declarator.
   *  \param c Right parenthesis around the declarator. */
  CT_BracedDeclarator (CTree *o, CTree *d, CTree *c) {
    AddSon (sons[0], o); AddSon (sons[1], 0); 
    AddSon (sons[2], d); AddSon (sons[3], c); 
  }
  /** Constructor.
   *  \param o Left parenthesis around the declarator.
   *  \param ws Declaration specifiers.
   *  \param d The declarator.
   *  \param c Right parenthesis around the declarator. */
  CT_BracedDeclarator (CTree *o, CTree *ws, CTree *d, CTree *c) {
    AddSon (sons[0], o); AddSon (sons[1], ws); 
    AddSon (sons[2], d); AddSon (sons[3], c); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 4); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 4, n); }
  /** Get the declarator. */
  CTree *Declarator () const { return sons[2]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 4, old_son, new_son);
  }
};

/** \class CT_ArrayDelimiter CTree.h Puma/CTree.h
 *  Tree node representing an array delimiter.
 *  Example: \code [10] \endcode or \code [*] \endcode */
class CT_ArrayDelimiter : public CTree {
  CTree *sons[4]; // star, static, quals, expr
  bool pos0;

public:
  /** Constructor.
   *  \param m The operator '*'.
   *  \param s The keyword 'static'.
   *  \param q The const/volatile qualifier sequence. 
   *  \param e The array size expression. 
   *  \param p Position of keyword 'static', true means before the
   *           qualifier sequence and false means behind it. */
  CT_ArrayDelimiter (CTree *m, CTree *s, CTree *q, CTree *e, bool p = false) {
    AddSon (sons[0], m); AddSon (sons[1], s); 
    AddSon (sons[2], q); AddSon (sons[3], e); pos0 = p;
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 4); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 4, n); }
  /** Get the operator '*'. */
  CT_Token *Star () const { return (CT_Token*)sons[0]; }
  /** Get the keyword 'static'. */
  CT_Token *Static () const { return (CT_Token*)sons[pos0?2:1]; }
  /** Get the const/volatile qualifier sequence. */
  CT_DeclSpecSeq *Qualifier () const { return (CT_DeclSpecSeq*)sons[pos0?1:2]; }
  /** Get the array size expression. */
  CTree *Expr () const { return sons[3]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 4, old_son, new_son);
  }
};

/** \class CT_ArrayDeclarator CTree.h Puma/CTree.h
 *  Tree node representing an array declarator.
 *  Example: \code a[10] \endcode */
class CT_ArrayDeclarator : public CT_Declarator, public CSemValue {
  CTree *sons[4]; // declarator, open, delim, close

public:
  /** Constructor.
   *  \param d The array declarator.
   *  \param o Left bracket around the delimiter.
   *  \param ad The array delimiter.
   *  \param c Right bracket around the delimiter. */
  CT_ArrayDeclarator (CTree *d, CTree *o, CTree *ad, CTree *c) {
    AddSon (sons[0], d); AddSon (sons[1], o); 
    AddSon (sons[2], ad); AddSon (sons[3], c); 
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
  /** Get the array declarator. */
  CTree *Declarator () const { return sons[0]; }
  /** Get the array delimiter. */
  CT_ArrayDelimiter *Delimiter () const 
   { return (CT_ArrayDelimiter*)sons[2]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 4, old_son, new_son);
  }
  /** Get the semantic information for the type of the declared array. */
  CTypeInfo *Type () const { return type; }
  /** Get the semantic information for the value of the declared array. */
  CExprValue *Value () const { return value; }
  /** Get the semantic information object. */
  CSemValue *SemValue () const { return (CSemValue*)this; }
};

/** \class CT_FctDeclarator CTree.h Puma/CTree.h
 *  Tree node representing a function declarator.
 *  Example: \code f(int a) const \endcode */
class CT_FctDeclarator : public CT_Declarator {
  CTree *sons[4]; // declarator, args, cv_quals, exception_specs

public:
  /** Constructor.
   *  \param d The function declarator.
   *  \param args The function parameter list.
   *  \param cv The function qualifiers.
   *  \param es The exception specifier. */
  CT_FctDeclarator (CTree *d, CTree *args, CTree *cv, CTree *es) {
    AddSon (sons[0], d); AddSon (sons[1], args); 
    AddSon (sons[2], cv); AddSon (sons[3], es); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 4); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 4, n); }
  /** Get the function declarator. */
  CTree *Declarator () const { return sons[0]; }
  /** Get the function parameter list. */
  CT_ArgDeclList *Arguments () const { return (CT_ArgDeclList*)sons[1]; }
  /** Get the function qualifier list. */
  CT_DeclSpecSeq *Qualifier () const { return (CT_DeclSpecSeq*)sons[2]; }
  /** Get the exception specifier. */
  CT_ExceptionSpec *ExceptionSpecs () const { return (CT_ExceptionSpec*)sons[3]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 4, old_son, new_son);
  }
};

/** \class CT_RefDeclarator CTree.h Puma/CTree.h
 *  Tree node representing a reference declarator.
 *  Example: \code &a \endcode */
class CT_RefDeclarator : public CT_Declarator {
  CTree *sons[2]; // ref, declarator

public:
  /** Constructor.
   *  \param r The reference operator '&'.
   *  \param d The declarator. */
  CT_RefDeclarator (CTree *r, CTree *d) { AddSon (sons[0], r); AddSon (sons[1], d); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 2; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 2, n); }
  /** Get the declarator. */
  CTree *Declarator () const { return sons[1]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) {
    CTree::ReplaceSon (sons, 2, old_son, new_son);
  }
};

/** \class CT_PtrDeclarator CTree.h Puma/CTree.h
 *  Tree node representing a pointer declarator.
 *  Example: \code *a \endcode */
class CT_PtrDeclarator : public CT_Declarator {
  CTree *sons[3]; // ptr, cv_quals, declarator

public:
  /** Constructor.
   *  \param p The pointer operator '*'.
   *  \param c The const/volatile pointer qualifier sequence.
   *  \param d The declarator. */
  CT_PtrDeclarator (CTree *p, CTree *c, CTree *d) {
    AddSon (sons[0], p); AddSon (sons[1], c); AddSon (sons[2], d); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 3); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 3, n); }
  /** Get the declarator. */
  CTree *Declarator () const { return sons[2]; }
  /** Get the const/volatile qualifier sequence. */
  CT_DeclSpecSeq *Qualifier () const { return (CT_DeclSpecSeq*)sons[1]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 3, old_son, new_son);
  }
};

/** \class CT_MembPtrDeclarator CTree.h Puma/CTree.h
 *  Tree node representing a member pointer declarator.
 *  Example: \code *X::a \endcode */
class CT_MembPtrDeclarator : public CT_Declarator {
  CTree *sons[5]; // class, colon, ptr, cv_quals, declarator

public:
  /** Constructor.
   *  \param c The class name.
   *  \param cc The scope operator '::'.
   *  \param p The name of the pointer.
   *  \param q The const/volatile pointer qualifier sequence.
   *  \param d The declarator. */
  CT_MembPtrDeclarator (CTree *c, CTree *cc, CTree *p, CTree *q, CTree *d) {
    AddSon (sons[0], c); AddSon (sons[1], cc); AddSon (sons[2], p); 
    AddSon (sons[3], q); AddSon (sons[4], d); 
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
  /** Get the name of the declared pointer. */
  CT_SimpleName *Name () const { return (CT_SimpleName*)sons[0]; }
  /** Get the declarator. */
  CTree *Declarator () const { return sons[4]; }
  /** Get the const/volatile qualifier sequence. */
  CT_DeclSpecSeq *Qualifier () const { return (CT_DeclSpecSeq*)sons[3]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 5, old_son, new_son);
  }
};

/** \class CT_BitFieldDeclarator CTree.h Puma/CTree.h
 *  Tree node representing a bit-field declarator.
 *  Example: \code a : 2 \endcode */
class CT_BitFieldDeclarator : public CT_Declarator, public CSemObject {
  CTree *sons[3]; // declarator, colon, expr

public:
  /** Constructor.
   *  \param d The declarator.
   *  \param c The colon between the declarator and the bit count.
   *  \param e The expression specifying the number of bits. */
  CT_BitFieldDeclarator (CTree *d, CTree *c, CTree *e = 0) {
    AddSon (sons[0], d); AddSon (sons[1], c); AddSon (sons[2], e); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 3); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 3, n); }
  /** Get the declarator. */
  CTree *Declarator () const { return sons[0]; }
  /** Get the expression specifying the number of bits. */
  CTree *Expr () const { return sons[2]; }
  /** Set the expression specifying the number of bits. */
  void FieldSize (CTree *s) { AddSon (sons[2], s); }
  /** Get the semantic information about the declared bit-field. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 3, old_son, new_son);
  }
};

/*****************************************************************************/
/*                                                                           */
/*                              Statements                                   */
/*                                                                           */
/*****************************************************************************/

/** \class CT_Statement CTree.h Puma/CTree.h
 *  Base class for all tree nodes representing statements. */
class CT_Statement : public CTree { 
protected:
  /** Constructor. */
  CT_Statement () {}
  /** Get this. */
  virtual CT_Statement *IsStatement () { return this; }
};

/** \class CT_LabelStmt CTree.h Puma/CTree.h
 *  Tree node representing a label statement.
 *  Example: \code incr_a: a++; \endcode */
class CT_LabelStmt : public CT_Statement {
  CTree *sons[3]; // id, colon, stmt

public:
  /** Constructor.
   *  \param id The name of the label.
   *  \param c The colon behind the label.
   *  \param stmt The statement following the label. */
  CT_LabelStmt (CTree *id, CTree *c, CTree *stmt) {
    AddSon (sons[0], id); AddSon (sons[1], c); AddSon (sons[2], stmt); 
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
  /** Get the statement. */
  CT_Statement *Statement () const { return (CT_Statement*)sons[2]; }
  /** Get the name of the label. */
  CT_SimpleName *Label () const { return (CT_SimpleName*)sons[0]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 3, old_son, new_son);
  }
};

/** \class CT_DefaultStmt CTree.h Puma/CTree.h
 *  Tree node representing a default statement of a switch statement.
 *  Example: \code default: break; \endcode */
class CT_DefaultStmt : public CT_Statement {
  CTree *sons[3]; // keyword, colon, stmt

public:
  /** Constructor.
   *  \param kw The keyword 'default'.
   *  \param c The colon behind the keyword.
   *  \param stmt The statement of the default case. */
  CT_DefaultStmt (CTree *kw, CTree *c, CTree *stmt) {
    AddSon (sons[0], kw); AddSon (sons[1], c); AddSon (sons[2], stmt); 
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
  /** Get the statement. */
  CT_Statement *Statement () const { return (CT_Statement*)sons[2]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 3, old_son, new_son);
  }
};

/** \class CT_TryStmt CTree.h Puma/CTree.h
 *  Tree node representing a try-catch statement.
 *  Example: \code try { f(); } catch (...) {} \endcode */
class CT_TryStmt : public CT_Statement {
  CTree *sons[3]; // try, stmt, handlers

public:
  /** Constructor.
   *  \param t The keyword 'try'.
   *  \param s The statement enclosed in the try-catch block.
   *  \param h The exception handler sequence. */
  CT_TryStmt (CTree *t, CTree *s, CTree *h) {
    AddSon (sons[0], t); AddSon (sons[1], s); AddSon (sons[2], h); 
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
  /** Get the enclosed statement. */
  CT_Statement *Statement () const { return (CT_Statement*)sons[1]; }
  /** Get the exception handler sequence. */
  CT_HandlerSeq *Handlers () const { return (CT_HandlerSeq*)sons[2]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 3, old_son, new_son);
  }
};

/** \class CT_CaseStmt CTree.h Puma/CTree.h
 *  Tree node representing a case statement.
 *  Example: \code case 42: a=42; \endcode */
class CT_CaseStmt : public CT_Statement {
  CTree *sons[4]; // keyword, expr, colon, stmt

public:
  /** Constructor.
   *  \param kw The keyword 'case'.
   *  \param expr The constant expression specifying the case value.
   *  \param c The colon.
   *  \param stmt The statement of the case. */
  CT_CaseStmt (CTree *kw, CTree *expr, CTree *c, CTree *stmt) {
    AddSon (sons[0], kw); AddSon (sons[1], expr); 
    AddSon (sons[2], c); AddSon (sons[3], stmt); 
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
  /** Get the statement. */
  CT_Statement *Statement () const { return (CT_Statement*)sons[3]; }
  /** Get the expression specifying the case value. */
  CTree *Expr () const { return sons[1]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 4, old_son, new_son);
  }
};

/** \class CT_ExprStmt CTree.h Puma/CTree.h
 *  Tree node representing an expression statement.
 *  Example: \code a+b; \endcode */
class CT_ExprStmt : public CT_Statement {
  CTree *sons[2]; // expr, semi_colon

public:
  /** Constructor.
   *  \param expr The expression.
   *  \param sc The trailing semi-colon. */
  CT_ExprStmt (CTree *expr, CTree *sc) { AddSon (sons[0], expr); AddSon (sons[1], sc); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 2); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 2, n); }
  /** Get the expression. */
  CTree *Expr () const { return sons[0]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 2, old_son, new_son);
  }
};

/** \class CT_DeclStmt CTree.h Puma/CTree.h
 *  Tree node representing a declaration statement.
 *  Example: \code int i = 0; \endcode */
class CT_DeclStmt : public CT_Statement {
  CTree *_decl;

public:
  /** Constructor.
   *  \param decl The declaration. */
  CT_DeclStmt (CTree *decl) { AddSon (_decl, decl); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 1; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return n == 0 ? _decl : (CTree*)0; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) 
   { CTree::ReplaceSon (_decl, old_son, new_son); }
};

/** \class CT_SwitchStmt CTree.h Puma/CTree.h
 *  Tree node representing a switch statement.
 *  Example: \code switch(a) { case 0: a++; } \endcode */
class CT_SwitchStmt : public CT_Statement, public CSemScope {
  CTree *sons[5]; // keyword, open, cond, close, stmt

public:
  /** Constructor.
   *  \param kw The keyword 'switch'.
   *  \param o Left parenthesis before the condition.
   *  \param cond The switch-expression. 
   *  \param c Right parenthesis behind the condition. 
   *  \param stmt The cases of the switch-statement. */
  CT_SwitchStmt (CTree *kw, CTree *o, CTree *cond, CTree *c, CTree *stmt) {
    AddSon (sons[0], kw); AddSon (sons[1], o); AddSon (sons[2], cond); 
    AddSon (sons[3], c); AddSon (sons[4], stmt); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 5; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 5, n); }
  /** Get the cases. */
  CT_Statement *Statement () const { return (CT_Statement*)sons[4]; }
  /** Get the switch-expression. */
  CTree *Condition () const { return sons[2]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 5, old_son, new_son);
  }
  /** Get the scope opened by the switch-statement. */
  CSemScope *SemScope () const { return (CSemScope*)this; }
};

/** \class CT_IfStmt CTree.h Puma/CTree.h
 *  Tree node representing a if-statement.
 *  Example: \code if(a==0) a++; \endcode */
class CT_IfStmt : public CT_Statement, public CSemScope {
  CTree *sons[5]; // keyword, open, cond, close, stmt

public:
  /** Constructor.
   *  \param kw The keyword 'if'.
   *  \param o Left parenthesis before the condition.
   *  \param cond The condition.
   *  \param c Right parenthesis behind the condition.
   *  \param stmt The controlled statement. */
  CT_IfStmt (CTree *kw, CTree *o, CTree *cond, CTree *c, CTree *stmt) {
    AddSon (sons[0], kw); AddSon (sons[1], o); AddSon (sons[2], cond); 
    AddSon (sons[3], c); AddSon (sons[4], stmt); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 5; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 5, n); }
  /** Get the controlled statement. */
  CT_Statement *Statement () const { return (CT_Statement*)sons[4]; }
  /** Get the condition. */
  CTree *Condition () const { return sons[2]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 5, old_son, new_son);
  }
  /** Get the scope opened by the if-statement. */
  CSemScope *SemScope () const { return (CSemScope*)this; }
};

/** \class CT_IfElseStmt CTree.h Puma/CTree.h
 *  Tree node representing a if-else-statement.
 *  Example: \code if(a==0) a++; else a=0; \endcode */
class CT_IfElseStmt : public CT_Statement, public CSemScope {
  CTree *sons[7]; // if, open, cond, close, if_stmt, else, else_stmt

public:
  /** Constructor.
   *  \param i The keyword 'if'.
   *  \param o Left parenthesis before the condition.
   *  \param cond The condition.
   *  \param c Right parenthesis behind the condition.
   *  \param is The statement controlled by the if-branch.
   *  \param e The keyword 'else'.
   *  \param es The statement controlled by the else-branch. */
  CT_IfElseStmt (CTree *i, CTree *o, CTree *cond, CTree *c, 
                 CTree *is, CTree *e, CTree *es) {
    AddSon (sons[0], i); AddSon (sons[1], o); AddSon (sons[2], cond); 
    AddSon (sons[3], c); AddSon (sons[4], is); AddSon (sons[5], e); 
    AddSon (sons[6], es); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 7; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 7, n); }
  /** Get the condition. */
  CTree *Condition () const { return sons[2]; }
  /** Get the statement controlled by the if-branch. */
  CT_Statement *IfPart () const { return (CT_Statement*)sons[4]; }
  /** Get the statement controlled by the else-branch. */
  CT_Statement *ElsePart () const { return (CT_Statement*)sons[6]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 7, old_son, new_son);
  }
  /** Get the scope opened by the if-statement. */
  CSemScope *SemScope () const { return (CSemScope*)this; }
};

/** \class CT_BreakStmt CTree.h Puma/CTree.h
 *  Tree node representing a break-statement.
 *  Example: \code break; \endcode */
class CT_BreakStmt : public CT_Statement {
  CTree *sons[2]; // key, semi_colon

public:
  /** Constructor.
   *  \param key The keyword 'break'.
   *  \param sc The trailing semi-colon. */
  CT_BreakStmt (CTree *key, CTree *sc) { AddSon (sons[0], key); AddSon (sons[1], sc); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 2; }
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
};

/** \class CT_ContinueStmt CTree.h Puma/CTree.h
 *  Tree node representing a continue-statement.
 *  Example: \code continue; \endcode */
class CT_ContinueStmt : public CT_Statement {
  CTree *sons[2]; // key, semi_colon

public:
  /** Constructor.
   *  \param key The keyword 'continue'.
   *  \param sc The trailing semi-colon. */
  CT_ContinueStmt (CTree *key, CTree *sc) { AddSon (sons[0], key); AddSon (sons[1], sc); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 2; }
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
};

/** \class CT_GotoStmt CTree.h Puma/CTree.h
 *  Tree node representing a goto-stmt.
 *  Example: \code goto incr_a; \endcode */
class CT_GotoStmt : public CT_Statement {
  CTree *sons[3]; // key, label, semi_colon

public:
  /** Constructor.
   *  \param key The keyword 'goto'.
   *  \param l The name of the jump label.
   *  \param sc The trailing semi-colon. */
  CT_GotoStmt (CTree *key, CTree *l, CTree *sc) {
    AddSon (sons[0], key); AddSon (sons[1], l); AddSon (sons[2], sc); 
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
  /** Get the name of the jump label. */
  CT_SimpleName *Label () const { return (CT_SimpleName*)sons[1]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 3, old_son, new_son);
  }
};

/** \class CT_ReturnStmt CTree.h Puma/CTree.h
 *  Tree node representing a return-statement.
 *  Example: \code return 1; \endcode */
class CT_ReturnStmt : public CT_Statement {
  CTree *sons[3]; // key, expr, semi_colon

public:
  /** Constructor.
   *  \param key The keyword 'return'.
   *  \param e The expression specifying the return value. 
   *  \param sc The trailing semi-colon. */
  CT_ReturnStmt (CTree *key, CTree *e, CTree *sc) {
    AddSon (sons[0], key); AddSon (sons[1], e); AddSon (sons[2], sc); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 3); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 3, n); }
  /** Get the expression specifying the return value. */
  CTree *Expr () const { return sons[1]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 3, old_son, new_son);
  }
};

/** \class CT_WhileStmt CTree.h Puma/CTree.h
 *  Tree node representing a while-statement.
 *  Example: \code while(a>0) a--; \endcode */
class CT_WhileStmt : public CT_Statement, public CSemScope {
  CTree *sons[5]; // key, open, cond, close, stmt

public:
  /** Constructor.
   *  \param kw The keyword 'while'.
   *  \param o Left parenthesis before the condition.
   *  \param cond The loop condition. 
   *  \param c Right parenthesis behind the condition. 
   *  \param stmt The controlled statement. */
  CT_WhileStmt (CTree *kw, CTree *o, CTree *cond, CTree *c, CTree *stmt) {
    AddSon (sons[0], kw); AddSon (sons[1], o); AddSon (sons[2], cond); 
    AddSon (sons[3], c); AddSon (sons[4], stmt); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 5; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 5, n); }
  /** Get the controlled statement. */
  CT_Statement *Statement () const { return (CT_Statement*)sons[4]; }
  /** Get the loop condition. */
  CTree *Condition () const { return sons[2]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 5, old_son, new_son);
  }
  /** Get the scope opened by the while-statement. */
  CSemScope *SemScope () const { return (CSemScope*)this; }
};

/** \class CT_DoStmt CTree.h Puma/CTree.h
 *  Tree node representing a do-while-statement.
 *  Example: \code do a--; while(a>0); \endcode */
class CT_DoStmt : public CT_Statement {
  CTree *sons[7]; // do, stmt, while, open, expr, close, semi_colon

public:
  /** Constructor.
   *  \param d The keyword 'do'.
   *  \param stmt The controlled statement.
   *  \param w The keyword 'while'.
   *  \param o Left parenthesis before the loop condition.
   *  \param e The loop condition.
   *  \param c Right parenthesis behind the loop condition.
   *  \param sc The trailing semi-colon. */
  CT_DoStmt (CTree *d, CTree *stmt, CTree *w, CTree *o, CTree *e, 
             CTree *c, CTree *sc) {
    AddSon (sons[0], d); AddSon (sons[1], stmt); AddSon (sons[2], w); 
    AddSon (sons[3], o); AddSon (sons[4], e); AddSon (sons[5], c); 
    AddSon (sons[6], sc); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 7; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 7, n); }
  /** Get the controlled statement. */
  CT_Statement *Statement () const { return (CT_Statement*)sons[1]; }
  /** Get the loop condition. */
  CTree *Expr () const { return sons[4]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 7, old_son, new_son);
  }
};

/** \class CT_ForStmt CTree.h Puma/CTree.h
 *  Tree node representing a for-statement.
 *  Example: \code for(int i=0; i<10; i++) f(i); \endcode */
class CT_ForStmt : public CT_Statement, public CSemScope {
  CTree *sons[8]; // key, open, init, cond, semi_colon, expr, close, stmt

public:
  /** Constructor.
   *  \param k The keyword 'for'.
   *  \param o Left parenthesis.
   *  \param i The loop initializer statement.
   *  \param co The loop condition.
   *  \param sc The semi-colon behind the loop condition.
   *  \param e The loop counter expression.
   *  \param c Right parenthesis.
   *  \param stmt The controlled statement. */
  CT_ForStmt (CTree *k, CTree *o, CTree *i, CTree *co, CTree *sc,
              CTree *e, CTree *c, CTree *stmt) {
    AddSon (sons[0], k); AddSon (sons[1], o); AddSon (sons[2], i); 
    AddSon (sons[3], co); AddSon (sons[4], sc); AddSon (sons[5], e); 
    AddSon (sons[6], c); AddSon (sons[7], stmt); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 8); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 8, n); }
  /** Get the loop initializer. */
  CTree *InitStmt () const { return sons[2]; }
  /** Get the loop condition. */
  CTree *Condition () const { return sons[3]; }
  /** Get the loop counter expression. */
  CTree *Expr () const { return sons[5]; }
  /** Get the controlled statement. */
  CT_Statement *Statement () const { return (CT_Statement*)sons[7]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 8, old_son, new_son);
  }
  /** Get the scope opened by the for-statement. */
  CSemScope *SemScope () const { return (CSemScope*)this; }
};

/** \class CT_Condition CTree.h Puma/CTree.h
 *  Tree node representing a control-statement condition.
 *  Example: \code int i = 0 \endcode */
class CT_Condition : public CT_Decl, public CSemObject {
  CTree *sons[3]; // declspecs, declarator, init

public:
  /** Constructor.
   *  \param dsl The declaration specifier sequence. 
   *  \param d The variable declarator. */
  CT_Condition (CTree *dsl, CTree *d) {
    AddSon (sons[0], dsl); AddSon (sons[1], d); AddSon (sons[2], 0);
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 3); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 3, n); }
  /** Get the declaration specifier sequence. */
  CT_DeclSpecSeq *DeclSpecs () const { return (CT_DeclSpecSeq*)sons[0]; }
  /** Get the declarator. */
  CTree *Declarator () const { return sons[1]; }
  /** Get the initializer of the declaration. */
  CT_ExprList *Initializer () const { return (CT_ExprList*)sons[2]; }
  /** Get the semantic information of the declared object. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
  /** Set the initializer. */
  void Initializer (CTree *i) { AddSon (sons[2], i); }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 3, old_son, new_son);
  }
};

/*****************************************************************************/
/*                                                                           */
/*                              Classes                                      */
/*                                                                           */
/*****************************************************************************/

/** \class CT_ClassDef CTree.h Puma/CTree.h
 *  Tree node representing a class definition.
 *  Example: \code class X : Y { int x; } \endcode */
class CT_ClassDef : public CT_Decl, public CSemObject {
  CTree *sons[4]; // key, name, bases, members
  CTree *obj_decl;

public:
  /** Constructor.
   *  \param k The keyword 'class' or 'struct'.
   *  \param n The name of the class.
   *  \param b The base class list. */
  CT_ClassDef (CTree *k, CTree *n, CTree *b = (CTree*)0) {
    AddSon (sons[0], k); AddSon (sons[1], n); AddSon (sons[2], b); 
    AddSon (sons[3], 0); AddSon (obj_decl, 0); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 4); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 4, n); }
  /** Get the name of the class. */
  CT_SimpleName *Name () const { return (CT_SimpleName*)sons[1]; }
  /** Get the member declarations list. */
  CT_MembList *Members () const { return (CT_MembList*)sons[3]; }
  /** Get the base class specifiers list. */
  CT_BaseSpecList *BaseClasses () const { return (CT_BaseSpecList*)sons[2]; }
  /** Get the object declaration node containing the class definition. */
  CT_ObjDecl *ObjDecl () const { return (CT_ObjDecl*)obj_decl; }
  /** Get the semantic information about the declared class. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
  /** Set the member declarations list. */
  void Members (CTree *m) { AddSon (sons[3], m); }
  /** Set the base class specifiers list. */
  void BaseClasses (CTree *bc) { AddSon (sons[2], bc); }
  /** Set the object declaration containing the class definition. */
  void ObjDecl (CTree *od) { AddSon (obj_decl, od); }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) {
    CTree::ReplaceSon (sons, 4, old_son, new_son);
  }
};
      
/** \class CT_UnionDef CTree.h Puma/CTree.h
 *  Tree node representing the definition of a union.
 *  Example: \code union U { int i; } \endcode */
class CT_UnionDef : public CT_ClassDef {
public:
  /** Constructor.
   *  \param k The keyword 'union'.
   *  \param n The name of the union.
   *  \param b The base union list. */
  CT_UnionDef (CTree *k, CTree *n, CTree *b = 0) : CT_ClassDef (k, n, b) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};
      
/** \class CT_MembList CTree.h Puma/CTree.h
 *  Tree node representing a member declarations list. */ 
class CT_MembList : public CT_DeclList, public CSemScope {
public:
  CT_MembList (int size = 10, int incr = 10) : 
    CT_DeclList (size, incr) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the scope opened by the member declarations list. */
  CSemScope *SemScope () const { return (CSemScope*)this; }
};

/** \class CT_MembInitList CTree.h Puma/CTree.h
 *  Tree node representing a constructor initializer list.
 *  Example: \code : Base(), m_Member(0) \endcode */
class CT_MembInitList : public CT_List, public CSemScope {
public:
  /** Constructor.
   *  \param size The initial size of the list. */
  CT_MembInitList (int size = 2) : 
    CT_List (size, 2, CT_List::OPEN) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the scope opened by the member initializer list. */
  CSemScope *SemScope () const { return (CSemScope*)this; }
};

/** \class CT_MembInit CTree.h Puma/CTree.h
 *  Tree node representing a member initializer.
 *  Example: \code m_Member(0) \endcode */
class CT_MembInit : public CT_Expression, public CSemObject {
  CTree *sons[2]; // name, init

public:
  /** Constructor.
   *  \param n The name of the member.
   *  \param i The member initializer. */
  CT_MembInit (CTree *n, CTree *i) { AddSon (sons[0], n); AddSon (sons[1], i); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 2; }
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
  /** Get the name of the member. */
  CT_SimpleName *Name () const { return (CT_SimpleName*)sons[0]; }
  /** Get the initializer. */
  CT_ExprList *Initializer () const { return (CT_ExprList*)sons[1]; }
  /** Get the semantic information about the initialized member. */
  CSemObject *SemObject () const { return (CSemObject*)this; }
};

/** \class CT_BaseSpecList CTree.h Puma/CTree.h
 *  Tree node representing a base specifier list.
 *  Example: \code : X, Y, Z \endcode */
class CT_BaseSpecList : public CT_List {
public:
  /** Constructor.
   *  \param size The initial size of the list. */
  CT_BaseSpecList (int size = 2) : 
    CT_List (size, 2, CT_List::OPEN|CT_List::SEPARATORS) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_AccessSpec CTree.h Puma/CTree.h
 *  Tree node representing an access specifier.
 *  Example: \code public: \endcode */
class CT_AccessSpec : public CTree {
  CTree *sons[2]; // access, colon

public:
  /** Constructor.
   *  \param a The access specifier, i.e. 'public', 'private', or 'protected'.
   *  \param c The trailing colon. */
  CT_AccessSpec (CTree *a, CTree *c) { AddSon (sons[0], a); AddSon (sons[1], c); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 2; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 2, n); }
  /** Get the access specifier type (token type). */
  int Access () const { return sons[0]->token ()->type (); }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 2, old_son, new_son);
  }
};

/** \class CT_BaseSpec CTree.h Puma/CTree.h
 *  Tree node representing a base class specifier.
 *  Example: \code public X \endcode */
class CT_BaseSpec : public CTree {
  CTree *sons[3]; // virtual, access, name

public:
  /** Constructor.
   *  \param v Optional keyword 'virtual'.
   *  \param a The optional access specifier.
   *  \param n The name of the base class. */
  CT_BaseSpec (CTree *v, CTree *a, CTree *n) {
    AddSon (sons[0], v); AddSon (sons[1], a); AddSon (sons[2], n); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 3); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 3, n); }
  /** Get the type of the access specifier (token type). */
  int Access () const { return sons[1]->token ()->type (); }
  /** The access specifier. */
  CTree *AccessSpec () const { return sons[1]; }
  /** Get the keyword 'virtual'. */
  CTree *Virtual () const { return sons[0]; }
  /** Get the name of the base class. */
  CT_SimpleName *Name () const { return (CT_SimpleName*)sons[2]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 3, old_son, new_son);
  }
};

/** \class CT_AccessDecl CTree.h Puma/CTree.h
 *  Tree node representing a member access declaration.
 *  Example: \code m_BaseClassMember; \endcode */
class CT_AccessDecl : public CT_Decl {
  CTree *sons[2]; // name, semi_colon

public:
  /** Constructor.
   *  \param n The name of the base class member.
   *  \param s The trailing semi-colon. */
  CT_AccessDecl (CTree *n, CTree *s) { AddSon (sons[0], n); AddSon (sons[1], s); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return 2; }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const { return CTree::Son (sons, 2, n); }
  /** Get the name of the base class member. */
  CT_QualName *Member () const { return (CT_QualName*)sons[0]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 2, old_son, new_son);
  }
};

/** \class CT_UsingDecl CTree.h Puma/CTree.h
 *  Tree node representing a using declaration.
 *  Example: \code using Base::m_Member; \endcode */
class CT_UsingDecl : public CT_AccessDecl {
  CTree *sons[2]; // using, typename

public:
  /** Constructor.
   *  \param u The keyword 'using'.
   *  \param n The name of the entity.
   *  \param s The trailing semi-colon. */
  CT_UsingDecl (CTree *u, CTree *n, CTree *s) : CT_AccessDecl (n, s) {
    AddSon (sons[0], u); AddSon (sons[1], 0); 
  }
  /** Constructor.
   *  \param u The keyword 'using'.
   *  \param t The keyword 'typename'.
   *  \param n The name of the entity.
   *  \param s The trailing semi-colon. */
  CT_UsingDecl (CTree *u, CTree *t, CTree *n, CTree *s) : CT_AccessDecl (n, s) {
    AddSon (sons[0], u); AddSon (sons[1], t); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 2) + CT_AccessDecl::Sons (); }
  /** Get the n-th son.
   *  \param n The index of the son.
   *  \return The n-th son or NULL. */
  CTree *Son (int n) const {
    int num = CTree::Sons (sons, 2);
    CTree *result = CTree::Son (sons, 2, n);
    return result ? result : CT_AccessDecl::Son (n-num);
  }
  /** Get the keyword 'typename'. */
  CTree *Typename () const { return sons[1]; }
  /** Replace a son.
   *  \param old_son The son to replace.
   *  \param new_son The new son. */
  void ReplaceSon (CTree *old_son, CTree *new_son) { 
    CTree::ReplaceSon (sons, 2, old_son, new_son);
    CT_AccessDecl::ReplaceSon (old_son, new_son);
  }
};

/*****************************************************************************/
/*                                                                           */
/*                              Wildcards                                    */
/*                                                                           */
/*****************************************************************************/

/** \class CT_Any CTree.h Puma/CTree.h
 *  Tree node representing a wildcard. */
class CT_Any : public CTree {
  CTree *sons[2]; // keyword, extension

public:
  /** Constructor.
   *  \param k The wildcard keyword.
   *  \param e The extension. */
  CT_Any (CTree *k, CTree *e = (CTree*)0) { AddSon (sons[0], k); AddSon (sons[1], e); }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 2); }
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
  /** Get the type of the wildcard (token type). */
  int AnyType () const { return sons[0]->token ()->type (); }
  /** Get the extension. */
  CT_AnyExtension *Extension () const { return (CT_AnyExtension*)sons[1]; }
};

/** \class CT_AnyList CTree.h Puma/CTree.h
 *  Tree node representing a list wildcard. */
class CT_AnyList : public CT_Any {
public:
  /** Constructor.
   *  \param k The wildcard keyword.
   *  \param e The extension. */
  CT_AnyList (CTree *k, CTree *e = (CTree*)0) : CT_Any (k, e) {}
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
};

/** \class CT_AnyExtension CTree.h Puma/CTree.h
 *  Tree node representing a wildcard extension. */
class CT_AnyExtension : public CTree, public CSemValue {
  CTree *sons[5]; // open, string, comma, cond, close

public:
  /** Constructor.
   *  \param o Left parenthesis before the extension. 
   *  \param n The name of the extension.
   *  \param co The comma between the name and the condition. 
   *  \param c The condition.
   *  \param cr Right parenthesis behind the extension. */
  CT_AnyExtension (CTree *o, CTree *n, CTree *co, CTree *c, CTree *cr) {
    AddSon (sons[0], o); AddSon (sons[1], n); AddSon (sons[2], co); 
    AddSon (sons[3], c); AddSon (sons[4], cr); 
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
  /** Get the condition. */
  CTree *Condition () const { return sons[3]; }
  /** Get the name string. */
  CT_Token *String () const { return (CT_Token*)sons[1]; }
  /** Get the extension name. */
  const char *Name () const { return value ? value->StrLiteral ()->String () : (const char*)0; }
  /** Get the value of the extension (the name). */
  CExprValue *Value () const { return value; }
  /** Get the semantic value information object. */
  CSemValue *SemValue () const { return (CSemValue*)this; }
};

/** \class CT_AnyCondition CTree.h Puma/CTree.h
 *  Tree node representing the condition of a wildcard. */
class CT_AnyCondition : public CTree {
  CTree *sons[3]; // arg1, arg2, arg3

public:
  /** Constructor.
   *  \param a1 The first argument.
   *  \param a2 The optional second argument.
   *  \param a3 The optional third argument. */
  CT_AnyCondition (CTree *a1, CTree *a2 = (CTree*)0, CTree *a3 = (CTree*)0) {
    AddSon (sons[0], a1); AddSon (sons[1], a2); AddSon (sons[2], a3); 
  }
  /** Get the identifier for this node type. Can be compared with NodeName(). */
  static const char *NodeId ();
  /** Get the name of the node. Can be compared with NodeId(). */
  const char *NodeName () const { return NodeId (); }
  /** Get the number of sons. */
  int Sons () const { return CTree::Sons (sons, 3); }
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
};


} // namespace Puma

#endif /* __CTree_h__ */
