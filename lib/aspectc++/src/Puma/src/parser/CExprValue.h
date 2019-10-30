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

#ifndef __CExprValue_h__
#define __CExprValue_h__

/** \file
 *  Semantic information for constant expression values in the syntax tree. */

#include <iostream>

namespace Puma {


class CTypeInfo;
class CConstant;
class CStrLiteral;
class CWStrLiteral;

/** \class CExprValue CExprValue.h Puma/CExprValue.h
 *  Base class for syntax tree nodes representing 
 *  expressions that can be resolved to a constant value 
 *  (arithmetic constants and string literals). */
class CExprValue {
  CTypeInfo *_type;
  
protected:
  /** Constructor.
   *  \param t The type of the value. */
  CExprValue (CTypeInfo *t) : _type (t) {}
  
public:
  /** Destructor. */
  virtual ~CExprValue () {}

  /** Get the type of the value. */
  CTypeInfo *Type () const { return _type; }
  /** Print the value on the given output stream.
   *  \param out The stream on which to print. */
  virtual void print (std::ostream &out) const = 0;
  
  /** Get a pointer to CConstant if this is an arithmetic constant. */
  virtual CConstant *Constant () const { return (CConstant*)0; }
  /** Get a pointer to CStrLiteral if this is a string literal. */
  virtual CStrLiteral *StrLiteral () const { return (CStrLiteral*)0; }
  /** Get a pointer to CWStrLiteral if this is a wide string literal. */
  virtual CWStrLiteral *WStrLiteral () const { return (CWStrLiteral*)0; }
};

inline std::ostream& operator << (std::ostream &out, const CExprValue &ev) {
  ev.print (out);
  return out;
}

} // namespace Puma

#endif /* __CExprValue_h__ */
