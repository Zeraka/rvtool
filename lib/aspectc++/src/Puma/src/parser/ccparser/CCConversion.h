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

#ifndef __CCConversion__
#define __CCConversion__

namespace Puma {


class CFunctionInfo;
class CTypeInfo;
class CTree;

class CCConversion {
public:
  enum ConvId {
    IDENTITY,
    LVALUE_TO_RVALUE,
    ARRAY_TO_POINTER,
    FUNCTION_TO_POINTER,
    BOOLEAN,
    INTEGRAL,
    INTEGRAL_PROMOTION,
    FLOATING,
    FLOATING_PROMOTION,
    FLOATING_INTEGRAL,
    POINTER,
    POINTER_TO_MEMBER,
    DERIVED_TO_BASE,
    QUALIFICATION,
    USER_DEFINED,
    
    STANDARD_SEQ,
    USER_DEFINED_SEQ,
    ELLIPSIS_SEQ
  };
  
protected:
  ConvId _Id;
  CTypeInfo *_ToType;
  CTypeInfo *_FromType;
  CTree *_Expr;
  CTree *_Base;

protected:
  CCConversion (ConvId);
  CCConversion (ConvId, CTypeInfo *, CTypeInfo *, CTree * = 0, CTree * = 0);

public:
  virtual ~CCConversion ();
  
  ConvId Id () const;
  CTypeInfo *ToType () const;
  CTypeInfo *FromType () const;
  CTree *Expr () const;
  CTree *Base () const;
  CFunctionInfo *ConvFunction () const;
  
  virtual void Apply ();
  void Update (CTree *);

  bool isIdentityConv () const;
  bool isLvalueToRvalueConv () const;
  bool isArrayToPointerConv () const;
  bool isFunctionToPointerConv () const;
  bool isBooleanConv () const;
  bool isIntegralConv () const;
  bool isIntegralPromotion () const;
  bool isFloatingPointConv () const;
  bool isFloatingPointPromotion () const;
  bool isFloatingIntegralConv () const;
  bool isPointerConv () const;
  bool isPointerToMemberConv () const;
  bool isDerivedToBaseConv () const;
  bool isQualificationConv () const;
  bool isUserDefinedConv () const;

  bool isStandardConvSeq () const;
  bool isUserDefinedConvSeq () const;
  bool isEllipsisConvSeq () const;
};

inline CCConversion::CCConversion (ConvId id) :
  _Id (id),
  _ToType ((CTypeInfo*)0),
  _FromType ((CTypeInfo*)0),
  _Expr ((CTree*)0),
  _Base ((CTree*)0)
 {}

inline CCConversion::ConvId CCConversion::Id () const
 { return _Id; }

inline CTypeInfo *CCConversion::ToType () const
 { return _ToType; }
inline CTypeInfo *CCConversion::FromType () const
 { return _FromType; }
inline CTree *CCConversion::Expr () const
 { return _Expr; }
inline CTree *CCConversion::Base () const
 { return _Base; }

inline bool CCConversion::isIdentityConv () const
 { return _Id == IDENTITY; }  
inline bool CCConversion::isLvalueToRvalueConv () const
 { return _Id == LVALUE_TO_RVALUE; }  
inline bool CCConversion::isArrayToPointerConv () const
 { return _Id == ARRAY_TO_POINTER; }  
inline bool CCConversion::isFunctionToPointerConv () const
 { return _Id == FUNCTION_TO_POINTER; }  
inline bool CCConversion::isBooleanConv () const
 { return _Id == BOOLEAN; }  
inline bool CCConversion::isIntegralConv () const
 { return _Id == INTEGRAL; }  
inline bool CCConversion::isIntegralPromotion () const
 { return _Id == INTEGRAL_PROMOTION; }  
inline bool CCConversion::isFloatingPointConv () const
 { return _Id == FLOATING; }  
inline bool CCConversion::isFloatingPointPromotion () const
 { return _Id == FLOATING_PROMOTION; }  
inline bool CCConversion::isFloatingIntegralConv () const
 { return _Id == FLOATING_INTEGRAL; }  
inline bool CCConversion::isPointerConv () const
 { return _Id == POINTER; }  
inline bool CCConversion::isPointerToMemberConv () const
 { return _Id == POINTER_TO_MEMBER; }  
inline bool CCConversion::isDerivedToBaseConv () const
 { return _Id == DERIVED_TO_BASE; }  
inline bool CCConversion::isQualificationConv () const
 { return _Id == QUALIFICATION; }  
inline bool CCConversion::isUserDefinedConv () const
 { return _Id == USER_DEFINED; } 

inline bool CCConversion::isStandardConvSeq () const
 { return _Id == STANDARD_SEQ; } 
inline bool CCConversion::isUserDefinedConvSeq () const
 { return _Id == USER_DEFINED_SEQ; } 
inline bool CCConversion::isEllipsisConvSeq () const
 { return _Id == ELLIPSIS_SEQ; } 

// the conversions

class CCIdentityConv : public CCConversion {
public:
  CCIdentityConv (CTypeInfo *p, CTypeInfo *a, CTree *e = 0, CTree *b = 0) :
    CCConversion (IDENTITY, p, a, e, b) {}
    
