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

#ifndef __Parser__
#define __Parser__

/** \file
 *  Generic parser abstraction. */

#include "Puma/Unit.h"
#include "Puma/Syntax.h"
#include "Puma/Builder.h"
#include "Puma/Semantic.h"
#include "Puma/CProject.h"
#include <iostream>

namespace Puma {


class CTranslationUnit;


/** \class Parser Parser.h Puma/Parser.h
 *  Generic parser abstraction. Setups the parser components
 *  ready to be used for parsing an input file (see class
 *  Puma::Syntax, Puma::Builder, and Puma::Semantic). 
 *
 *  The result of parsing a source file is the so-called 
 *  translation unit (see class Puma::CTranslationUnit). It
 *  encapsulates the result of the syntactic and semantic
 *  analyses (syntax tree, semantic information database, 
 *  preprocessor tree). */
class Parser {
  Syntax *syntax;
  Builder *builder;
  Semantic *semantic;
  
public:
  /** Constructor.
   *  \param x The syntax analysis component.
   *  \param b The syntax tree builder.
   *  \param s The semantic analysis component. */
  Parser (Syntax &x, Builder &b, Semantic &s) :
    syntax (&x), builder (&b), semantic (&s) {}

  /** Parse the given input file. Supports different preprocessing
   *  modes. 0 means to pass the preprocessed tokens to the parser. 
   *  1 means to print the preprocessed tokens on stdout and to not 
   *  parse at all. Mode 2 means the same as mode 1 except that the
   *  preprocessed tokens are not printed to stdout.
   *  \param file The input file.
   *  \param p The source project information.
   *  \param pre_mode The preprocessing mode (0-2).
   *  \param os Preprocessor output stream if mode is 1.
   *  \return A newly created object containing the parse results.
   *          To be destroyed by the caller. */
  CTranslationUnit *parse (Unit &file, CProject &p, int pre_mode = 0, std::ostream& os = std::cout);

  /** Configure the parser components. Calls the corresponding 
   *  configure methods of the parser components.
   *  \param c The configuration settings. */
  void configure (Config &c);
};


} // namespace Puma

#endif /* __Parser__ */
