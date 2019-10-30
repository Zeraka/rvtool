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

#ifndef __CTypeClass_h__
#define __CTypeClass_h__

/** \file
 *  Type of a class. */
 
#ifndef __CTypeInfo_h__
#warning !!!      DO NOT INCLUDE THIS FILE      !!!  
#warning !!! INCLUDE FILE "CTypeInfo.h" INSTEAD !!!  
#endif

namespace Puma {


class CClassInfo;
class CRecord;
class CStructure;


/** \class CTypeClass CTypeClass.inc Puma/CTypeInfo.h
 * Type of a class.
 * Examples:
 * \code
 * class X x;    // x has type 'class X'
 * struct Y y;   // y has type 'class Y'
 * \endcode
 * \ingroup types */
class CTypeClass : public CTypeRecord {
public:
  /** Constructor. Type has id CTypeInfo::TYPE_CLASS.
   *  \param c The semantic information about the class. */
  CTypeClass (CRecord *c);
  /** Destructor. */
  ~CTypeClass ();

  /** Get the semantic information about the class. */
  CClassInfo *ClassInfo () const;

  /** If this is the type of a pseudo class instance, 
   *  create a real instance of the corresponding template. 
   *  \param scope The scope in which to instantiate the template. 
   *  \return true if instantiation succeeded. */
  bool instantiate (CStructure *scope);
};

inline CTypeClass::CTypeClass (CRecord *c) :
  CTypeRecord (CTypeInfo::TYPE_CLASS, c)
 {}
inline CTypeClass::~CTypeClass ()
 {}
 

} // namespace Puma

#endif /* __CTypeClass_h__ */
