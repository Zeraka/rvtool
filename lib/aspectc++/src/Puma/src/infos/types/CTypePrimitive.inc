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

#ifndef __CTypePrimitive_h__
#define __CTypePrimitive_h__

/** \file
 *  Primitive types. */

#ifndef __CTypeInfo_h__
#warning !!!      DO NOT INCLUDE THIS FILE      !!!  
#warning !!! INCLUDE FILE "CTypeInfo.h" INSTEAD !!!  
#endif

namespace Puma {


/** \class CTypePrimitive CTypePrimitive.inc Puma/CTypeInfo.h
 * Primitive type. Fundamental arithmetic types and
 * type void are called primitive types.
 * \ingroup types */
class CTypePrimitive : public CTypeInfo {
  const char *_Text;
  long int _Size;

public:
  /** Constructor.
   *  \param name The type name.
   *  \param id The type identifier.
   *  \param size The size of the type in bits. */
  CTypePrimitive (const char *name, TypeId id, long int size);
  /** Destructor. */
  ~CTypePrimitive ();

  /** Get the type name. */
  const char *TypeName () const;

  /** Get the size of the primitive type in bits. */
  long int Size() const;
  /** Set the size of the primitive type in bits.
   *  @param size The size of the type in bits. */
  void Size(long int size);

  /** Count the number of bits required for the given number.
   *  @param number The number. */
  template<typename T>
  static long int SizeOfNumber(T number);
};

inline CTypePrimitive::CTypePrimitive (const char *t, CTypeInfo::TypeId id, long int size) :
  CTypeInfo (&CTYPE_EMPTY, id),
  _Text (t),
  _Size (size)
 {}
inline CTypePrimitive::~CTypePrimitive ()
 {}

inline const char *CTypePrimitive::TypeName () const
 { return _Text; }

inline long int CTypePrimitive::Size () const
 { return _Size; }

inline void CTypePrimitive::Size (long int size)
 { _Size = size; }

template<typename T>
inline long int CTypePrimitive::SizeOfNumber(T number)
 { if (number == 0) return 1;
   if (number < 0) number -= number;
   long int size;
   for (size = 0; number != 0; number >>= 1, size++);
   return size;
 }

// Basic types //////////////////////////////////////////////////

/** bool */
extern CTypePrimitive CTYPE_BOOL;
/** _Bool */
extern CTypePrimitive CTYPE_C_BOOL;
/** char */
extern CTypePrimitive CTYPE_CHAR;
/** signed char */
extern CTypePrimitive CTYPE_SIGNED_CHAR;
/** unsigned char */
extern CTypePrimitive CTYPE_UNSIGNED_CHAR;
/** wchar_t */
extern CTypePrimitive CTYPE_WCHAR_T;
/** short */
extern CTypePrimitive CTYPE_SHORT;
/** unsigned short */
extern CTypePrimitive CTYPE_UNSIGNED_SHORT;
/** int */
extern CTypePrimitive CTYPE_INT;
/** unsigned int */
extern CTypePrimitive CTYPE_UNSIGNED_INT;
/** long */
extern CTypePrimitive CTYPE_LONG;
/** unsigned long */
extern CTypePrimitive CTYPE_UNSIGNED_LONG;
/** long long */
extern CTypePrimitive CTYPE_LONG_LONG;
/** unsigned long long */
extern CTypePrimitive CTYPE_UNSIGNED_LONG_LONG;
/** __int128 */
extern CTypePrimitive CTYPE_INT128;
/** unsigned __int128 */
extern CTypePrimitive CTYPE_UNSIGNED_INT128;
/** float */
extern CTypePrimitive CTYPE_FLOAT;
/** double */
extern CTypePrimitive CTYPE_DOUBLE;
/** long double */
extern CTypePrimitive CTYPE_LONG_DOUBLE;
/** void */
extern CTypePrimitive CTYPE_VOID;
/** unknown_t */
extern CTypePrimitive CTYPE_UNKNOWN_T;
/** Undefined type. */
extern CTypePrimitive CTYPE_UNDEFINED;
/** Any type. */
extern CTypePrimitive CTYPE_ELLIPSIS;


} // namespace Puma

#endif /* __CTypePrimitive_h__ */
