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

#ifndef __CCNameLookup_h__
#define __CCNameLookup_h__

#include "Puma/Array.h"
#include "Puma/ErrorSink.h"
#include "Puma/DString.h"
#include "Puma/CStructure.h"
#include <vector>
#include <map>

namespace Puma {


class CT_SimpleName;
class CBaseClassInfo;

class CCNameLookup {
  typedef std::map<CClassInfo*, bool> BaseClassMap;
  
  Array<CObjectInfo*> objects;
  Array<CObjectInfo*> namespaces;
  BaseClassMap        baseclasses;
  CClassInfo*         virtualbase;
  Array<CClassInfo*>  virtualbases;
  Array<CStructure*>  objscopes;
  CT_SimpleName *     sname;
  DString             name;
  ErrorSink &         err;
  unsigned int        pos;
  unsigned char       flags;
  CStructure*         current_scope;
  bool                allow_instantiate;
  
  typedef CStructure::ObjectsByName::iterator          ObjectsIter;
  typedef CStructure::ObjectInfoList::reverse_iterator ObjectListRIter;

public:
  enum Flags { 
    IS_TYPE         = 1, // lookup a type name ignoring all non-types        
    IS_NAMESPACE    = 2, // lookup a namespace name ignoring everything else
    ASS_NS_LOOKUP   = 4, // lookup in associated namespace (overload resolution)
    NO_MEMBER_FCTS  = 8,
    NO_MEMBER_ALIAS = 16,
    IS_FRIEND       = 32 // lookup until nearest non-class scope
  };
 
public:
  CCNameLookup (ErrorSink &, CStructure* curr_scope = 0, bool allow_inst = true);
  
  // usage: lookup(name, scope, nested, base classes & usings)
  void lookup (const DString&, CStructure *, bool = false, bool = false, int = 0);
  void lookup (CT_SimpleName *, CStructure *, bool = false, bool = false);
  void lookupType (const DString&, CStructure *, bool = false, bool = false, int = 0);
  void lookupType (CT_SimpleName *, CStructure *, bool = false, bool = false);
  void lookupNamespace (const DString&, CStructure *, int = 0);
  void lookupNamespace (CT_SimpleName *, CStructure *);
  void lookupInAssNs (const DString&, CStructure *, int = 0);
  void lookupInAssNs (CT_SimpleName *, CStructure *);
  void lookupOpName (const DString&, CStructure *, int = 0);
  void lookupOpName (CT_SimpleName *, CStructure *);
  void lookupFunction (const DString&, CStructure *, int = 0);
  void lookupFunction (CT_SimpleName *, CStructure *);
  void lookupFriendClass (const DString&, CStructure *, int = 0);
  void lookupFriendClass (CT_SimpleName *, CStructure *);
  
  // resulting set of object infos
  unsigned Objects () const;
  CObjectInfo *Object (unsigned = 0) const;
  
  // reset the internal state and reject 
  // the result of previous lookups
  void reset (CStructure* curr_scope = 0, bool allow_inst = true);

private: 
  void lookup (CStructure *, bool, bool);
  void unqualifiedLookup (CStructure *, bool, bool base_usings);
  CObjectInfo *unqualLookup (CStructure *, bool);
  void unqualTypeLookup (CStructure *);
  void lookupUsings (CStructure *);
  bool namespaceVisited (CObjectInfo *);
  void addObject (CObjectInfo *, CStructure *);
  bool invisibleFriend (CObjectInfo *) const;
  bool knownHere (CObjectInfo *) const;
  CStructure *getNameScope (CT_SimpleName *, CStructure *) const;
  CStructure *getNonTemplateScope (CStructure* scope) const;
  bool isDistinctSubObject (CBaseClassInfo *bcinfo);
  void findMostDominant (unsigned long num);
  bool dominates (CObjectInfo *cand1, CObjectInfo *cand2);
  bool isSameSubObject (CClassInfo *ci, CClassInfo *bc1);
  bool isAnonymous () const;
};

inline unsigned CCNameLookup::Objects () const
 { return objects.length (); }
inline CObjectInfo *CCNameLookup::Object (unsigned i) const
 { return objects.lookup (i); }

inline void CCNameLookup::addObject (CObjectInfo *info, CStructure *scope) 
 { objects.append (info); virtualbases.append (virtualbase); 
   objscopes.append (scope); }
inline void CCNameLookup::reset (CStructure* curr_scope, bool allow_inst)
 { objects.reset (); namespaces.reset (); baseclasses.clear (); objscopes.reset ();
   virtualbase = 0; virtualbases.reset (); flags = 0; 
   current_scope = curr_scope; allow_instantiate = allow_inst; }

inline void CCNameLookup::lookup (const DString& sn, CStructure *scope,
 bool nested, bool base_usings, int p) 
 { sname = 0; name = sn; pos = p; if (!isAnonymous()) lookup (scope, nested, base_usings); flags = 0; }

inline void CCNameLookup::lookupType (const DString& sn, 
 CStructure *scope, bool nested, bool base_usings, int p) 
 { flags = IS_TYPE; lookup (sn, scope, nested, base_usings, p); }

inline void CCNameLookup::lookupType (CT_SimpleName *sn, 
 CStructure *scope, bool nested, bool base_usings) 
 { flags = IS_TYPE; lookup (sn, scope, nested, base_usings); }

inline void CCNameLookup::lookupFriendClass (const DString& sn, 
 CStructure *scope, int p) 
 { flags = IS_TYPE | IS_FRIEND; lookup (sn, scope, true, true, p); }

inline void CCNameLookup::lookupFriendClass (CT_SimpleName *sn, 
 CStructure *scope) 
 { flags = IS_TYPE | IS_FRIEND; lookup (sn, scope, true, true); }

inline void CCNameLookup::lookupNamespace (const DString& sn, 
 CStructure *scope, int p) 
 { flags = IS_NAMESPACE; lookup (sn, scope, true, true, p); }

inline void CCNameLookup::lookupNamespace (CT_SimpleName *sn, 
 CStructure *scope) 
 { flags = IS_NAMESPACE; lookup (sn, scope, true, true); }

inline void CCNameLookup::lookupFunction (const DString& sn, 
 CStructure *scope, int p) 
 { flags = NO_MEMBER_ALIAS; lookup (sn, scope, false, false, p); }

inline void CCNameLookup::lookupFunction (CT_SimpleName *sn, 
 CStructure *scope) 
 { flags = NO_MEMBER_ALIAS; lookup (sn, scope, false, false); }

inline void CCNameLookup::lookupInAssNs (const DString& sn, 
 CStructure *scope, int p) 
 { flags = ASS_NS_LOOKUP; lookup (sn, scope, false, false, p); }

inline void CCNameLookup::lookupInAssNs (CT_SimpleName *sn, 
 CStructure *scope) 
 { flags = ASS_NS_LOOKUP; lookup (sn, scope, false, false); }

inline void CCNameLookup::lookupOpName (const DString& sn, 
 CStructure *scope, int p) 
 { flags = NO_MEMBER_FCTS; lookup (sn, scope, true, true, p); }

inline void CCNameLookup::lookupOpName (CT_SimpleName *sn, 
 CStructure *scope) 
 { flags = NO_MEMBER_FCTS; lookup (sn, scope, true, true); }


} // namespace Puma

#endif /* __CCNameLookup_h__ */
