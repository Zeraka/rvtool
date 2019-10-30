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

#ifndef __CObjectInfo_h__
#define __CObjectInfo_h__

/** \file
 *  Basic semantic information class. */

#include "Puma/CSourceInfo.h"
#include "Puma/CProtection.h"
#include "Puma/CStorage.h"
#include "Puma/CLinkage.h"
#include "Puma/CLanguage.h"
#include "Puma/CSpecifiers.h"
#include "Puma/Array.h"
#include "Puma/DString.h"

namespace Puma {


class CRecord;
class CStructure;
class CFileInfo;
class CClassInfo;
class CBaseClassInfo;
class CMemberAliasInfo;
class CTypeInfo;
class CScopeInfo;
class CLabelInfo;
class CUnionInfo;
class CLocalScope;
class CEnumInfo;
class CUsingInfo;
class CEnumeratorInfo;
class CTypedefInfo;
class CFunctionInfo;
class CArgumentInfo;
class CAttributeInfo;
class CTemplateParamInfo;
class CTemplateInfo;
class CNamespaceInfo;
class CClassInstance;
class CUnionInstance;
class CFctInstance;
class CSemDatabase;
class CTree;
class CT_ExprList;
class CTemplateInstance;


/** \class CObjectInfo CObjectInfo.h Puma/CObjectInfo.h 
 *  Abstract base class of all semantic information classes.
 *  Provides all semantic information about an entity (class,
 *  function, object, etc). 
 *
 *  A semantic object is identified by its object ID. %Semantic
 *  information objects for the same kind of entity have the
 *  same object ID (like object ID CObjectInfo::FUNCTION_INFO
 *  for all semantic objects of functions). 
 * 
 *  Example:
 *
 *  \code 
 * // check if sem_obj is a semantic object for a function
 * if (sem_obj.Id() == Puma::CObjectInfo::FUNCTION_INFO) {
 *   ...
 * }
 * // same check
 * if (sem_obj.FunctionInfo()) {
 *   ...
 * }
 *  \endcode 
 *
 *  %Semantic information objects are created by the semantic
 *  analysis component of %Puma (see Puma::Semantic) during 
 *  the parse process and are collected in the semantic
 *  information database (see Puma::CSemDatabase). 
 *
 *  There are several relations between the semantic objects 
 *  forming the semantic tree. There is one semantic tree for 
 *  each translation unit. 
 *
 *  The root of the semantic tree usually is the semantic 
 *  object for the file scope (see Puma::CFileInfo). It contains 
 *  all the other scopes of the analysed source file, such 
 *  as namespaces and class definitions, function definitions, 
 *  global variables, and so on. The semantic tree is destroyed 
 *  by destroying the root object of the tree. This recursively 
 *  destroys all sub-objects of the tree. */
class CObjectInfo {
public:
  /** %Semantic information object types. */
  enum ObjectId {
    /** %Semantic information about the file scope. */
    FILE_INFO, 
    /** %Semantic information about a union. */
    UNION_INFO, 
    /** %Semantic information about a class. */
    CLASS_INFO,
    /** %Semantic information about a base class specifier. */
    BASECLASS_INFO,
    /** %Semantic information about class/namespace member alias. */
    MEMBERALIAS_INFO,
    /** %Semantic information about an enumeration. */
    ENUM_INFO, 
    /** %Semantic information about a typedef. */
    TYPEDEF_INFO, 
    /** %Semantic information about a function, method, or overloaded operator. */
    FUNCTION_INFO, 
    /** %Semantic information about a label. */
    LABEL_INFO,
    /** %Semantic information about an enumerator constant. */
    ENUMERATOR_INFO,
    /** %Semantic information about an object (variables etc). */
    ATTRIBUTE_INFO, 
    /** %Semantic information about a template parameter. */
    TEMPLATE_PARAM_INFO,
    /** %Semantic information about a class or function template. */
    TEMPLATE_INFO,
    /** %Semantic information about an instance of a class template. */
    CLASS_INSTANCE_INFO,
    /** %Semantic information about an instance of a union template. */
    UNION_INSTANCE_INFO,
    /** %Semantic information about an instance of a function template. */
    FCT_INSTANCE_INFO,
    /** %Semantic information about a function parameter. */
    ARGUMENT_INFO, 
    /** %Semantic information about a local scope (block scope). */
    LOCAL_INFO, 
    /** %Semantic information about a named or anonymous namespace. */
    NAMESPACE_INFO,
    /** %Semantic information about a using declaration. */
    USING_INFO
  };

private:
  DString            _Name;
  const char        *_QualName;         // Full qualified name
  bool              _abs, _tdef,
                    _unnamed;           // Flags that were used for QualName()
  CTypeInfo         *_TypeInfo;
  CObjectInfo       *_BaseObject;       // corresponding object of base class
  CObjectInfo       *_Next;             // next linked object
  CObjectInfo       *_Prev;             // previous linked object
  CSemDatabase      *_SemDB;
  CTree             *_Tree;             // corresponding syntax tree
  ObjectId           _Id;               // object type
  CSpecifiers        _Specifiers;
  CSourceInfo        _SourceInfo;       // location in source file
  CProtection::Type  _Protection;
  CLinkage::Type     _Linkage;
  CStorage::Type     _Storage;            
  CLanguage          _Language;

protected:
  /** Qualified name scope. Set only for CAttributeInfo,
   *  CFunctionInfo, and CRecord. */
  CStructure        *_QualScope;
  /** The scope of a friend class or function. This is not 
   *  the scope in which the friend class or function 
   *  was declared. A friend function of a class may be declared
   *  first inside the scope of a class definition. But the
   *  declared function does not belong to this scope. In fact
   *  it belongs to the nearest non-class scope (usually the
   *  file scope). This is the assigned scope. */
  CStructure        *_AssignedScope;
  /** Set of semantic objects connected to this object in
   *  any way. */
  Array<CStructure*> _Registered;         
  
public: 
  /** Destructor. */
  ~CObjectInfo ();
  
