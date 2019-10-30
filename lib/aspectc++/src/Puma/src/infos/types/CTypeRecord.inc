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

#ifndef __CTypeRecord_h__
#define __CTypeRecord_h__

/** \file
 *  Type of a class or union. */
 
#ifndef __CTypeInfo_h__
#warning !!!      DO NOT INCLUDE THIS FILE      !!!  
#warning !!! INCLUDE FILE "CTypeInfo.h" INSTEAD !!!  
#endif

namespace Puma {


class CRecord;

/** \class CTypeRecord CTypeRecord.inc Puma/CTypeInfo.h
 * Type of a class or union.
 * \ingroup types */
class CTypeRecord : public CTypeInfo {
  bool _hasConstMember;

  long int _Size;
  long int _Align;

  CRecord *_Record;

protected:
  /** Constructor.
   *  \param id The type identifier.
   *  \param cu The class or union. */
  CTypeRecord (TypeId id, CRecord *cu);

public:
  /** Destructor. */
  ~CTypeRecord ();

  /** Get the size of the class or union in bits. */
  long int Size ();
  /** Get the alignment of the type. */
  long int Align ();

  /** Set whether the class or union has a 
   *  const member and thus is not modifiable.
   *  \param v True if it has a const member. */
  void hasConstMember (bool v);
  /** Check if the class or union type has a 
   *  const member and thus is not modifiable. */
  bool hasConstMember () const;

  /** Check if the class or union is complete.
   *  \param pos Optional source code position. */
  bool isComplete (unsigned long pos = 0) const;

  /** Get the class or union. */
  CRecord *Record () const;
  /** Set the class or union. */
  void Record (CRecord *);
};

inline CTypeRecord::CTypeRecord (CTypeInfo::TypeId id, CRecord *r) :
  CTypeInfo (&CTYPE_EMPTY, id),
  _hasConstMember (false),
  _Size (0),
  _Align (0),
  _Record (r)
 {}
inline CTypeRecord::~CTypeRecord ()
 {}

inline CRecord *CTypeRecord::Record () const 
 { return _Record; }
inline void CTypeRecord::Record (CRecord *r)
 { _Record = r; }

inline long int CTypeRecord::Align () 
 { return (Size (), _Align); }

inline bool CTypeRecord::hasConstMember () const
 { return _hasConstMember; }
inline void CTypeRecord::hasConstMember (bool v)
 { _hasConstMember = v; }


} // namespace Puma

#endif /* __CTypeRecord_h__ */
