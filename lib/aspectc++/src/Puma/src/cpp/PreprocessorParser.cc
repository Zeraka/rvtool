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

#include "Puma/Token.h"
#include "Puma/CTokens.h"
#include "Puma/CCommentTokens.h"
#include "Puma/PreMacro.h"
#include "Puma/PreAnswer.h"
#include "Puma/PreTree.h"
#include "Puma/ErrorStream.h"
#include "Puma/UnitManager.h"
#include "Puma/PrePredicate.h"
#include "Puma/PreTreeNodes.h"
#include "Puma/PreLevelState.h"
#include "Puma/PreParserState.h"
#include "Puma/PreMacroManager.h"
#include "Puma/PreFileIncluder.h"
#include "Puma/PreprocessorParser.h"
#include "Puma/PrePredicateManager.h"

#include <list>
#include <string>
#include <string.h>


namespace Puma {


// Get the next token to parse.
Token *PreprocessorParser::next () { 
  return parseToken (); 
}


// Free the preprocessor syntax tree.
void PreprocessorParser::freeSyntaxTree (PreTree *node) {
  int i;
    
  if (! node) return;
    
  for (i = 0; i < node->sons (); i++) 
    freeSyntaxTree (node->son (i));
    
  for (i = 0; i < node->daughters (); i++)
    delete node->daughter (i);
    
  delete node;
}


// Free the preprocessor syntax tree.
void PreprocessorParser::freeSyntaxTree () { 
  freeSyntaxTree (_parserState->syntaxTree); 
  _parserState->syntaxTree = (PreTree*)0; 
}


// Invoke the parse process.
void PreprocessorParser::parse () {
  // Do not parse more than one time.
  if (_parserState->syntaxTree) 
    return;
    
  Token *token, *last = 0;

  // Print the result of the parse process if not in silent mode.
  while ((token = parseToken ()))
    if (! _silentMode) {
      if (last && 
          ((last->type () == TOK_CCOMMENT && 
            token->type () != TOK_CCOMMENT) ||
           (last->location ().filename ().name () !=
            token->location ().filename ().name ())))
        *_out << "\n";

      last = token;
      *_out << token->text () << std::flush;
    }
}


// Configure the preprocessor.
void PreprocessorParser::configure (const Config &c, bool process_includes) {
  _scanner.configure (c);

  unsigned num = c.Options ();
  for (unsigned i = 0; i < num; i++) {
    const ConfOption *o = c.Option (i);
    if (!strcmp (o->Name (), "--gnu") || !strcmp (o->Name (), "--gnu-2.95")) {
      supportGNU (true);
    } else if (! strcmp (o->Name (), "--lang-c")) {
      undefMacro ("__cplusplus");
    } else if (! strcmp (o->Name (), "-D")) {
      if (o->Arguments () < 1) continue;
      undefMacro (o->Argument (0));
      if (o->Arguments () == 2)
        defMacro (o->Argument (0), o->Argument (1));
      else
        defMacro (o->Argument (0));
    } else if (! strcmp (o->Name (), "-U")) {
      if (o->Arguments () != 1) continue;
      undefMacro (o->Argument (0));
    } else if (! strcmp (o->Name (), "-I")) {
      if (o->Arguments () != 1) continue;
      addInclPath (o->Argument (0));
    } else if (! strcmp (o->Name (), "--isystem")) {
      if (o->Arguments () != 1) continue;
      addSysInclPath (o->Argument (0));
    } else if (! strcmp (o->Name (), "-A")) {
      if (o->Arguments () != 2) continue;
      defPredicate (o->Argument (0), o->Argument (1));
    } else if (! strcmp (o->Name (), "--lock-macro")) {
      if (o->Arguments () < 1) continue;
      undefMacro (o->Argument (0));
      if (o->Arguments () == 2)
        defMacro (o->Argument (0), o->Argument (1), PreMacro::LOCK_MACRO);
      else
        defMacro (o->Argument (0), 0, PreMacro::LOCK_MACRO);
    } else if (! strcmp (o->Name (), "--inhibit-macro")) {
      if (o->Arguments () != 1) continue;
      defMacro (o->Argument (0), 0, PreMacro::INHIBIT_MACRO);
    }
  }

  // C99 _Pragma operator, same as #pragma
  PreMacro *pragma_macro = _macroManager->getMacro ("_Pragma");
  if (! (pragma_macro && pragma_macro->numArgs () == 1))
    defMacro ("_Pragma(x)", " ", PreMacro::LOCK_MACRO);

  if (process_includes) {
    CUnit *forced_includes = new CUnit (*parserState ()->err);
    forced_includes->name ("<forced-includes>");
    _locals->add (forced_includes);
    for (unsigned i = 0; i < num; i++) {
      const ConfOption *o = c.Option (i);
      if (! strcmp (o->Name (), "--include")) {
        if (o->Arguments () != 1) continue;
        const char *filename = o->Argument (0);
        *forced_includes << "#include ";
        if (filename[0] == '\"' || filename[0] == '<')
          *forced_includes << filename;
        else
          *forced_includes << "\"" << filename << "\"";
        *forced_includes << std::endl;
        //Unit *unit = fileIncluder ()->handleIncludeOption (o->Argument (0));
      }
    }
    *forced_includes << endu;
    fileIncluder ()->pushOnStack (forced_includes);
  }
}


// Check if macro name has format "name(args)".
bool PreprocessorParser::hasArguments(const char *name) const {
  int len = strlen(name);
  return len != 0 && name[len-1] == ')';
}


// Extract the name of macro given as "name(args)".
std::string PreprocessorParser::getMacroName(const char *nameAndArgs) const {
  std::string name = nameAndArgs;
  return name.substr(0, name.find('('));
}


// Extract the arguments of macro given as "name(args)".
void PreprocessorParser::getMacroArgs(const char *nameAndArgs, std::list<std::string>& args) const {
  std::string work = nameAndArgs;
  std::string::size_type arg_pos = work.find('(');
  if (arg_pos != std::string::npos) {
    // check if there are characters in front of '('
    if (arg_pos == 0)
      return; // syntax error

    std::string::size_type pos = arg_pos+1; // skip opening brace, begin of the argument list
    while ((arg_pos = work.find_first_of(",)", pos)) != std::string::npos) {
      std::string arg = work.substr(pos, arg_pos-pos);
      if (arg[0] == ' ' || arg[arg.length()-1] == ' ')
        return; // syntax error

      args.push_back(arg);
      pos = arg_pos+1; // skip comma or closing brace
    }
  }
}


// Define a new macro.
void PreprocessorParser::defMacro (const char *name, const char *body, char flags) const {
  if (! name)
    return;

  // empty macro body defaults to integer 1
  if (! body)
    body = "1";

  if (! hasArguments(name)) {
    _macroManager->addMacro(new PreMacro(name, body, flags));
  }
  else {
    // "name(args)" format given, extract name and arguments first
    std::string macro_name = getMacroName(name);
    std::list<std::string> args; getMacroArgs(name, args);
    PreMacro *macro = new PreMacro(macro_name.c_str(), body, flags);
    for (std::list<std::string>::iterator i = args.begin(); i != args.end(); ++i)
      macro->addArg(*i);
    _macroManager->addMacro(macro);
  }
}


// Undefine a macro.
void PreprocessorParser::undefMacro(const char *name) const {
  if (! name)
    return;

  if (! hasArguments(name)) {
    _parserState->removeMacroFromStack(*_macroStack, name);
    _macroManager->removeMacro(name);
  }
  else {
    // "name(args)" format given, extract name first
    std::string macro_name = getMacroName(name);
    _parserState->removeMacroFromStack(*_macroStack, macro_name.c_str());
    _macroManager->removeMacro(macro_name.c_str());
  }
}


// Define a new predicate.
void PreprocessorParser::defPredicate (const char *name, const char *answer) const {
  if (! name || ! answer) 
    return;
    
  PrePredicate *predicate;
    
  if ((predicate = _predicateManager->getPredicate (name)))
    predicate->addAnswer (answer);
  else {
    predicate = new PrePredicate (name, answer);
    _predicateManager->addPredicate (predicate);
  }
}


// Undefine a predicate.
void PreprocessorParser::undefPredicate (const char *name) const { 
  _predicateManager->removePredicate (name); 
}


// Return the preprocessor syntax tree.
PreTree *PreprocessorParser::syntaxTree () const { 
  return _parserState->syntaxTree; 
}


// Add a new non-system include path.
void PreprocessorParser::addInclPath (const char *path) const { 
  _fileIncluder->addIncludePath (path); 
}


// Add a new system include path.
void PreprocessorParser::addSysInclPath (const char *path) const { 
  _fileIncluder->addSystemIncludePath (path); 
}


void PreprocessorParser::supportGNU (bool v) {
  _support_gnu = v;
  if (_support_gnu) {
    defineGNUMacros ();
  }
}


void PreprocessorParser::defUndefinedMacro (const char *name, const char *body, char flags) const {
  if (! _macroManager->getMacro (name))
    defMacro (name, body, flags);
}


void PreprocessorParser::defineGNUMacros () {
  // GNU aspect may add more
}


} // namespace Puma
