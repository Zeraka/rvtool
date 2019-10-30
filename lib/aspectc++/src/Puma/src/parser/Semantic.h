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

#ifndef __Semantic_h__
#define __Semantic_h__

/** \file
 *  Basic semantic analysis component. */
 
/** \page semantic Semantic Analysis
 */

#include "Puma/CSemDatabase.h"
#include "Puma/CObjectInfo.h"
#include "Puma/ErrorSink.h"
#include "Puma/PtrStack.h"
#include "Puma/Unit.h"
#include "Puma/BCStack.h"

namespace Puma {


class CTree;
class CFileInfo;
class CEnumInfo;
class CStructure;
class CTemplateInfo;
class CSemDeclSpecs;
class CT_DeclSpecSeq;
class CSemDeclarator;

/*DEBUG*/extern int TRACE_SCOPES;

/** \class Semantic Semantic.h Puma/Semantic.h
 *  %Semantic analysis base class. Implements the basic semantic
 *  analysis tasks. To be derived to implement language specific
 *  semantic analyses. 
 *
 *  There are two kinds of semantic analyses in %Puma. The first
 *  is the semantic analysis performed in parallel with the 
 *  syntactic analysis (see Syntax) of the source code to differentiate 
 *  ambigous syntactic constructs, resolve names, and so on. The other 
 *  is an additional semantic analysis of the syntax tree (mainly of 
 *  expressions) based on a tree visitor (see CVisitor) to resolve function 
 *  calls, calculate the value of constant expressions, resolve the type 
 *  of an expression, and so on. This class performs the first kind of 
 *  semantic analysis. 
 *
 *  The semantic object is organized as a multi-level stack. If a grammar 
 *  rule is parsed then a new stack level is created. The semantic objects
 *  created while parsing the rule are pushed on this level of the stack. 
 *  When parsing the grammar rule is finished, then the current stack level 
 *  is discarded. If the grammar rule could not be parsed successfully 
 *  then the semantic objects pushed on the stack are destroyed before
 *  the current stack level is discarded. */
class Semantic : public PtrStack<CObjectInfo> {
protected:
  /** Database of all semantic objects of the semantic tree. */
  CSemDatabase *_db;
  /** Top level semantic object representing the file scope. */
  CFileInfo *_file;
  /** Error output stream. */
  ErrorSink *_err;
  /** Counter for generating anonymous (private) names. */
  long _Anonymous;

  /** The current enumeration, if inside the definition of an enumeration. */
  CEnumInfo *current_enum; 
  /** The current function, if inside the definition of a function. */
  CStructure *current_fct; 
  /** The current scope. */
  CStructure *current_scope; 

  /** Stack for the type analysis of declarations. */
  BCStack<CSemDeclSpecs*, 256> _sem_decl_specs;
  /** Stack for collecting declaration specifiers. */
  BCStack<CT_DeclSpecSeq*, 256> _decl_specs;
  /** Stack for the state of analysing a function parameter list. */
  BCStack<bool, 256> _in_param_decl_clause;
  /** Stack for the state of analysing a declaration. */
  BCStack<bool, 256> _in_decl;

protected:
  /** Constructor. */
  Semantic () {}
  /** Destructor. */
  virtual ~Semantic ();
  
public:
  /** Initialize the semantic analysis. 
   *  \param db The semantic object database. 
   *  \param file The input file. */
  virtual void init (CSemDatabase &db, Unit &file);
  /** Configure the semantic analysis.
   *  \param c The configuration settings. */
  virtual void configure (Config &c) {}

  /** Set the error output stream object. 
   *  \param e The error output stream. */
  void error_sink (ErrorSink &e);
  /** Undo the declaration analysis of the given syntax tree node.
   *  \param tree The syntax tree node. */
  void undo (CTree *tree);
  /** Create a new anonymous (private) name. 
   *  \return A newly allocated node of type CT_PrivateName. 
   *          Has to be destroyed by the caller. */
  CTree *PrivateName ();

  /** Save the current semantic analysis state. */
  void save_state ();
  /** Discard the saved semantic analysis state. */
  void forget_state ();
  /** Restore the saved semantic analysis state. */
  void restore_state ();

