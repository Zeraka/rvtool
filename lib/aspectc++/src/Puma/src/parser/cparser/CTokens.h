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

#ifndef __C_TOKENS__
#define __C_TOKENS__

/** \file
 *  C/C++ token types. */

namespace Puma {


/** Types of tokens for language C and C++. These types
 *  can be compared to Puma::Token::type(). */
enum CTokens {
  //////// core tokens, i.e. numbers, operator symbols, brackets, etc.

  /** @ */
  TOK_AT = 100,
  /** 0 */
  TOK_ZERO_VAL,
  /** Integer constant. */
  TOK_INT_VAL,
  /** Floating point constant. */
  TOK_FLT_VAL,
  /** String constant. */
  TOK_STRING_VAL,
  /** Character constant. */
  TOK_CHAR_VAL,
  /** , */
  TOK_COMMA,
  /** = */
  TOK_ASSIGN,
  /** ? */
  TOK_QUESTION,
  /** | */
  TOK_OR,
  /** ^ */
  TOK_ROOF,
  /** & */
  TOK_AND,
  /** + */
  TOK_PLUS,
  /** %- */
  TOK_MINUS,
  /** * */
  TOK_MUL,
  /** / */
  TOK_DIV,
  /** % */
  TOK_MODULO,
  /** < */
  TOK_LESS,
  /** > */
  TOK_GREATER,
  /** ( */
  TOK_OPEN_ROUND,
  /** ) */
  TOK_CLOSE_ROUND,
  /** [ */
  TOK_OPEN_SQUARE,
  /** ] */
  TOK_CLOSE_SQUARE,
  /** { */
  TOK_OPEN_CURLY,
  /** } */
  TOK_CLOSE_CURLY,
  /** ; */
  TOK_SEMI_COLON,
  /** : */
  TOK_COLON,
  /** ! */
  TOK_NOT,
  /** ~ */
  TOK_TILDE,
  /** "." */
  TOK_DOT,
  /** *= */
  TOK_MUL_EQ,
  /** /= */
  TOK_DIV_EQ,
  /** %= */
  TOK_MOD_EQ,
  /** += */
  TOK_ADD_EQ,
  /** -= */
  TOK_SUB_EQ,
  /** <<= */
  TOK_LSH_EQ,
  /** >>= */
  TOK_RSH_EQ,
  /** &= */
  TOK_AND_EQ,
  /** ^= */
  TOK_XOR_EQ,
  /** |= */
  TOK_IOR_EQ,
  /** || */
  TOK_OR_OR,
  /** && */
  TOK_AND_AND,
  /** == */
  TOK_EQL,
  /** != */
  TOK_NEQ,
  /** <= */
  TOK_LEQ,
  /** >= */
  TOK_GEQ,
  /** << */
  TOK_LSH,
  /** >> */
  TOK_RSH,
  /** ".*" */
  TOK_DOT_STAR,
  /** ->* */
  TOK_PTS_STAR,
  /** ++ */
  TOK_INCR,
  /** -- */
  TOK_DECR,
  /** -> */
  TOK_PTS,
  /** :: */
  TOK_COLON_COLON,
  /** "..." */
  TOK_ELLIPSIS,

  /** Core token group separator for the scanner! */
  TOK_FIRST_CORE = TOK_AT,
  TOK_LAST_CORE = TOK_ELLIPSIS,

  //////// C keywords

  /** asm */
  TOK_ASM,
  /** auto */
  TOK_AUTO,
  /** break */
  TOK_BREAK,
  /** case */
  TOK_CASE,
  /** char */
  TOK_CHAR,
  /** const */
  TOK_CONST,
  /** continue */
  TOK_CONTINUE,
  /** default */
  TOK_DEFAULT,
  /** do */
  TOK_DO,
  /** double */
  TOK_DOUBLE,
  /** else */
  TOK_ELSE,
  /** enum */
  TOK_ENUM,
  /** extern */
  TOK_EXTERN,
  /** float */
  TOK_FLOAT,
  /** for */
  TOK_FOR,
  /** goto */
  TOK_GOTO,
  /** if */
  TOK_IF,
  /** inline */
  TOK_INLINE,
  /** int */
  TOK_INT,
  /** long */
  TOK_LONG,
  /** register */
  TOK_REGISTER,
  /** restrict */
  TOK_RESTRICT,
  /** return */
  TOK_RETURN,
  /** short */
  TOK_SHORT,
  /** signed */
  TOK_SIGNED,
  /** sizeof */
  TOK_SIZEOF,
  /** static */
  TOK_STATIC,
  /** struct */
  TOK_STRUCT,
  /** switch */
  TOK_SWITCH,
  /** typedef */
  TOK_TYPEDEF,
  /** union */
  TOK_UNION,
  /** unsigned */
  TOK_UNSIGNED,
  /** void */
  TOK_VOID,
  /** volatile */
  TOK_VOLATILE,
  /** while */
  TOK_WHILE,

