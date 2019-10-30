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

#ifndef __CTypeFunction_h__
#define __CTypeFunction_h__

/** \file
 *  Type of a function. */
 
#ifndef __CTypeInfo_h__
#warning !!!      DO NOT INCLUDE THIS FILE      !!!  
#warning !!! INCLUDE FILE "CTypeInfo.h" INSTEAD !!!  
#endif

#include "Puma/CTypeList.inc"

namespace Puma {


class CFunctionInfo;
class CRecord;


/** \class CTypeFunction CTypeFunction.inc Puma/CTypeInfo.h
 * Type of a function.
 * Example:
 * \code
 * void foo(int);  // foo has type 'function returning void with one argument int'
 *                 // type structure:
 *                 // CTypeFunction args=int
 *                 //   CTypePrimitive void
 * \endcode
 * \ingroup types */
class CTypeFunction : public CTypeQualified {
  CTypeList *_ArgTypes;
  CFunctionInfo *_Function;
  bool _hasPrototype;

public:
  /** Constructor. Type has id CTypeInfo::TYPE_FUNCTION.
   *  \param base The base type (return type). 
   *  \param args The argument type list. 
   *  \param prototype Function has prototype. */
  CTypeFunction (CTypeInfo *base, CTypeList *args, bool prototype);
  /** Destructor. */
  ~CTypeFunction ();

  /** Get the argument type list. */
  CTypeList *ArgTypes () const;
  /** Get the return type. */
  CTypeInfo *ReturnType () const;  

  /** Get the semantic information about the function. */
  CFunctionInfo *FunctionInfo () const;
  /** Get the semantic information about the class if
   *  the function is a class method. */
  CRecord *Record () const;

  /** Set the semantic information about the function. */
  void FunctionInfo (CFunctionInfo *);
  
  /** Check if the function is declared \e static. */
  bool isStatic () const;
  /** Check if the function has a prototype. */
  bool hasPrototype () const;
};

inline CTypeFunction::CTypeFunction (CTypeInfo *base, CTypeList *l, 
  bool prototype) :
  CTypeQualified (base, false, false, false, CTypeInfo::TYPE_FUNCTION),
  _ArgTypes (l),
  _Function (0),
  _hasPrototype (prototype)
 {}
inline CTypeFunction::~CTypeFunction ()
 {}
 
inline bool CTypeFunction::hasPrototype () const
 { return _hasPrototype; }
inline CTypeList *CTypeFunction::ArgTypes () const 
 { return _ArgTypes; }
inline CTypeInfo *CTypeFunction::ReturnType () const 
 { return BaseType (); }
inline CFunctionInfo *CTypeFunction::FunctionInfo () const 
 { return _Function; }
inline void CTypeFunction::FunctionInfo (CFunctionInfo *info)
 { _Function = info; }


} // namespace Puma

#endif /* __CTypeFunction_h__ */
