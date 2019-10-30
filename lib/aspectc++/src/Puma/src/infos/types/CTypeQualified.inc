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

#ifndef __CTypeQualified_h__
#define __CTypeQualified_h__

/** \file
 *  Type qualification. */

#ifndef __CTypeInfo_h__
#warning !!!      DO NOT INCLUDE THIS FILE      !!!  
#warning !!! INCLUDE FILE "CTypeInfo.h" INSTEAD !!!  
#endif

namespace Puma {


/** \class CTypeQualified CTypeQualified.inc Puma/CTypeInfo.h
 * Type qualification. There are three type qualifier:
 * \e const, \e volatile, and \e restrict.
 * Examples:
 * \code
 * const int i = 0;     // i has type 'const int'
 *                      // type structure:
 *                      // CTypeQualified const
 *                      //   CTypePrimitive int
 * char * const s = 0;  // s has type 'const pointer to char'
 *                      // type structure:
 *                      // CTypeQualified const
 *                      //   CTypePointer
 *                      //     CTypePrimitive char
 * \endcode
 * \ingroup types */
class CTypeQualified : public CTypeInfo {
  bool _QualConst;
  bool _QualVolatile;
  bool _QualRestrict;

protected:
  /** Constructor.
   *  \param base The base type.
   *  \param c Type has qualifier \e const.
   *  \param v Type has qualifier \e volatile.
   *  \param r Type has qualifier \e restrict.
   *  \param id The type identifier. */
  CTypeQualified (CTypeInfo *base, bool c, bool v, bool r, TypeId id);
  
public:
  /** Constructor. Type has id CTypeInfo::TYPE_QUALIFIED.
   *  \param base The base type.
   *  \param c Type has qualifier \e const.
   *  \param v Type has qualifier \e volatile.
   *  \param r Type has qualifier \e restrict. */
  CTypeQualified (CTypeInfo *base, bool c, bool v, bool r);
  /** Destructor. */
  ~CTypeQualified ();

  /** Check if the type has qualifier \e const. */
  bool isConst () const;
  /** Check if the type has qualifier \e volatile. */
  bool isVolatile () const;
  /** Check if the type has qualifier \e restrict. */
  bool isRestrict () const;

  /** Set whether the type has qualifier \e const. 
   *  \param v True if qualified. */
  void isConst (bool v);
  /** Set whether the type has qualifier \e volatile. 
   *  \param v True if qualified. */
  void isVolatile (bool v);
  /** Set whether the type has qualifier \e restrict. 
   *  \param v True if qualified. */
  void isRestrict (bool v);
};

inline CTypeQualified::CTypeQualified (CTypeInfo *base, bool c, bool v, bool r, 
 CTypeInfo::TypeId id) :
  CTypeInfo (base, id),
  _QualConst (c),
  _QualVolatile (v),
  _QualRestrict (r)
 {}
inline CTypeQualified::CTypeQualified (CTypeInfo *base, bool c, bool v, bool r) :
  CTypeInfo (base, CTypeInfo::TYPE_QUALIFIED),
  _QualConst (c),
  _QualVolatile (v),
  _QualRestrict (r)
 {}
inline CTypeQualified::~CTypeQualified ()
 {}
 
inline bool CTypeQualified::isConst () const
 { return _QualConst; }
inline bool CTypeQualified::isVolatile () const
 { return _QualVolatile; }
inline bool CTypeQualified::isRestrict () const
 { return _QualRestrict; }

inline void CTypeQualified::isConst (bool v)
 { _QualConst = v; }
inline void CTypeQualified::isVolatile (bool v)
 { _QualVolatile = v; }
inline void CTypeQualified::isRestrict (bool v)
 { _QualRestrict = v; }


} // namespace Puma

#endif /* __CTypeQualified_h__ */
