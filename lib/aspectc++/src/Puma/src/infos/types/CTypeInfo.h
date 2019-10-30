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

#ifndef __CTypeInfo_h__
#define __CTypeInfo_h__

/** \file
 *  Entity type information. */

#include "Puma/Printable.h"

namespace Puma {


class CSemDatabase;
class CRecord;
class CClassInfo;
class CUnionInfo;
class CEnumInfo;
class CFunctionInfo;
class CTypeList;
class CTypeQualified;
class CTypeFunction;
class CTypeBitField; 
class CTypePointer; 
class CTypeMemberPointer; 
class CTypeTemplateParam; 
class CTypeAddress; 
class CTypeArray; 
class CTypeVarArray;
class CTypeClass;
class CTypeUnion;
class CTypeEnum;
class CTypePrimitive;
class CTypeRecord;
class CT_ExprList;
class CObjectInfo;
class CScopeInfo;
class CTemplateParamInfo;


/** \class CTypeInfo CTypeInfo.h Puma/CTypeInfo.h
 *  Type information for an entity (class, function, object, etc). 
 *  There are two kinds of types: fundamental types like 'int', 
 *  and compound types like 'class X {int i;}'. Types describe 
 *  objects, references, or functions. 
 *
 *  A type is identified by its ID.
 *  \code 
 * // check if type is a function type
 * if (type.Id() == Puma::CTypeInfo::TYPE_FUNCTION) {
 *   ...
 * }
 * // same check
 * if (type.TypeFunction()) {
 *   ...
 * }
 * // same check
 * if (type.isFunction()) {
 *   ...
 * }
 * \endcode 
 * \ingroup types */
class CTypeInfo : public Printable {
  // needed for type printing
  enum PrintState {
    PRINT_STD = 0,  // standard
    PRINT_PRE = 1,  // printing a declarator prefix like '*' or '&'
    PRINT_ABS = 2,  // printing absolute names
    PRINT_TPL = 4,  // printing template argument list
    PRINT_TDN = 8,  // print typedef names instead of defined type
    PRINT_ELA = 16, // print elaborated type specifiers (struct, enum, union)
    PRINT_INE = 32, // elaborated type speficier already issued ("in elaborated")
    PRINT_UNN = 64  // print unnamed namespace as '<unnamed>'
  };

public:
  /** Type identifiers. */
  enum TypeId {
    // DO NOT CHANGE THIS ORDER!!!
    /** bool */
    TYPE_BOOL,
    /** _Bool */
    TYPE_C_BOOL,
    /** signed char */
    TYPE_SIGNED_CHAR,
    /** unsiged char */
    TYPE_UNSIGNED_CHAR,
    /** char */
    TYPE_CHAR,
    /** unsigned short */
    TYPE_UNSIGNED_SHORT,
    /** short */
    TYPE_SHORT,
    /** unsigned int */
    TYPE_UNSIGNED_INT,
    /** wchar_t */
    TYPE_WCHAR_T,
    /** int */
    TYPE_INT,
    /** unsigned long */
    TYPE_UNSIGNED_LONG,
    /** long */
    TYPE_LONG,
    /** unsigned long long */
    TYPE_UNSIGNED_LONG_LONG,
    /** long long */
    TYPE_LONG_LONG,
    /** unsigned __int128 */
    TYPE_UNSIGNED_INT128,
    /** __int128 */
    TYPE_INT128,
    /** float */
    TYPE_FLOAT,
    /** double */
    TYPE_DOUBLE,
    /** long double */
    TYPE_LONG_DOUBLE,

    /** void */
    TYPE_VOID,
    /** Undefined type. */
    TYPE_UNDEFINED,
    /** unknown_t */
    TYPE_UNKNOWN_T,
    /** Any type. */
    TYPE_ELLIPSIS,
    
