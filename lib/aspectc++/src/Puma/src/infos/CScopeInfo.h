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

#ifndef __CScopeInfo_h__
#define __CScopeInfo_h__

/** \file
 *  Semantic information about a scope. */

#include "Puma/CObjectInfo.h"
#include "Puma/Array.h"

namespace Puma {


class CLocalScope;
class CFunctionInfo;
class CNamespaceInfo;
class CClassInfo;
class CUnionInfo;
class CTemplateInfo;

/** \class CScopeInfo CScopeInfo.h Puma/CScopeInfo.h
 *  Semantic information about a scope. Several syntactic
 *  constructs have its own scope, such as class definitions,
 *  functions, and compound statements. */
class CScopeInfo : public CObjectInfo {
  Array<CScopeInfo*> _Children; // child scopes
  CScopeInfo *_Parent; // parent scope
 
protected:
  /** Constructor.
   *  \param id The object type. */
  CScopeInfo (ObjectId id);

public:
  /** Destructor. */
  ~CScopeInfo ();

  /** Get the number of child scopes (nested scopes). */
  unsigned Children () const;
  /** Get the n-th child scope.
   *  \param n The index of the child scope.
   *  \return The n-th child scope or NULL. */
  CScopeInfo *Child (unsigned) const;
  /** Get the parent scope of this scope. The top
   *  scope is the file scope. 
   *  \return The parent scope, or this scope if no parent. */
  CScopeInfo *Parent () const;
  /** Check if this is a file (file scope). */
  bool isFile () const;
  /** Check if this is namespace. */
  bool isNamespace () const;
  /** Check if this is class or union. */
  bool isRecord () const;
  /** Check if this is a class. */
  bool isClass () const;
  /** Check if this is a union. */
  bool isUnion () const;
  /** Check if this is a function. */
  bool isFunction () const;
  /** Check if this is a class method. */
  bool isMethod () const;
  /** Check if this is a class template. */
  bool isClassTemplate () const;
  /** Check if this is a function template. */
  bool isFctTemplate () const;
  /** Check if this is a local scope. */
  bool isLocalScope () const;
  /** Check if this is a local class or union. */
  bool isLocalRecord () const;
  /** Check if this is a local class. */
  bool isLocalClass () const;
  /** Check if this is a local union. */
  bool isLocalUnion () const;
  /** Check if this is the global (file) scope. */
  bool GlobalScope () const;
  
  /** Check if this scope is inside a class or 
   *  function template scope. */
  bool insideTemplate () const;
  /** Check if this scope is inside a class or
   *  union scope. */
  bool insideRecord () const;
  /** Check if this scope is inside a class scope. */
  bool insideClass () const;
  /** Check if this scope is inside a union scope. */
  bool insideUnion () const;
  /** Check if this scope is inside a function scope. */
  bool insideFunction () const;
  /** Check if this scope is inside a class method scope. */
  bool insideMethod () const;

  /** Set the parent scope of this scope.
   *  \param scope The parent scope. */
  void Parent (const CScopeInfo *scope);
  /** Add a child scope to this scope.
   *  \param scope The child scope. */
  void addChild (CScopeInfo *scope);
  /** Remove a child scope.
   *  \param scope The child scope. */
  void removeChild (const CScopeInfo *scope);
  
  /** Create a new local scope information object. */
  CLocalScope *newLocalScope ();
  /** Create a new function or function template 
   *  instance semantic object. 
   *  \param inst \e true if to a function template instance. */
  CFunctionInfo *newFunction (bool inst = false);
  /** Create a new class or class template 
   *  instance semantic object. 
   *  \param inst \e true if to a class template instance. */
  CClassInfo *newClass (bool inst = false);
  /** Create a new union or union template 
   *  instance semantic object. 
   *  \param inst \e true if to a union template instance. */
  CUnionInfo *newUnion (bool inst = false);
  /** Create a new namespace semantic object. */
  CNamespaceInfo *newNamespace ();
  /** Create a new template semantic object. */
  CTemplateInfo *newTemplate ();

  /** Delete the given local scope semantic object. 
   *  \param scope The local scope. */
  void deleteLocalScope (const CLocalScope *scope);
  /** Delete the given function semantic object.
   *  \param fct The function. */
  void deleteFunction (const CFunctionInfo *fct);
  /** Delete the given class semantic object.
   *  \param c The class. */
  void deleteClass (const CClassInfo *c);
  /** Delete the given union semantic object.
   *  \param u The union. */
  void deleteUnion (const CUnionInfo *u);
  /** Delete the given namespace semantic object. 
   *  \param ns The namespace. */
  void deleteNamespace (const CNamespaceInfo *ns);
  /** Delete the given template semantic object.
   *  \param tpl The template. */
  void deleteTemplate (const CTemplateInfo *tpl);
};


inline CScopeInfo::CScopeInfo (CObjectInfo::ObjectId id) :
  CObjectInfo (id),
  _Children (1, 20),
  _Parent ((CScopeInfo*)this)
 {}
 
inline unsigned CScopeInfo::Children () const
 { return _Children.length (); }
inline CScopeInfo *CScopeInfo::Child (unsigned n) const
 { return _Children.lookup (n); }
inline CScopeInfo *CScopeInfo::Parent () const
 { return _Parent; }

inline bool CScopeInfo::isLocalScope () const
 { return LocalScope (); }
inline bool CScopeInfo::isFile () const 
 { return FileInfo (); }
inline bool CScopeInfo::isNamespace () const 
 { return NamespaceInfo (); }
inline bool CScopeInfo::isRecord () const 
 { return Record (); }
inline bool CScopeInfo::isClass () const 
 { return ClassInfo (); }
inline bool CScopeInfo::isUnion () const 
 { return UnionInfo (); }
inline bool CScopeInfo::isFunction () const 
 { return FunctionInfo (); }
inline bool CScopeInfo::GlobalScope () const 
 { return FileInfo (); }

inline bool CScopeInfo::insideFunction () const 
 { return isFunction () || (Parent () && Parent () != this && 
                            Parent ()->insideFunction ()); }
inline bool CScopeInfo::insideMethod () const 
 { return isMethod () || (Parent () && Parent () != this && 
                          Parent ()->insideMethod ()); }
inline bool CScopeInfo::insideRecord () const 
 { return isRecord () || (Parent () && Parent () != this && 
                          Parent ()->insideRecord ()); }
inline bool CScopeInfo::insideTemplate () const 
 { return isTemplate () || (Parent () && Parent () != this && 
                            Parent ()->insideTemplate ()); }
inline bool CScopeInfo::insideClass () const 
 { return isClass () || (Parent () && Parent () != this && 
                         Parent ()->insideClass ()); }
inline bool CScopeInfo::insideUnion () const 
 { return isUnion () || (Parent () && Parent () != this && 
                         Parent ()->insideUnion ()); }

inline bool CScopeInfo::isLocalClass () const 
 { return isClass () && insideFunction (); }
inline bool CScopeInfo::isLocalUnion () const 
 { return isUnion () && insideFunction (); }
inline bool CScopeInfo::isLocalRecord () const 
 { return isRecord () && insideFunction (); }


} // namespace Puma

#endif /* __CScopeInfo_h__ */
