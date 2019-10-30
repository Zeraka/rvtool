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

#include "Puma/PreAssertionEvaluator.h"
#include "Puma/PrePredicateManager.h"
#include "Puma/PreprocessorParser.h"
#include "Puma/PreMacroManager.h"
#include "Puma/PreParserState.h"
#include "Puma/PrePredicate.h"
#include "Puma/TokenStream.h"
#include "Puma/ErrorStream.h"
#include "Puma/CScanner.h"
#include "Puma/CTokens.h"
#include "Puma/PreMacro.h"
#include "Puma/Token.h"
#include "Puma/Unit.h"
#include <sstream>
#include <string.h>

namespace Puma {


PreAssertionEvaluator::PreAssertionEvaluator (PreprocessorParser &parser,
                                              TokenStream *stream) {
  _scanner          = stream;
  _macroManager     = parser.macroManager ();
  _predicateManager = parser.predicateManager ();
  _err              = parser.parserState ()->err;
  _parser           = &parser;
}


// Evaluate an assertion.
void PreAssertionEvaluator::evaluateAssertion (Unit *unit) const {
  if (! unit) return;

  Token        *token;
  PrePredicate *predicate = 0;

  // Skip white spaces and comments after the `#'.
  while ((token = _scanner->next ())) 
    if (! token->is_whitespace () && ! token->is_comment ())
      break;
    
  // Now get the predicate identifier.
  if (token) {
    if (! token->is_identifier ())
      token = (Token*) 0;
    else
      predicate = _predicateManager->getPredicate (token->text ());
  }
    
  // No identifier found -> parse error.
  if (! token) {
    *_err << (_macroManager->getMacro ("defined"))->location ()
          << sev_error << "Missing predicate after `#'" 
          << endMessage; 
    replaceWith ("0", unit);
    return;
  }

  // Now look for a left parenthesis, an answer text and a right 
  // parenthesis like `(answer)'. If there really is an answer 
  // return its size.
  int size = checkForAnswer (token->text ());
    
  // There is no answer so our task is only to test whether the
  // given predicate exists. Depending on the result of this test 
  // the assertion will be replaced by the integer constant `0' 
  // or `1'.
  if (size == 0) {
    if (predicate) 
      replaceWith ("1", unit);
    else                
      replaceWith ("0", unit);
    return;
  }

  // There is an unterminated answer and that has to be skipped.
  if (size == -1) {
    while ((token = _scanner->next ()));
      replaceWith ("0", unit);
    return;
  }

  // Skip white spaces and comments.
  while ((token = _scanner->next ())) 
    if (token->is_open ())
      break;

  // Collect the answer text.
  std::ostringstream answer;
  while ((token = _scanner->next ())) {
    if (token->is_close ())
      break;
    answer << token->text ();
  }

  // Now we have a predicate and a answer for that predicate. 
  // If the predicate exists and if the answer is valid replace 
  // the assertion with a `1' else with `0'.
  if (predicate) {
    if (predicate->isAnswer (answer.str ().c_str ()))
      replaceWith ("1", unit);
    else    
      replaceWith ("0", unit);
  } else
    replaceWith ("0", unit);
}


// Replace the assertion with the integer constant `1' or `0'.
void PreAssertionEvaluator::replaceWith (const char *value, Unit *unit) const {
  _parser->cscanner ().fill_unit (value, *unit);
}


// Test whether there is an answer for the predicate.
int PreAssertionEvaluator::checkForAnswer (const char *predicate) const {
  int size = 0;
  int pos = _scanner->length () - 1;

  // Get the top token source.
  UnitTokenSrc *src = _scanner->lookup (pos);
    
  // Get the unit and the current token.
  Token *token = src->current ();
  Unit  *unit  = src->unit ();

  // Skip white spaces and comments.
  do {
    while (! (token = (Token*) unit->next (token))) {
      if (! ((--pos >= 0) && (src = _scanner->lookup (pos)))) 
        return 0;
      unit  = src->unit ();
      token = src->current ();
    }
  } while (token->is_whitespace () || token->is_comment ());

  // Look for the left parenthesis.
  if (! token->is_open ())
    return 0;

  // Look for the right parenthesis and count the answer size.
  do {
    while (! (token = (Token*) unit->next (token))) {
      if (! ((--pos >= 0) && (src = _scanner->lookup (pos)))) {
        // There is no token left to scan. A left parenthesis does 
        // not exist. That isn't allowed and causes a parse error.
        *_err << (_macroManager->getMacro ("defined"))->location ()
              << sev_error << "Missing `)' after answer for " 
              << "predicate `" << predicate << "'" << endMessage; 
        return -1;
      }
      unit  = src->unit ();
      token = src->current ();
    }
    size += strlen (token->text ());
        
    // Left parenthesis aren't allowed within the answer text. 
    if (token->is_open ()) {
      *_err << (_macroManager->getMacro ("defined"))->location ()
            << sev_error << "Missing `)' after answer for " 
            << "predicate `" << predicate << "'" << endMessage; 
      return -1;
    }
  } while (! token->is_close ());
             
  // Can only be reached if an answer was found.
  return size;
}


} // namespace Puma