    /** Class type. */
    TYPE_CLASS,
    /** Union type. */
    TYPE_UNION,
    /** Enumeration type. */
    TYPE_ENUM,
    /** Pointer type. */
    TYPE_POINTER,
    /** Reference type. */
    TYPE_ADDRESS,
    /** Member pointer type. */
    TYPE_MEMBER_POINTER,
    /** Function type. */
    TYPE_FUNCTION,
    /** Array type. */
    TYPE_ARRAY,
    /** Variable length array type. */
    TYPE_VAR_ARRAY,
    /** Qualified type. */
    TYPE_QUALIFIED,
    /** Bit field type. */
    TYPE_BIT_FIELD,
    
    /** Template parameter type. */
    TYPE_TEMPLATE_PARAM,
    /** No type. */
    TYPE_EMPTY
  };
  
  /** Internal representation of size_t. */
  static CTypeInfo *CTYPE_SIZE_T;
  /** Internal representation of ptrdiff_t. */
  static CTypeInfo *CTYPE_PTRDIFF_T;

private:
  CTypeInfo   *_Base;
  TypeId       _Id;
  CObjectInfo *_TypedefInfo;

public:
  /** Constructor.
   *  \param base The base type of a compound type.
   *  \param id The type ID. */
  CTypeInfo (CTypeInfo *base, TypeId id);
  /** Destructor. */
  ~CTypeInfo ();

  /** Check if this type equals the given type. 
   *  \param type The type to compare with. */
  bool operator ==(const CTypeInfo &type) const;
  /** Check if this type not equals the given type. 
   *  \param type The type to compare with. */
  bool operator !=(const CTypeInfo &type) const;
  /** Check if this type equals the given type. 
   *  \param type The type to compare with.
   *  \param matchTemplateParams True if template parameters are matched. 
   *  \param noDependentFctParams True if dependent function params do not match. */
  bool equals (const CTypeInfo &type, bool matchTemplateParams = false, bool noDependentFctParams = false) const;
  /** Check if this type equals the given type if both types top-level qualifiers removed.
   *  \param type The type to compare with. */
  bool equalsUnqualified (const CTypeInfo &type) const;

  /** Print the textual representation of this 
   *  type on the given stream.
   *  \param os The output stream. */
  void print (std::ostream& os) const;
  /** Print the textual representation of this 
   *  type on the given stream.
   *  \param os The output stream. 
   *  \param name Optional name of the entity to print. 
   *  \param abs Print qualified names with root qualifier. 
   *  \param tdef Print the name of a typedef instead of the underlying type.
   *  \param elaborated_type_spec Print elaborated type specifier before 
   *                              class, union, and enumeration types.
   *  \param unnamed Print unnamed namespaces as '\<unnamed\>' */
  void TypeText (std::ostream &os, const char *name = (const char*)0,
                 bool abs = false, bool tdef = false, 
                 bool elaborated_type_spec = false,
                 bool unnamed = false) const;
  
  /** Get the dimension of an array type. */
  long int Dimension () const;
  /** Get the size in bits of a type. */
  long int Size () const;
  /** Get the alignment of a type. */
  long int Align () const;

  /** Get the type identifier. */
  TypeId Id () const;

  /** Get the base type of a compound type. 
   *  \return The base type or this if not a compound type. */
  CTypeInfo *BaseType () const;
  /** Set the base type of a compount type.
   *  \param base The base type. */
  void BaseType (CTypeInfo *base);

  /** Check if this is a typedef type. */
  bool isTypedef () const;
  /** Get the typedef information if this is a typedef type. */
  CObjectInfo *TypedefInfo () const;
  /** Set the typedef information if this is a typedef type. */
  CTypeInfo *TypedefInfo (CObjectInfo *);

  /** Check if this is a complete type. Optionally limited to
   *  a specific source code position. A type is complete
   *  if it is not undefined, not void, not an fixed length
   *  array without dimension, and not a class or enumeration 
   *  that is only declared but not defined.
   *  \param pos Optional source code position. */
  bool isComplete (unsigned long pos = 0) const;
  
  /** Check if this type or one of its base types depends on 
   *  a template parameter. 
   *  \param consider_unknown_t Consider unknown_t as dependent. 
   *  \param is_named_type Type of named type. */
  bool isDependent (bool consider_unknown_t = true, bool is_named_type = false) const;

