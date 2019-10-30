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
#include "Puma/PreprocessorParser.h"
#include "Puma/PreMacroExpander.h"
#include "Puma/PreMacroOpTokens.h"
#include "Puma/PreMacroManager.h"
#include "Puma/PreParserState.h"
#include "Puma/StringSource.h"
#include "Puma/ErrorStream.h"
#include "Puma/TokenStream.h"
#include "Puma/UnitManager.h"
#include "Puma/CScanner.h"
#include "Puma/CTokens.h"
#include "Puma/PreMacro.h"
#include "Puma/StrCol.h"
#include "Puma/Token.h"
#include "Puma/MacroUnit.h"
#include <sstream>
#include <string.h>
#include <stdio.h>

namespace Puma {


PreMacroExpander::PreMacroExpander (PreprocessorParser* parser)
 {
    _preexpander = new PreMacroExpander ();
    _in_include = false;
    reset (parser);
 }


PreMacroExpander::PreMacroExpander ()
 {
    _in_include = false;
    _preexpander = (PreMacroExpander*) 0;
 }


// Reset the expander to the initial state.
void PreMacroExpander::reset (PreprocessorParser* parser)
 {
    if (! _preexpander)
        _preexpander  = new PreMacroExpander ();
    _macroManager = parser->macroManager ();
    _unitManager  = parser->locals ();
    _macroStack   = parser->macroStack ();
    _err          = parser->parserState ()->err;
    _parser       = parser;
    _in_include   = false;
 }


// Reset the expander to a special initial state.
void PreMacroExpander::reset (PreprocessorParser* parser,
                              Array<PreMacro*>* stack)
 {
    reset (parser);
    _macroStack = stack;
 }


// Prescan an argument.
Unit& PreMacroExpander::prescanArgument (Unit* arg) const
 {
    int i;

    // Create a new macro stack.
    Array<PreMacro*> macroStack;

    // Copy the old macro stack to the new macro stack.
    for (i = 0; i < _macroStack->length (); i++)
        macroStack.append (_macroStack->get (i));

    // Create a new macro expander for the argument, to expand
    // the macros of the argument unit separately.
    _preexpander->reset (_parser, &macroStack);

    // Expand the macros and get an unit as result again.
    Unit *unit = _preexpander->expandMacros (arg);

    // Ensure that the macros on the macro stack will not
    // be deleted.
    for (i = 0; i < macroStack.length (); i++)
        macroStack[i] = (PreMacro*) 0;

    return *unit;
 }


// Expand all macros of the given unit.
Unit* PreMacroExpander::expandMacros (Unit* expandUnit)
 {
    if (! expandUnit) return (Unit*) 0;

    Token*         token;
    PreMacro*      macro;
    Unit*          unit = new MacroUnit (); // Create an empty unit.

    PreParserState state;                  // Create a new parser state object.
    TokenStream    stream;                // Create a new token stream object.

    PreAssertionEvaluator evaluator (*_parser, &stream);

    // If the special macro `defined' is defined we want to expand
    // all macros of an expression and at the same time there are
    // some special things to observe.
    bool isExpression = _macroManager->getMacro ("defined") ? true : false;

    // Ensure that we scan for tokens from our own token stream.
    _scanner = &stream;

    // Push the unit to expand on stream.
    pushUnit (stream, expandUnit);

    // Scan the unit for macros and assertions.
    while ((token = stream.next ()))
    {
        // Recognize end of macro expansion and adjust macro stack accordingly.
        state.updateMacroStack (*_macroStack, stream.length ());

        // An assertion was found and will be evaluated.
        if (token->type () == TOK_MO_HASH && isExpression && token->is_macro_op ())
            evaluator.evaluateAssertion (unit);

        // Search for C++ identifiers and check whether it is a macro.
        else if (token->is_core ())
        {
            if ((macro = _macroManager->getMacro (token->dtext ())))
            {
                // Check whether a macro is allowed to expand.
                if (! state.isSelfReferentialMacro (macro, *_macroStack))
                {
                    // Try to expand the macro.
                    if (expandMacro (macro, token))
                    {
                        // Remember the expanded macro to prevent for
                        // expanding the macro twice.
                        state.putMacroOnStack (*_macroStack, macro, stream.length ());

                        // Scan the next token.
                        continue;
                    }
                }
            }

            // Identifier isn't a macro call. So append it unchanged to
            // return unit.
            if (! token->is_identifier () || ! isExpression)
                unit->append (*((Token*) token->duplicate ()));

            // But if the special macro `defined' is defined we are
            // expanding macros in an expression and therefore every
            // identifier that is not a macro has to be replaced by a
            // single 0.
            else
                _parser->cscanner ().fill_unit ("0", *unit);
        }

        // Every other token will be putted unchanged into the
        // return unit.
        else
            unit->append (*((Token*) token->duplicate ()));
    }

    // Recognize end of macro expansion and adjust macro stack accordingly.
    state.updateMacroStack (*_macroStack, stream.length ());

    // Return the macro expanded unit.
    return unit;
 }


// Expand all macros of the given string.
char* PreMacroExpander::expandMacros (const char* string)
 {
    if (! string) return (char*) 0;

    // To expand macros within a string we simply build an unit from
    // the string and call the method to expand macros within an unit
    // with that very unit as argument.
    Unit *in = buildUnit (string);
    Unit *out = expandMacros (in);

    // Now we have a macro expanded unit and want to return a string
    // again. So we build a string from that unit and return it.
    char *result = (out ? out->toString () : 0);
    delete in;
    delete out;

    return result;
 }


// Check whether there really is a function-like macro call.
bool PreMacroExpander::checkForGoOn (bool mode) const
 {
    // Here it is not possible to call next() to get the next tokens to
    // check whether this is really a macro call or something else, due to
    // next() returns a token from the stream and cannot go back to this
    // token at end of the function. But we have to reach the tokens again
    // to evaluate the macro call if it is one. So we have to go through
    // the unit independently.
    long pos = _scanner->length () - 1;

    // Get the top token source.
    UnitTokenSrc *src = _scanner->lookup (pos);

    // Get the unit and the current token.
    Token* token = src->current ();
    Unit*  unit  = src->unit ();

    // Skip white spaces and comments.
    do {
        // If the end of an unit is reached we have to go to the next
        // unit on stack and scan that unit too.
        while (! (token = (Token*) unit->next (token)))
        {
                // If there is no unit left on stack, it's not a macro call.
            if (! ((--pos >= 0) && (src = _scanner->lookup (pos))))
                return false;
            unit  = src->unit ();
            token = src->current ();
        }
    } while (token->is_whitespace () || token->is_comment ());

    if (token->is_core ())
    {
        // If there is a left parenthesis it is a function-like macro.
        if (mode) {
            if (token->is_open ())
                return true;
        }

        // This is the special `defined' mode. After the `defined' keyword
        // there can be either a left parenthesis OR immediatelly an identifier.
        // This is the case of an identifier on which we also return true.
        else if (token->is_identifier ())
            return true;
    }

    // It was not a call to a funtion-like macro.
    return false;
 }


// Build a unit from a string.
Unit* PreMacroExpander::buildUnit (const char* string) const
 {
    // Create the component scanner and an empty unit.
    Unit *unit = new MacroUnit ();

    // Allow macro concatenation and stringification operators
    _parser->cscanner ().allow_macro_ops ();

    // Skip leading white spaces.
    while (StrCol::isSpace(*string)) string++;

    int len = strlen(string);
    if (StrCol::isSpace(string[len-1]))
    {
        // Skip trailing white spaces.
        while (len > 1 && StrCol::isSpace(string[len-1])) len--;
        char* buf = StrCol::dup (string, len);

        // Scan the string and fill the unit.
        _parser->cscanner ().fill_unit (buf, *unit);
        delete[] buf;
    }
    else
    {
        // Scan the string and fill the unit.
        _parser->cscanner ().fill_unit (string, *unit);
    }

    // Reset scanner
    _parser->cscanner ().allow_macro_ops (false);

    // Return the filled unit.
    return unit;
 }


// Compare two macro argument identifier.
bool PreMacroExpander::equal (const char* str1, const char* str2) const
 {
    if (! str1 || ! str2) return false;

    int i;

    // Skip leading white spaces.
    while (StrCol::isSpace (*str1)) str1++;

    // Go to the end of the identifier.
    for (i = 0; ! StrCol::isSpace (str1[i]) && str1[i] != '\0'; i++);

    // Compare the two identifiers.
    if (strncmp (str1, str2, i) == 0 && str2[i] == '\0')
        return true;

    return false;
 }


// Test whether a string is an argument of a macro.
int PreMacroExpander::isArg (const char* string, PreMacro* macro) const
 {
    const PreMacro::MacroArgs *args = macro->getArgs ();
    if (!args) return -1; // -> no args, no hit

    // Go through the argument list of the macro and compare the
    // string with the arguments.
    int i = 0;
    for (PreMacro::MacroArgs::const_iterator iter = args->begin ();
        iter != args->end (); ++iter)
    {
        // Return the index of the argument in argument array
        // if found.
        if (equal (string, (*iter).c_str ()))
            return i;
        i++;
    }

    // -1 signals that it`s not an argument.
    return -1;
 }


// Replace an argument unchanged.
void PreMacroExpander::replaceUnchangedArg (int index, Unit* argUnit,
        PreArgDesc &args, Unit* unit, Token *token, bool varnargs) const
 {
    Token* prev = prevPreprocessingToken (token, unit);
    Token* next = nextPreprocessingToken (token, unit);

    // It's not an empty argument or an additional argument.
    if (index <= args.numArgs ()-1)
    {
        MacroUnit& arg = args.getArg (index);

        // If no concatenation operator was found the argument has
        // to be prescanned before it will be inserted into the unit.
        // During prescan all macros in the argument will be expanded.

        if (prev && prev->is_macro_op () && prev->type () == TOK_MO_HASHHASH)
        {
            // The argument has to be inserted in unit as it is.
            *argUnit += arg;

            // If the argument consists only of spaces and comments,
            // then the concat operator can be deleted. Concating X
            // with spaces and comments results in X.
            if (onlySpacesAndComments (arg))
            {
                // The concat operator has a special meaning when placed
                // between a comma and a variable argument. If the variable
                // argument is left out when the macro is used, then the
                // comma before the concat operator will be deleted.
                if (varnargs && index == args.numArgs ()-1)
                {
                    // Search the comma.
                    Token* comma = prevPreprocessingToken (prev, unit);
                    if (comma && comma->type () == TOK_COMMA)
                    {
                        unit->kill (comma); // Delete the comma.
                    }
                }
                unit->kill (prev);  // Delete the concat operator.
            }
            // The concat operator is removed when placed between a
            // comma and a variable argument.
            else if (varnargs && index == args.numArgs ()-1)
            {
                // Search the comma.
                Token* comma = prevPreprocessingToken (prev, unit);
                if (comma && comma->type () == TOK_COMMA)
                {
                    unit->kill (prev);  // Delete the concat operator.
                }
            }
        }
        else if (next && next->is_macro_op () && next->type () == TOK_MO_HASHHASH)
        {
            // The argument has to be inserted in unit as it is.
            *argUnit += arg;

            // If the argument consists only of spaces and comments,
            // then the concat operator can be deleted. Concating X
            // with spaces and comments results in X.
            if (onlySpacesAndComments (args.getArg (index)))
            {
                unit->kill (next);  // Delete the concat operator.
            }
        }
        else
        {
            // Concatenation operator not found. Prescan the argument.
            Unit* unit = &prescanArgument (&arg);
            argUnit->move ((Token*)argUnit->last (), *unit);
            delete unit;
        }
    }
    // Rest argument of a variadic macro.
    else if (varnargs && prev)
    {
        // The concat operator has a special meaning when placed
        // between a comma and a variable argument. If the variable
        // argument is left out when the macro is used, then the
        // comma before the concat operator will be deleted.
        if (prev->is_macro_op () && prev->type () == TOK_MO_HASHHASH)
        {
            // Search the comma.
            Token* comma = prevPreprocessingToken (prev, unit);
            if (comma && comma->type () == TOK_COMMA)
            {
                unit->kill (comma); // Delete the comma.
            }
            unit->kill (prev);  // Delete the concat operator.
        }
    }
 }


// Get the next non-whitespace non-comment token
Token* PreMacroExpander::nextPreprocessingToken (Token* token, Unit* unit) const {
  for (token = (Token*) unit->next (token); token; token = (Token*) unit->next (token))
    if (! token->is_whitespace () && ! token->is_comment ())
      break;
  return token;
}


// Get the previous non-whitespace non-comment token
Token* PreMacroExpander::prevPreprocessingToken (Token* token, Unit* unit) const {
  for (token = (Token*) unit->prev (token); token; token = (Token*) unit->prev (token))
    if (! token->is_whitespace () && ! token->is_comment ())
      break;
  return token;
}


// Check if the given unit contains only spaces and comments.
bool PreMacroExpander::onlySpacesAndComments (MacroUnit& unit) const {
  for (Token* token = (Token*) unit.first (); token; token = (Token*) unit.next (token))
    if (! token->is_whitespace () && ! token->is_comment ())
      return false;
  return true;
}


// Stringify and replace an argument.
void PreMacroExpander::replaceStringifiedArg (int index, Unit* unit, PreArgDesc &args) const
 {
    Unit*       argUnit;
    Token*      token;

    // If valid index stringify the argument.
    if (index <= args.numArgs ()-1)
    {
        // Get the argument.
        argUnit = &args.getArg (index);

        // If it is an empty unit build an empty string token.
        if (argUnit->empty ())
           _parser->cscanner ().fill_unit ("\"\"", *unit);

        // If not an empty unit concatenate all tokens to
        // a single string constant token.
        else
        {
            std::ostringstream str;

            // Build the string constant.
            str << "\"";
            for (token = (Token*) argUnit->first (); token;
                 token = (Token*) argUnit->next (token))
            {
                // Escape double quotes and backslashes in strings or character constants
                if (token->type () == TOK_STRING_VAL || token->type () == TOK_CHAR_VAL)
                {
                    const char *strval = (const char*) token->text ();
                    unsigned int len = (unsigned int) strlen (strval);
                    for (unsigned int i = 0; i < len; i++)
                    {
                        if (strval[i] == '"' || strval[i] == '\\')
                            str << "\\";
                        str << strval[i];
                    }
                }

                // Add non white spaces unchanged.
                else if (! token->is_whitespace ())
                    str << token->text ();

                // Reduce several white spaces to a single space.
                else if (argUnit->next (token) && argUnit->prev (token))
                    str << " ";
            }
            str << "\"";

            // Create a new string constant token.
            _parser->cscanner ().fill_unit (str.str ().c_str (), *unit);
        }
    }

    // Not a valid index (an empty argument), build an empty
    // string constant.
    else
       _parser->cscanner ().fill_unit ("\"\"", *unit);
 }


// Process the stringification macro operator `#'.
void PreMacroExpander::processStringOp (PreMacro* macro, Unit* unit, PreArgDesc &args) const
 {
    Token* token = (Token*) unit->first ();
    Token* hash  = (Token*) 0;
    Unit   tmpUnit;
    int    index;

    // Scan the unit.
    for (; token; token = (Token*) unit->next (token))
    {
      // Recognize the stringification operator `#'.
        if (token->is_macro_op () && token->type () == TOK_MO_HASH)
        {
            if (! hash) hash = token;          // Remember a single hashmark.
            else        hash = (Token*) 0;
        }

        // Recognize a C++ identifier.
        else if (token->is_identifier ())
        {
            // Test whether the identifier is an argument and has to be
            // replaced with the text given at macro call.
            if ((index = isArg (token->text (), macro)) != -1 && args.numArgs ())
            {
                // Add a single space before the argument text.
                //if (!_in_include)
                //  _parser->cscanner ().fill_unit (" ", tmpUnit);

                // Replace the argument.
                if (hash)
                    replaceStringifiedArg (index, &tmpUnit, args);
                else
                    replaceUnchangedArg (index, &tmpUnit, args, unit, token, macro->varArgs ());

                // Add a single space behind the argument text.
                //if (!_in_include)
                //  _parser->cscanner ().fill_unit (" ", tmpUnit);

                // Insert the replaced text into unit.
                unit->move_before (token, tmpUnit);

                // Remove the recognized argument identifier from unit.
                token = (Token*) unit->prev (token);
                unit->kill ((Token*) unit->next (token));
            }

            // Remove the stringification operator `#'.
            if (hash) {
                unit->kill (hash);
                hash = (Token*) 0;
            }
        }
    }
 }


// Process the concatenation macro operator `##'.
void PreMacroExpander::processConcatOp (Unit* unit) const {
  Token *token = unit->first ();
  Token *last  = (Token*)0;
  Token *op    = (Token*)0;
  Token *curr, *next;

  // Scan the unit.
  for (; token; token = unit->next (token)) {

    if (token->is_whitespace ())
      continue;

    // handle the ## operator
    if (token->is_macro_op () && token->type () == TOK_MO_HASHHASH) {
      // no problem, if there is no token on the left side; there might have
      // been an empty argument expansion
      if (!last)
        last = op;
      op = token;
      continue;
    }

    // handle the normal non-whitespace, non-## tokens
    if (op) {
      // transform: concatenate and re-scan the tokens before and after the operator
      std::string conc;

      // tokens before ## until the next whitespace
      for (curr = last; curr && !curr->is_whitespace (); curr = unit->prev (curr)) {
        conc = curr->text () + conc;
        last = curr;
      }

      // tokens after ## until the next whitespace
      for (curr = token; curr && !curr->is_whitespace (); curr = unit->next (curr)) {
        conc += curr->text ();
        token = curr;
      }

      // re-scan the concatenated tokens
      Unit tmp;
      _parser->cscanner ().allow_macro_ops ();
      _parser->cscanner ().fill_unit (conc.c_str (), tmp);
      _parser->cscanner ().allow_macro_ops (false);

      // handle problems
      if (!tmp.first ()) {
        *_err << op->location () << sev_error << "pasting \"" 
              << last->text () << "\" and \"" << token->text ()
              << "\" does not give a valid preprocessing token" << endMessage;
        return;
      }

      // mark all tokens as macro-generated
      next = tmp.first ();
      for (curr = next; curr; curr = tmp.next (curr)) {
        curr->location (op->location ());
      }

      // move the generated tokens behind the expression <x> ## <y>
      unit->move (token, tmp);
      // remove the expression
      unit->kill (last, token);

      // go into the right state for further replacements
      token = last = next;
      op    = (Token*)0;
    }
    else {
      // remember that the current token is the 'last' non-whitespace now
      last = token;
    }
  }

  // if 'op' is still set, the right argument was missing
  if (op) {
    // remove the ## operator; there might have been an empty argument expansion
    unit->kill (op);
  }
}


// Substitutes a macro.
void PreMacroExpander::substituteMacro (PreMacro* macro, PreArgDesc &args) const
 {
    // Build a unit from the macro body.
    Unit* unit = new MacroUnit (macro->getBodyUnit (_parser->cscanner ()));

    // Process the stringification macro operator `#' if this is a
    // function-like macro.
    if (macro->isFunction ())
        processStringOp (macro, unit, args);

    // Set the location of every token in unit to the location of the
    // macro call.
    for (Token* token = (Token*) unit->first (); token;
         token = (Token*) unit->next (token)) {
        token->location (args.beginToken ()->location ());
    }

    // Process the concatenation macro operator `##'.
    processConcatOp (unit);

    // Set the macro begin and end tokens.
    ((MacroUnit*)unit)->MacroBegin (args.beginToken ());
    ((MacroUnit*)unit)->MacroEnd (args.endToken ());

    // Push the macro expanded unit on token stack so that the parsers
    // can parse it (and maybe find some new macro calls).
    pushUnit (*_scanner, unit);

    // Add the unit to the unit manager.
    _unitManager->add (unit);
 }


// Process the special keyword `defined'.
void PreMacroExpander::processDefined (PreMacro* macro, PreArgDesc &args) const
 {
    Token* token = (Token*) 0;
    bool   error = false;

    // If we recognize the form "defined (MACRO)" evaluate the arguments.
    if (args.numArgs ())
    {
        // If the argument isn't empty get the argument, else we have
        // a syntax error.
        if (! args.getArg (0).empty ())
        {
            // Skip white-spaces and comments.
            MacroUnit& arg = args.getArg (0);
            for (token = (Token*) arg.first (); token; token = (Token*) arg.next (token))
                if (! token->is_whitespace () && ! token->is_comment ())
                    break;
        }
        else
            error = true;
    }

    // If we recognize the form "defined MACRO" check if there is really
    // a following macro name identifier.
    else if (checkForGoOn (false))
    {
        // Skip whitespaces and comments.
        while ((token = _scanner->next ()))
            if (! token->is_whitespace () && ! token->is_comment ())
                break;
    }

    // Append token to a new unit. It's either a white space or one
    // of the integer constants 0 or 1.
    Unit* unit = new MacroUnit ();

    // If the macro name is found check if the macro is defined.
    if (token)
    {
        // Build a new integer constant token with the value 1
        // if the macro is defined else 0.
        if (_macroManager->getMacro (token->dtext ()))
           _parser->cscanner ().fill_unit ("1", *unit);
        else
           _parser->cscanner ().fill_unit ("0", *unit);

        // Set the macro begin and end tokens.
        ((MacroUnit*)unit)->MacroBegin (args.beginToken ());
        ((MacroUnit*)unit)->MacroEnd (args.endToken ());
    }
    else
        error = true;

    // If a syntax error appeared report it and replace the identifier
    // with a white space.
    if (error) {
        *_err << macro->location () << sev_error
              << "`defined' used without macro name"
              << endMessage;

        _parser->cscanner ().fill_unit (" ", *unit);
    }

    // Push the unit on token stack so that the parsers can parse
    // it (and maybe find some new macro calls).
    pushUnit (*_scanner, unit);

    // Add the unit to the unit manager.
    _unitManager->add (unit);
 }


// Expand a macro.
bool PreMacroExpander::expandMacro (PreMacro* macro)
 {
    _scanner = (TokenStream*) (_parser->scanner ());
    Token* token = _scanner->current () ? _scanner->current () : _scanner->next ();
    return expandMacro (macro, token);
 }


// Expand the given macro.
bool PreMacroExpander::expandMacro (PreMacro* macro, Token *token) const
 {
    if (! macro || ! token) return false;

    PreArgDesc args;
    args.beginToken (token);
    args.endToken (token);

    // Function-like macro (maybe).
    if (macro->isFunction ())
    {
        // Special keyword `defined' recognized.
        if (strcmp (macro->getName (), "defined") == 0)
        {
            // Process the keyword.
            if (checkForGoOn ())
                collectArgs (macro, args);
            processDefined (macro, args);
        }
        // Check whether the macro name is followed by a left parenthesis.
        // If found substitute the macro call, else it isn't a macro call.
        else if (! checkForGoOn ())
        {
            return false;
        }
        else
        {
            collectArgs (macro, args);
            substituteMacro (macro, args);
        }
    }

    // Substitute a simple macro.
    else
        substituteMacro (macro, args);

    return true;
 }


// Collect arguments of a function-like macro call.
void PreMacroExpander::collectArgs (PreMacro* macro, PreArgDesc &args) const
 {
    int      depth = 1, numArgs = 0;
    bool     args_found, has_args = (macro->numArgs () != 0);
    Unit*    unit;
    Token*   token, *dup;
    Location location;

    // Get the current location.
    PreMacro* defined = _macroManager->getMacro ("defined");
    if (defined != 0)
      location = defined->location ();
    else
      location = _scanner->current ()->location ();

    // Skip whitespaces and comments.
    while ((token =_scanner->next ()))
        if (token->is_core ())
            break;

    // Get the arguments.
    while (token && depth)
    {
        args_found = false;
        unit = 0;

        // Scan for the arguments.
        while ((token = _scanner->next ()))
        {
            // Recognize the commas and parenthesis.
            if (token->is_core ())
            {
                // Another argument found.
                if (token->is_comma () && depth == 1 &&
                    ! (macro->varArgs () && numArgs == (macro->numArgs () - 1))) {
                    if (! args_found && has_args) {
                        args_found = true;
                        if (! unit) {
                            args.newArg ();
                            unit = &args.currArg ();
                        }
                    }
                    break;
                }

                // Maybe a macro call, ignore the following commas.
                else if (token->is_open ()) {
                    depth++;
                }

                // Maybe the end of a macro call found.
                else if (token->is_close ()) {
                    if (! --depth) {        // All done ?
                        if (! args_found && has_args) {
                            args_found = true;
                            if (! unit) {
                                args.newArg ();
                                unit = &args.currArg ();
                            }
                        }
                        args.endToken (token);
                        break;
                    }
                }
            }

            // Collect all tokens of current argument.
            args_found = true;
            if (has_args) {
                if (! unit) {
                    args.newArg ();
                    unit = &args.currArg ();
                }
                if (token->is_comment ()) {
                  // Reduce a comment to a single space
                  _parser->cscanner ().fill_unit (" ", *unit);
                } else {
                  dup = (Token*) token->duplicate ();
                  unit->append (*dup);
                }
            }
        }

        // Increment argument counter.
        if (args_found)
            numArgs++;
    }

    // Report the errors occured.
    if (numArgs < macro->numArgs () && ! (macro->varArgs () &&
        numArgs == (macro->numArgs () - 1))) {
        *_err << location << sev_warning << "macro `"
              << macro->getName () << "' called with too few ("
              << numArgs << ") arguments" << endMessage;
    } else if (numArgs > macro->numArgs () && ! macro->varArgs ()) {
        *_err << location << sev_warning << "macro `"
              << macro->getName () << "' called with too many ("
              << numArgs << ") arguments" << endMessage;
    }
    if (depth)
        *_err << location << sev_warning << "unterminated call to macro `"
              << macro->getName () << "'" << endMessage;
 }


// Push unit on token stream.
void PreMacroExpander::pushUnit (TokenStream &stream, Unit *unit) const
 {
    if (unit) {
        if (unit->isMacroExp ())
          ((MacroUnit*)unit)->CallingUnit (stream.top ());
        stream.push (unit);
    }
 }


} // namespace Puma
