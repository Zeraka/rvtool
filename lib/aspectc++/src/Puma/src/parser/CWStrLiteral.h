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

#ifndef __CWStrLiteral_h__
#define __CWStrLiteral_h__

/** \file
 *  Wide string literal abstraction. */

#include "Puma/CExprValue.h"
  
namespace Puma {


/** \class CWStrLiteral CWStrLiteral.h Puma/CWStrLiteral.h
 *  Wide string literal abstraction. Holds the wide string
 *  value, its length, and the string type. */
class CWStrLiteral : public CExprValue {
  const wchar_t *_string;
  unsigned long _len;
  
public:
  /** Constructor.
   *  \param s The wide string array. 
   *  \param len The length of the string.
   *  \param t The type of the string. */
  CWStrLiteral (const wchar_t *s, unsigned long len, CTypeInfo *t) : 
    CExprValue (t), _string (s), _len (len) {}
  /** Destructor. Frees the string. */
  virtual ~CWStrLiteral () { if (_string) delete[] _string; }

  /** Print the wide string on the given output stream.
   *  \param out The output stream. */
  virtual void print (std::ostream &out) const { out << "\"??L\""; }
  /** Get the wide string. */
  const wchar_t *String () const { return _string; }
  /** Get the length of the string. */
  unsigned long Length () const { return _len; }

  /** Get this. */
  CWStrLiteral *WStrLiteral () const { return (CWStrLiteral*)this; }
};


} // namespace Puma

#endif /* __CWStrLiteral_h__ */