  /** Check if this is a local type. A type is local if it was 
   *  declared in a local scope. */
  bool isLocal () const;

  /** Check if this type is \e const qualified. */
  bool isConst () const;
  /** Check if this type is \e volatile qualified. */
  bool isVolatile () const;
  /** Check if this type is \e restrict qualified. */
  bool isRestrict () const;

  /** Get the class or union of a class or union type.
   *  \return The class or union, or NULL if not such a type. */  
  CRecord *Record () const;
  /** Get the class information if this is a class type.
   *  \return The class information or NULL if not a class type. */  
  CClassInfo *ClassInfo () const;
  /** Get the union information if this is a union type.
   *  \return The union information or NULL if not a union type. */  
  CUnionInfo *UnionInfo () const;
  /** Get the enumeration information if this is an enumeration type.
   *  \return The enumeration information or NULL if not an enumeration type. */  
  CEnumInfo *EnumInfo () const;
  /** Get the function information if this is a function type.
   *  \return The function information or NULL if not a function type. */  
  CFunctionInfo *FunctionInfo () const;

  /** Get the base type of a pointer type. */
  CTypeInfo *PtrBaseType () const;
  /** Get the argument type list of a function or qualified type. */
  CTypeList *ArgTypes () const;
  /** Get the virtual type of this type. If this type is a qualified, 
   *  bit-field, or reference type then the virtual type is the 
   *  virtual type of the base type of this type. 
   *  \return The base type or this. */
  CTypeInfo *VirtualType () const;
  /** Get the unqualified version of this type.
   *  \return The unqualified type or this if not qualified. */
  CTypeInfo *UnqualType () const;
  /** Get the non-reference type version of this type.
   *  \return The non-reference type or this if not a reference type. */
  CTypeInfo *NonReferenceType () const;
    
  /** Get the function type of a pointer-to-function type.
   *  \return The function type or NULL if not a pointer to function. */
  CTypeFunction *PtrToFct () const;
  /** Get the array type of a pointer-to-array type.
   *  \return The array type or NULL if not a pointer to array. */
  CTypeArray *PtrToArray () const;
  
  /** Get the pointer to CTypeQualified if this is a qualified type. 
   *  \return The valid pointer or NULL. */
  CTypeQualified *TypeQualified () const;
  /** Get the pointer to CTypeFunction if this is a function type.
   *  \return The valid pointer or NULL. */
  CTypeFunction *TypeFunction () const;
  /** Get the pointer to CTypeBitField if this is a bit-field type.
   *  \return The valid pointer or NULL. */
  CTypeBitField *TypeBitField () const; 
  /** Get the pointer to CTypePointer if this is a pointer type.
   *  \return The valid pointer or NULL. */
  CTypePointer *TypePointer () const; 
  /** Get the pointer to CTypeMemberPointer if this is a member pointer type.
   *  \return The valid pointer or NULL. */
  CTypeMemberPointer *TypeMemberPointer () const; 
  /** Get the pointer to CTypeAddress if this is a reference type.
   *  \return The valid pointer or NULL. */
  CTypeAddress *TypeAddress () const; 
  /** Get the pointer to CTypeArray if this is an array type.
   *  \return The valid pointer or NULL. */
  CTypeArray *TypeArray () const; 
  /** Get the pointer to CTypeVarArray if this is a variable length array type.
   *  \return The valid pointer or NULL. */
  CTypeVarArray *TypeVarArray () const;
  /** Get the pointer to CTypeClass if this is a class type.
   *  \return The valid pointer or NULL. */
  CTypeClass *TypeClass () const;
  /** Get the pointer to CTypeUnion if this is a union type.
   *  \return The valid pointer or NULL. */
  CTypeUnion *TypeUnion () const;
  /** Get the pointer to CTypeRecord if this is a class or union type.
   *  \return The valid pointer or NULL. */
  CTypeRecord *TypeRecord () const;
  /** Get the pointer to CTypeEnum if this is an enumeration type.
   *  \return The valid pointer or NULL. */
  CTypeEnum *TypeEnum () const;
  /** Get the pointer to CTypePrimitive if this is a primitive type.
   *  \return The valid pointer or NULL. */
  CTypePrimitive *TypePrimitive () const;
  /** Get the pointer to CTypeInfo if this is type Puma::CTYPE_EMPTY.
   *  \return The valid pointer or NULL. */
  CTypeInfo *TypeEmpty () const;
  /** Get the pointer to CTypeTemplateParam if this is a template parameter type.
   *  \return The valid pointer or NULL. */
  CTypeTemplateParam *TypeTemplateParam () const;

