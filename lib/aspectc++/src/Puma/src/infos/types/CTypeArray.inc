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

#ifndef __CTypeArray_h__
#define __CTypeArray_h__

/** \file
 *  Type of an array. */
 
#ifndef __CTypeInfo_h__
#warning !!!      DO NOT INCLUDE THIS FILE      !!!  
#warning !!! INCLUDE FILE "CTypeInfo.h" INSTEAD !!!  
#endif

namespace Puma {


/** \class CTypeArray CTypeArray.inc Puma/CTypeInfo.h
 * Type of an array.
 * Examples:
 * \code
 * int i[10];      // i has type 'array of int'
 *                 // type structure:
 *                 // CTypeArray dim=10
 *                 //   CTypePrimitive int
 * char* sa[5];    // sa has type 'array of pointer to char'
 *                 // type structure:
 *                 // CTypeArray dim=5
 *                 //   CTypePointer
 *                 //     CTypePrimitive char
 * \endcode
 * \ingroup types */
class CTypeArray : public CTypeInfo {
  long int _Size;
  bool _isFixed;
  CTypeQualified *_Quals;
  CTypeTemplateParam *_DepDim;
  bool _hasDim;

protected:
  /** Constructor.
   *  \param base The base type of the array.
   *  \param quals The type qualifiers.
   *  \param id The type identifier. */
  CTypeArray (CTypeInfo *base, CTypeQualified *quals, TypeId id);
  
public:
  /** Constructor. Type has id CTypeInfo::TYPE_ARRAY.
   *  \param base The base type of the array.
   *  \param quals The type qualifiers.
   *  \param has_dimension The array was declared with a constant dimension. */
  CTypeArray (CTypeInfo *base, CTypeQualified *quals = 0, bool has_dimension = false);
  /** Destructor. */
  ~CTypeArray ();

  /** Set the dimension of the array.
   *  \param dim The dimension. */
  void Dimension (long int dim);
  /** Get the dimension of the array. */
  long int Dimension () const;
  /** Set whether the array was declared with a constant dimension. 
   *  \param constant Array has constant dimension. */
  void hasDimension (bool constant);
  /** Check if the array has a constant dimension. */
  bool hasDimension () const;
  
  /** Set whether the array has a fixed length. 
   *  \param fixed Has fixed length. */
  void isFixed (bool fixed);
  /** Check if the array has a fixed length. */
  bool isFixed () const;
  
  /** Get the array type qualifiers. */
  CTypeQualified *Qualifiers () const;
  
  /** Set the template parameter information if the dimension 
   *  of the array depends on this template parameter. 
   *  \param param The template parameter. */
  void DepDim (CTypeTemplateParam *param);
  /** Get the template parameter information if the dimension
   *  of the array depends on a template parameter.
   *  \return The template parameter or NULL. */
  CTypeTemplateParam *DepDim () const;
};

inline CTypeArray::CTypeArray (CTypeInfo *base, CTypeQualified *quals, bool has_dimension) :
  CTypeInfo (base, CTypeInfo::TYPE_ARRAY),
  _Size (0),
  _isFixed (false),
  _Quals (quals),
  _DepDim (0),
  _hasDim (has_dimension)
 {}
inline CTypeArray::CTypeArray (CTypeInfo *base, CTypeQualified *quals, CTypeInfo::TypeId id) :
  CTypeInfo (base, id),
  _Size (0),
  _isFixed (false),
  _Quals (quals),
  _DepDim (0),
  _hasDim (false)
 {}
inline CTypeArray::~CTypeArray ()
 {}

inline void CTypeArray::Dimension (long int d)
 { _Size = d; }
inline long int CTypeArray::Dimension () const
 { return _Size; }

inline void CTypeArray::isFixed (bool v)
 { _isFixed = v; }
inline bool CTypeArray::isFixed () const
 { return _isFixed; }

inline void CTypeArray::hasDimension (bool v)
 { _hasDim = v; }
inline bool CTypeArray::hasDimension () const
 { return _hasDim || _Size != 0; }

inline CTypeQualified *CTypeArray::Qualifiers () const
 { return _Quals; }
 
inline void CTypeArray::DepDim (CTypeTemplateParam *tp) 
 { _DepDim = tp; }
inline CTypeTemplateParam *CTypeArray::DepDim () const
 { return _DepDim; }


} // namespace Puma

#endif /* __CTypeArray_h__ */
