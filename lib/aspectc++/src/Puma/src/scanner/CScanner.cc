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

#include "Puma/CScanner.h"
#include "Puma/ErrorStream.h"
#include "Puma/Source.h"
#include "Puma/Token.h"
#include "Puma/Location.h"
#include "Puma/Unit.h"
#include "Puma/CTokens.h"
#include "Puma/CCLexer.h"

#include <string.h>

namespace Puma {


void CScanner::lexer (CLexer& lexer) {
  _lexer = &lexer;
}


void CScanner::configure (const Config &c) {
  // evaluate configuration options
  bool lang_cc = c.Option ("--lang-c++");
  bool lang_c  = lang_cc || c.Option ("--lang-c");

  // default settings
  if (! lang_c && ! lang_cc) {
    lang_c = true;
    lang_cc = true;
  }

  _config_cplusplus = lang_cc;

  allow_std_cplusplus    (lang_cc);
  allow_std_c            (lang_c);
  allow_visual_c         (lang_c && c.Option ("--vc"));
  allow_visual_cplusplus (lang_cc && c.Option ("--vc"));
  allow_gnu_c            (lang_c && c.Option ("--gnu"));
  allow_gnu_cplusplus    (lang_cc && c.Option ("--gnu"));
  allow_type_traits      (lang_cc && c.Option ("--builtin-type-traits"));
}


Token *CScanner::make_token (CLexer::State &lexer_state) {
  Token *result = 0;
  char short_buffer[512];
  char *tok_buffer = short_buffer;
  int rows = 0;
  int columns = 0;
  int offset = loc.column ();

  char *src  = (char*)lexer_state.start ();
  int len = lexer_state.len ();
  CLexer::IdType expr = lexer_state.id ();
  LanguageID lid = lexer_state.lang ();

  // dynamically allocate a huge buffer
  if (len >= (int)sizeof (short_buffer) - 1)
    tok_buffer = new char[len + 1];

  if (lexer_state.lines () == 1) {
    // a token without newline => can be copied directly
    memcpy (tok_buffer, src, len);
    tok_buffer[len] = '\0';
    result = new Token (expr, lid, tok_buffer);
    result->location (loc);
    loc.setup (loc.filename (), loc.line (), loc.column () + len);
  }
  else {
    // special token with newline => more complicated
    char *dest = tok_buffer;
    char *end  = src + len;

    int last = 0;
    Array<int> *cl = 0;
    while (src < end) {
      if (*src == '\\' && src + 1 < end && *(src + 1) == '\n') {
        src += 2;
        rows++;
        columns = 1;
        offset = 0;
        if (!cl) cl = new Array<int>(10,10);
        cl->append (last);
        last = 0;
      }
      else if (*src == '\\' && src + 2 < end && *(src + 1) == '\x0d' &&
               *(src + 2) == '\n') {
        src += 3;
        rows++;
        columns = 1;
        offset = 0;
        if (!cl) cl = new Array<int>(10,10);
        cl->append (last);
        last = 0;
      }
      else {
        if (*src == '\n') {
          rows++;
          columns = 1;
          offset = 0;
        } else {
          columns++;
        }
        *dest = *src;
        dest++;
        src++;
        last++;
      }
    }
    *dest = '\0';

    result = new Token (expr, lid, tok_buffer);
    result->location (loc);

    // set the next token location
    if (rows > 0 || columns > 0)
      loc.setup (loc.filename (), loc.line () + rows, offset + columns);

    // attach the continuation line marks
    if (cl) result->cont_lines (cl);
  }

  // free a dynamically allocated huge buffer
  if (len >= (int)sizeof (short_buffer))
    delete[] tok_buffer;

  return result;
}


void CScanner::scan_all (CLexer &lexer, CLexerBuffer &lexer_buffer, Unit &unit) {
  CLexer::State lexer_state (lexer, lexer_buffer);
  if (macro_ops) lexer_state.macro_unit ();

  loc.setup (unit.name () ? unit.name () : "<anonymous unit>", 1, 1);

  // cout << "---" << endl;
  while (lexer_state.next ()) {
    if (lexer_state.error ()) {
      err << sev_error << loc
        << "Invalid token '" << lexer_state.text ().c_str () << "'" << endMessage;
      break;
    }
    else {
      Token *new_token = make_token (lexer_state);
      unit.append (*new_token);
    }
  }
}


void CScanner::fill_unit (Source &in, Unit &unit) {
  int size = in.size ();
  if (size == 0)
    return;
  if (size == -1) {
    err << sev_error << "can't scan file of unknown size" << endMessage;
    return;
  }
  char buf[size+1];
    if (in.read (buf, size) != size) {
    err << sev_error << "can't load input file" << endMessage;
    return;
  }
  buf[size] = '\0'; // termination character needed for CLexer

  CLexer &lexer = (_lexer ? *_lexer : _config_cplusplus ? CCLexer::instance (0) : CLexer::instance (0));
  CLexerBuffer lexer_buffer;
  lexer_buffer.init (buf, size);
  scan_all (lexer, lexer_buffer, unit);
}


void CScanner::fill_unit (const char *in, Unit &unit) {
  CLexer &lexer = (_lexer ? *_lexer : _config_cplusplus ? CCLexer::instance (0) : CLexer::instance (0));
  CLexerBuffer lexer_buffer;
  lexer_buffer.init (in);
  scan_all (lexer, lexer_buffer, unit);
}


} // namespace Puma
