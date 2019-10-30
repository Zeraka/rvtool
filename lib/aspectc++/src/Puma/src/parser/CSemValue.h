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

#ifndef __CSemValue_h__
#define __CSemValue_h__

/** \file
 *  Semantic value information for the syntax tree. */

#include "Puma/CExprValue.h"
#include "Puma/CTypeInfo.h"

namespace Puma {


/** \class CSemValue CSemValue.h Puma/CSemValue.h
 *  Semantic information object about values in the syntax 
 *  tree. Provides the value and type of an expression or
 *  entity (name). */
class CSemValue {
  bool value_ref;
  bool type_ref;

protected:
  /** The semantic value information object. */
  CExprValue *value;
  /** The type information object. */
  CTypeInfo *type;

public:
  /** Constructor. */
  CSemValue ();
  /** Destructor. Frees the type and value. */
  virtual ~CSemValue ();

  /** Set the semantic value information object.
   *  Will be freed in the destructor.
   *  \param v The value info object. */
  void setValue (CExprValue *v);
  /** Set the semantic value information object as reference.
   *  Will not be destroyed in the destructor.
   *  \param v The value info object. */
  void setValueRef (CExprValue *v);
  /** Set the type information object.
   *  Will be freed in the destructor.
   *  \param t The type info object. */
  void setType (CTypeInfo *t);
  /** Set the type information object as reference.
   *  Will not be destroyed in the destructor.
   *  \param t The type info object. */
  void setTypeRef (CTypeInfo *t);
  /** Get the semantic information about the value. */
  CExprValue* getValue() { return value; }
  /** Get the type of the value. */
  CTypeInfo* getType() { return type; }
};

inline CSemValue::CSemValue () : 
  value_ref (false),
  type_ref (true),
  value ((CExprValue*)0), 
  type (&CTYPE_UNDEFINED)
 {}
inline CSemValue::~CSemValue () 
 { if (value && ! value_ref) delete value; 
   if (type && ! type_ref) CTypeInfo::Destroy (type); }

inline void CSemValue::setValue (CExprValue *v) 
 { if (value && ! value_ref) delete value; 
   value = v; value_ref = false; }
inline void CSemValue::setValueRef (CExprValue *v) 
 { if (value && ! value_ref) delete value; 
   value = v; value_ref = true; }

inline void CSemValue::setType (CTypeInfo *t) 
 { if (type && ! type_ref) CTypeInfo::Destroy (type); 
   type = t; type_ref = false; }
inline void CSemValue::setTypeRef (CTypeInfo *t) 
 { if (type && ! type_ref) CTypeInfo::Destroy (type); 
   type = t; type_ref = true; }


} // namespace Puma

#endif /* __CSemValue_h__ */
