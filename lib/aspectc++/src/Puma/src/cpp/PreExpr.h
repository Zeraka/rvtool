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

#ifndef __pre_expr__
#define __pre_expr__

// Node class for the preprocessor expression parser. 

#include "Puma/Location.h"
#include "Puma/Limits.h"

namespace Puma {


class ErrorStream;

class PreExpr {
  bool _result;

public:
  long double val;

  // Public constructor.
  PreExpr (ErrorStream *, Location);

  // Do not use these contructors outside of the
  // C preprocessor expression parser.
  PreExpr () {};
  ~PreExpr () {};
  PreExpr (long double v) : val (v) {};

  // Evaluate the given expression string.
  void evaluatePreExpr (char *expression_string);

  // Return value of evaluated expression.
  bool value () const { return _result; }

  // String conversion method.
  static long double convertChar (char *value);

  // Return value converted to unsigned integer.
  U_LONG_LONG toUnsignedInt() { return (U_LONG_LONG)val; }

  // Return value converted to signed integer.
  LONG_LONG toSignedInt() { return (LONG_LONG)val; }
};


} // namespace Puma

#endif /* __pre_expr__ */