  /** Compare the addresses of this object and all objects
   *  linked to this object with the address of the given 
   *  object.
   *  \return True if the addresses are the same and thus
   *          the given object describes the same entity. */
  bool operator ==(const CObjectInfo &) const;
  /** Compare the addresses of this object and all objects
   *  linked to this object with the address of the given 
   *  object.
   *  \return True if the addresses are different and thus
   *          the given object does not describe the same entity. */
  bool operator !=(const CObjectInfo &) const;

  /** Get a pointer to CObjectInfo for any semantic object type. */
  CObjectInfo        *ObjectInfo () const;
  /** Return a pointer to CLabelInfo if the entity is a label. 
   *  The object type has to be one of:
   *  \li CObjectInfo::LABEL_INFO
   * 
   *  \return The valid pointer or NULL. */
  CLabelInfo         *LabelInfo () const;
  /** Return a pointer to CMemberAliasInfo if the entity is a member alias.
   *  The object type has to be one of:
   *  \li CObjectInfo::MEMBERALIAS_INFO
   *
   *  \return The valid pointer or NULL. */
  CMemberAliasInfo   *MemberAliasInfo () const; 
  /** Return a pointer to CBaseClassInfo if the entity is a base class specifier.
   *  The object type has to be one of:
   *  \li CObjectInfo::BASECLASS_INFO 
   *
   *  \return The valid pointer or NULL. */
  CBaseClassInfo     *BaseClassInfo () const;
  /** Return a pointer to CUsingInfo if the entity is a using-directive.
   *  The object type has to be one of:
   *  \li CObjectInfo::USING_INFO
   *
   *  \return The valid pointer or NULL. */
  CUsingInfo         *UsingInfo () const;
  /** Return a pointer to CTypedefInfo if the entity is a typedef.
   *  The object type has to be one of:
   *  \li CObjectInfo::TYPEDEF_INFO
   *
   *  \return The valid pointer or NULL. */
  CTypedefInfo       *TypedefInfo () const;
  /** Return a pointer to CArgumentInfo if the entity is a function parameter.
   *  The object type has to be one of:
   *  \li CObjectInfo::ARGUMENT_INFO
   *
   *  \return The valid pointer or NULL. */
  CArgumentInfo      *ArgumentInfo () const;
  /** Return a pointer to CAttributeInfo if the entity is an object or 
   *  enumeration constant.
   *  The object type has to be one of:
   *  \li CObjectInfo::ATTRIBUTE_INFO
   *  \li CObjectInfo::ENUMERATOR_INFO
   *
   *  \return The valid pointer or NULL. */
  CAttributeInfo     *AttributeInfo () const;
  /** Return a pointer to CTemplateParemInfo if the entity is a template parameter.
   *  The object type has to be one of:
   *  \li CObjectInfo::TEMPLATE_PARAM_INFO
   *
   *  \return The valid pointer or NULL. */
  CTemplateParamInfo *TemplateParamInfo () const; 
  /** Return a pointer to CStructure if the entity is a namespace, class, 
   *  function, or any other construct that can contain other entities.
   *  The object type has to be one of:
   *  \li CObjectInfo::NAMESPACE_INFO
   *  \li CObjectInfo::FILE_INFO
   *  \li CObjectInfo::CLASS_INFO
   *  \li CObjectInfo::CLASS_INSTANCE_INFO
   *  \li CObjectInfo::UNION_INFO
   *  \li CObjectInfo::UNION_INSTANCE_INFO
   *  \li CObjectInfo::FUNCTION_INFO,
   *  \li CObjectInfo::FCT_INSTANCE_INFO
   *  \li CObjectInfo::TEMPLATE_INFO
   *  \li CObjectInfo::LOCAL_INFO
   *
   *  \return The valid pointer or NULL. */
  CStructure         *Structure () const; 
  /** Return a pointer to CFileInfo if this is the file scope.
   *  The object type has to be one of:
   *  \li CObjectInfo::FILE_INFO
   *
   *  \return The valid pointer or NULL. */
  CFileInfo          *FileInfo () const;  
  /** Return a pointer to CRecord if the entity is a class or union.
   *  The object type has to be one of:
   *  \li CObjectInfo::CLASS_INFO
   *  \li CObjectInfo::CLASS_INSTANCE_INFO
   *  \li CObjectInfo::UNION_INFO
   *  \li CObjectInfo::UNION_INSTANCE_INFO
   *
   *  \return The valid pointer or NULL. */
  CRecord            *Record () const;        
  /** Return a pointer to CLocalScope if the entity is a local scope
   *  (block scope).
   *  The object type has to be one of:
   *  \li CObjectInfo::LOCAL_INFO
   *
   *  \return The valid pointer or NULL. */
  CLocalScope        *LocalScope () const;
  /** Return a pointer to CScopeInfo if the entity defines a scope.
   *  The object type has to be one of:
   *  \li CObjectInfo::NAMESPACE_INFO
   *  \li CObjectInfo::FILE_INFO
   *  \li CObjectInfo::CLASS_INFO
   *  \li CObjectInfo::CLASS_INSTANCE_INFO
   *  \li CObjectInfo::UNION_INFO
   *  \li CObjectInfo::UNION_INSTANCE_INFO
   *  \li CObjectInfo::FUNCTION_INFO
   *  \li CObjectInfo::FCT_INSTANCE_INFO
   *  \li CObjectInfo::TEMPLATE_INFO
   *  \li CObjectInfo::LOCAL_INFO
   *
   *  \return The valid pointer or NULL. */
  CScopeInfo         *ScopeInfo () const; 
  /** Return a pointer to CClassInfo if the entity is a class.
   *  The object type has to be one of:
   *  \li CObjectInfo::CLASS_INFO
   *  \li CObjectInfo::CLASS_INSTANCE_INFO
   *
   *  \return The valid pointer or NULL. */
  CClassInfo         *ClassInfo () const;
  /** Return a pointer to CUnionInfo if the entity is a union.
   *  The object type has to be one of:
   *  \li CObjectInfo::UNION_INFO
   *  \li CObjectInfo::UNION_INSTANCE_INFO
   *
   *  \return The valid pointer or NULL. */
  CUnionInfo         *UnionInfo () const;
  /** Return a pointer to CEnumInfo if the entity is an enumeration.
   *  The object type has to be one of:
   *  \li CObjectInfo::ENUM_INFO
   *
   *  \return The valid pointer or NULL. */
  CEnumInfo          *EnumInfo () const;
  /** Return a pointer to CFunctionInfo if the entity is a function,
   *  method, or overloaded operator.
   *  The object type has to be one of:
   *  \li CObjectInfo::FUNCTION_INFO
   *  \li CObjectInfo::FCT_INSTANCE_INFO
   *
   *  \return The valid pointer or NULL. */
  CFunctionInfo      *FunctionInfo () const;
  /** Return a pointer to CNamespaceInfo if the entity is a namespace.
   *  The object type has to be one of:
   *  \li CObjectInfo::NAMESPACE_INFO
   *  \li CObjectInfo::FILE_INFO
   *
   *  \return The valid pointer or NULL. */
  CNamespaceInfo     *NamespaceInfo () const;
  /** Return a pointer to CEnumeratorInfo if the entity is an 
   *  enumeration constant.
   *  The object type has to be one of:
   *  \li CObjectInfo::ENUMERATOR_INFO
   *
   *  \return The valid pointer or NULL. */
  CEnumeratorInfo    *EnumeratorInfo () const;
  /** Return a pointer to CTemplateInfo if the entity is a template.
   *  The object type has to be one of:
   *  \li CObjectInfo::TEMPLATE_INFO
   *
   *  \return The valid pointer or NULL. */
  CTemplateInfo      *TemplateInfo () const; 
  /** Return a pointer to CClassInstance if the entity is an instance
   *  of a class template.
   *  The object type has to be one of:
   *  \li CObjectInfo::CLASS_INSTANCE_INFO
   *
   *  \return The valid pointer or NULL. */
  CClassInstance     *ClassInstance () const; 
  /** Return a pointer to CFctInstance if the entity is an instance
   *  of a function template.
   *  The object type has to be one of:
   *  \li CObjectInfo::FCT_INSTANCE_INFO
   *
   *  \return The valid pointer or NULL. */
  CFctInstance       *FctInstance () const; 
  /** Return a pointer to CUnionInstance if the entity is an instance
   *  of a union template.
   *  The object type has to be one of:
   *  \li CObjectInfo::UNION_INSTANCE_INFO
   *
   *  \return The valid pointer or NULL. */
  CUnionInstance     *UnionInstance () const; 

