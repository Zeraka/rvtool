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

#include "Puma/Unit.h"
#include "Puma/Parser.h"
#include "Puma/TokenStream.h"
#include "Puma/UnitManager.h"
#include "Puma/CSemDatabase.h"
#include "Puma/PreMacroManager.h"
#include "Puma/CTranslationUnit.h"
#include "Puma/PreprocessorParser.h"
#include <iostream>

namespace Puma {


void Parser::configure (Config &c) {
  syntax->configure (c);
  semantic->configure (c);
}

CTranslationUnit *Parser::parse (Unit &unit, CProject &p, int pre_mode, std::ostream& os) {
  CTranslationUnit *tu = new CTranslationUnit (unit, p);

  // configure syntax and semantic object
  configure (p.config ());

  // prepare C preprocessor
  TokenStream stream;           // linearize tokens from several files
  stream.push (&unit);
  p.unitManager ().init ();
  PreprocessorParser cpp (&p.err (), &p.unitManager (), &tu->local_units (), os);
  cpp.macroManager ()->init (unit.name ());
  cpp.stream (&stream);
  cpp.configure (p.config ());

  // prepare semantic analysis
  semantic->init (tu->db (), unit);
  ((ErrorCollector&)builder->err ()).index (0);
  semantic->error_sink (builder->err ());

  // run only C preprocessor
  if (pre_mode > 0) {
    if (pre_mode == 2)
      cpp.silentMode ();
    cpp.parse ();
  } else {
    TokenProvider provider (cpp);        // look ahead token buffer
    tu->tree (syntax->run (provider));
    Token *after_program = provider.current();
    if (after_program) { // error, e.g. too many closing brackets
      p.err() << sev_error << after_program->location()
              << "tokens after end of program (too many closing brackets?)"
              << endMessage;
    }
  }
  tu->cpp_tree (cpp.syntaxTree ());
  builder->errors (p.err ());
  return tu;
}


} // namespace Puma
