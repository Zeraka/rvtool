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

#include "Puma/Syntax.h"
#include "Puma/CTree.h"
#include "Puma/CTokens.h"
#include "Puma/Semantic.h"

namespace Puma {


void Syntax::configure (Config &config) {
  if (config.Option ("--verbose-errors"))
    _verbose_errors = true;
}


CTree *Syntax::run (TokenProvider &tp) {
  token_provider = &tp;
  token_provider->init();
  _have_error = false;
  _problem_token = (Token*)0;
  locate_token ();
  return trans_unit () ? builder ().Top () : (CTree*)0;
}


Token *Syntax::locate_token () {
  Token *token;
  while ((token = token_provider->current ()) && ! token->is_core ()) {
    if (token->is_directive ())
      handle_directive ();
    else
      token_provider->next ();
  }
  return token;
}

bool Syntax::look_ahead (int token_type, unsigned n) {
  Token *token;
  State state;
  bool result;
  
  result = false;
  if (n == 1) {
    token = token_provider->current ();
    result = (token && token->type () == token_type);
  } else {
    state = token_provider->get_state ();
    while ((--n) > 0) {
      token_provider->next ();
      locate_token ();
    }
    token = token_provider->current ();
    result = (token && token->type () == token_type);
    token_provider->set_state (state);
  }
  return result;
}

bool Syntax::look_ahead (int *token_types, unsigned n) {
  Token *token;
  State state;
  bool result;
  
  result = false;
  if (n == 1) {
    token = token_provider->current ();
    if (token) {
      while (*token_types) {
        if (*token_types == token->type ()) {
          result = true;
          break;
        }
        ++token_types;
      }
    }
  } else {
    state = token_provider->get_state ();
    while ((--n) > 0) {
      token_provider->next ();
      locate_token ();
    }
    token = token_provider->current ();
    if (token) {
      while (*token_types) {
        if (*token_types == token->type ()) {
          result = true;
          break;
        }
        ++token_types;
      }
    }
    token_provider->set_state (state);
  }
  return result;
}

Syntax::State Syntax::save_state () {
  semantic ().save_state ();
  return token_provider->get_state ();
}

void Syntax::forget_state () {
  semantic ().forget_state ();
}

void Syntax::restore_state () {
  for (long i = builder ().Length () - 1; i >= 0; i--)
    semantic ().undo (builder ().Get (i));
  semantic ().restore_state ();
}

void Syntax::restore_state (State state) {
  token_provider->set_state (state);
  for (long i = builder ().Length () - 1; i >= 0; i--)
    semantic ().undo (builder ().Get (i));
  semantic ().restore_state ();
}

void Syntax::set_state (State state) {
  token_provider->set_state (state);
}

bool Syntax::accept (CTree *result, State s) {
  if (result) {
    forget_state ();
    builder ().forget_state ();
    builder ().Push (result); 
    return true;
  } else {
    set_state (s);
    restore_state ();
    return false;
  }
}

CTree* Syntax::accept (CTree *result) {
  return result;
}

bool Syntax::parse_token (int token_type) {
  Token *t = token_provider->current ();
  if (t && t->type () == token_type) {
    _problem_token = (Token*)0;
    token_provider->next ();
    locate_token ();
    builder ().Push (builder ().token (t));
    return true;
  } else {
    if (! _problem_token)
      _problem_token = token_provider->current ();
    return false;
  }
}

bool Syntax::parse (int *token) {
  while (*token) {
    if (parse (*token))
      return true;
    token++;
  }
  return false;
}

void Syntax::skip_block (int open, int close, bool inclusive) {
  Token *current;
  int token;
  int depth = 0;

  while ((current = token_provider->current ())) {
    token = current->type ();

    if (token == open)
      depth++;
    else if (token == close)
      depth--;
    else if (depth > 0 && token == TOK_OPEN_ROUND) {
      skip_round_block ();
      continue;
    }

    if (depth == 0 && !inclusive)
      break;

    token_provider->next ();
    locate_token ();

    if (depth == 0)
      break;
  }
}

void Syntax::skip_round_block () {
  skip_block (TOK_OPEN_ROUND, TOK_CLOSE_ROUND);
}

void Syntax::skip_curly_block () {
  skip_block (TOK_OPEN_CURLY, TOK_CLOSE_CURLY);
}

bool Syntax::parse_block (int open, int close) {
  int depth = 0;
  int token;
  while ((token = look_ahead ())) {
    consume ();
    if (token == open)
      depth++;
    else if (token == close)
      depth--;
    if (depth == 0)
      return true;
  }
  return false;
}

bool Syntax::parse_round_block () {
  return parse_block (TOK_OPEN_ROUND, TOK_CLOSE_ROUND);
}

bool Syntax::parse_curly_block () {
  return parse_block (TOK_OPEN_CURLY, TOK_CLOSE_CURLY);
}

bool Syntax::skip (int *stop_tokens, bool inclusive) {
  Token *current;
  int token;

  while ((current = token_provider->current ())) {
    token = current->type ();

    if (is_in (token, stop_tokens))
      break;

    if (token == TOK_OPEN_CURLY)
      skip_curly_block ();
    else if (token == TOK_OPEN_ROUND)
      skip_round_block ();
    else {
      token_provider->next ();
      locate_token ();
    }
  }

  if (current && inclusive && current->type () != TOK_CLOSE_CURLY) {
    token_provider->next ();
    locate_token ();
  }

  return false;
}

bool Syntax::skip (int stop_token, bool inclusive) {
  Token *current;
  int token;

  while ((current = token_provider->current ())) {
    token = current->type ();

    if (token == stop_token)
      break;

    if (token == TOK_OPEN_CURLY)
      skip_curly_block ();
    else if (token == TOK_OPEN_ROUND)
      skip_round_block ();
    else {
      token_provider->next ();
      locate_token ();
    }
  }

  if (current && inclusive && current->type () != TOK_CLOSE_CURLY) {
    token_provider->next ();
    locate_token ();
  }

  return false;
}

void Syntax::skip () {
  token_provider->next ();
  locate_token ();
}

bool Syntax::is_in (int token, int *set) const {
  int i = 0;
  while (set[i] && set[i] != token) 
    i++;
  return (set[i] != 0);
}


} // namespace Puma