  /** Return a pointer to CTemplateInfo if the entity is a template.
   *  The object type has to be one of:
   *  \li CObjectInfo::CLASS_INFO
   *  \li CObjectInfo::UNION_INFO
   *  \li CObjectInfo::FUNCTION_INFO
   *  \li CObjectInfo::TEMPLATE_PARAM_INFO
   *
   *  \return The valid pointer or NULL. */
  CTemplateInfo      *Template () const;
  /** Return a pointer to CTemplateInstance if the entity is an
   *  instance of a class or function template.
   *  The object type has to be one of:
   *  \li CObjectInfo::CLASS_INSTANCE_INFO
   *  \li CObjectInfo::UNION_INSTANCE_INFO
   *  \li CObjectInfo::FCT_INSTANCE_INFO
   *
   *  \return The valid pointer or NULL. */
  CTemplateInstance  *TemplateInstance () const;
  
  /** Get the scope in which the entity was declared. */
  CScopeInfo         *Scope () const; 
  /** Get the scope of qualified names. The scope of a class member
   *  for instance is the corresponding class. If a function is declared
   *  in a namespace, then the qualified scope is that namespace. */
  CStructure         *QualifiedScope () const;
  /** Get the class containing the class member described by this object.
   *  The object type has to be one of:
   *  \li CObjectInfo::FUNCTION_INFO
   *  \li CObjectInfo::FCT_INSTANCE_INFO
   *  \li CObjectInfo::ATTRIBUTE_INFO
   *  \li CObjectInfo::ENUMERATOR_INFO 
   *
   *  \return The class or union, or NULL if not a member of a class or union. */
  CRecord            *ClassScope () const;
  /** Get the scope of a friend class or function. This is not 
   *  the scope in which the friend class or function 
   *  was declared. A friend function of a class may be declared
   *  first inside the scope of a class definition. But the
   *  declared function does not belong to this scope. In fact
   *  it belongs to the nearest non-class scope (usually the
   *  file scope). This is the assigned scope. 
   *  \return The assigned scope or NULL. */
  CStructure          *AssignedScope () const; 

