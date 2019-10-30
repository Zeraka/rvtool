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

#ifndef __PumaPreprocessor_h__
#define __PumaPreprocessor_h__

#include "ACConfig.h"
#include "ACToken.h"
#include "IncludeGraph.h"
#include "CodeWeaver.h"

#include "Puma/TokenStream.h"
#include "Puma/PreprocessorParser.h"
#include "Puma/CProject.h"

// This class performs preprocesses a translation unit and returns its
// content token by token. Upon request, it can also record the returned
// tokens.

class PumaPreprocessor {

  string _tunit_name;
  Puma::Unit *_unit;
  ACConfig &_conf;
  Puma::CProject &_project;
  Puma::TokenStream _stream;
  Puma::UnitManager _local_units;
  Puma::PreprocessorParser _cpp;
  Puma::TokenProvider _provider;
  Puma::Unit *_recorded_tokens;
  ACToken _record_start;
  ACToken _record_end;

  bool keywords_enabled (Puma::Token *tok) const;

public:
  PumaPreprocessor (const string &tunit_name, ACConfig &conf, Puma::CProject &project);

  Puma::Unit *unit () const { return _unit; }

  // functions to access the token stream
  ACToken next_token ();
  ACToken curr_token ();
  ACToken look_ahead (int n = 1);
  ACToken locate_token ();

  // functions to record the token stream
  void start_recording ();
  Puma::Unit *stop_recording ();

  Puma::ErrorStream &err () const;

  // return the include graph *after* preprocessing the translation unit
  void get_include_graph (IncludeGraph &include_graph);

  // get the file unit in which a token is located
  // (if necessary follow macro expansions and intros)
  static Puma::FileUnit *source_unit (ACToken token);

  // check whether a token is located within the files of the project
  bool is_in_project(ACToken token) const;
};

#endif // __PumaPreprocessor_h__
