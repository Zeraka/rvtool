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

#ifndef __CSemDeclarator_h__
#define __CSemDeclarator_h__

/** \file
 *  Semantic declarator analysis. */

#include "Puma/CTypeInfo.h"

namespace Puma {


class CTree;
class ErrorSink;
class CT_SimpleName;
class CT_DeclSpecSeq;


/** \class CSemDeclarator CSemDeclarator.h Puma/CSemDeclarator.h
 *  Class for analysing an entity declarator. The result of the 
 *  analysis is a type and the name of the declared entity 
 *  (name and type of function, object, etc). */
class CSemDeclarator {
  ErrorSink *_err;
  CTypeInfo *_type;
  CT_SimpleName *_name;

public:
  /** Constructor.
   *  \param err The error stream on which to report errors.
   *  \param t The base type of the declared entity (function return type, data type, etc).
   *  \param d The declarator to analyse.
   *  \param lang_c Support language C. */
  CSemDeclarator (ErrorSink *err, CTypeInfo *t, CTree *d, bool lang_c = true);
  /** Destructor. Destroys the resulting type. */
  ~CSemDeclarator () { if (_type) CTypeInfo::Destroy (_type); }
  /** Get the name of the declared entity. */
  CT_SimpleName *Name () const { return _name; }
  /** Get the resulting type. */
  CTypeInfo *Type () const { return _type; }
  /** Set the type of the declared entity. This type will be 
   *  destroyed in the destructor of this class. 
   *  \param t The type. */
  void Type (CTypeInfo *t) { _type = t; }

private:
  void HandleQualifiers (CT_DeclSpecSeq *, bool = false);
};


} // namespace Puma

#endif /* __CSemDeclarator_h__ */
