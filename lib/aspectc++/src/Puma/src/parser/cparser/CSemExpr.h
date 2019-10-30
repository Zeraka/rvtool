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

#ifndef __CSemExpr_h__
#define __CSemExpr_h__

#include "Puma/CTypeInfo.h"
#include "Puma/ErrorSink.h"
#include "Puma/Limits.h"
#include "Puma/Config.h"
#include "Puma/Stack.h"
#include "Puma/CTree.h"

namespace Puma {

class CStructure;
class CObjectInfo;


class CSemExpr {
protected:
  ErrorSink &err;            // error stream
  CStructure *current_scope;

protected: // expression evaluation
  int conv_to_ptr; // convert array/function type to pointer type

  enum SemObjType { 
    CLASS, UNION, ENUM, 
    TYPEDEF, FCT, 
    ATTRIB, ARG, 
    TAG, NON_TAG, 
    ANY 
  };

public: // public interface
  CSemExpr (ErrorSink &, CStructure *scope);

  CTypeInfo *resolveExpr (CTree *expr, CTree *base = (CTree*)0);
  void resolveInit (CObjectInfo *, CTree *, CTree *, bool = false);

public:
  CTypeInfo *resolve (CTree *, CTree *);
  CTypeInfo *resolve (CT_SimpleName *, CTree *);
  CTypeInfo *resolve (CT_String *, CTree *);
  CTypeInfo *resolve (CT_WideString *, CTree *);
  CTypeInfo *resolve (CT_CmpdLiteral *, CTree *);
  CTypeInfo *resolve (CT_Bool *, CTree *);
  CTypeInfo *resolve (CT_Character *, CTree *);
  CTypeInfo *resolve (CT_WideCharacter *, CTree *);
  CTypeInfo *resolve (CT_Integer *, CTree *);
  CTypeInfo *resolve (CT_Float *, CTree *);
  CTypeInfo *resolve (CT_BracedExpr *, CTree *);
  CTypeInfo *resolve (CT_BinaryExpr *, CTree *);
  CTypeInfo *resolve (CT_UnaryExpr *, CTree *);
  CTypeInfo *resolve (CT_AddrExpr *, CTree *);
  CTypeInfo *resolve (CT_DerefExpr *, CTree *);
  CTypeInfo *resolve (CT_PostfixExpr *, CTree *);
  CTypeInfo *resolve (CT_IndexExpr *, CTree *);
  CTypeInfo *resolve (CT_SizeofExpr *, CTree *);
  CTypeInfo *resolve (CT_AlignofExpr *, CTree *);
  CTypeInfo *resolve (CT_OffsetofExpr *, CTree *);
  CTypeInfo *resolve (CT_IfThenExpr *, CTree *);
  CTypeInfo *resolve (CT_CastExpr *, CTree *);
  CTypeInfo *resolve (CT_CallExpr *, CTree *);
  CTypeInfo *resolve (CT_MembPtrExpr *, CTree *);

  // binary expressions
  CTypeInfo *binary_rel (CT_BinaryExpr *);
  CTypeInfo *binary_ass (CT_BinaryExpr *);
  CTypeInfo *binary_log (CT_BinaryExpr *);
  CTypeInfo *binary_and (CT_BinaryExpr *);
  CTypeInfo *binary_equ (CT_BinaryExpr *);
  CTypeInfo *binary_mul (CT_BinaryExpr *);
  CTypeInfo *binary_add (CT_BinaryExpr *);
  CTypeInfo *binary_shift (CT_BinaryExpr *);
  CTypeInfo *binary_comma (CT_BinaryExpr *);
  CTypeInfo *apply_binary_op (CTree *, int);

  // constant expression
  static bool isConstExpr (CTree *, bool = false);
  static bool isConstIntExpr (CTree *);
  static bool isConstantMembPtr (CTree *node);

