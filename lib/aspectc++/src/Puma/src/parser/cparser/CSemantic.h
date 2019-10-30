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

#ifndef __CSemantic_h__
#define __CSemantic_h__

#include "Puma/Semantic.h"
#include "Puma/CBuilder.h"
#include "Puma/CLinkage.h"
#include "Puma/CProtection.h"
#include "Puma/CTree.h"
#include "Puma/PtrStack.h"

namespace Puma {
  class CSyntax;
}

namespace Puma {

class CSemantic : public Semantic {
  CSyntax &_syntax;
  CBuilder &_builder;

protected:
  enum SemObjType { 
    CLASS, UNION, ENUM,  TYPEDEF, FCT,  ATTRIB, ARG,  TAG, NON_TAG,  ANY 
  };

  bool in_arg_decl;
  bool _in_arg_decl_seq;
  bool is_type_name;
  bool support_implicit_int;
  PtrStack<CStructure> non_record_scopes;

  void declareImplicitFcts () {} // builtin functions

public:
  CSemantic (CSyntax &, CBuilder &);

  virtual void init (CSemDatabase &, Unit &);
  void configure (Config &);

protected:
  virtual CBuilder &builder () const;
  virtual CSyntax &syntax () const;
  virtual void Delete ();

public:
  // control parse process
  virtual CTree *id_expr ();
  virtual CTree *typedef_name ();
  virtual CTree *init_declarator ();
  virtual CTree *abst_declarator ();
  virtual CTree *direct_abst_declarator ();
  virtual CTree *param_decl_clause ();
  virtual CTree *finish_fct_def ();
  virtual CTree *arg_decl_seq ();
  virtual CTree *decl_spec_seq1 ();
  virtual CTree *decl_spec_seq_err ();
  virtual CTree *declare_parameter ();
  virtual CTree *identifier_list ();
  virtual bool finish_param_check (bool);
  virtual CTree *begin_decl ();
  virtual CTree *array_delim ();
  virtual CTree *enumerator_def ();

  // add scope information to tree nodes
  virtual CTree *trans_unit ();
  virtual CTree *class_spec ();
  virtual CTree *cmpd_stmt ();
  virtual CTree *select_stmt ();
  virtual CTree *iter_stmt ();

  // introduce names... add new entries to the class database
  virtual void introduce_label (CTree *&);
  virtual CTree *introduce_object (CTree* = 0);
  virtual CTree *introduce_named_type ();
  virtual CTree *introduce_enum ();
  virtual CTree *introduce_enumerator ();
  virtual CTree *introduce_function ();
  virtual CTree *introduce_parameter ();
  virtual CTree *introduce_class ();
  virtual CTree *introduce_member ();
  virtual CTree *introduce_tag ();

public: // change or get semantic state information
  void enter_arg_decl_seq ();
  void leave_arg_decl_seq ();
  void begin_param_check ();
  bool in_arg_decl_seq () const;
  bool decl_spec_seq ();
  bool empty_decl_spec_seq ();
  virtual bool implicit_int ();
  virtual CProtection::Type protection () const;

public:
  void reenter_class_scope ();
  void enter_scope (CStructure *scp);
  void reenter_scope (CStructure *scp);
  void leave_scopes ();

protected:
  CObjectInfo *lookup (const char *, SemObjType, bool = false) const;
  CObjectInfo *lookup (CT_SimpleName *, SemObjType, bool = false) const;
  CObjectInfo *lookup (const char *, CStructure *, SemObjType, bool) const;
  CObjectInfo *lookupBuiltin (const char* name) const;

  CLinkage::Type determine_linkage (CSemDeclSpecs *, SemObjType, 
    CObjectInfo * = (CObjectInfo*)0) const;
  CStorage::Type determine_storage_class (CSemDeclSpecs *, SemObjType, 
    CLinkage::Type) const;

  // helper
  bool typeMatch (CTypeInfo *, CTypeInfo *) const;
  CStructure *findParent () const;
  void setSpecifiers (CObjectInfo *, CSemDeclSpecs *) const;
  bool isRedefiningTypedef (CObjectInfo*, int obj_type) const;
  bool isRedefinedFunction (CFunctionInfo*) const;

  // create class DB objects
  CAttributeInfo *createAttribute (const char *, CStructure *, CTypeInfo *,
                                   bool = false);
  CTypedefInfo *createTypedef (const char *, CStructure *, CTypeInfo *);
  void createParameter (CFunctionInfo *, CTypeInfo *);
  CFunctionInfo *createFunction (const char *, CStructure *, CTypeInfo *);

  CTypeInfo *resolveExpr (CTree *expr, CTree *base) const;
};

inline CSemantic::CSemantic (CSyntax &s, CBuilder &b) : 
  _syntax (s), _builder (b),
  in_arg_decl (false),
  _in_arg_decl_seq (false),
  is_type_name (false),
  support_implicit_int (true)
 {}

inline CBuilder &CSemantic::builder () const 
 { return _builder; }

inline CSyntax &CSemantic::syntax () const 
 { return _syntax; }

inline CObjectInfo *CSemantic::lookup (const char *id, SemObjType type, 
  bool nested) const
 { return lookup (id, current_scope, type, nested); }
inline CObjectInfo *CSemantic::lookup (CT_SimpleName *id, SemObjType type, 
  bool nested) const
 { return lookup (id->Text (), current_scope, type, nested); }

inline bool CSemantic::in_arg_decl_seq () const 
 { return _in_arg_decl_seq; }
inline void CSemantic::enter_arg_decl_seq () 
 { _in_arg_decl_seq = true; _in_param_decl_clause.push (true); }
inline void CSemantic::leave_arg_decl_seq () 
 { _in_arg_decl_seq = false; _in_param_decl_clause.pop (); }

inline CProtection::Type CSemantic::protection () const
 { return CProtection::PROT_NONE; }

} // namespace Puma

#endif /* __CSemantic_h__ */