  /** C token group separator for the scanner! */
  TOK_FIRST_C = TOK_ASM,
  TOK_LAST_C = TOK_WHILE,

  //////// Common C extension keywords

  /** __asm */
  TOK_ASM_2,
  /** __inline */
  TOK_INLINE_2,

  /** Common C extension token group separator for the scanner! */
  TOK_FIRST_EXT_C = TOK_ASM_2,
  TOK_LAST_EXT_C = TOK_INLINE_2,

  //////// GNU C extension keywords

  /** __alignof */
  TOK_ALIGNOF,
  /** __alignof */
  TOK_ALIGNOF_2,
  /** __alignof__ */
  TOK_ALIGNOF_3,
  /** __asm__ */
  TOK_ASM_3,
  /** _Bool */
  TOK_C_BOOL,
  /** __const */
  TOK_CONST_2,
  /** __const__ */
  TOK_CONST_3,
  /** __inline__ */
  TOK_INLINE_3,
  /** __int128 */
  TOK_INT128,
  /** __builtin_offsetof */
  TOK_OFFSETOF,
  /** __restrict */
  TOK_RESTRICT_2,
  /** __restrict__ */
  TOK_RESTRICT_3,
  /** __signed */
  TOK_SIGNED_2,
  /** __signed__ */
  TOK_SIGNED_3,
  /** __volatile */
  TOK_VOLATILE_2,
  /** __volatile__ */
  TOK_VOLATILE_3,
  /** __thread */
  TOK_THREAD,
  /** typeof */
  TOK_TYPEOF,
  /** __typeof */
  TOK_TYPEOF_2,
  /** __typeof__ */
  TOK_TYPEOF_3,
  /** __decltype */
  TOK_TYPEOF_4,
  /** __attribute__ */
  TOK_ATTRIBUTE,
  /** __extension__ */
  TOK_EXTENSION,
  /** __psv__ */
  TOK_PSV,
  /** __pmp__ */
  TOK_PMP,
  /** __label__ */
  TOK_LABEL,
  /** __external__ */
  TOK_EXTERNAL,

  /** GNU C extension token group separator for the scanner! */
  TOK_FIRST_GNUC = TOK_ALIGNOF,
  TOK_LAST_GNUC = TOK_EXTERNAL,

  //////// Visual C/C++ extension keywords

  /** __int64 */
  TOK_INT64,
  /** _cdecl */
  TOK_CDECL,
  /** __cdecl */
  TOK_CDECL_2,
  /** _stdcall */
  TOK_STDCALL,
  /** __stdcall */
  TOK_STDCALL_2,
  /** _fastcall */
  TOK_FASTCALL,
  /** __fastcall */
  TOK_FASTCALL_2,
  /** __if_exists */
  TOK_IF_EXISTS,
  /** __if_not_exists */
  TOK_IF_NOT_EXISTS,

  /** Visual C/C++ extension token group separator for the scanner! */
  TOK_FIRST_VC = TOK_INT64,
  TOK_LAST_VC = TOK_IF_NOT_EXISTS,

  //////// C++ keywords

  /** true, false */
  TOK_BOOL_VAL,
  /** bool */
  TOK_BOOL,
  /** catch */
  TOK_CATCH,
  /** class */
  TOK_CLASS,
  /** const_cast */
  TOK_CONST_CAST,
  /** delete */
  TOK_DELETE,
  /** dynamic_cast */
  TOK_DYN_CAST,
  /** explicit */
  TOK_EXPLICIT,
  /** export */
  TOK_EXPORT,
  /** friend */
  TOK_FRIEND,
  /** mutable */
  TOK_MUTABLE,
  /** namespace */
  TOK_NAMESPACE,
  /** new */
  TOK_NEW,
  /** operator */
  TOK_OPERATOR,
  /** private */
  TOK_PRIVATE,
  /** protected */
  TOK_PROTECTED,
  /** public */
  TOK_PUBLIC,
  /** reinterpret_cast */
  TOK_REINT_CAST,
  /** static_cast */
  TOK_STAT_CAST,
  /** template */
  TOK_TEMPLATE,
  /** this */
  TOK_THIS,
  /** throw */
  TOK_THROW,
  /** try */
  TOK_TRY,
  /** typeid */
  TOK_TYPEID,
  /** typename */
  TOK_TYPENAME,
  /** using */
  TOK_USING,
  /** virtual */
  TOK_VIRTUAL,
  /** wchar_t */
  TOK_WCHAR_T,