  void Apply () { /* nothing to be applied */ }
};

class CCLvalueToRvalueConv : public CCConversion {
public:
  CCLvalueToRvalueConv (CTypeInfo *p, CTypeInfo *a, CTree *e = 0, CTree *b = 0) :
    CCConversion (LVALUE_TO_RVALUE, p, a, e, b) {}
};

class CCArrayToPointerConv : public CCConversion {
public:
  CCArrayToPointerConv (CTypeInfo *p, CTypeInfo *a, CTree *e = 0, CTree *b = 0) :
    CCConversion (ARRAY_TO_POINTER, p, a, e, b) {}
};

class CCFunctionToPointerConv : public CCConversion {
public:
  CCFunctionToPointerConv (CTypeInfo *p, CTypeInfo *a, CTree *e = 0, CTree *b = 0) :
    CCConversion (FUNCTION_TO_POINTER, p, a, e, b) {}
};

class CCBooleanConv : public CCConversion {
public:
  CCBooleanConv (CTypeInfo *p, CTypeInfo *a, CTree *e = 0, CTree *b = 0) :
    CCConversion (BOOLEAN, p, a, e, b) {}
};

class CCIntegralConv : public CCConversion {
public:
  CCIntegralConv (CTypeInfo *p, CTypeInfo *a, CTree *e = 0, CTree *b = 0) :
    CCConversion (INTEGRAL, p, a, e, b) {}
};

class CCIntegralPromotion : public CCConversion {
public:
  CCIntegralPromotion (CTypeInfo *p, CTypeInfo *a, CTree *e = 0, CTree *b = 0) :
    CCConversion (INTEGRAL_PROMOTION, p, a, e, b) {}
};

class CCFloatingPointConv : public CCConversion {
public:
  CCFloatingPointConv (CTypeInfo *p, CTypeInfo *a, CTree *e = 0, CTree *b = 0) :
    CCConversion (FLOATING, p, a, e, b) {}
};

class CCFloatingPointPromotion : public CCConversion {
public:
  CCFloatingPointPromotion (CTypeInfo *p, CTypeInfo *a, CTree *e = 0, CTree *b = 0) :
    CCConversion (FLOATING_PROMOTION, p, a, e, b) {}
};

class CCFloatingIntegralConv : public CCConversion {
public:
  CCFloatingIntegralConv (CTypeInfo *p, CTypeInfo *a, CTree *e = 0, CTree *b = 0) :
    CCConversion (FLOATING_INTEGRAL, p, a, e, b) {}
};

class CCPointerConv : public CCConversion {
public:
  CCPointerConv (CTypeInfo *p, CTypeInfo *a, CTree *e = 0, CTree *b = 0) :
    CCConversion (POINTER, p, a, e, b) {}
};

class CCPointerToMemberConv : public CCConversion {
public:
  CCPointerToMemberConv (CTypeInfo *p, CTypeInfo *a, CTree *e = 0, CTree *b = 0) :
    CCConversion (POINTER_TO_MEMBER, p, a, e, b) {}
};

class CCDerivedToBaseConv : public CCConversion {
public:
  CCDerivedToBaseConv (CTypeInfo *p, CTypeInfo *a, CTree *e = 0, CTree *b = 0) :
    CCConversion (DERIVED_TO_BASE, p, a, e, b) {}
};

class CCQualificationConv : public CCConversion {
public:
  CCQualificationConv (CTypeInfo *p, CTypeInfo *a, CTree *e = 0, CTree *b = 0) :
    CCConversion (QUALIFICATION, p, a, e, b) {}
};

class CCUserDefinedConv : public CCConversion {
  CFunctionInfo *_ConvFunction;

public:
  CCUserDefinedConv (CTypeInfo *p, CTypeInfo *a, CFunctionInfo *cf, CTree *e = 0, CTree *b = 0) :
    CCConversion (USER_DEFINED, p, a, e, b), 
    _ConvFunction (cf) {}
    
  CFunctionInfo *ConvFunction () const;
  void Apply ();
};

inline CFunctionInfo *CCUserDefinedConv::ConvFunction () const
 { return _ConvFunction; }


} // namespace Puma

#endif /* __CCConversion__ */