  // helper
  static bool isLvalue (CTree *);
  CTypeInfo* applyImplicitConv (CTree* expr, CTree* base, CTypeInfo* type);
  void overflow (CTree *, int, bool &) const;
  int scan_character (CTree *, LONG_LONG &) const;
  LONG_LONG next_character (const char *&, bool &) const;
  static void passOnConstant (CTree *node, CTree *base, CTypeInfo *t1);
  static CTypeInfo *cast_to (CTypeInfo *, CTree *, CTree *, bool = true);
  static CTypeInfo *promote (CTree *, CTree *);
  static CObjectInfo *findObject (CTree *, bool const_expr = false);
  CTypeInfo *cond_expr_type (CT_IfThenExpr *node,
      CTypeInfo *t1, CTree *op1, CTypeInfo *t2, CTree *op2,
      CTypeInfo *t3, CTree *op3);

protected: // initializer evaluation
  struct SubObj { 
    CObjectInfo *info;
    CTypeInfo *type; 
    int size; 
    
    SubObj () 
     { info = 0; type = 0; size = 0; }
    SubObj (CObjectInfo *i, CTypeInfo *t, int s)
     { info = i; type = t; size = s; }
    SubObj &operator =(const SubObj& s) 
     { info = s.info; type = s.type; size = s.size; return *this; }
  };

  void resolveInit (CObjectInfo *, CTree *, CTree *, bool, Stack<SubObj> &);
  void singleExprInit (CTree *, CTree *, CTypeInfo *, bool);
  void findFirst (CTree *, CTypeInfo *&, CObjectInfo *&, CObjectInfo *&, 
                  CObjectInfo *, Stack<SubObj> &, bool) const;
  void findNext (CTree *, CTypeInfo *&, CObjectInfo *&, CObjectInfo *&, 
                 CObjectInfo *, Stack<SubObj> &, bool) const;
  
  // helper
  void pushSubObj (CTypeInfo *&, CObjectInfo *&, Stack<SubObj> &) const;
  void popSubObj (CTypeInfo *&, CObjectInfo *&, CObjectInfo *&, 
                  Stack<SubObj> &) const;
  CObjectInfo *nextMember (CStructure *, CObjectInfo *) const;
  
protected: // name lookup
  CObjectInfo *lookup (CT_SimpleName *, SemObjType, bool = false) const;
  CObjectInfo *lookup (const char *, CStructure *, SemObjType, bool, 
                       unsigned long int) const;

  // helper
  CObjectInfo *Type (const char *, CStructure *, SemObjType, unsigned long int) const;
  CObjectInfo *Attribute (const char *, CStructure *, unsigned long int) const;
  CObjectInfo *Function (const char *, CStructure *, unsigned long int) const;
  bool known_here (CObjectInfo *, unsigned long int) const;

public: // common helper, also for CSemVisitor
  static bool isPtrToObj (CTypeInfo *, unsigned long = 0);
  static bool isModifiable (CTypeInfo *, unsigned long = 0);
  static bool compatible (CTypeInfo *, CTypeInfo *, CConstant* value = 0);
  static bool compatibleBase (CTypeInfo *, CTypeInfo *, CConstant* value = 0);
};

inline CSemExpr::CSemExpr (ErrorSink &e, CStructure *s) : 
  err (e), current_scope (s), conv_to_ptr (1) {
}
inline bool CSemExpr::isPtrToObj (CTypeInfo *type, unsigned long pos) {
  CTypeInfo* base = type->VirtualType ()->BaseType ();
  return type->isPointer () && base && (base->VirtualType ()->is_void () || base->isComplete (pos));
}
inline bool CSemExpr::isModifiable (CTypeInfo *type, unsigned long pos) {
  return ! (! type->isComplete (pos) || type->isArray () || type->isConst () ||
             (type->isRecord () && type->VirtualType ()->TypeRecord ()->
              hasConstMember ()));
}
inline bool CSemExpr::compatibleBase (CTypeInfo *t1, CTypeInfo *t2, CConstant* value) {
  return compatible (t1->VirtualType ()->BaseType (), t2->VirtualType ()->BaseType (), value);
}


} // namespace Puma

#endif /* __CSemExpr_h__ */