  /** Check if this is a qualified type. */
  bool isQualified () const;
  /** Check if this is a pointer type. */
  bool isPointer () const;
  /** Check if this is a pointer or array type. */
  bool isPointerOrArray () const;
  /** Check if this is a reference type. */
  bool isAddress () const;
  /** Check if this is a class or union type. */
  bool isRecord () const;
  /** Check if this is a class type. */
  bool isClass () const;
  /** Check if this is a union type. */
  bool isUnion () const;
  /** Check if this is a class or union type. */
  bool isClassOrUnion () const;
  /** Check if this is an array type. */
  bool isArray () const;
  /** Check if this is a fixed length array type. */
  bool isFixedArray () const;
  /** Check if this is a variable length type. */
  bool isVarArray () const;
  /** Check if this is an arithmetic type. */
  bool isArithmetic () const;
  /** Check if this is a pointer or arithmetic type. */
  bool isScalar () const;
  /** Check if this is an array, class, or union type. */
  bool isAggregate () const;
  /** Check if this is an integer type. */
  bool isInteger () const;
  /** Check if this is type \e void. */
  bool isVoid () const;
  /** Check if this is an enumeration type. */
  bool isEnum () const;
  /** Check if this is a floating point type. */
  bool isReal () const;
  /** Check if this is a member pointer type. */
  bool isMemberPointer () const;
  /** Check if this is an undefined type. */
  bool isUndefined () const;
  /** Check if this is a function type. */
  bool isFunction () const;
  /** Check if this is a bit-field type. */
  bool isBitField () const;
  /** Check if this is a class member function type. */
  bool isMethod () const;
  /** Check if this is a non-class-member function type. */
  bool isStdFunction () const;
  /** Check if this is a template type. */
  bool isTemplate () const;
  /** Check if this is a template instance type. */
  bool isTemplateInstance () const;
  /** Check if this is a template parameter type. */
  bool isTemplateParam () const;
  /** Check if this is a type template parameter type. */
  bool isTypeParam () const;
  /** Check if this is a non-type template parameter type. */
  bool isNonTypeParam () const;
  /** Check if this is an object type. An object type is a 
   *  complete non-function type. 
   *  \param pos Optional source code position. */
  bool isObject (unsigned long pos = 0) const;
  /** Check if this is a plain old data (POD) type. */
  bool isPOD () const;
  /** Check if this is a trivial type:
   * (1) a scalar type
   * (2) a trivial class type
   * (3) an array of (1), (2), or (3)
   */
  bool isTrivial () const;

  // Primitive types.
  /** Check if the ID of this type is CTypeInfo::TYPE_BOOL or CTypeInfo::TYPE_C_BOOL. */
  bool is_bool () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_CHAR. */
  bool is_char () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_WCHAR_T. */
  bool is_wchar_t () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_SHORT. */
  bool is_short () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_INT. */
  bool is_int () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_LONG. */
  bool is_long () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_LONG_LONG. */
  bool is_long_long () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_INT128. */
  bool is_int128 () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_SIGNED_CHAR. */
  bool is_signed_char () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_UNSIGNED_CHAR. */
  bool is_unsigned_char () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_UNSIGNED_SHORT. */
  bool is_unsigned_short () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_UNSIGNED_INT. */
  bool is_unsigned_int () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_UNSIGNED_LONG. */
  bool is_unsigned_long () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_UNSIGNED_LONG_LONG. */
  bool is_unsigned_long_long () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_UNSIGNED_INT128. */
  bool is_unsigned_int128 () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_FLOAT. */
  bool is_float () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_DOUBLE. */
  bool is_double () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_LONG_DOUBLE. */
  bool is_long_double () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_VOID. */
  bool is_void () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_UNDEFINED. */
  bool is_undefined () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_UNKNOWN_T. */
  bool is_unknown_t () const;
  /** Check if the ID of this type is CTypeInfo::TYPE_ELLIPSIS. */
  bool is_ellipsis () const;
  
