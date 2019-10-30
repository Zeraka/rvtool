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

#ifndef __CLanguage_h__
#define __CLanguage_h__

/** \file
 *  Entity name encoding. */

#include "Puma/DString.h"

namespace Puma {


/** \class CLanguage CLanguage.h Puma/CLanguage.h
 *  Language specific encoding of entity names. The language
 *  is specified using the 'extern' linkage specifier.
 *
 *  Following languages are supported: "C", "C++". C entity 
 *  names are not encoded. C++ entity names usually are encoded
 *  according to the C++ V3 ABI mangling (see 
 *  http://www.codesourcery.com/cxx-abi/abi.html#mangling).
 *
 *  Example: 
 *  \code 
 * void foo(char);             // encoded as: _Z3fooc
 * extern "C" void bar(int);   // encoded as: bar
 *  \endcode */
class CLanguage {
public:
  /** Entity language encoding types. */
  enum LangType {
    /** Language C. */
    LANG_C,           
    /** Language C++. */
    LANG_CPLUSPLUS,   
    /** Neither C nor C++. */
    LANG_OTHER,       
    /** No explicit language encoding. */
    LANG_UNDEFINED    
  };
    
private:
  LangType m_Type;
  DString m_Text;

public:
  /** Constructor. */
  CLanguage () : m_Type (LANG_UNDEFINED), m_Text ("") {}
  
  /** Set the language encoding type.
   *  \param lt The language type. */
  void Type (LangType lt) { 
    m_Type = lt; m_Text = (lt==LANG_C)?"C":(lt==LANG_CPLUSPLUS)?"C++":""; 
  }
  /** Set the language encoding type.
   *  \param lt The language type.
   *  \param txt The language identifier for languages other than C or C++. */
  void Type (LangType lt, const char *txt) { 
    m_Type = lt; m_Text = (lt==LANG_C)?"C":(lt==LANG_CPLUSPLUS)?"C++":(lt==LANG_OTHER)?txt:""; 
  }

  /** Get the language type. */
  LangType Type () const { return m_Type; }
  /** Get the language identifier like "C" or "C++".
   *  \return The language identifier or the empty string. */
  const DString &Text () const { return m_Text; }

  /** Compare two language encodings.
   *  \param lang The language encoding to compare with. */
  bool operator== (const CLanguage &lang) { return (m_Type == lang.Type ()) && (m_Text == lang.Text ()); }
  /** Compare two language encodings.
   *  \param lt The language encoding type to compare with. */
  bool operator== (LangType lt) { return (m_Type == lt); }
};


} // namespace Puma

#endif /* __CLanguage_h__ */
