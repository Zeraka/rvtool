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

#ifndef __CStructure_h__
#define __CStructure_h__

/** \file
 *  Semantic information about entities that can contain other entity declarations. */

#include "Puma/CScopeInfo.h"
#include "Puma/Array.h"

#include <map>
#include <set>
#include <list>
#include <string>
#include "Puma/DString.h"

namespace Puma {


class CAttributeInfo;
class CTemplateParamInfo;
class CFunctionInfo;
class CClassInfo;
class CUnionInfo;
class CEnumInfo;
class CTypedefInfo;
class CNamespaceInfo;
class CUsingInfo;
class CMemberAliasInfo;


/** \class CStructure CStructure.h Puma/CStructure.h
 *  Base class of all semantic information classes for
 *  entities that can contain other entity declarations
 *  (like classes, namespaces, functions). */
class CStructure : public CScopeInfo {
public:
  /** List type for semantic objects. */
  typedef std::list<CObjectInfo*> ObjectInfoList;
  /** Map type for entity name to semantic object mapping. */
  typedef std::map<DString,ObjectInfoList> ObjectsByName;
  /** Set type for semantic objects. */
  typedef std::set<CObjectInfo*> ObjectInfoSet;

private:
  CStructure                *_Shared;
  Array<CObjectInfo*>        _Objects; // ALL objects (mixed, in the order added to)
  Array<CAttributeInfo*>     _Attributes;
  Array<CTemplateParamInfo*> _TemplateParams;
  Array<CFunctionInfo*>      _Functions;
  Array<CObjectInfo*>        _Types;
  Array<CUsingInfo*>         _Usings;     
  Array<CRecord*>            _Friends;
  Array<CNamespaceInfo*>     _Namespaces;     
  Array<CMemberAliasInfo*>   _Aliases;     
  ObjectsByName              _ObjectMap;
  ObjectInfoSet              _ObjectSet;

protected:
  /** Constructor.
   *  \param id The object type. */
  CStructure (ObjectId id);

public:
  /** Destructor. */
  ~CStructure ();

  /** Get the entity name to semantic object map. */
  ObjectsByName& ObjectInfoMap();
  /** Get the entity name to semantic object map. */
  const ObjectInfoSet& ObjectInfos() const;

  /** Get the number of contained semantic objects. */
  unsigned Objects () const;
  /** Get the number of contained semantic objects for
   *  entities with the given name. 
   *  \param name The name of the entity. */
  unsigned Objects (const DString& name) const;
  /** Get the number of contained semantic objects for
   *  type declarations. */
  unsigned Types () const;
  /** Get the number of contained semantic objects for
   *  declarations of types with the given name. 
   *  \param name The name of the type. */
  unsigned Types (const DString& name) const;
  /** Get the number of contained semantic objects for
   *  object declarations. */
  unsigned Attributes () const;
  /** Get the number of contained semantic objects for
   *  declarations of objects with the given name. 
   *  \param name The name of the object. */
  unsigned Attributes (const DString& name) const;
  /** Get the number of contained semantic objects for
   *  template parameter declarations. */
  unsigned TemplateParams () const;
  /** Get the number of contained semantic objects for
   *  declarations of template parameters with the given name. 
   *  \param name The name of the template parameter. */
  unsigned TemplateParams (const DString& name) const;
  /** Get the number of contained semantic objects for
   *  function declarations. */
  unsigned Functions () const;
  /** Get the number of contained semantic objects for
   *  declarations of functions with the given name. 
   *  \param name The name of the function. */
  unsigned Functions (const DString& name) const;
  /** Get the number of contained semantic objects for
   *  used members. */
  unsigned Usings () const;
  /** Get the number of contained semantic objects for
   *  used members with the given name. 
   *  \param name The name of the member. */
  unsigned Usings (const DString& name) const;
  /** Get the number of contained semantic objects for
   *  namespace declarations. */
  unsigned Namespaces () const;
  /** Get the number of contained semantic objects for
   *  declarations of namespaces with the given name. 
   *  \param name The name of the namespace. */
  unsigned Namespaces (const DString& name) const;
  /** Get the number of contained semantic objects for
   *  friend declarations. */
  unsigned Friends () const;
  /** Get the number of contained semantic objects for
   *  declarations of friends with the given name. 
   *  \param name The name of the friend. */
  unsigned Friends (const DString& name) const;

