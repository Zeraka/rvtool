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

#ifndef __c_sem_decl_specs_h__
#define __c_sem_decl_specs_h__

/** \file
 *  Semantic declaration specifier analysis. */

#include "Puma/CTypeInfo.h"
#include "Puma/CTree.h"

namespace Puma {


class ErrorSink;


/** \class CSemDeclSpecs CSemDeclSpecs.h Puma/CSemDeclSpecs.h
 *  Class for analysing a sequence of declaration specifiers.
 *  The result of the analysis is a type, i.e. the type of 
 *  the declared entity (function, object, etc). */
class CSemDeclSpecs {
  // initialization context
  ErrorSink *_err;
  CT_DeclSpecSeq *_dss;
  
  // analysis results:
  // -----------------
  // counters
  int _names;
  int _class_specs;
  int _union_specs;
  int _enum_specs;
  // nodes/objects
  int _prim_map[CT_PrimDeclSpec::NumTypes];
  Token *_prim_token[CT_PrimDeclSpec::NumTypes];
  CT_SimpleName *_name;

  // final results after all checks:
  // -------------------------------
  CTypeInfo *_type;
  bool _def_class, _def_union, _def_enum;

  // analyses the syntax tree nodes of the decl-spec sequence. Return false
  // if an error was detected. 
  bool analyze_seq ();
  
  // analyzes the current declaration specifier in the sequence and set some
  // attributes according to the result. Returns false if the specifier type
  // is unknown.
  bool analyze (CTree *spec);

  // check functions (used after analyse_seq()
  bool check_prim_decl_specs (bool &have_type);
  bool check_complex_decl_specs (bool &have_type);  
  bool check_storage_class ();
  bool check_signed_unsigned ();
  bool check_long_short (); 
  bool check_finally ();
  
  // type creation code
  CTypeInfo *create_type ();
  CTypeInfo *create_qualifiers (CTypeInfo *type);
  
public:
  /** Constructor.
   *  \param es The error stream on which to report errors.
   *  \param dss The declaration specifier sequence to be analysed.
   *  \param support_implicit_int True if implicit type int shall be supported. */
  CSemDeclSpecs (ErrorSink *es, CT_DeclSpecSeq *dss, bool support_implicit_int = false);
  /** Destructor. Destroys the created type. */
  ~CSemDeclSpecs ();

  /** Get a copy of the resulting type. 
   *  \return The copy of the type. Has to be destroyed by the caller. */
  CTypeInfo *make_type () const;
  /** Get the declaration specifier sequence. */
  CT_DeclSpecSeq *DeclSpecSeq () const;

  /** Check if the given declaration specifier was declared.
   *  \param dst The declaration specifier type. */
  bool declared (CT_PrimDeclSpec::Type dst) const;
  /** Check if a class was defined in the declaration specifier sequence. */
  bool defClass () const;
  /** Check if an union was defined in the declaration specifier sequence. */
  bool defUnion () const;
  /** Check if an enumeration was defined in the declaration specifier sequence. */
  bool defEnum () const;
};

inline CSemDeclSpecs::~CSemDeclSpecs () 
 { if (_type) CTypeInfo::Destroy (_type); }
  
inline CTypeInfo *CSemDeclSpecs::make_type () const 
 { return CTypeInfo::Duplicate (_type); }
inline CT_DeclSpecSeq *CSemDeclSpecs::DeclSpecSeq () const 
 { return _dss; }

inline bool CSemDeclSpecs::declared (CT_PrimDeclSpec::Type as) const 
 { return _prim_map[as] > 0; }

inline bool CSemDeclSpecs::defClass () const
 { return _def_class; }
inline bool CSemDeclSpecs::defUnion () const
 { return _def_union; }
inline bool CSemDeclSpecs::defEnum () const
 { return _def_enum; }


} // namespace Puma

#endif /* __c_sem_decl_specs_h__ */