  //////// C++ alternative representation of operators (ISO 646)

  /** and */
  TOK_AND_AND_ISO_646,
  /** and_eq */
  TOK_AND_EQ_ISO_646,
  /** bitand */
  TOK_AND_ISO_646,
  /** bitor */
  TOK_OR_ISO_646,
  /** compl */
  TOK_TILDE_ISO_646,
  /** not */
  TOK_NOT_ISO_646,
  /** not_eq */
  TOK_NEQ_ISO_646,
  /** or */
  TOK_OR_OR_ISO_646,
  /** or_eq */
  TOK_IOR_EQ_ISO_646,
  /** xor */
  TOK_ROOF_ISO_646,
  /** xor_eq */
  TOK_XOR_EQ_ISO_646,

  /** C++ token group separator for the scanner! */
  TOK_FIRST_CC = TOK_BOOL_VAL,
  TOK_LAST_CC = TOK_XOR_EQ_ISO_646,

  //////// Common C++ extension keywords

  /** __wchar_t */
  TOK_WCHAR_T_2,

  /** Common C++ extension token group separator for the scanner! */
  TOK_FIRST_EXT_CC = TOK_WCHAR_T_2,
  TOK_LAST_EXT_CC = TOK_WCHAR_T_2,

  //////// GNU C++ extension keywords

  /** GNU C++ extension token group separator for the scanner! */
  TOK_FIRST_GNUCC,
  TOK_LAST_GNUCC,

  //////// C++ type trait keywords

  /** __has_nothrow_assign */
  TOK_HAS_NOTHROW_ASSIGN,
  /** __has_nothrow_copy */
  TOK_HAS_NOTHROW_COPY,
  /** __has_nothrow_constructor */
  TOK_HAS_NOTHROW_CTOR,
  /** __has_trivial_assign */
  TOK_HAS_TRIVIAL_ASSIGN,
  /** __has_trivial_copy */
  TOK_HAS_TRIVIAL_COPY,
  /** __has_trivial_constructor */
  TOK_HAS_TRIVIAL_CTOR,
  /** __has_trivial_destructor */
  TOK_HAS_TRIVIAL_DTOR,
  /** __has_virtual_destructor */
  TOK_HAS_VIRTUAL_DTOR,
  /** __is_abstract */
  TOK_IS_ABSTRACT,
  /** __is_base_of */
  TOK_IS_BASE_OF,
  /** __is_class */
  TOK_IS_CLASS,
  /** __is_empty */
  TOK_IS_EMPTY,
  /** __is_enum */
  TOK_IS_ENUM,
  /** __is_pod */
  TOK_IS_POD,
  /** __is_trivial */
  TOK_IS_TRIVIAL,
  /** __is_polymorphic */
  TOK_IS_POLYMORPHIC,
  /** __is_union */
  TOK_IS_UNION,

  /** GNU C++ extension token group separator for the scanner! */
  TOK_FIRST_TYPETRAIT = TOK_HAS_NOTHROW_ASSIGN,
  TOK_LAST_TYPETRAIT = TOK_IS_UNION,

  //////// C++1X extension keywords
  /** static_assert */
  TOK_STATIC_ASSERT,
  
  /** C++1X extension token group separator for the scanner! */
  TOK_FIRST_CC1X = TOK_STATIC_ASSERT,
  TOK_LAST_CC1X = TOK_STATIC_ASSERT,

  //////// AspectC++ extension keywords

  /** pointcut */
  TOK_POINTCUT,
  /** aspect */
  TOK_ASPECT,
  /** advice */
  TOK_ADVICE,
  /** slice */
  TOK_SLICE,
  /** unknown_t */
  TOK_UNKNOWN_T,

  /** AspectC++ extension token group separator for the scanner! */
  TOK_FIRST_AC = TOK_POINTCUT,
  TOK_LAST_AC = TOK_UNKNOWN_T,

  /** Any identifier that is not a keyword. */
  TOK_ID,

  // These two always have to be the ** last ** token listed here!!!

  /** Epsilon. */
  TOK_EPSILON,
  /** Number of known token types. */
  TOK_NO
};

} // namespace Puma

#endif /* __C_TOKENS__ */
