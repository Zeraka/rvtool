// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2013  The 'ac++' developers (see aspectc.org)
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

#ifndef __PumaToken_h__
#define __PumaToken_h__

#include "Puma/Token.h"
#include "Puma/CTokens.h"

// PumaToken - Provides a uniform wrapper for a Token object.
class PumaToken {
public:
  enum Kind {
    // Translate from Puma token kinds into our subset of Puma ones.
    TOK_ASSIGN       = Puma::TOK_ASSIGN,
    TOK_ALIGNAS      = -1, // FIXME: not defined in Puma yet
    TOK_ATTRIBUTE    = Puma::TOK_ATTRIBUTE,
    TOK_CLASS        = Puma::TOK_CLASS,
    TOK_CLOSE_CURLY  = Puma::TOK_CLOSE_CURLY,
    TOK_CLOSE_ROUND  = Puma::TOK_CLOSE_ROUND,
    TOK_CLOSE_SQUARE = Puma::TOK_CLOSE_SQUARE,
    TOK_COLON        = Puma::TOK_COLON,
    TOK_COLON_COLON  = Puma::TOK_COLON_COLON,
    TOK_COMMA        = Puma::TOK_COMMA,
    TOK_CONST        = Puma::TOK_CONST,
    TOK_ENUM         = Puma::TOK_ENUM,
    TOK_FRIEND       = Puma::TOK_FRIEND,
    TOK_GREATER      = Puma::TOK_GREATER,
    TOK_ID           = Puma::TOK_ID,
    TOK_LESS         = Puma::TOK_LESS,
    TOK_NAMESPACE    = Puma::TOK_NAMESPACE,
    TOK_MUL          = Puma::TOK_MUL,
    TOK_OPEN_CURLY   = Puma::TOK_OPEN_CURLY,
    TOK_OPEN_ROUND   = Puma::TOK_OPEN_ROUND,
    TOK_OPEN_SQUARE  = Puma::TOK_OPEN_SQUARE,
    TOK_PRIVATE      = Puma::TOK_PRIVATE,
    TOK_PROTECTED    = Puma::TOK_PROTECTED,
    TOK_PTS          = Puma::TOK_PTS,
    TOK_PUBLIC       = Puma::TOK_PUBLIC,
    TOK_SEMI_COLON   = Puma::TOK_SEMI_COLON,
    TOK_STATIC       = Puma::TOK_STATIC,
    TOK_STRUCT       = Puma::TOK_STRUCT,
    TOK_TEMPLATE     = Puma::TOK_TEMPLATE,
    TOK_TILDE        = Puma::TOK_TILDE,
    TOK_TYPEDEF      = Puma::TOK_TYPEDEF,
    TOK_UNION        = Puma::TOK_UNION,
    TOK_USING        = Puma::TOK_USING,
    TOK_VIRTUAL      = Puma::TOK_VIRTUAL,
    TOK_VOLATILE     = Puma::TOK_VOLATILE,
    TOK_ASPECT       = Puma::TOK_ASPECT,
    TOK_ADVICE       = Puma::TOK_ADVICE,
    TOK_POINTCUT     = Puma::TOK_POINTCUT,
    TOK_SLICE        = Puma::TOK_SLICE,
    // Attention: we assume that Puma tokens are enumerated from 0 to a number smaller than 0x10000
    TOK_EOF          = 0x10000,
    TOK_ATTRDECL,
    TOK_INVALID,
    TOK_UNKNOWN
  };

private:
  Puma::Token *_token;
  Kind _kind;

public:
  PumaToken () : _token(0), _kind (TOK_INVALID) {}
  explicit PumaToken (Puma::Token *token, bool acxx_keyword = false) :
      _token(token), _kind (token ? Kind (token->type ()) : TOK_EOF) {
    if (acxx_keyword && _kind == TOK_ID) {
      const char *text = this->text ();
      if (!strcmp(text, "aspect"))
        _kind = TOK_ASPECT;
      else if (!strcmp(text, "advice"))
        _kind = TOK_ADVICE;
      else if (!strcmp(text, "pointcut"))
        _kind = TOK_POINTCUT;
      else if (!strcmp(text, "slice"))
        _kind = TOK_SLICE;
      else if (!strcmp(text, "attribute"))
        _kind = TOK_ATTRDECL;
    }
  }

  bool is_core () const { return _token->is_core (); }
  bool is_directive () const { return _token->is_directive (); }
  bool is_macro_generated () const { return _token->is_macro_generated (); }
  Puma::Unit *unit () const { return _token->unit (); }
  Puma::Location location () const { return _token->location (); }
  int type () const { return _kind; }
  const char *text () const { return _token ? _token->text () : (const char*)0; }

  // Interface to get the underlying token.
  Puma::Token *get() const { return _token; }
  operator bool () const { return _token != 0; }
  bool operator== (const PumaToken &rhs) const { return _token == rhs._token; }
  bool operator!= (const PumaToken &rhs) const { return _token != rhs._token; }
  // FIXME: Are those actually useful?
  bool operator< (const PumaToken &rhs) const { return _token < rhs._token; }
  bool operator> (const PumaToken &rhs) const { return _token > rhs._token; }
  bool operator<= (const PumaToken &rhs) const { return _token <= rhs._token; }
  bool operator>= (const PumaToken &rhs) const { return _token <= rhs._token; }
};

#endif /* __PumaToken_h__ */
