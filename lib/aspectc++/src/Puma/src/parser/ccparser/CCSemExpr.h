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

#ifndef __CCSemExpr_h__
#define __CCSemExpr_h__

#include "Puma/ErrorSink.h"
#include "Puma/Limits.h"
#include "Puma/Config.h"
#include "Puma/CTree.h"

namespace Puma {

class CStructure;
class CObjectInfo;
class CClassInfo;
class CTypeInfo;
class CCNameLookup;
class CCOverloading;
class CRecord;


class CCSemExpr {
protected:
  ErrorSink &err;            // error stream
  CStructure *current_scope;
  CT_CallExpr *fct_call;     // when evaluating first operand of function call
  bool dependent_args;      
  bool dependent;            // expression dependent on template parameter

public: // public interface
  CCSemExpr (ErrorSink &, CStructure *scope);

  CTypeInfo *resolveExpr (CTree *expr, CTree *base = (CTree*)0);
  CTypeInfo *resolveInit (CTree *node, CTree *base);

  void configure (Config &) {}
  
public: // basic expression operations
  static bool isLvalue (CTree *);
  static bool isRvalue (CTree *);
  static bool isModifiable (CTypeInfo *, unsigned long = 0);
  static bool isAddrExpr (CTree *);

  // constant expressions
  static bool isConstIntExpr (CTree *);
  // not yet implemented! always returns true!
  static bool isConstExpr (CTree *); 
  // constant integral expressions
  static bool isConstIntegralExpr (CTree *);

  // type conversions
  static CTypeInfo *usualArithmeticConv (CTree *, CTree *);
  static CTypeInfo *convLvalueToRvalue (CTree *, CTree * = (CTree*)0);
  static CTypeInfo *castToType (CTypeInfo *, CTree *, bool = true);
  static CTypeInfo *intPromotion (CTree *);

  static CObjectInfo *findObject (CTree *);
  static bool sameOrBaseClass (CRecord *, CRecord *);
  static bool baseClass (CClassInfo *, CClassInfo *);

  // return true if evaluated expression
  // depend on template parameter
  bool isDependent () const;
  // (for a given expression)
  static bool isDependent (CTree *);

  // expression evaluation
  CTypeInfo *resolve (CTree *, CTree *);
  CTypeInfo *resolve (CT_Expression *, CTree *);
  CTypeInfo *resolve (CT_CallExpr *, CTree *);
  CTypeInfo *resolve (CT_MembPtrExpr *, CTree *);
  CTypeInfo *resolve (CT_SimpleName *, CTree *);
  CTypeInfo *resolve (CT_BinaryExpr *, CTree *);
  CTypeInfo *resolve (CT_UnaryExpr *, CTree *);
  CTypeInfo *resolve (CT_PostfixExpr *, CTree *);
  CTypeInfo *resolve (CT_ConstructExpr *, CTree *);
  CTypeInfo *resolve (CT_CmpdLiteral *, CTree *);
  CTypeInfo *resolve (CT_NewExpr *, CTree *);
  CTypeInfo *resolve (CT_DeleteExpr *, CTree *);
  CTypeInfo *resolve (CT_IfThenExpr *, CTree *);
  CTypeInfo *resolve (CT_DerefExpr *, CTree *);
  CTypeInfo *resolve (CT_AddrExpr *, CTree *);
  CTypeInfo *resolve (CT_CastExpr *, CTree *);
  CTypeInfo *resolve (CT_DynamicCast *, CTree *);
  CTypeInfo *resolve (CT_StaticCast *, CTree *);
  CTypeInfo *resolve (CT_ReintCast *, CTree *);
  CTypeInfo *resolve (CT_ConstCast *, CTree *);
  CTypeInfo *resolve (CT_TypeidExpr *, CTree *);
  CTypeInfo *resolve (CT_ThrowExpr *, CTree *);
  CTypeInfo *resolve (CT_SizeofExpr *, CTree *);
  CTypeInfo *resolve (CT_AlignofExpr *, CTree *);
  CTypeInfo *resolve (CT_TypeTraitExpr *, CTree *);
  CTypeInfo *resolve (CT_OffsetofExpr *, CTree *);
  CTypeInfo *resolve (CT_IndexExpr *, CTree *);
  CTypeInfo *resolve (CT_BracedExpr *, CTree *);
  CTypeInfo *resolve (CT_Bool *, CTree *);
  CTypeInfo *resolve (CT_Character *, CTree *);
  CTypeInfo *resolve (CT_WideCharacter *, CTree *);
  CTypeInfo *resolve (CT_String *, CTree *);
  CTypeInfo *resolve (CT_WideString *, CTree *);
  CTypeInfo *resolve (CT_Integer *, CTree *);
  CTypeInfo *resolve (CT_Float *, CTree *);

protected:
  // binary expressions
  CTypeInfo *binary_ass (CT_BinaryExpr *, CTree *);
  CTypeInfo *binary_comma (CT_BinaryExpr *, CTree *);
  CTypeInfo *binary_log (CT_BinaryExpr *, CTree *);
  CTypeInfo *binary_bit (CT_BinaryExpr *, CTree *);
  CTypeInfo *binary_equ (CT_BinaryExpr *, CTree *);
  CTypeInfo *binary_rel (CT_BinaryExpr *, CTree *);
  CTypeInfo *binary_shift (CT_BinaryExpr *, CTree *);
  CTypeInfo *binary_add (CT_BinaryExpr *, CTree *);
  CTypeInfo *binary_mul (CT_BinaryExpr *, CTree *);
  CTypeInfo *binary_ptm (CT_BinaryExpr *, CTree *);
  void apply_binary_op (CTree *, int);

  // operator overloading
  CTypeInfo *overloadedOperator (CTree *, CTree *, const char *, int, 
                                 CTree *, CTree * = 0);

  // literal evaluation
  void overflow (CTree *, int, bool &) const;
  int scanCharacter (CTree *, LONG_LONG &) const;
  LONG_LONG nextCharacter (const char *&, bool &) const;

  // helper
  bool depBaseClass () const;
  CTree *baseTree (CTree *) const;
  bool isDependent (CTree *, bool);
  bool resetDependent ();
  bool isClassOrEnum (CTypeInfo *) const;
  bool sameUnqualBaseType (CTypeInfo *, CTypeInfo *) const;
  bool checkDeclsFound (CTree *, CCNameLookup &) const;
  void passOnConstant (CTree *, CTypeInfo *) const;
  bool validOvlFctSet (CTree *, const char *, const char *, CCOverloading &);
  void builtinOpExpr (CTree *, CTree *, const char *, int, CTypeInfo *, CTypeInfo *, CTypeInfo * = 0) const;
  bool lookupBuiltin (CT_SimpleName* sn, CCNameLookup& nl);
};

inline CCSemExpr::CCSemExpr (ErrorSink &e, CStructure *s) : 
  err (e), 
  current_scope (s), 
  fct_call (0), 
  dependent_args (false),
  dependent (false)
 {}

inline bool CCSemExpr::isDependent () const
 { return dependent; }


} // namespace Puma

#endif /* __CCSemExpr_h__ */