  /** Get the root node of the semantic tree. */
  CFileInfo* getFile ();

public:
  /** Enter a local scope. Creates a new CLocalScope and pushes
   *  it on the scope stack. */
  void enter_local_scope ();
  /** Leave all scopes entered up to the current scope. Make 
   *  the parent scope of the current scope to the current 
   *  scope. */
  virtual void leave_scopes ();
  /** Enter a function parameter declaration clause. */
  virtual void enter_param_decl_clause ();
  /** Leave a function parameter declaration clause. */
  virtual void leave_param_decl_clause ();
  /** Check if in the declaration of a function parameter clause. */
  bool in_param_decl_clause ();

public:
  /** Finish the analysis of the current declaration. */
  void finish_decl ();
  /** Finish the analysis of the current declarator. */
  void finish_declarator ();

protected:
  /** Apply command settings on the given semantic object.
   *  That is setting the semantic database, the source file,
   *  the start token of the analysed syntactic construct, and
   *  the syntax tree node.
   *  \param info The semantic information object.
   *  \param tree The analysed syntax tree. */
  void common_settings (CObjectInfo *info, CTree *tree);
  /** Apply command settings on the given semantic object.
   *  That is setting the semantic database, the source file,
   *  the start token of the analysed syntactic construct, 
   *  the syntax tree node, the type, and the name.
   *  \param info The semantic information object.
   *  \param tree The analysed syntax tree. 
   *  \param d Declarator information object. */
  void common_settings (CObjectInfo *info, CTree *tree , CSemDeclarator *d);
  /** Delete the top item on the semantic stack. 
   *  To be implemented by derived classes. */
  virtual void Delete ();
  /** Push the given semantic information object on the semantic stack. 
   *  \param info The semantic object. */
  void Push (CObjectInfo* info);
  
  /** Check if currently analysing a declaration. */
  bool in_decl () const;
  /** Begin analysing a declaration.
   *  \param ds The declaration specifiers. */
  void decl_begin (CSemDeclSpecs *ds);
  /** Finish analysing a declaration. */
  void decl_end ();
  /** Begin analysing a declaration specifier sequence. 
   *  \param dss Empty object for the declaration specifiers. */
  void decl_specs_begin (CT_DeclSpecSeq *dss);
  /** Finish analysing a declaration specifier sequence. */
  void decl_specs_end ();
  
  /** Get the semantic information for the currently analysed 
   *  declaration specifier sequence. */
  CSemDeclSpecs *sem_decl_specs () const;
  /** Get the currently analysed declaration specifier sequence. */
  CT_DeclSpecSeq *decl_specs () const;
  
  /** Enter the given scope. Sets the parent of the entered
   *  scope to the current scope.
   *  \param scope The scope to enter. */
  virtual void enter_scope (CStructure *scope);
  /** Reenter the given scope. Does not reset the parent of 
   *  entered scope.
   *  \param scope The scope to reenter. */
  virtual void reenter_scope (CStructure *scope);
};

inline CFileInfo* Semantic::getFile ()
 { return _file; }

inline void Semantic::save_state () 
 { New (); }
inline void Semantic::forget_state () 
 { leave_scopes (); }
inline void Semantic::restore_state () 
 { Destroy (); }

inline bool Semantic::in_decl () const 
// { return _in_decl.length () > 0 ? _in_decl.top () : false; }
 { return _in_decl.empty () ? false : _in_decl.top (); }
inline void Semantic::finish_decl ()
 { if (in_decl ()) decl_end (); }

inline void Semantic::error_sink (ErrorSink &e) 
 { _err = &e; }

inline CSemDeclSpecs *Semantic::sem_decl_specs () const
 { return _sem_decl_specs.top (); }
inline CT_DeclSpecSeq *Semantic::decl_specs () const
 { return _decl_specs.top (); }
inline void Semantic::decl_specs_begin (CT_DeclSpecSeq *dss) 
 { _decl_specs.push (dss); }
inline void Semantic::decl_specs_end () 
 { _decl_specs.pop (); }


} // namespace Puma

#endif /* __Semantic_h__ */
