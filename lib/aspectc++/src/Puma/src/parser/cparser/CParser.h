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

#ifndef __CParser__
#define __CParser__

/** \file
 *  C parser abstraction. */

#include "Puma/Parser.h"
#include "Puma/CSyntax.h"
#include "Puma/CBuilder.h"
#include "Puma/CSemantic.h"

namespace Puma {


/** \class CParser CParser.h Puma/CParser.h
 *  C parser abstraction. Setups the C parser components
 *  ready to be used for parsing C input files (see class
 *  Puma::CSyntax, Puma::CBuilder, and Puma::CSemantic). */
class CParser : public Parser {
  CSyntax _syntax;
  CBuilder _builder;
  CSemantic _semantic;
  
public:
  /** Constructor. */
  CParser () : Parser (_syntax, _builder, _semantic), 
               _syntax (_builder, _semantic),
               _semantic (_syntax, _builder) {}

  /** Get the C syntactic analysis object. */
  CSyntax &syntax () const { return (CSyntax&)_syntax; }
  /** Get the C tree builder object. */
  CBuilder &builder () const { return (CBuilder&)_builder; }
  /** Get the C semantic analysis object. */
  CSemantic &semantic () const { return (CSemantic&)_semantic; }
};


} // namespace Puma

#endif /* __CParser__ */
