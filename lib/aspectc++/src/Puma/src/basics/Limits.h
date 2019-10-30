// This file is part of PUMA.
// Copyright (C) 1999-2015  The PUMA developer team.
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

#ifndef PUMA_Limits_H
#define PUMA_Limits_H

/** \file
 * Type size limits.
 * \ingroup basics */

#include <limits.h>

namespace Puma {

// type `long long' support

#if defined(__GNUC__) || defined(_MSC_VER)
#if defined(__SIZEOF_INT128__)
/** Type `long long`. */
typedef __int128 LONG_LONG;
/** Type `unsigned long long`. */
typedef unsigned __int128 U_LONG_LONG;
#else /* __SIZEOF_INT128__ */
/** Type `long long`. */
typedef long long LONG_LONG;
/** Type `unsigned long long`. */
typedef unsigned long long U_LONG_LONG;
#endif /* __SIZEOF_INT128__ */
#else /* __GNUC__ */
/** Type `long long`. */
typedef long LONG_LONG;
/** Type `unsigned long long`. */
typedef unsigned long U_LONG_LONG;
#endif /* __GNUC__ */

#ifndef LLONG_MAX
#  if defined(__GNUC__) || defined(_MSC_VER)
#    define LLONG_MAX        9223372036854775807LL
#    define LLONG_MIN        (-LLONG_MAX - 1LL)
#    define ULLONG_MAX       (LLONG_MAX * 2ULL + 1ULL)
#  else /* __GNUC__ */
#    define LLONG_MAX        LONG_MAX
#    define LLONG_MIN        LONG_MIN
#    define ULLONG_MAX       ULONG_MAX
#  endif /* __GNUC__ */
#endif /* LLONG_MAX */

// type size (in bits)

#define BOOL_SIZE             8
#define CHAR_SIZE             8
#define SCHAR_SIZE            8
#define UCHAR_SIZE            8
#define SHORT_SIZE            16
#define USHORT_SIZE           16
#define WCHAR_T_SIZE          32
#define INT_SIZE              32
#define UINT_SIZE             32
#define LONG_SIZE             32
#define ULONG_SIZE            32
#define LLONG_SIZE            64
#define ULLONG_SIZE           64
#define INT128_SIZE           128
#define UINT128_SIZE          128

#define FLOAT_SIZE            32
#define DOUBLE_SIZE           64
#define LDOUBLE_SIZE          96

#define PTR_SIZE              32
#define MEMB_PTR_SIZE         64

#define ENUM_SIZE             32

}
 // namespace Puma

#if (defined(__GNUC__) || defined(_MSC_VER)) && defined(__SIZEOF_INT128__)

#include <ostream>

namespace Puma {

/** Output stream operator for type `long long` based on `__int128`.
 * \param os The output stream.
 * \param value The value to output.
 * \return A reference to the output stream. */
inline std::ostream &operator << (std::ostream &os, LONG_LONG value) {
  os << (long long) value;
  return os;
}
/** Output stream operator for type `unsigned long long` based on `__int128`.
 * \param os The output stream.
 * \param value The value to output.
 * \return A reference to the output stream. */
inline std::ostream &operator << (std::ostream &os, U_LONG_LONG value) {
  os << (unsigned long long) value;
  return os;
}

}

#endif /* __SIZEOF_INT128__ */

#endif /* PUMA_Limits_H */
