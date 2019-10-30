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

#ifndef __CTypeMemberPointer_h__
#define __CTypeMemberPointer_h__

/** \file
 *  Type of a member pointer. */
 
#ifndef __CTypeInfo_h__
#warning !!!      DO NOT INCLUDE THIS FILE      !!!  
#warning !!! INCLUDE FILE "CTypeInfo.h" INSTEAD !!!  
#endif

namespace Puma {


class CObjectInfo;
class CRecord;
class CTemplateParamInfo;


/** \class CTypeMemberPointer CTypeMemberPointer.inc Puma/CTypeInfo.h
 * Type of a member pointer.
 * Examples:
 * \code
 * struct X { int a; void f(int); };
 * int X::* aptr = &X::a;          // aptr has type 'member pointer to int'
 *                                 // type structure:
 *                                 // CTypeMemberPointer class=X
 *                                 //   CTypePrimitive int
 * void (X::*fptr)(int) = &X::f;   // fptr has type 'member pointer to function returning void with one argument int'
 *                                 // type structure:
 *                                 // CTypeMemberPointer class=X
 *                                 //   CTypeFunction args=int
 *                                 //     CTypePrimitive void
 * \endcode
 * \ingroup types */
class CTypeMemberPointer : public CTypePointer {
  CObjectInfo *_Class;

public:
  /** Constructor. Type has id CTypeInfo::TYPE_MEMBER_POINTER.
   *  \param base The base type (type of the member). 
   *  \param info The class containing the member. */
  CTypeMemberPointer (CTypeInfo *base, CObjectInfo *info);
  /** Destructor. */
  ~CTypeMemberPointer ();

  /** Get the class or union containing the member. */
  CRecord *Record () const;
  /** Get the template parameter information if the member's 
   *  class is a type template parameter. */
  CTemplateParamInfo *TemplateParam () const;
};

inline CTypeMemberPointer::CTypeMemberPointer (CTypeInfo *base, CObjectInfo *c) :
  CTypePointer (base, CTypeInfo::TYPE_MEMBER_POINTER),
  _Class (c)
 {}
inline CTypeMemberPointer::~CTypeMemberPointer ()
 {}


} // namespace Puma

#endif /* __CTypeMemberPointer_h__ */