  /** Get the type of this semantic object. */
  ObjectId            Id () const;
  /** Get the name of entity described by this semantic object. */
  const DString&      Name () const; 
  /** Get the qualified name of the entity described by this semantic object. 
   *  \param abs Create root qualified name (like \::X\::Y\::Z).
   *  \param tdef Insert the name of a typedef instead of the named type.
   *  \param unnamed Don't ignore unnamed namespaces (like \<unnamed\>\::foo). */
  const char         *QualName (bool abs = false, bool tdef = false,
                                bool unnamed = false);
  /** Get the semantic information object for the definition of an entity.
   *  Some entities, like functions and classes, can be declared several
   *  times before a definition of the entity appears. The semantic objects
   *  for the definition and declarations are linked. This method searches
   *  the linked semantic objects for the semantic object of the definition
   *  of the entity.
   *  \return The semantic object for the definition or this object if
   *          no definition found. */
  CObjectInfo        *DefObject () const;
  /** Get the data type of the entity. */
  CTypeInfo          *TypeInfo () const;
  /** Get the source file information. Contains the position and token 
   *  of the entity in the source file. */
  CSourceInfo        *SourceInfo () const;
  /** Get the semantic information database object containing this
   *  semantic object. 
   *  \deprecated Use CObjectInfo::SemDB() instead. */
  CSemDatabase       *ClassDB () const; 
  /** Get the semantic information database object containing this
   *  semantic object. */
  CSemDatabase       *SemDB () const; 
  /** Get the syntax tree node for the entity described by this 
   *  semantic object. */
  CTree              *Tree () const;
  /** Get the next semantic object linked with this object. Usually
   *  the definition and the declarations of an entity are linked. */
  CObjectInfo        *NextObject () const;
  /** Get the previous semantic object linked with this object. Usually
   *  the definition and the declarations of an entity are linked. */
  CObjectInfo        *PrevObject () const;
  /** Get the semantic object for the base class entity this entity
   *  is overloading. 
   *  \note Not yet implemented! 
   *  \return Always returns NULL. */
  CObjectInfo        *BaseObject () const;
  /** Get the initializer of the entity.
   *  \return The initializer expression or NULL if no initializer. */
  CTree              *Init () const;
  /** Get the member protection of the entity, if it is a class member. */
  CProtection::Type   Protection () const;
  /** Get the linkage of the entity. */
  CLinkage::Type      Linkage () const;
  /** Get the storage class of the entity. */
  CStorage::Type      Storage () const;
  /** Get the entity encoding language. */
  const CLanguage    &Language () const;
  /** Get the entity encoding language. */
  CLanguage          &Language ();
  /** Get the declaration specifiers. */
  const CSpecifiers  &Specifiers () const;