  /** Check if this is a signed integer type. */
  bool is_signed () const;
  /** Check if this is an unsigned integer type. */
  bool is_unsigned () const;

  /** Get the arithmetic conversion rank of the type.
   *  \note Language C only! */
  unsigned conv_rank () const;
  /** Check if the conversion rank of this type is greater
   *  than the conversion rank of the given type.
   *  \note Language C only!
   *  \param type The type to compare with. */
  bool operator >(const CTypeInfo &type) const;
  /** Check if the conversion rank of this type equals or
   *  is greater than the conversion rank of the given type.
   *  \note Language C only!
   *  \param type The type to compare with. */
  bool operator >=(const CTypeInfo &type) const;
  /** Check if the conversion rank of this type equals or
   *  is greater than the conversion rank of the given type.
   *  \note Language C only!
   *  \param type The type to compare with. */
  bool operator <=(const CTypeInfo &type) const;
  /** Check if the conversion rank of this type is less
   *  than the conversion rank of the given type.
   *  \note Language C only!
   *  \param type The type to compare with. */
  bool operator <(const CTypeInfo &type) const;

  /** Get the rank of this type if it is an arithmetic type.
   *  \note Language C++ only! 
   *  \return The rank or 0 if not an arithmetic type. */
  unsigned rank () const;

  /** Check if this is the type of a template instance argument
   *  declaration. These declarations are generated and have
   *  to be handled special. */
  bool isTemplateInstanceArg () const;

public:
  /** Make a duplicate of the given type.
   *  \param type The type to duplicate. */
  static CTypeInfo *Duplicate (const CTypeInfo *type);
  /** Maka a duplicate of this type. */
  CTypeInfo *Duplicate () const;

  /** Destroy the given type.
   *  \param type The type to destroy. */
  static void Destroy (CTypeInfo *type);

  /** Print the mangled textual representation of
   *  the type on the given stream. According to the
   *  C++ V3 ABI mangling (see http://www.codesourcery.com/cxx-abi/abi.html).
   *  \param os The output stream. */
  void Mangled (std::ostream &os) const;

private:
  void TypeText (char, std::ostream &, const char * = (const char*)0) const;
  void printName (char, std::ostream &, CObjectInfo *) const;
  void printScope (char, std::ostream &, CObjectInfo *) const;
};

/** Empty type. */
extern CTypeInfo CTYPE_EMPTY;

} // namespace Puma


#include "Puma/CTypePrimitive.inc"
#include "Puma/CTypeQualified.inc"
#include "Puma/CTypeAddress.inc"
#include "Puma/CTypeBitField.inc"
#include "Puma/CTypeArray.inc"
#include "Puma/CTypeVarArray.inc"
#include "Puma/CTypePointer.inc"
#include "Puma/CTypeMemberPointer.inc"
#include "Puma/CTypeFunction.inc"
#include "Puma/CTypeEnum.inc"
#include "Puma/CTypeRecord.inc"
#include "Puma/CTypeClass.inc"
#include "Puma/CTypeUnion.inc"
#include "Puma/CTypeTemplateParam.inc"


