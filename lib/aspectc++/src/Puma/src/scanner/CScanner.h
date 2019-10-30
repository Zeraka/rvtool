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

#ifndef __c_scanner_h__
#define __c_scanner_h__

#include "Puma/Location.h"
#include "Puma/LanguageID.h"
#include "Puma/Tokenizer.h"
#include "Puma/Unit.h"
#include "Puma/ErrorSink.h"
#include "Puma/Config.h"
#include "Puma/Source.h"
#include "Puma/CCLexer.h"

namespace Puma {

class Token;

class CScanner : public Tokenizer {
  Location loc;
  ErrorSink &err;
  bool _config_cplusplus;
  CLexer *_lexer;

  // flags to enable/disable certain features
  bool directives;
  bool wildcards;
  bool macro_ops;
  bool std_c;
  bool std_cplusplus;
  bool gnu_c;
  bool gnu_cplusplus;
  bool visual_c;
  bool visual_cplusplus;
  bool type_traits;
  bool cc1x;

private:
  // internal functions
  Token *make_token (CLexer::State &lexer_state);
  Token *scan ();
  void scan_all (CLexer &lexer, CLexerBuffer &lexer_buffer, Unit &unit);
  void scan_all (Unit &unit);

public:
  CScanner (ErrorSink &error_stream) :
    err (error_stream), _config_cplusplus (true), _lexer (0),
    directives(true), wildcards (false), macro_ops (false),
    std_c (true), std_cplusplus (true),
    gnu_c (false), gnu_cplusplus (false),
    visual_c (false), visual_cplusplus (false),
    type_traits (false), cc1x (false) {}

  virtual ~CScanner () {}

  void configure (const Config &);
  void lexer (CLexer& lexer);

  void fill_unit (Source &in, Unit &unit);
  void fill_unit (const char *in, Unit &unit);

  void allow_directives (bool flag = true)       { directives = flag; }
  void allow_wildcards (bool flag = true)        { wildcards = flag; }
  void allow_macro_ops (bool flag = true)        { macro_ops = flag; }
  void allow_std_c (bool flag = true)            { std_c = flag; }
  void allow_std_cplusplus (bool flag = true)    { std_cplusplus = flag; }
  void allow_gnu_c (bool flag = true)            { gnu_c = flag; }
  void allow_gnu_cplusplus (bool flag = true)    { gnu_cplusplus = flag; }
  void allow_visual_c (bool flag = true)         { visual_c = flag; }
  void allow_visual_cplusplus (bool flag = true) { visual_cplusplus = flag; }
  void allow_type_traits (bool flag = true)      { type_traits = flag; }
  void allow_cc1x (bool flag = true)             { cc1x = flag; }
};


} // namespace Puma

#endif /* __c_scanner_h__ */