  /** Check if the entity is a type. */
  bool                isType () const;
  /** Check if the entity is an object. */
  bool                isObject () const;
  /** Check if the entity is anonymous (has no explicit name). */
  bool                isAnonymous () const;
  /** Check if the entity is a class or function template, or a 
   *  template template parameter. */
  bool                isTemplate () const;
  /** Check if the entity is a class or function template instance. */
  bool                isTemplateInstance () const;
  /** Check if the entity describes a built-in type or function.
   *  In this case the entity has no syntax tree (Tree() returns NULL). */
  bool                isBuiltin () const;
  /** Check if the entity is a method or data member of a class. */
  bool                isClassMember () const;
  /** Check if the entity is declared \e virtual. */
  bool                isVirtual () const;
  /** Check if the entity is declared \e static. */
  bool                isStatic () const;
  /** Check if the entity is declared \e __thread. */
  bool                isThreadLocal () const;
  /** Check if the entity is declared \e extern. */
  bool                isExtern () const;
  /** Check if the entity is declared \e mutable. */
  bool                isMutable () const;
  /** Check if the entity is declared \e register. */
  bool                isRegister () const;
  /** Check if the entity is declared \e explicit. */
  bool                isExplicit () const;
  /** Check if the entity is declared \e inline. */
  bool                isInline () const;
  /** Check if the entity is declared \e auto. */
  bool                isAuto () const;
  /** Check if the given semantic object is registered as
   *  being connected to this semantic object in any way.
   *  \param s The semantic object. */
  bool                isRegistered (const CStructure* s) const;
  /** Check if the entity is local. An entity is local if it was 
   *  declared in a local scope. */
  bool                isLocal () const;

