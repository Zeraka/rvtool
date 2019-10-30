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

#ifndef __c_unit_h__
#define __c_unit_h__

/** \file
 *  C/C++ token chain. */

#include "Puma/Unit.h"
#include "Puma/CScanner.h"
#include "Puma/ErrorSink.h"
#include <sstream>

namespace Puma {


/** \class CUnit CUnit.h Puma/CUnit.h
 *  Specialized token unit for C/C++ tokens. Provides
 *  a streaming interface for easy scanning of C/C++
 *  tokens using a Puma::CScanner object. The input
 *  stream is scanned either by calling Puma::CUnit::scan()
 *  or by using the stream manipulator Puma::endu.
 *
 *  Example: 
 *  \code  
 * Puma::ErrorStream es;
 * Puma::CUnit unit(es)
 * unit << "int main() {" << std::endl;
 * unit << "  int fac0 = 1;" << std::endl;
 * for (int i = 1; i < 10; i++) {
 *   unit << "  int fac" << i << " = " << i 
 *        << " * fac" << (i-1) << ";" << std::endl;
 * }
 * unit << "}\n" << Puma::endu;
 *  \endcode */
class CUnit : public Unit, public std::ostringstream {
  CScanner _scanner;

public:
  /** Constructor.
   *  \param err The error stream on which to report errors. */
  CUnit (ErrorSink &err) : _scanner (err) {}
  /** Destructor. */
  virtual ~CUnit () {}
  
  /** Get the scanner used to scan the string stream. */
  CScanner &scanner () const { return (CScanner&)_scanner; }
  /** Scan the stream now. Produces the token chain. */
  void scan ();
};

/** End-of-unit stream manipulator. Triggers scanning the 
 *  C/C++ source code stream. 
 *  \param outs The Puma::CUnit stream object. */
inline std::ostream &endu (std::ostream &outs) {
  ((CUnit&)outs).scan ();
  return outs;
}


} // namespace Puma

#endif /* __c_unit_h__ */
