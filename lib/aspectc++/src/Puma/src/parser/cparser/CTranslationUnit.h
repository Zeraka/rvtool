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

#ifndef __CTranslationUnit__
#define __CTranslationUnit__

/** \file
 *  Abstraction of a translation unit. */

#include "Puma/CSemDatabase.h"
#include "Puma/UnitManager.h"
#include "Puma/CProject.h"
#include "Puma/Builder.h"
#include "Puma/Unit.h"

namespace Puma {


class CTree;
class PreTree;


/** \class CTranslationUnit CTranslationUnit.h Puma/CTranslationUnit.h
 *  Abstraction of a translation unit. A translation unit is a 
 *  single implementation file that a compiler can translate 
 *  to an object file. In particular it contains all the code 
 *  from included header files. 
 *
 *  A translation unit is created by the parser (see Puma::Parser).
 *  It encapsulates the input file and the parse results (syntax
 *  trees, semantic database). When it is destroyed, it also destroys
 *  the parse results. */
class CTranslationUnit {
  Unit *_unit;
  CTree *_syntax_tree;
  PreTree *_cpp_tree;
  CSemDatabase _class_db;
  UnitManager _local_units;

public:
  /** Constructor.
   *  \param u The token unit of the input file. 
   *  \param p The project information object. */
  CTranslationUnit (Unit &u, CProject &p) :
    _unit (&u),
    _syntax_tree ((CTree*)0),
    _cpp_tree ((PreTree*)0),
    _class_db (p),
    _local_units (p.err ()) {}
  /** Destructor. Destroys the C/C++ syntax tree and the
   *  preprocessor syntax tree. The semantic information
   *  objects are destroyed when the semantic database 
   *  member object is destroyed. If this is a template
   *  instance translation unit then destroy the unit too. */
  ~CTranslationUnit ();

  /** Set the C/C++ syntax tree. 
   *  \param t The root node of the syntax tree. */
  void tree (CTree *t) { _syntax_tree = t; }
  /** Set the preprocessor syntax tree.
   *  \param t The root node of the syntax tree. */
  void cpp_tree (PreTree *t) { _cpp_tree = t; }

  /** Get the C/C++ syntax tree. */
  CTree *tree () const { return _syntax_tree; }
  /** Get the C preprocessor syntax tree. */
  PreTree *cpp_tree () const { return _cpp_tree; }

  /** Get the token unit of the input file. */
  Unit *unit () const { return _unit; }
  /** Get the semantic information database. */
  CSemDatabase &db () const { return (CSemDatabase&)_class_db; }
  /** Get the unit manager for local units (macro expansions etc). */
  UnitManager &local_units () const { return (UnitManager&)_local_units; }

private:
  void destroy (CTree *t) { Builder::destroy(t); }
  void destroy (PreTree *);
};


} // namespace Puma

#endif /* __CTranslationUnit__ */
