// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2003  The 'ac++' developers (see aspectc.org)
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

#include "PumaPreprocessor.h"
#include "IncludeGraph.h"
#include "IntroductionUnit.h"

using namespace Puma;
using namespace std;


PumaPreprocessor::PumaPreprocessor (const string &tunit_name, ACConfig &conf, CProject &project) :
  _tunit_name (tunit_name), _conf (conf), _project (project), _local_units(_project.err ()),
  _cpp (&_project.err (), &_project.unitManager (), &_local_units, cout),
  _provider(_cpp), _recorded_tokens (0) {

  // scan file
  _unit = _project.scanFile (tunit_name.c_str());
  if (!_unit)
    return;

  // prepare C preprocessor
  _stream.push (_unit);
  _project.unitManager ().init ();
  _cpp.macroManager ()->init (_unit->name ());
  _cpp.stream (&_stream);
  _cpp.configure (_project.config ());
  _provider.init ();

  // fetch the first token
  locate_token ();

}

// return the include graph *after* preprocessing the translation unit
void PumaPreprocessor::get_include_graph (IncludeGraph &include_graph) {
  // finally parse the preprocessor syntax tree in order to create an
  // include graph of the original source code
  include_graph.init (_cpp.syntaxTree());
}

Puma::ErrorStream &PumaPreprocessor::err () const {
  return _project.err ();
}

bool PumaPreprocessor::keywords_enabled (Puma::Token *tok) const {
  if (!tok)
    return false;

  Unit *unit = tok->unit ();
  while (unit && unit->isMacroExp ()) {
    unit = ((MacroUnit*)unit)->CallingUnit ();
  }

  if (!unit->isFile ()) // TODO: is that case possible?
    return false;

  if (!_project.isBelow(unit))
    return false;

  size_t len = strlen (unit->name ());
  if (len > 2 && strcmp (unit->name () + (len - 3), ".ah") == 0)
    return true;

  return _conf.keywords();
}

ACToken PumaPreprocessor::next_token () {
  _provider.next ();
  ACToken result = locate_token ();
  if (_recorded_tokens && result != ACToken::TOK_EOF)
    _recorded_tokens->append(*result.get ()->duplicate());
  return result;
}

ACToken PumaPreprocessor::curr_token () {
  Token *token = _provider.current ();
  return ACToken (token, keywords_enabled (token));
}

ACToken PumaPreprocessor::look_ahead (int n) {
  TokenProvider::State state = _provider.get_state ();
  while (n > 0) {
    _provider.next ();
    locate_token ();
    n--;
  }
  Token *token = _provider.current ();
  _provider.set_state (state);
  return ACToken (token, keywords_enabled(token));
}

ACToken PumaPreprocessor::locate_token () {
  Token *token;
  while ((token = _provider.current ()) && ! token->is_core ()) {
// TODO: handle directives here?
//    if (token->is_directive ())
//      handle_directive ();
//    else
      _provider.next ();
  }
  return ACToken (token, keywords_enabled(token));
}

// functions to record the token stream

void PumaPreprocessor::start_recording () {
  if (!_recorded_tokens) {
    _recorded_tokens = new Unit;
//    _recorded_tokens->name(((Unit*)_jpm.source_unit(curr_token()))->name ());
    _recorded_tokens->append(*curr_token().get ()->duplicate());
  }
}

Unit *PumaPreprocessor::stop_recording () {
  Unit *result = _recorded_tokens;
  _recorded_tokens = 0;
  return result;
}

FileUnit *PumaPreprocessor::source_unit (ACToken token) {
  Unit *unit   = token.unit ();
  while (unit && unit->isMacroExp ()) {
    unit = ((MacroUnit*)unit)->CallingUnit ();
  }
  // if the 'insert unit' is an 'introduction unit', find the intro target unit
  IntroductionUnit *intro_unit = IntroductionUnit::cast (unit);
  if (intro_unit) unit = intro_unit->final_target_unit ();

  // at this point we must have reached a file unit
  if (!unit->isFile ())
    return 0;

  return (FileUnit*)unit;
}

// check whether a token is located within the files of the project
bool PumaPreprocessor::is_in_project(ACToken token) const {
  FileUnit *funit = source_unit(token);
  return funit && _project.isBelow(funit);
}
