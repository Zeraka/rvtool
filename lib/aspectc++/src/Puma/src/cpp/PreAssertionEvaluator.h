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

#ifndef __pre_assertion_evaluator__
#define __pre_assertion_evaluator__

// The assertion evaluator provides methods to evaluate an assertion.

#include "Puma/PreprocessorParser.h"

namespace Puma {


class PrePredicateManager;
class PreMacroManager;
class TokenStream;
class ErrorStream;
class Unit;

class PreAssertionEvaluator {
  TokenStream         *_scanner;
  PreMacroManager     *_macroManager;
  PrePredicateManager *_predicateManager;
  ErrorStream         *_err;
  PreprocessorParser  *_parser;

  // Test whether there is an answer for the predicate.
  int checkForAnswer (const char *) const;
        
  // Replace the assertion with the integer constant `1' or `0'.
  void replaceWith (const char *, Unit *) const;

public:
  // Constructor.
  PreAssertionEvaluator (PreprocessorParser &, TokenStream *);    
  ~PreAssertionEvaluator () {}    

  // Evaluate the given assertion. 
  void evaluateAssertion (Unit*) const;
};


} // namespace Puma

#endif /* __pre_assertion_evaluator__ */
