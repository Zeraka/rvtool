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

#ifndef __ClangPreprocessor_h__
#define __ClangPreprocessor_h__

#include "ACConfig.h"
#include "ACProject.h"
#include "ACToken.h"
#include "ACErrorStream.h"
#include "IncludeGraph.h"

// This class performs preprocesses a translation unit and returns its
// content token by token. Upon request, it can also record the returned
// tokens.

class ClangPreprocessor {
public:
  typedef std::vector<ACToken> TokenVector;

private:
  ACConfig &_conf;
  ACProject &_project;
  TokenVector _recorded_tokens;
  ACToken _current_token;
  string _tunit_name;

  bool keywords_enabled (clang::SourceLocation loc) const;

public:
  ClangPreprocessor (const string& tunit_name, ACConfig &conf,
      IncludeGraph &include_graph, ACProject &project);
  ~ClangPreprocessor ();

  // functions to access the token stream
  ACToken next_token ();
  ACToken curr_token ();
  ACToken look_ahead (int n = 1);

  // functions to record the token stream
  void start_recording ();
  TokenVector stop_recording ();

  ACErrorStream &err () const;

  // get the file unit in which a token is located
  // (if necessary follow macro expansions and intros)
  ACFileID source_unit (ACToken token) const;

  // check whether a token is located within the files of the project
  bool is_in_project (ACToken token) const;

  std::string token_text (ACToken) const;
  unsigned token_line_number (ACToken) const;

  unsigned tunit_len() const;
};

#endif // __ClangPreprocessor_h__