  /** Get the n-th contained semantic object. 
   *  \param n The index of the semantic object. */
  CObjectInfo *Object (unsigned n) const;
  /** Get the n-th contained semantic object for
   *  entities with the given name. 
   *  \param name The name of the entity.
   *  \param n Optional index of the semantic object (defaults to 0). */
  CObjectInfo *Object (const DString& name, unsigned n = 0) const;
  /** Get the n-th contained semantic object for a type. 
   *  \param n The index of the semantic object. */
  CObjectInfo *Type (unsigned n) const;
  /** Get the n-th contained semantic object for
   *  types with the given name. 
   *  \param name The name of the type.
   *  \param n Optional index of the semantic object (defaults to 0). */
  CObjectInfo *Type (const DString& name, unsigned n = 0) const;
  /** Get the n-th contained semantic object for
   *  typedefs with the given name. 
   *  \param name The name of the typedef.
   *  \param n Optional index of the semantic object (defaults to 0). */
  CTypedefInfo *Typedef (const DString& name, unsigned n = 0) const;
  /** Get the n-th contained semantic object for a using-directive.
   *  \param n The index of the semantic object. */
  CUsingInfo *Using (unsigned n) const;
  /** Get the n-th contained semantic object for
   *  used members with the given name. 
   *  \param name The name of the member.
   *  \param n Optional index of the semantic object (defaults to 0). */
  CUsingInfo *Using (const DString& name, unsigned n = 0) const;
  /** Get the n-th contained semantic object for a namespace. 
   *  \param n The index of the semantic object. */
  CNamespaceInfo *Namespace (unsigned n) const;
  /** Get the n-th contained semantic object for
   *  namespaces with the given name. 
   *  \param name The name of the namespace.
   *  \param n Optional index of the semantic object (defaults to 0). */
  CNamespaceInfo *Namespace (const DString& name, unsigned n = 0) const;
  /** Get the n-th contained semantic object for a friend. 
   *  \param n The index of the semantic object. */
  CRecord *Friend (unsigned n) const;
  /** Get the n-th contained semantic object for
   *  friends with the given name. 
   *  \param name The name of the friend.
   *  \param n Optional index of the semantic object (defaults to 0). */
  CRecord *Friend (const DString& name, unsigned n = 0) const;
  /** Get the n-th contained semantic object for an object. 
   *  \param n The index of the semantic object. */
  CAttributeInfo *Attribute (unsigned n) const;
  /** Get the n-th contained semantic object for
   *  objects with the given name. 
   *  \param name The name of the object.
   *  \param n Optional index of the semantic object (defaults to 0). */
  CAttributeInfo *Attribute (const DString& name, unsigned n = 0) const;
  /** Get the n-th contained semantic object for a template parameter. 
   *  \param n The index of the semantic object. */
  CTemplateParamInfo *TemplateParam (unsigned n) const;
  /** Get the n-th contained semantic object for
   *  template parameters with the given name. 
   *  \param name The name of the template parameter.
   *  \param n Optional index of the semantic object (defaults to 0). */
  CTemplateParamInfo *TemplateParam (const DString& name, unsigned n = 0) const;
  /** Get the n-th contained semantic object for a function. 
   *  \param n The index of the semantic object. */
  CFunctionInfo *Function (unsigned n) const;
  /** Get the n-th contained semantic object for
   *  functions with the given name. 
   *  \param name The name of the function.
   *  \param n Optional index of the semantic object (defaults to 0). */
  CFunctionInfo *Function (const DString& name, unsigned n = 0) const;
  /** Get the contained semantic object for functions with the 
   *  given name and type. 
   *  \param name The name of the function.
   *  \param type The type of the function. */
  CFunctionInfo *Function (const DString& name, CTypeInfo *type) const;

  /** Check if an alias exists for the given entity in this scope.
   *  \param info The semantic object of the entity. */
  bool isMemberAlias (const CObjectInfo *info) const;
  /** Get the member alias information for the aliased entity.
   *  \param info The semantic object of the entity. */
  CMemberAliasInfo *MemberAlias (const CObjectInfo *info) const;

  /** Set a common data object for the linked semantic objects.
   *  \param share The common data object. */
  void setShared (CStructure *share);
  
  /** Add a semantic object. 
   *  \param info The semantic object. */
  void addObject (CObjectInfo *info);
  /** Add a semantic object for an object.
   *  \param info The semantic object. */
  void addAttribute (CAttributeInfo *info);
  /** Add a semantic object for a template parameter.
   *  \param info The semantic object. */
  void addTemplateParam (CTemplateParamInfo *info);
  /** Add a semantic object for a function.
   *  \param info The semantic object. */
  void addFunction (CFunctionInfo *info);
  /** Add a semantic object for a type.
   *  \param info The semantic object. */
  void addType (CObjectInfo *info);
  /** Add a semantic object for a using-directive.
   *  \param info The semantic object. */
  void addUsing (CUsingInfo *info);
  /** Add a semantic object for a namespace.
   *  \param info The semantic object. */
  void addNamespace (CNamespaceInfo *info);
  /** Add a semantic object for a friend.
   *  \param info The semantic object. */
  void addFriend (CRecord *info);
  
