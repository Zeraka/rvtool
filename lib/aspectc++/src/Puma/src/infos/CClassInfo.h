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

#ifndef __CClassInfo_h__
#define __CClassInfo_h__

/** \file 
 *  Semantic information about a class. */

#include "Puma/CRecord.h"
#include "Puma/CBaseClassInfo.h"
#include "Puma/CProtection.h"

namespace Puma {

class CTypeClass;


/** \class CClassInfo CClassInfo.h Puma/CClassInfo.h
 *  Semantic information about a class. Note that 'struct's
 *  are ordinary classes where the member access type 
 *  defaults to \e public. Note also that a 'union', 
 *  although syntactically very similar, is not a class 
 *  and thus not represented by CClassInfo. */
class CClassInfo : public CRecord {
  Array<CBaseClassInfo*> _BaseClasses;
  Array<CClassInfo*> _Derived;
  // True if class has dependent base classes
  bool _DepBaseClass;

protected:
  /** Constructor.
   *  \param id The object type. */
  CClassInfo (ObjectId id);

public: 
  /** Constructor. */
  CClassInfo ();
  /** Destructor. If the object type is CObjectInfo::CLASS_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
  ~CClassInfo ();

  /** Get the semantic object of the class definition. 
   *  \see CObjectInfo::DefObject() */
  CClassInfo *DefObject () const;
  /** Get the type information for the class. */
  CTypeClass *TypeInfo () const;

  /** Get the number of the base classes of the class. */
  unsigned BaseClasses () const;
  /** Get the number of classes derived from the class. */
  unsigned DerivedClasses() const;
  /** Get the n-th base class. 
   *  \param n The index of the base class. 
   *  \return The base class or if \e n is not valid. */
  CBaseClassInfo *BaseClass (unsigned n) const;
  /** Get the base class with the given name. 
   *  \param name The name of the base class.
   *  \return The base class or NULL if there is no base 
   *          class with the given name. */
  CBaseClassInfo *BaseClass (const char *name) const;
  /** Get the n-th derived class. 
   *  \param n The index of the derived class. 
   *  \return The derived class or NULL if \e n is not valid. */
  CClassInfo *DerivedClass (unsigned n) const;
  /** Get the derived class with the given name. 
   *  \param name The name of the derived class.
   *  \return The derived class or NULL if there is no derived
   *          class with the given name. */
  CClassInfo *DerivedClass (const char *name) const;

  /** Check if the given class is a base class of this class.
   *  \param base The base class.
   *  \param recursive If \e false then only the direct base classes
   *                   are considered. */
  bool isBaseClass (const CClassInfo *base, bool recursive = false) const;
  /** Check if the given class is derived from this class.
   *  \param dc The derived class.
   *  \param recursive If \e false then only the directly derived classes
   *                   are considered. */
  bool isDerivedClass (const CClassInfo *dc, bool recursive = false) const;
  /** Set that this class has a base class that depends on template 
   *  parameters. 
   *  \param depends \e true if there is a dependent base class. */
  void hasDepBaseClass (bool depends);
  /** Check if this class has a base class that depends on 
   *  template parameters. */
  bool hasDepBaseClass () const;

  /** Add a base class to this class.
   *  \param bc The base class. */
  void addBaseClass (CBaseClassInfo *bc);
  /** Add a derived class to this class.
   *  \param dc The derived class. */
  void addDerivedClass (CClassInfo *dc);
  /** Remove the given base class from this class.
   *  \param bc The base class. */
  void removeBaseClass (const CBaseClassInfo *bc);
  /** Remove the given base class from this class.
   *  \param bc The base class. */
  void removeBaseClass (const CClassInfo *bc);
  /** Remove the given derived class from this class.
   *  \param dc The derived class. */
  void removeDerivedClass (const CClassInfo *dc);

  /** Create a new base class semantic object.
   *  \param bc The semantic information about the of the base class
   *            or NULL to create an empty base class semantic object. */
  CBaseClassInfo *newBaseClass (CClassInfo *bc = 0);

  /** Get the accessibility of a particular member or base class member.
   *  \param oi The semantic object for the member. */
  CProtection::Type Accessibility (CObjectInfo *oi) const;

  /** Check if the given function overrides any virtual function
   *  defined in this class or any of its base classes.
   *  \param fi The function. */
  bool overridesVirtual (const CFunctionInfo *fi) const;

  /** Check if the class is declared using keyword 'struct'. */
  bool isStruct () const;
  /** Check if this class is an aggregate according to 
   *  paragraph 8.5.1-1 of the ISO C++ standard. */
  bool isAggregate () const;
  /** Yields true if the class has no non-static data members other than
   *  bit-fields of length 0, no virtual member functions, no virtual
   *  base classes, and no base class B for which isEmpty() is false.
   *  \return true if empty class. */
  bool isEmpty () const;
  /** Yields true if the class declares or inherits a virtual function. */
  bool isPolymorphic () const;
  /** Yields true if the class has a pure virtual function. */
  bool isAbstract () const;
  /** Yields true if the class has a virtual destructor. */
  bool hasVirtualDtor () const;
};

inline CClassInfo::CClassInfo (ObjectId id) :
  CRecord (id),
  _BaseClasses (1, 2),
  _Derived (1, 2),
  _DepBaseClass (false)
 {}
inline CClassInfo::CClassInfo () :
  CRecord (CObjectInfo::CLASS_INFO),
  _BaseClasses (1, 2),
  _Derived (1, 2),
  _DepBaseClass (false)
 {}
 
inline CTypeClass *CClassInfo::TypeInfo () const
 { return (CTypeClass*)CObjectInfo::TypeInfo (); }

inline unsigned CClassInfo::BaseClasses () const
 { return _BaseClasses.length (); }
inline unsigned CClassInfo::DerivedClasses () const
 { return _Derived.length (); }
inline CBaseClassInfo *CClassInfo::BaseClass (unsigned n) const
 { return _BaseClasses.lookup (n); }
inline CClassInfo *CClassInfo::DerivedClass (unsigned n) const
 { return _Derived.lookup (n); }

inline bool CClassInfo::hasDepBaseClass () const
 { return _DepBaseClass; }
inline void CClassInfo::hasDepBaseClass (bool v)
 { _DepBaseClass = v; }

} // namespace Puma

#endif /* __CClassInfo_h__ */
