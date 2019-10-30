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

#ifndef __CCParser__
#define __CCParser__

#include "Puma/Parser.h"
#include "Puma/CCSyntax.h"
#include "Puma/CCBuilder.h"
#include "Puma/CCSemantic.h"

namespace Puma {


class CCParser : public Parser {
  CCSyntax _syntax;
  CCBuilder _builder;
  CCSemantic _semantic;

  unsigned _options;

public:

  // public parser options
  enum { SKIP_FCT_BODY = 0x01 };

  // parser functions
  CCParser () : Parser (_syntax, _builder, _semantic), 
                _syntax (_builder, _semantic),
                _semantic (_syntax, _builder),
                _options (0) {}

  // parse a unit of a project
  CTranslationUnit *parse (Unit &u, CProject &p, int m = 0, std::ostream& os = std::cout) {
    syntax ().config_skip_fct_body (_options & SKIP_FCT_BODY);
    return Parser::parse (u, p, m, os);
  }

  // set and get parser options
  unsigned Options () const { return _options; }
  void Options (unsigned o) { _options = o; }
  
  CCSyntax &syntax () const { return (CCSyntax&)_syntax; }
  CCBuilder &builder () const { return (CCBuilder&)_builder; }
  CCSemantic &semantic () const { return (CCSemantic&)_semantic; }
};


} // namespace Puma

#endif /* __CCParser__ */
