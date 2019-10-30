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

#include "Puma/TokenStream.h"
#include "Puma/Unit.h"
#include "Puma/Token.h"
#include "Puma/StrCol.h"
#include "Puma/SysCall.h"
#include "Puma/CScanner.h"
#include "Puma/FdSource.h"
#include "Puma/CTokens.h"
#include "Puma/Filename.h"
#include "Puma/PreParser.h"
#include "Puma/UnitManager.h"
#include "Puma/ErrorStream.h"
#include "Puma/PreParserState.h"
#include "Puma/PreFileIncluder.h"
#include "Puma/PreprocessorParser.h"
#include <sstream>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

namespace Puma {


PreFileIncluder::PreFileIncluder (PreprocessorParser& parser) {
  // Set the assigned preprocessor and remember dependent objects
  preprocessor (&parser);

  // Search for include files in all known include directories.
  // Needed for the #include_next directive to tell the FileIncluder
  // which directories should be skipped.
  _skip = 0;

  _firstIncludeDir = _includeDirs.end();
}

// Set the associated preprocessor parser
void PreFileIncluder::preprocessor (PreprocessorParser *pp) {
  _parser      = pp;
  _state       = pp->parserState ();
  _unitManager = pp->unitManager ();
  _err         = _state->err;
}

// Add a new non-system include path.
void PreFileIncluder::addIncludePath (const char* path) {
  // do not add include paths twice!
  if (path && ! isIncludePath(path)) {
    // add before the first non-system include dir (as last non-system include dir searched)
    _includeDirs.insert(_firstIncludeDir, path);
    --_firstIncludeDir;
    _includeDirLookup[path] = false;
  }
}

// Add a new system include path.
void PreFileIncluder::addSystemIncludePath (const char *path) {
  if (path) {
    std::map<std::string,bool>::iterator sysDir = _includeDirLookup.find(path);
    if (sysDir != _includeDirLookup.end()) {
      // already known system include path?
      if (sysDir->second == true)
        return; // ignore

      // previously added as non-system include path; remove it
      std::list<std::string>::iterator dirsIdx = _includeDirs.end();
      for (--dirsIdx; dirsIdx != _includeDirs.begin(); --dirsIdx) {
        if (*dirsIdx == path) {
          _includeDirs.erase(dirsIdx);
          break;
        }
      }
    }

    // add as first path in list (as last searched include dir)
    _includeDirs.push_front (path);
    _includeDirLookup[path] = true; // mark as system include dir
  }
}

// Check if the given include directory is a system include directory.
bool PreFileIncluder::isSystemIncludePath(const char *path) const {
  if (path) {
    const std::map<std::string,bool>::const_iterator &lookupIdx = _includeDirLookup.find(path);
    return (lookupIdx != _includeDirLookup.end() && lookupIdx->second == true);
  }
  return false;
}

// Check if the given directory is a known include directory.
bool PreFileIncluder::isIncludePath(const char *path) const {
  return (path && _includeDirLookup.find(path) != _includeDirLookup.end());
}

// Include a file.
Unit* PreFileIncluder::handleIncludeOption (const char* string)
 {
    Unit *unit = 0;
    if (! string) return unit;

    // If file name isn't empty try to locate the file, else
    // we have a syntax error.
    if (strlen (string))
        unit = searchFile (string, true);
    else {
        unit = (Unit*)0;
        *_err << sev_error
              << "empty file name in `--include'"
              << endMessage;
    }

    return unit;
 }


// Include a file.
Unit* PreFileIncluder::includeFile (const char* string)
 {
    if (! string) return (Unit*) 0;

    // Build the token unit from string.
    Unit* unit   = buildUnit (string);

    Token* first = (Token*) unit->first (); 
    Token* last  = (Token*) unit->last ();

    if (first && last) 
    {
       // Pattern "..." found. Include from cwd.
       if (first->is_core () && first->type () == TOK_STRING_VAL) 
       {
          return includeFromCwd (unit);
       } 

       // Pattern <...> found. Include from system.
       else if (first->is_core () && first->type () == TOK_LESS && 
                last->is_core () && last->type () == TOK_GREATER) 
       {
          return includeFromSystem (unit);
       }
    } 

    // Only reached in case of a syntax error.
    if (_state->currToken)
        *_err << _state->currToken->location ();
    *_err << sev_error << "`#include' expects \"filename\" or <filename>" 
          << endMessage;

    // Return an empty unit.
    return (Unit*)0;
 }


// Include the next file of the given name.
Unit* PreFileIncluder::includeNextFile (const char* string)
 {
    if (! string) return (Unit*) 0;

    // Get the name of the current file.
    const char* location = _state->currToken->location ().filename ().name ();
    char*       path     = StrCol::dup (location ? location : "");

    // Get the current path only.
    char* backslash = strrchr (path, '/');

    if (backslash)
    {
        *backslash = '\0';

        // Calculate the paths to skip.
        std::list<std::string>::reverse_iterator dirsIdx = _includeDirs.rbegin();
        for (_skip = 1; dirsIdx != _includeDirs.rend() && *dirsIdx != path; ++_skip, ++dirsIdx);
    }

    // Now include the file.
    Unit* unit = includeFile (string);

    // Reset to search in all paths.
    _skip = 0;

    delete[] path;
    return unit;
 }


// Try to include a file from current working directory.
Unit* PreFileIncluder::includeFromCwd (Unit* unit)
 {
    // At first get the name of the file.
    int   size = strlen (((Token*) unit->first ())->text ());
    char* name = new char[size + 5];

    strncpy (name, ((Token*) unit->first ())->text (), size-1);
    name[size-1] = '\0';
    delete unit;

    // If file name isn't empty try to locate the file, else
    // we have a syntax error.
    if (strlen (name + 1))
        unit = searchFile (name + 1, true);
    else {
        unit = (Unit*)0;
        *_err << _state->currToken->location () << sev_error
              << "empty file name in `#include'"
              << endMessage;
    }

    delete[] name;
    return unit;
 }


// Try to include the file from system include directories.
Unit* PreFileIncluder::includeFromSystem (Unit* unit)
 {
    Token* token = (Token*) unit->first ();
    std::ostringstream file;

    // Get the file name.
    for (token = (Token*) unit->next ((Token*) unit->first ()); ;
         token = (Token*) unit->next (token)) {
        if (token->is_core () && token->type () == TOK_GREATER)
            break;
        file << token->text ();
    }

    delete unit;

    // If file name is empty we have a syntax error.
    if (! file.str ().length ()) {
        *_err << _state->currToken->location () << sev_error
              << "empty file name in `#include'"
              << endMessage;
        return (Unit*)0;
    }

    // Try to locate the file.
    unit = searchFile (file.str ().c_str (), false);

    return unit;
 }


// Try to locate the file.
Unit* PreFileIncluder::searchFile (const char* file, bool search_in_cwd)
 {
    Unit *unit = (Unit*) 0;

    // check if this is an absolute path
    Filename filename (file);
    if (filename.is_absolute ()) {
      unit = doInclude (file, file, true);
      return unit;
    }

    // If pattern `#include "..."' found try to include the file 
    // from current working directory first.
    if (search_in_cwd)
    {
        // Get the current working directory.
        const char* path = _state->currToken ? _state->currToken->location ().filename ().name () : 0;
        char*       cwd  = StrCol::dup (path ? path : "");
        char* backslash  = strrchr (cwd, '/');

        // Combine the current working directory with the file
        // to include to the full filename.
        if (backslash) {
            *backslash     = '\0';
            char* fullname = new char[strlen (cwd) + strlen (file) + 3];
            sprintf (fullname, "%s/%s", cwd, file);
            if (! path || strcmp(fullname, path) != 0)
              unit = doInclude (fullname, file, false);
            delete[] fullname;
        } else if (! path || strcmp(file, path) != 0)
            unit = doInclude (file, file, false);

        delete[] cwd;
    }

    if (! unit)
    {
      // If there are no include paths left it's an error case.
      if (_includeDirs.size() - _skip <= 0)
      {
        if (_state->currToken)
          *_err << _state->currToken->location ();
        *_err << sev_error
              << "no include path in which to find `"
              << file << "'" << endMessage;
      }
      else
      {
        // Go through the include directories and try to
        // include the file.
        std::list<std::string>::reverse_iterator dirsIdx = _includeDirs.rbegin();
        for (int i = 0; i < _skip && dirsIdx != _includeDirs.rend(); ++i, ++dirsIdx);

        while (dirsIdx != _includeDirs.rend())
        {
            // Build file name like `path/file'
            std::string fullname = *dirsIdx + "/" + file;
            ++dirsIdx;

            // Now try to include the file from current system 
            // include directory. If this is not the last directory
            // in list report no errors.
            if (dirsIdx != _includeDirs.rend())
              unit = doInclude (fullname.c_str(), file, false);
            else
              unit = doInclude (fullname.c_str(), file, true);

            if (unit)
              break; // File found.
        }
      }
    }

    // Return filled unit or a empty unit if file not found.
    return unit;
 }


// Include the file.
Unit* PreFileIncluder::doInclude (const char* fullname, const char* file, 
                                  bool report_errors)
 {
    int    fd;
    Unit   *unit;

    if (fullname[0] == '.' && fullname[1] == '/')
     {
       fullname += 2;
     }

    // If we already have included the file sometime before,
    // we don't need to scan it again and use the old one.
    if (! (unit = _unitManager->get (fullname, true)))
    {
        // Open the file. If not there report the error or return silently.
        if ((fd = SysCall::open (fullname, O_RDONLY)) == -1)
        {
            if (report_errors)
             {
                if (_state->currToken)
                    *_err << _state->currToken->location ();
                *_err << file << ": "
                      << sev_error << "no such file or directory" 
                      << endMessage;
             }
            return (Unit*) 0;
        }

        // If file is a directory an error occured.
        FileInfo fi;
        if (SysCall::fstat (fd, fi, _err) && fi.is_dir ())
        {
            if (report_errors)
            {
                if (_state->currToken)
                    *_err << _state->currToken->location ();
                *_err << sev_error << "input file `" << file
                      << "' is a directory" << endMessage;
            }
            SysCall::close (fd, _err);
            return (Unit*) 0;
        }

        // It's the first time we include this file, so we have
        // to build a new unit of it.
        unit = _unitManager->scanSource (fullname, new FdSource (fd), true);
        SysCall::close (fd, _err);

        if (! unit) return (Unit*) 0;
    }

    // Push the unit on token stack.
    pushOnStack (unit);

    return unit;
 }


// check if this file has an include guard that is already active
bool PreFileIncluder::hasActiveIncludeGuard (Unit *unit) {
  // first check if this unit was already included
  _guarded = false;
  std::map<Unit*, Token*>::iterator iter = _includedUnits.find (unit);
  if (iter != _includedUnits.end ()) {
    Token *guard = (*iter).second;
    // iff the file has a guard variable and it is defined, true is returned
    if (guard && _parser->macroManager ()->getMacro (guard->text ()))
      _guarded = true;
  }
  else {
    _includedUnits.insert (std::pair<Unit*,Token*> (unit, hasIncludeGuard (unit)));
  }
  return _guarded;
}
  

// check if this file has an include guard
Token *PreFileIncluder::hasIncludeGuard (Unit *unit) {
  Token *guard = 0;
  Token *tok = (Token*)unit->first ();
  // skip comments and whitespace
  while (tok && (tok->is_whitespace () || tok->is_comment ()))
    tok = (Token*)unit->next (tok);
  // the next token has to be #ifndef
  if (!(tok && tok->is_preprocessor () && tok->type () == TOK_PRE_IFNDEF))
    return 0;
  tok = (Token*)unit->next (tok);
  // now whitespace
  if (!(tok && tok->is_whitespace ()))
    return 0;
  tok = (Token*)unit->next (tok);
  // the next has be an identifier => the name of the guard macro
  if (!(tok && tok->is_identifier ()))
    return 0;
  guard = tok;
  tok = (Token*)unit->next (tok);
  // skip comments and whitespace
  while (tok && (tok->is_whitespace () || tok->is_comment ()))
    tok = (Token*)unit->next (tok);
  // the next token has to be #define
  if (!(tok && tok->is_preprocessor () && tok->type () == TOK_PRE_DEFINE))
    return 0;
  tok = (Token*)unit->next (tok);
  // now whitespace
  if (!(tok && tok->is_whitespace ()))
    return 0;
  tok = (Token*)unit->next (tok);
  // the next has be an identifier => the name of the guard macro
  if (!(tok && tok->is_identifier ()))
    return 0;
  // check if the identifier is our guard variable
  if (strcmp (tok->text (), guard->text ()) != 0)
    return 0;
  tok = (Token*)unit->next (tok);
  // find the corresponding #endif
  int level = 1;
  while (tok) {
    if (tok->is_preprocessor ()) {
      if (tok->type () == TOK_PRE_IF || tok->type () == TOK_PRE_IFDEF ||
        tok->type () == TOK_PRE_IFNDEF)
        level++;
      else if (tok->type () == TOK_PRE_ENDIF) {
        level--;
        if (level == 0)
          break;
      }
    }
    tok = (Token*)unit->next (tok);
  }
  if (level > 0)
    return 0;
  tok = (Token*)unit->next (tok);
  // skip comments and whitespace
  while (tok && (tok->is_whitespace () || tok->is_comment ()))
    tok = (Token*)unit->next (tok);
  // here we should have reached the end of the unit!
  if (tok)
    return 0;

  // at this point we are sure that there is an include guard in this file
  return guard;
}


// Push the unit on token stack.
void PreFileIncluder::pushOnStack (Unit *unit) {
  if (!hasActiveIncludeGuard (unit))
    ((TokenStream*) (_parser->scanner ()))->push (unit);
}


// Return true if string is not of format "..." or <...>.
bool PreFileIncluder::assumeMacroCalls (const char* string) const
 {
    if (! string) return true;

    // Skip leading white spaces.
    while (StrCol::isSpace (*string)) string++;

    // Now there has to be a '"' or '<'.
    if (*string != '"' && *string != '<')
        return true; 

    // Skip trailing white spaces.
    int i = strlen (string) - 1;
    for (; i >= 1; i--)
      if (! StrCol::isSpace (string[i])) 
        break;

    if (i < 1) return true;

    // Now there has to be a '"' or '>'.    
    if (*string == '"' && string[i] == '"') return false;
    if (*string == '<' && string[i] == '>') return false;

    return true;
 }


// Build a macro expanded unit from a string.
Unit* PreFileIncluder::buildUnit (const char* string)
 {
    Token*    token, *tmp;
    Unit*     unit = new Unit ();

    // Macro operations (# and ##) are allowed
    _parser->cscanner ().allow_macro_ops ();

    // Reset the macro expander and component scanner.
    _expander.reset (_parser);

    // Scan the string.
    if (assumeMacroCalls (string)) {
        _expander.inInclude (true);
        char *mstr = _expander.expandMacros (string);
        _expander.inInclude (false);
        _parser->cscanner ().fill_unit (mstr, *unit);
        delete[] mstr;
    } else
        _parser->cscanner ().fill_unit (string, *unit);

    // Remove spaces at begin of unit.
    for (token = (Token*) unit->first (); token; ) 
    {
        if (!token->is_whitespace ())
            break;
        tmp   = token;
        token = (Token*) unit->next (token);
        unit->kill (tmp);
    }

    // Remove spaces at end of unit.
    for (token = (Token*) unit->last (); token; ) 
    {
        if (! token->is_whitespace ())
            break;
        tmp   = token;
        token = (Token*) unit->prev (token);
        unit->kill (tmp);
    }

    _parser->cscanner ().allow_macro_ops (false);
    return unit;
 }


} // namespace Puma
