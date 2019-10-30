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

#ifndef __CTypeList_h__
#define __CTypeList_h__

/** \file
 *  Type list. */
 
#ifndef __CTypeFunction_h__
#warning !!!      DO NOT INCLUDE THIS FILE      !!!
#warning !!! INCLUDE FILE "CTypeInfo.h" INSTEAD !!!
#endif

#include "Puma/Array.h"

namespace Puma {


class CTypeInfo;
class CT_ArgDeclList;


/** \class CTypeList CTypeList.inc Puma/CTypeList.h
 * %List of types. Used for instance for the list of function
 * parameter types.
 * \ingroup types */
class CTypeList {
  Array<CTypeInfo*> _List;
  CT_ArgDeclList *_ArgumentList;

public:
  /** Constructor. 
   *  \param len The initial length of the list. */
  CTypeList (int len = 1);
  /** Destructor. */
  ~CTypeList ();

  /** Get the number of types in the list. */
  unsigned Entries () const;
  /** Get the n-th type.
   *  \param n The index of the type. */
  CTypeInfo *Entry (unsigned n) const;
  /** Replace the n-th type in the list by the given type.
   *  \param n The index of the type to replace.
   *  \param type The new type. */
  void ReplaceEntry (unsigned n, CTypeInfo *type);
  
  /** Get the argument declaration list for K&R functions. */
  CT_ArgDeclList *ArgumentList () const;
  /** Set the argument declaration list for K&R functions. 
   *  \param args The argument declaration list. */
  void ArgumentList (CT_ArgDeclList *args);
  
  /** Add a type to the list.
   *  \param type The type to add. */
  void AddEntry (CTypeInfo *type);
  
  /** Check if one of the types in the list depends on 
   *  a template parameter. */
  bool isDependent () const;
};

inline CTypeList::CTypeList (int len) :
  _List (len, 2),
  _ArgumentList ((CT_ArgDeclList*)0)
 {}
inline CTypeList::~CTypeList ()
 {}

inline CT_ArgDeclList *CTypeList::ArgumentList () const
 { return _ArgumentList; }
inline void CTypeList::ArgumentList (CT_ArgDeclList *al)
 { _ArgumentList = al; }

inline unsigned CTypeList::Entries () const
 { return _List.length (); }
inline CTypeInfo *CTypeList::Entry (unsigned i) const
 { return _List.lookup (i); }
 
inline void CTypeList::AddEntry (CTypeInfo *info) 
 { if (info) _List.append (info); }
inline void CTypeList::ReplaceEntry (unsigned i, CTypeInfo *info)
 { if (info && (i < Entries ())) _List[i] = info; }


} // namespace Puma

#endif /* __CTypeList_h__ */