namespace Puma {

inline CTypeInfo::CTypeInfo (CTypeInfo *info, CTypeInfo::TypeId id) :
  _Base (info ? info : this), 
  _Id (id),
  _TypedefInfo (0)
 {}
inline CTypeInfo::~CTypeInfo () 
 {}

inline CTypeInfo *CTypeInfo::Duplicate () const 
 { 
return Duplicate (this);
 }

inline CTypeInfo::TypeId CTypeInfo::Id () const 
 { return _Id; }

inline bool CTypeInfo::operator !=(const CTypeInfo &type) const 
 { return ! (*this == type); }

inline void CTypeInfo::print (std::ostream& out) const 
 { TypeText ((char)PRINT_STD, out); }
inline void CTypeInfo::TypeText (std::ostream &out, const char *t, bool abs,
    bool tdef, bool elaborated_type_spec, bool unnamed) const
 { char flags = (char)(abs ? PRINT_ABS : PRINT_STD);
   if (tdef) flags |= (char)PRINT_TDN; 
   if (elaborated_type_spec) flags |= (char)PRINT_ELA;
   if (unnamed) flags |= (char)PRINT_UNN;
   TypeText (flags, out, t); }

inline long int CTypeInfo::Dimension () const
 { return TypeArray () ? TypeArray ()->Dimension () : 
          TypeBitField () ? TypeBitField ()->Dimension () : -1; }
 
inline bool CTypeInfo::isTypedef () const
 { return (bool)_TypedefInfo; }
inline CObjectInfo *CTypeInfo::TypedefInfo () const
 { return _TypedefInfo; }

inline bool CTypeInfo::isConst () const
 { return TypeQualified () && TypeQualified ()->isConst (); }
inline bool CTypeInfo::isVolatile () const
 { return TypeQualified () && TypeQualified ()->isVolatile (); }
inline bool CTypeInfo::isRestrict () const
 { return TypeQualified () && TypeQualified ()->isRestrict (); }
  
inline CRecord *CTypeInfo::Record () const
 { return TypeRecord () ? TypeRecord ()->Record () :
          TypeFunction () ? TypeFunction ()->Record () :
          TypeMemberPointer () ? TypeMemberPointer ()->Record () : (CRecord*)0; }
inline CEnumInfo *CTypeInfo::EnumInfo () const
 { return TypeEnum () ? TypeEnum ()->EnumInfo () : (CEnumInfo*)0; }
inline CFunctionInfo *CTypeInfo::FunctionInfo () const
 { return TypeFunction () ? TypeFunction ()->FunctionInfo () : (CFunctionInfo*)0; }

inline CTypeInfo *CTypeInfo::BaseType () const
 { assert(_Base); return _Base; }
inline void CTypeInfo::BaseType (CTypeInfo *type)
 { _Base = type; }
inline CTypeInfo *CTypeInfo::PtrBaseType () const
 { return VirtualType ()->BaseType (); }

inline CTypeList *CTypeInfo::ArgTypes () const
 { return TypeFunction () ? TypeFunction ()->ArgTypes () :
          TypeQualified () ? TypeQualified ()->ArgTypes () : (CTypeList*)0; }

inline CTypePrimitive *CTypeInfo::TypePrimitive () const
 { return _Id < TYPE_CLASS ? (CTypePrimitive*)this : (CTypePrimitive*)0; }
inline CTypeQualified *CTypeInfo::TypeQualified () const
 { return _Id == TYPE_QUALIFIED ? (CTypeQualified*)this : (CTypeQualified*)0; }
inline CTypeFunction *CTypeInfo::TypeFunction () const
 { return _Id == TYPE_FUNCTION ? (CTypeFunction*)this : (CTypeFunction*)0; }
inline CTypeBitField *CTypeInfo::TypeBitField () const 
 { return _Id == TYPE_BIT_FIELD ? (CTypeBitField*)this : (CTypeBitField*)0; }
inline CTypePointer *CTypeInfo::TypePointer () const
 { return _Id == TYPE_POINTER || _Id == TYPE_MEMBER_POINTER ? (CTypePointer*)this : (CTypePointer*)0; }
inline CTypeMemberPointer* CTypeInfo::TypeMemberPointer () const
 { return _Id == TYPE_MEMBER_POINTER ? (CTypeMemberPointer*)this : (CTypeMemberPointer*)0; }
inline CTypeAddress *CTypeInfo::TypeAddress () const
 { return _Id == TYPE_ADDRESS ? (CTypeAddress*)this : (CTypeAddress*)0; }
inline CTypeArray *CTypeInfo::TypeArray () const
 { return _Id == TYPE_ARRAY || _Id == TYPE_VAR_ARRAY ? (CTypeArray*)this : (CTypeArray*)0; }
inline CTypeVarArray *CTypeInfo::TypeVarArray () const
 { return _Id == TYPE_VAR_ARRAY ? (CTypeVarArray*)this : (CTypeVarArray*)0; }
inline CTypeRecord *CTypeInfo::TypeRecord () const
 { return _Id == TYPE_CLASS || _Id == TYPE_UNION ? (CTypeRecord*)this : (CTypeRecord*)0; }
inline CTypeClass *CTypeInfo::TypeClass () const
 { return _Id == TYPE_CLASS ? (CTypeClass*)this : (CTypeClass*)0; }
inline CTypeUnion *CTypeInfo::TypeUnion () const
 { return _Id == TYPE_UNION ? (CTypeUnion*)this : (CTypeUnion*)0; }
inline CTypeEnum *CTypeInfo::TypeEnum () const
 { return _Id == TYPE_ENUM ? (CTypeEnum*)this : (CTypeEnum*)0; }
inline CTypeInfo *CTypeInfo::TypeEmpty () const
 { return _Id == TYPE_EMPTY ? (CTypeInfo*)this : (CTypeInfo*)0; }
inline CTypeTemplateParam *CTypeInfo::TypeTemplateParam () const
 { return _Id == TYPE_TEMPLATE_PARAM ? (CTypeTemplateParam*)this : (CTypeTemplateParam*)0; }

inline CTypeInfo *CTypeInfo::VirtualType () const 
 { return TypeQualified () || TypeBitField () || TypeAddress () ?
          BaseType ()->VirtualType () : (CTypeInfo*)this; }
inline CTypeInfo *CTypeInfo::UnqualType () const 
 { return TypeQualified () ? BaseType ()->UnqualType () : (CTypeInfo*)this; }
inline CTypeInfo *CTypeInfo::NonReferenceType () const 
 { return TypeAddress () ? BaseType ()->NonReferenceType () : (CTypeInfo*)this; }

inline bool CTypeInfo::isQualified () const 
 { return TypeQualified (); }
inline bool CTypeInfo::isTemplateParam () const 
 { return VirtualType ()->TypeTemplateParam (); }
inline bool CTypeInfo::isPointer () const 
 { return VirtualType ()->TypePointer (); }
inline bool CTypeInfo::isMemberPointer () const
 { return VirtualType ()->TypeMemberPointer (); }
inline bool CTypeInfo::isClass () const
 { return VirtualType ()->TypeClass (); }
inline bool CTypeInfo::isUnion () const
 { return VirtualType ()->TypeUnion (); }
inline bool CTypeInfo::isClassOrUnion () const
 { return isRecord (); }
inline bool CTypeInfo::isArray () const
 { return VirtualType ()->TypeArray (); }
inline bool CTypeInfo::isFixedArray () const
 { return VirtualType ()->TypeArray () && 
          VirtualType ()->TypeArray ()->isFixed (); }
inline bool CTypeInfo::isVarArray () const
 { return VirtualType ()->TypeVarArray (); }
inline bool CTypeInfo::isEnum () const
 { return VirtualType ()->TypeEnum (); }
inline bool CTypeInfo::isFunction () const
 { return VirtualType ()->TypeFunction (); }
inline bool CTypeInfo::isBitField () const
 { return TypeBitField (); }
inline bool CTypeInfo::isUndefined () const
 { return VirtualType ()->is_undefined (); }
inline bool CTypeInfo::isInteger () const 
 { return VirtualType ()->_Id < TYPE_FLOAT || VirtualType ()->isEnum (); }
inline bool CTypeInfo::isReal () const 
 { CTypeInfo *base = VirtualType ();
   return base->is_float () || base->is_double () || base->is_long_double (); }
inline bool CTypeInfo::isMethod () const 
 { CTypeInfo *base = VirtualType ();
   return base->TypeFunction () && base->TypeFunction ()->Record (); }
inline bool CTypeInfo::isStdFunction () const
 { return VirtualType ()->TypeFunction () && ! isMethod (); }
inline bool CTypeInfo::isPointerOrArray () const 
 { CTypeInfo *base = VirtualType ();
   return base->TypePointer () || base->TypeArray (); }
inline bool CTypeInfo::isAddress () const 
 { return TypeAddress () || ((TypeQualified () || TypeBitField ()) && 
                             BaseType ()->isAddress ()); }
inline bool CTypeInfo::isRecord () const 
 { return VirtualType ()->TypeRecord (); }
inline bool CTypeInfo::isScalar () const 
 { return isArithmetic () || isPointer (); }
inline bool CTypeInfo::isAggregate () const 
 { return isArray () || isClass () || isUnion (); }
inline bool CTypeInfo::isVoid () const
 { return VirtualType ()->is_void (); }
inline bool CTypeInfo::isObject (unsigned long pos) const
 { return ! isFunction () && (pos ? isComplete (pos) : true); }
inline bool CTypeInfo::isTypeParam () const
 { return isTemplateParam () && 
          VirtualType ()->TypeTemplateParam ()->isType (); }
inline bool CTypeInfo::isNonTypeParam () const
 { return isTemplateParam () && 
          ! VirtualType ()->TypeTemplateParam ()->isType (); }

inline bool CTypeInfo::is_bool () const 
 { return _Id == TYPE_BOOL || _Id == TYPE_C_BOOL; }
inline bool CTypeInfo::is_char () const 
 { return _Id == TYPE_CHAR; }
inline bool CTypeInfo::is_wchar_t () const 
 { return _Id == TYPE_WCHAR_T; }
inline bool CTypeInfo::is_short () const 
 { return _Id == TYPE_SHORT; }
inline bool CTypeInfo::is_int () const 
 { return _Id == TYPE_INT; }
inline bool CTypeInfo::is_long () const 
 { return _Id == TYPE_LONG; }
inline bool CTypeInfo::is_long_long () const 
 { return _Id == TYPE_LONG_LONG; }
inline bool CTypeInfo::is_int128 () const 
 { return _Id == TYPE_INT128; }
inline bool CTypeInfo::is_signed_char () const 
 { return _Id == TYPE_SIGNED_CHAR; }
inline bool CTypeInfo::is_unsigned_char () const 
 { return _Id == TYPE_UNSIGNED_CHAR; }
inline bool CTypeInfo::is_unsigned_short () const 
 { return _Id == TYPE_UNSIGNED_SHORT; }
inline bool CTypeInfo::is_unsigned_int () const 
 { return _Id == TYPE_UNSIGNED_INT; }
inline bool CTypeInfo::is_unsigned_long () const 
 { return _Id == TYPE_UNSIGNED_LONG; }
inline bool CTypeInfo::is_unsigned_long_long () const 
 { return _Id == TYPE_UNSIGNED_LONG_LONG; }
inline bool CTypeInfo::is_unsigned_int128 () const 
 { return _Id == TYPE_UNSIGNED_INT128; }
inline bool CTypeInfo::is_float () const 
 { return _Id == TYPE_FLOAT; }
inline bool CTypeInfo::is_double () const 
 { return _Id == TYPE_DOUBLE; }
inline bool CTypeInfo::is_long_double () const 
 { return _Id == TYPE_LONG_DOUBLE; }
inline bool CTypeInfo::is_void () const 
 { return _Id == TYPE_VOID; }
inline bool CTypeInfo::is_unknown_t () const 
 { return _Id == TYPE_UNKNOWN_T; }
inline bool CTypeInfo::is_undefined () const 
 { return _Id == TYPE_UNDEFINED; }
inline bool CTypeInfo::is_ellipsis () const 
 { return _Id == TYPE_ELLIPSIS; }

inline bool CTypeInfo::operator <=(const CTypeInfo &type) const
 { return ! (*this > type); }
inline bool CTypeInfo::operator >=(const CTypeInfo &type) const
 { return ! (*this < type); }
inline bool CTypeInfo::operator ==(const CTypeInfo &type) const
 { return equals(type); }


} // namespace Puma

#endif /* __CTypeInfo_h__ */