  /** Remove the semantic object.
   *  \param info The semantic object. */
  void removeObject (const CObjectInfo *info);
  /** Remove the semantic object for an object.
   *  \param info The semantic object. */
  void removeAttribute (const CAttributeInfo *info);
  /** Remove the semantic object for a template parameter.
   *  \param info The semantic object. */
  void removeTemplateParam (const CTemplateParamInfo *info);
  /** Remove the semantic object for a function.
   *  \param info The semantic object. */
  void removeFunction (const CFunctionInfo *info);
  /** Remove the semantic object for a type.
   *  \param info The semantic object. */
  void removeType (const CObjectInfo *info);
  /** Remove the semantic object for a using-directive.
   *  \param info The semantic object. */
  void removeUsing (const CUsingInfo *info);
  /** Remove the semantic object for a namespace.
   *  \param info The semantic object. */
  void removeNamespace (const CNamespaceInfo *info);
  /** Remove the semantic object for a friend.
   *  \param info The semantic object. */
  void removeFriend (const CRecord *info);
  /** Remove the semantic object for a member alias.
   *  \param info The semantic object. */
  void removeMemberAlias (const CMemberAliasInfo *info);

  /** Unregister this semantic object at all contained
   *  semantic objects. */
  void removeRegisterEntry ();

  /** Add a semantic object to the name map. 
   *  \warning For internal use only!
   *  \param info The semantic object. */
  void addNamedObject (CObjectInfo* info);
  /** Remove the given semantic object for the entity
   *  with the given name from the name map. 
   *  \warning For internal use only!
   *  \param name The name of the entity. 
   *  \param info The semantic object. */
  void removeNamedObject (const DString& name, CObjectInfo* info);

  /** Create a new semantic object for an object declaration. */
  CAttributeInfo *newAttribute ();
  /** Create a new semantic object for an enumeration. */
  CEnumInfo *newEnum ();
  /** Create a new semantic object for a using-directive. */
  CUsingInfo *newUsing ();
  /** Create a new semantic object for a typedef. */
  CTypedefInfo *newTypedef ();
  /** Create a new semantic object for a member alias. 
   *  \param member The aliased member. 
   *  \param direct_member Add as direct member. */
  CMemberAliasInfo *newMemberAlias (CObjectInfo *member, bool direct_member = false);
  
  /** Remove and destroy the given semantic object for an object. 
   *  \param info The semantic object. */
  void deleteAttribute (const CAttributeInfo *info); 
  /** Remove and destroy the given semantic object for an enumeration. 
   *  \param info The semantic object. */
  void deleteEnum (const CEnumInfo *info);
  /** Remove and destroy the given semantic object for a using-directive. 
   *  \param info The semantic object. */
  void deleteUsing (const CUsingInfo *info);
  /** Remove and destroy the given semantic object for a typedef. 
   *  \param info The semantic object. */
  void deleteTypedef (const CTypedefInfo *info);
  /** Remove and destroy the given semantic object for a member alias. 
   *  \param info The semantic object. */
  void deleteMemberAlias (const CMemberAliasInfo *info);
};

inline CStructure::CStructure (CObjectInfo::ObjectId id) :
  CScopeInfo (id),
  _Shared (this),
  _Objects (5, 20),
  _Attributes (5, 10),
  _TemplateParams (1, 2),
  _Functions (5, 20),
  _Types (5, 20),
  _Usings (1, 20),
  _Friends (1, 10),
  _Namespaces (1, 10),
  _Aliases (1, 20)
 {}
 
inline unsigned CStructure::Objects () const
 { return _Objects.length (); }
inline unsigned CStructure::Types () const
 { return _Types.length (); }
inline unsigned CStructure::Attributes () const
 { return _Attributes.length (); }
inline unsigned CStructure::TemplateParams () const
 { return _TemplateParams.length (); }
inline unsigned CStructure::Functions () const
 { return _Functions.length (); }
inline unsigned CStructure::Usings () const
 { return _Usings.length (); }
inline unsigned CStructure::Namespaces () const
 { return _Namespaces.length (); }
inline unsigned CStructure::Friends () const
 { return _Friends.length (); }
inline CObjectInfo *CStructure::Object (unsigned n) const
 { return _Objects.lookup (n); }
inline CObjectInfo *CStructure::Type (unsigned n) const
 { return _Types.lookup (n); }
inline CAttributeInfo *CStructure::Attribute (unsigned n) const
 { return _Attributes.lookup (n); }
inline CTemplateParamInfo *CStructure::TemplateParam (unsigned n) const
 { return _TemplateParams.lookup (n); }
inline CFunctionInfo *CStructure::Function (unsigned n) const
 { return _Functions.lookup (n); }
inline CUsingInfo *CStructure::Using (unsigned n) const
 { return _Usings.lookup (n); }
inline CNamespaceInfo *CStructure::Namespace (unsigned n) const
 { return _Namespaces.lookup (n); }
inline CRecord *CStructure::Friend (unsigned n) const
 { return _Friends.lookup (n); }

inline bool CStructure::isMemberAlias (const CObjectInfo *info) const
 { return (MemberAlias (info)); }

inline CStructure::ObjectsByName& CStructure::ObjectInfoMap()
 { return _Shared->_ObjectMap; }
inline const CStructure::ObjectInfoSet& CStructure::ObjectInfos() const
 { return _Shared->_ObjectSet; }

} // namespace Puma

#endif /* __CStructure_h__ */
