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

#ifndef __pre_expr_lexer_h__
#define __pre_expr_lexer_h__

#include "lexertl/generator.hpp"
#include "lexertl/lookup.hpp"
#include "lexertl/rules.hpp"
#include "lexertl/state_machine.hpp"

#include "Puma/PreExprParser.h"
#include "Puma/LexerBuffer.h"
#include "Puma/LexerMap.h"

#include <string>

namespace Puma {

class PreExprLexer {

  // implementation of the lexer: a lexertl state machine
  lexertl::state_machine _state_machine;

protected:

  // create the state machine that is needed to scan
  // the preprocessor expression tokens (to be done only once!)
  void setup () {
    lexertl::rules rules;
    add_rules (rules);
    lexertl::generator::build (rules, _state_machine);
  }

public:

  typedef lexertl::rules::id_type IdType;

  // This type is needed to run the lexer and get the tokens
  // one by one
  class State {
    PreExprLexer &_lexer;
    LexerBuffer::ConstIterator _iter;
    LexerBuffer::ConstIterator _end;
    lexertl::match_results<LexerBuffer::ConstIterator, std::size_t> _results;
    bool _error;

  public:

    State (PreExprLexer &lexer, LexerBuffer &buffer) : _lexer (lexer),
      _iter (buffer.begin ()), _end (buffer.end ()),
      _results (_iter, _end), _error (false) {}

    bool error () const { return _error; }
    IdType id () const { return _results.id; }
    int len () const { return &*_results.end - &*_results.start; }
    std::string text () const { return std::string (_results.start, _results.end); }

    bool next () {
      _error = false;
      lexertl::lookup (_lexer._state_machine, _results);
      if (_results.id == 0)
        return false;
      if (_results.id == _results.npos())
        _error = true;
      return true;
    }
  };
  friend class State;

  // virtual destructor needed to avoid warning
  virtual ~PreExprLexer () {}

  // singleton pattern; only one instance
  static PreExprLexer &instance() {

    typedef LexerMap<PreExprLexer> Map;
    static Map  lexer_map;
    Map::iterator elem = lexer_map.find (0); //only one config used for cpp
    if (elem == lexer_map.end ()) {
      elem = lexer_map.create (0);
      elem->second->setup ();
    }
    return *(elem->second);
  }

  // define all regular expressions for the C tokens
  virtual void add_rules (lexertl::rules &rules) {
    // macros
    rules.insert_macro("Dec0", "[0-9]");
    rules.insert_macro("Dec", "[1-9]");
    rules.insert_macro("Oct", "[0-7]");
    rules.insert_macro("Hex", "[0-9a-fA-F]");
    rules.insert_macro("U", "[uU]");
    rules.insert_macro("L", "[lL]");
    rules.insert_macro("Exp", "[Ee]");
    rules.insert_macro("F", "[fF]");

    // operators
    rules.push ("\"?\"", TOK_EXPR_QUESTION);
    rules.push ("\"|\"", TOK_EXPR_BITOR);
    rules.push ("\"^\"", TOK_EXPR_BITXOR);
    rules.push ("\"&\"", TOK_EXPR_BITAND);
    rules.push ("\"+\"", TOK_EXPR_PLUS);
    rules.push ("\"-\"", TOK_EXPR_MINUS);
    rules.push ("\"*\"", TOK_EXPR_STAR);
    rules.push ("\"/\"", TOK_EXPR_DIV);
    rules.push ("\"%\"", TOK_EXPR_MOD);
    rules.push ("\"<\"", TOK_EXPR_LESS);
    rules.push ("\">\"", TOK_EXPR_GREATER);
    rules.push ("\"(\"", TOK_EXPR_LPAREN);
    rules.push ("\")\"", TOK_EXPR_RPAREN);
    rules.push ("\":\"", TOK_EXPR_COLON);
    rules.push ("\"!\"", TOK_EXPR_NOT);
    rules.push ("\"~\"", TOK_EXPR_TILDE);
    rules.push ("\"||\"", TOK_EXPR_OR);
    rules.push ("\"&&\"", TOK_EXPR_AND);
    rules.push ("\"==\"", TOK_EXPR_EQ);
    rules.push ("\"!=\"", TOK_EXPR_NOTEQ);
    rules.push ("\"<=\"", TOK_EXPR_LEQ);
    rules.push ("\">=\"", TOK_EXPR_GEQ);
    rules.push ("\"<<\"", TOK_EXPR_SHIFTL);
    rules.push ("\">>\"", TOK_EXPR_SHIFTR);

    // an integer constant:
    rules.push ("({Dec}{Dec0}*|(0))({U}|{L}|{L}{U}|{U}{L}|{L}{L}|{U}{L}{L}|{L}{U}{L}|{L}{L}{U})?", TOK_EXPR_DEC);
    rules.push ("0{Oct}+({U}|{L}|{L}{U}|{U}{L}|{L}{L}|{U}{L}{L}|{L}{U}{L}|{L}{L}{U})?", TOK_EXPR_OCT);
    rules.push ("0[xX]{Hex}+({U}|{L}|{L}{U}|{U}{L}|{L}{L}|{U}{L}{L}|{L}{U}{L}|{L}{L}{U})?", TOK_EXPR_HEX);

    // a floating point constant:
    rules.push ("{Dec0}+(\\.{Dec0}*)?({Exp}([\\+\\-])?{Dec0}+)?({L}|{F})?", TOK_EXPR_FLOAT);
    rules.push ("\\.{Dec0}+({Exp}([\\+\\-])?{Dec0}+)?({L}|{F})?", TOK_EXPR_FLOAT);

    // a hex floating point constant:
    rules.push ("0[xX]{Hex}+(\\.{Hex}*)?[pP]([\\+\\-])?{Dec0}+({L}|{F})?", TOK_EXPR_FLOAT);
    rules.push ("0[xX]\\.{Hex}+[pP]([\\+\\-])?{Dec0}+({L}|{F})?", TOK_EXPR_FLOAT);

    // a character constant:
    rules.push ("(L)?\\\'([^\\\\\\\']|\\\\.)*\\\'", TOK_EXPR_CHAR);

    // whitespace
    rules.push ("\\s+", TOK_EXPR_WSPACE);
  }
};

} // namespace Puma

#endif /* __pre_expr_lexer_h__ */
