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

#ifndef __c_lexer_h__
#define __c_lexer_h__

#include "Puma/Token.h"
#include "Puma/CTokens.h"
#include "Puma/PreParser.h"
#include "Puma/CCommentTokens.h"
#include "Puma/PreMacroOpTokens.h"
#include "Puma/LanguageID.h"
#include "Puma/CLexerBuffer.h"
#include "Puma/Lexer.h"

#include <string>

namespace Puma {

class CLexer : public Lexer {
public:
  typedef lexertl::rules::id_type IdType;

  // This type is needed to run the lexer and get the tokens
  // one by one
  class State {
    CLexer &_lexer;
    CLexerBuffer::ConstIterator _start;
    CLexerBuffer::ConstIterator _iter;
    CLexerBuffer::ConstIterator _end;
    lexertl::match_results<CLexerBuffer::ConstIterator, std::size_t> _results;
    enum { LM_NORMAL, LM_PRE, LM_DIR } _line_mode;
    bool _error;
    int _number;

  public:
    State (CLexer &lexer, CLexerBuffer &buffer) : _lexer (lexer),
      _start (buffer.begin ()), _iter (buffer.begin ().adjust ()),
      _end (buffer.end ()), _results (_iter, _end), _line_mode (LM_NORMAL),
      _error (false), _number (0) {}

    void macro_unit () {
      // this is to avoid an ambiguity between macro op tokens and a directive
      // (which always starts at the beginning of the line)
      _results.bol = false;
    }

    bool error () const { return _error; }
    IdType id () const { return _results.id; }
    LanguageID lang () const { return LanguageID ((const char*)_results.user_id); }
    int continuations () const { return _results.end.continuations(); }
    int lines () const { return _results.end.lines(); }
    int number () const { return _number; }
    const char *start () const { return _number == 1 ? &*_start : &*_results.start; }
    int len () const { return &*_results.end - start (); }
    std::string text () const { return std::string (_results.start, _results.end); }

    bool next () {
      _error = false;
      _results.end.reset_counters ();
      lexertl::lookup (_lexer._state_machine, _results);
      if (_results.id == 0)
        return false;
      if (_results.id == _results.npos())
        _error = true;
      else {
        if ((const char*)_results.user_id == Token::dir_id)
          _line_mode = LM_DIR;
        else if ((const char*)_results.user_id == Token::pre_id)
          _line_mode = LM_PRE;
        if (_line_mode == LM_DIR)
          _results.user_id = LID(Token::dir_id);
        if (_results.bol)
          _line_mode = LM_NORMAL;
      }
      _number++;
      return true;
    }
  };
  friend class State;

  // virtual destructor needed to avoid warning
  virtual ~CLexer () {}

  // get instance of the lexer
  static CLexer& instance(unsigned int id) {
    return Lexer::instance<CLexer>(id);
  }

  // define all regular expressions for the C tokens
  virtual void add_rules (lexertl::rules &rules);

  // function called by 'add_rules' used to define the
  // C keywords
  virtual void add_keywords (lexertl::rules &rules);

  // different kinds of keywords; selected depending on the standard to be supported
  virtual void add_c89_keywords (lexertl::rules &rules);
  virtual void add_c99_keywords (lexertl::rules &rules);
};

} // namespace Puma

#endif /* __c_lexer_h__ */