  /** Set the name of the entity.
   *  \param s The name. */
  void                Name (const char* s) { Name(DString(s==0?"":s)); }
  /** Set the name of the entity.
   *  \param ds The name. */
  void                Name (const DString& ds);
  /** Set the type of the entity. 
   *  \param type The type information. */
  void                TypeInfo (CTypeInfo *type);
  /** Set the base class object for the entity this entity overloads.
   *  \param base The base object. */
  void                BaseObject (CObjectInfo *base);
  /** Set the member access protection of the entity. 
   *  \param p The protection. */
  void                Protection (CProtection::Type p);
  /** Set the linkage of the entity.
   *  \param l The linkage. */
  void                Linkage (CLinkage::Type l);
  /** Set the storage class of the entity. 
   *  \param s The storage class. */
  void                Storage (CStorage::Type s);
  /** Set the declaration specifiers. */
  void                Specifiers (const CSpecifiers&);
  /** Set the source file information for the entity.
   *  \param finfo The file information. */
  void                FileInfo (CFileInfo *finfo);
  /** Set the syntax tree node of the entity.
   *  \param tree The syntax tree node. */
  void                Tree (CTree *tree);    
  /** Set the semantic information database object containing
   *  this semantic object. 
   *  \deprecated Use CObjectInfo::SemDB(CSemDatabase*) instead. 
   *  \param db The semantic database. */
  void                ClassDB (CSemDatabase *db);
  /** Set the semantic information database object containing
   *  this semantic object. 
   *  \param db The semantic database. */
  void                SemDB (CSemDatabase *db);
  /** Set the link to next semantic object. Usually the semantic objects
   *  for the definition and declaration of an entity are linked. 
   *  \param obj The next object in the chain. */
  void                NextObject (CObjectInfo *obj); 
  /** Set the link to next semantic object. Usually the semantic objects
   *  for the definition and declaration of an entity are linked.
   *  \param obj The previous object in the chain. */
  void                PrevObject (CObjectInfo *obj); 
  /** Unlink this semantic object. Usually the semantic objects
   *  for the definition and declaration of an entity are linked. 
   *  This method removes this object from the chain. */
  void                Unlink (); 
  /** Register the given semantic object as being connected to 
   *  this semantic object in any way.
   *  \param s The semantic object. */
  void                Register (CStructure *s);
  /** Unregister the given semantic object as being connected to 
   *  this semantic object in any way.
   *  \param s The semantic object. */
  void                Unregister (CStructure *s);
  /** Set whether the entity was declared \e virtual.
   *  \param v True for yes, false for no. */
  void                isVirtual (bool v);
  /** Set whether the entity was declared \e static.
   *  \param v True for yes, false for no. */
  void                isStatic (bool v);
  /** Set whether the entity was declared \e __thread.
   *  \param v True for yes, false for no. */
  void                isThreadLocal (bool v);
  /** Set whether the entity was declared \e extern.
   *  \param v True for yes, false for no. */
  void                isExtern (bool v);
  /** Set whether the entity was declared \e mutable.
   *  \param v True for yes, false for no. */
  void                isMutable (bool v);
  /** Set whether the entity was declared \e register.
   *  \param v True for yes, false for no. */
  void                isRegister (bool v);
  /** Set whether the entity was declared \e explicit.
   *  \param v True for yes, false for no. */
  void                isExplicit (bool v);
  /** Set whether the entity was declared \e inline.
   *  \param v True for yes, false for no. */
  void                isInline (bool v);
  /** Set whether the entity was declared \e auto.
   *  \param v True for yes, false for no. */
  void                isAuto (bool v);
  /** Set the assigned scope of the entity. This is the scope of
   *  a friend class or function. It is not the scope in which the 
   *  friend class or function was declared. A friend function of 
   *  a class may be declared first inside the scope of a class 
   *  definition. But the declared function does not belong to 
   *  this scope. In fact it belongs to the nearest non-class scope 
   *  (usually the file scope). This is the assigned scope.
   *  \param s The assigned scope. */
  void                AssignedScope (CStructure *s);

protected:  
  /** Constructor.
   *  \param id The semantic object type. */
  CObjectInfo (ObjectId id);
};

inline CObjectInfo::CObjectInfo (CObjectInfo::ObjectId id) :
  _Name              (),
  _QualName          ((const char*)0),
  _abs               (false),
  _tdef              (false),
  _unnamed           (false),
  _TypeInfo          ((CTypeInfo*)0),
  _BaseObject        ((CObjectInfo*)0),
  _Next              ((CObjectInfo*)this),
  _Prev              ((CObjectInfo*)this),
  _SemDB             ((CSemDatabase*)0),
  _Tree              ((CTree*)0),
  _Id                (id),
  _Protection        (CProtection::PROT_NONE), 
  _Linkage           (CLinkage::LINK_NONE),
  _Storage           (CStorage::CLASS_NONE),
  _QualScope         ((CStructure*)0),
  _AssignedScope     ((CStructure*)0),
  _Registered        (1, 10)
 {}

inline CObjectInfo::ObjectId CObjectInfo::Id () const
 { return _Id; }

inline bool CObjectInfo::operator !=(const CObjectInfo &info) const
 { return ! (*this == info); }

inline const DString& CObjectInfo::Name () const 
 { return _Name; }
inline CTypeInfo *CObjectInfo::TypeInfo () const
 { return _TypeInfo; }
inline CSourceInfo *CObjectInfo::SourceInfo () const
 { return (CSourceInfo*)&_SourceInfo; }
inline CProtection::Type CObjectInfo::Protection () const
 { return _Protection; }
inline CLinkage::Type CObjectInfo::Linkage () const
 { return _Linkage; }
inline CStorage::Type CObjectInfo::Storage () const
 { return _Storage; }
inline CLanguage &CObjectInfo::Language ()
 { return _Language; }
inline const CLanguage &CObjectInfo::Language () const
 { return _Language; }
inline CObjectInfo *CObjectInfo::BaseObject () const
 { return _BaseObject; }
inline CTree *CObjectInfo::Tree () const 
 { return _Tree; }
inline CSemDatabase *CObjectInfo::ClassDB () const
 { return _SemDB; }
inline CSemDatabase *CObjectInfo::SemDB () const
 { return _SemDB; }

inline void CObjectInfo::Protection (CProtection::Type s)
 { _Protection = s; }
inline void CObjectInfo::Linkage (CLinkage::Type s)
 { _Linkage = s; }
inline void CObjectInfo::Storage (CStorage::Type s)
 { _Storage = s; }
inline void CObjectInfo::BaseObject (CObjectInfo *info)
 { _BaseObject = info; }
inline void CObjectInfo::Tree (CTree *t)    
 { _Tree = t; }
inline void CObjectInfo::ClassDB (CSemDatabase *db) 
 { _SemDB = db; }
inline void CObjectInfo::SemDB (CSemDatabase *db) 
 { _SemDB = db; }

inline void CObjectInfo::Register (CStructure *scope)
 { _Registered.append (scope); }

inline bool CObjectInfo::isAnonymous () const 
 { return (Name ().empty () || *Name ().c_str () == '%' ||
     *Name ().c_str () == '<'); // the last one is for <unnamed> namespaces
 }
inline bool CObjectInfo::isBuiltin () const
 { return ! Tree (); }

inline const CSpecifiers &CObjectInfo::Specifiers () const
 { return _Specifiers; }
inline void CObjectInfo::Specifiers (const CSpecifiers& specs)
 { _Specifiers = specs; }

inline bool CObjectInfo::isVirtual () const
 { return _Specifiers == CSpecifiers::SPEC_VIRTUAL; }
inline bool CObjectInfo::isStatic () const
 { return _Specifiers == CSpecifiers::SPEC_STATIC; }
inline bool CObjectInfo::isThreadLocal () const
 { return _Specifiers == CSpecifiers::SPEC_THREAD; }
inline bool CObjectInfo::isExtern () const
 { return _Specifiers == CSpecifiers::SPEC_EXTERN; }
inline bool CObjectInfo::isMutable () const
 { return _Specifiers == CSpecifiers::SPEC_MUTABLE; }
inline bool CObjectInfo::isRegister () const
 { return _Specifiers == CSpecifiers::SPEC_REGISTER; }
inline bool CObjectInfo::isExplicit () const
 { return _Specifiers == CSpecifiers::SPEC_EXPLICIT; }
inline bool CObjectInfo::isInline () const
 { return _Specifiers == CSpecifiers::SPEC_INLINE; }
inline bool CObjectInfo::isAuto () const
 { return _Specifiers == CSpecifiers::SPEC_AUTO; }

inline void CObjectInfo::isVirtual (bool v)
 { if (v) _Specifiers += CSpecifiers::SPEC_VIRTUAL; 
   else   _Specifiers -= CSpecifiers::SPEC_VIRTUAL; }
inline void CObjectInfo::isStatic (bool v)
 { if (v) _Specifiers += CSpecifiers::SPEC_STATIC; 
   else   _Specifiers -= CSpecifiers::SPEC_STATIC; }
inline void CObjectInfo::isThreadLocal (bool v)
 { if (v) _Specifiers += CSpecifiers::SPEC_THREAD; 
   else   _Specifiers -= CSpecifiers::SPEC_THREAD; }
inline void CObjectInfo::isExtern (bool v)
 { if (v) _Specifiers += CSpecifiers::SPEC_EXTERN; 
   else   _Specifiers -= CSpecifiers::SPEC_EXTERN; }
inline void CObjectInfo::isMutable (bool v)
 { if (v) _Specifiers += CSpecifiers::SPEC_MUTABLE; 
   else   _Specifiers -= CSpecifiers::SPEC_MUTABLE; }
inline void CObjectInfo::isRegister (bool v)
 { if (v) _Specifiers += CSpecifiers::SPEC_REGISTER; 
   else   _Specifiers -= CSpecifiers::SPEC_REGISTER; }
inline void CObjectInfo::isExplicit (bool v)
 { if (v) _Specifiers += CSpecifiers::SPEC_EXPLICIT; 
   else   _Specifiers -= CSpecifiers::SPEC_EXPLICIT; }
inline void CObjectInfo::isInline (bool v)
 { if (v) _Specifiers += CSpecifiers::SPEC_INLINE; 
   else   _Specifiers -= CSpecifiers::SPEC_INLINE; }
inline void CObjectInfo::isAuto (bool v)
 { if (v) _Specifiers += CSpecifiers::SPEC_AUTO; 
   else   _Specifiers -= CSpecifiers::SPEC_AUTO; }

// link objects

inline CObjectInfo *CObjectInfo::NextObject () const
 { return _Next; }
inline CObjectInfo *CObjectInfo::PrevObject () const
 { return _Prev; }

inline CMemberAliasInfo *CObjectInfo::MemberAliasInfo () const
 { return _Id == MEMBERALIAS_INFO ? (CMemberAliasInfo*)this : (CMemberAliasInfo*)0; }
inline CBaseClassInfo *CObjectInfo::BaseClassInfo () const
 { return _Id == BASECLASS_INFO ? (CBaseClassInfo*)this : (CBaseClassInfo*)0; }
inline CClassInfo *CObjectInfo::ClassInfo () const
 { return _Id == CLASS_INFO || _Id == CLASS_INSTANCE_INFO ? (CClassInfo*)this : (CClassInfo*)0; }
inline CUnionInfo *CObjectInfo::UnionInfo () const
 { return _Id == UNION_INFO || _Id == UNION_INSTANCE_INFO ? (CUnionInfo*)this : (CUnionInfo*)0; }
inline CEnumInfo *CObjectInfo::EnumInfo () const
 { return _Id == ENUM_INFO ? (CEnumInfo*)this : (CEnumInfo*)0; }
inline CUsingInfo *CObjectInfo::UsingInfo () const
 { return _Id == USING_INFO ? (CUsingInfo*)this : (CUsingInfo*)0; }
inline CEnumeratorInfo *CObjectInfo::EnumeratorInfo () const
 { return _Id == ENUMERATOR_INFO ? (CEnumeratorInfo*)this : (CEnumeratorInfo*)0; }
inline CTypedefInfo *CObjectInfo::TypedefInfo () const
 { return _Id == TYPEDEF_INFO ? (CTypedefInfo*)this : (CTypedefInfo*)0; }
inline CFunctionInfo *CObjectInfo::FunctionInfo () const
 { return _Id == FUNCTION_INFO || _Id == FCT_INSTANCE_INFO ? (CFunctionInfo*)this : (CFunctionInfo*)0; }
inline CArgumentInfo *CObjectInfo::ArgumentInfo () const
 { return _Id == ARGUMENT_INFO ? (CArgumentInfo*)this : (CArgumentInfo*)0; }
inline CAttributeInfo *CObjectInfo::AttributeInfo () const
 { return _Id == ATTRIBUTE_INFO || _Id == ENUMERATOR_INFO ? (CAttributeInfo*)this : (CAttributeInfo*)0; }
inline CNamespaceInfo *CObjectInfo::NamespaceInfo () const
 { return _Id == NAMESPACE_INFO || _Id == FILE_INFO ? (CNamespaceInfo*)this : (CNamespaceInfo*)0; }
inline CLabelInfo *CObjectInfo::LabelInfo () const
 { return _Id == LABEL_INFO ? (CLabelInfo*)this : (CLabelInfo*)0; }
inline CFileInfo *CObjectInfo::FileInfo () const
 { return _Id == FILE_INFO ? (CFileInfo*)this : (CFileInfo*)0; }
inline CStructure *CObjectInfo::Structure () const
 { return NamespaceInfo () || ClassInfo () || UnionInfo () || 
          FunctionInfo () || TemplateInfo () || LocalScope () ? 
          (CStructure*)this : (CStructure*)0; }
inline CRecord *CObjectInfo::Record () const
 { return ClassInfo () || UnionInfo () ? (CRecord*)this : (CRecord*)0; }
inline CLocalScope *CObjectInfo::LocalScope () const
 { return _Id == LOCAL_INFO ? (CLocalScope*)this : (CLocalScope*)0; }
inline CTemplateParamInfo *CObjectInfo::TemplateParamInfo () const
 { return _Id == TEMPLATE_PARAM_INFO ? (CTemplateParamInfo*)this : (CTemplateParamInfo*)0; }
inline CTemplateInfo *CObjectInfo::TemplateInfo () const
 { return _Id == TEMPLATE_INFO ? (CTemplateInfo*)this : (CTemplateInfo*)0; }
inline CScopeInfo *CObjectInfo::ScopeInfo () const
 { return /*_Id == SCOPE_INFO ||*/ Structure () ? (CScopeInfo*)this : (CScopeInfo*)0; }
inline CObjectInfo *CObjectInfo::ObjectInfo () const
 { return (CObjectInfo*)this; }
inline CClassInstance *CObjectInfo::ClassInstance () const
 { return _Id == CLASS_INSTANCE_INFO ? (CClassInstance*)this : (CClassInstance*)0; }
inline CUnionInstance *CObjectInfo::UnionInstance () const
 { return _Id == UNION_INSTANCE_INFO ? (CUnionInstance*)this : (CUnionInstance*)0; }
inline CFctInstance *CObjectInfo::FctInstance () const
 { return _Id == FCT_INSTANCE_INFO ? (CFctInstance*)this : (CFctInstance*)0; }

inline CStructure *CObjectInfo::QualifiedScope () const
 { return _QualScope; }

inline void CObjectInfo::AssignedScope (CStructure *s)
 { _AssignedScope = s; }
inline CStructure *CObjectInfo::AssignedScope () const
 { return _AssignedScope; }


} // namespace Puma

#endif /* __CObjectInfo_h__ */
