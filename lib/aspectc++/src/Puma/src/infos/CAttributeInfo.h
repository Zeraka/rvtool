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

#ifndef __CAttributeInfo_h__
#define __CAttributeInfo_h__

/** \file 
 *  Semantic information about local/global objects and class data members. */

#include "Puma/CScopeRequest.h"

namespace Puma {


class CT_InitDeclarator;
class CT_ExprList;
class CTemplateInfo;
class CRecord;
class CNamespaceInfo;
class CTree;

/** \class CAttributeInfo CAttributeInfo.h Puma/CAttributeInfo.h
 *  Semantic information about a local or global object or a 
 *  class data member. */
class CAttributeInfo : public CScopeRequest {
  CTemplateInfo *_TemplateInfo;
  CTree *_Init;

protected:
  /** Constructor.
   *  \param id The object type. */
  CAttributeInfo (ObjectId id);

public:
  /** Constructor. */
  CAttributeInfo ();
  /** Destructor. If the object type is CObjectInfo::ATTRIBUTE_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
  ~CAttributeInfo ();

  /** Get the semantic object for the object definition. 
   *  \see CObjectInfo::DefObject() */
  CAttributeInfo *DefObject () const;
  /** Get the syntax tree node of the object. */
  CT_InitDeclarator *Tree () const;
  /** Get the syntax tree node of the initializer. */
  CTree *Init ();
  /** Set the syntax tree node of the initializer. */
  void Init (CTree*);

  /** Get the template information object if this is a static 
   *  data member of a class template.
   *  \return The template information object or NULL. */
  CTemplateInfo *TemplateInfo () const;
  /** Set the template information object of a static data 
   *  member of a class template.
   *  \param info The semantic object of the template. */
  void TemplateInfo (CTemplateInfo *info);

  /** Get the class or union if this is a data member of a class or union. 
   *  \return The class or union, or NULL if not a member. */
  CRecord *Record () const;
  /** Get the namespace if the object is declared in a namespace. 
   *  \return The namespace or NULL if not declared in a namespace. */
  CNamespaceInfo *Namespace () const;
  /** Set the qualification scope of the object. This can be a namespace
   *  for objects declared in a namespace or a class/union for data 
   *  members of a class/union.
   *  \param scope The qualification scope. */
  void QualifiedScope (CStructure *scope);
};

inline CAttributeInfo::CAttributeInfo () :
  CScopeRequest (CObjectInfo::ATTRIBUTE_INFO),
  _TemplateInfo ((CTemplateInfo*)0),
  _Init (0)
 {}
inline CAttributeInfo::CAttributeInfo (CObjectInfo::ObjectId id) :
  CScopeRequest (id),
  _TemplateInfo ((CTemplateInfo*)0),
  _Init (0)
 {}

inline CTemplateInfo *CAttributeInfo::TemplateInfo () const
 { return _TemplateInfo; }
inline void CAttributeInfo::TemplateInfo (CTemplateInfo *info) 
 { _TemplateInfo = info; }

inline void CAttributeInfo::QualifiedScope (CStructure *s)
 { _QualScope = s; }

inline void CAttributeInfo::Init (CTree* init)
 { _Init = init; }

} // namespace Puma

#endif /* __CAttributeInfo_h__ */
