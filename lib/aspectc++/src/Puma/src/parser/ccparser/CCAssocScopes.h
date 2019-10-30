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

#ifndef __CCAssocScopes__
#define __CCAssocScopes__

// determine classes and namespaces associated with the 
// arguments of an unqualified function call (§3.4.2)

#include "Puma/Array.h"
#include "Puma/ErrorSink.h"

namespace Puma {


class CStructure;
class CT_ExprList;
class CClassInfo;
class CNamespaceInfo;
class CTypeInfo;
class CTypeList;
class CTemplateInfo;
class CT_SimpleName;
class CRecord;
class CTree;

class CCAssocScopes {
  Array<CRecord*>        classes;    // associated classes
  Array<CNamespaceInfo*> namespaces; // associated namespaces 
 
public:
  CCAssocScopes (CT_ExprList * = 0);
  ~CCAssocScopes ();

  void determineAssocScopes (CTypeInfo *);

  // associated classes and namespaces
  unsigned Classes () const;
  unsigned Namespaces () const;
  CRecord *Class (unsigned);
  CNamespaceInfo *Namespace (unsigned);

private:
  void determineAssocScopes (CT_ExprList *);
  void determineAssocScopes (CTemplateInfo *, CTree *);
  void determineAssocScopes (CTypeList *);
  void addNamespace (CNamespaceInfo *, bool add_inline_parent = true);
  void addEnclosingNamespace (CStructure *);
  void addBaseClasses (CClassInfo *);
};

inline CCAssocScopes::CCAssocScopes (CT_ExprList *argument_list) 
 { if (argument_list) determineAssocScopes (argument_list); }
inline CCAssocScopes::~CCAssocScopes ()
 {}

inline unsigned CCAssocScopes::Classes () const
 { return (unsigned)classes.length (); }
inline unsigned CCAssocScopes::Namespaces () const
 { return (unsigned)namespaces.length (); }
inline CRecord *CCAssocScopes::Class (unsigned c)
 { return classes.lookup (c); }
inline CNamespaceInfo *CCAssocScopes::Namespace (unsigned n)
 { return namespaces.lookup (n); }


} // namespace Puma

#endif /* __CCAssocScopes__ */
