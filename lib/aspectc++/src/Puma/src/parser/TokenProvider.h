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

#ifndef __TokenProvider__
#define __TokenProvider__

/** \file
 *  Token provider abstraction. */

#include "Puma/Token.h"
#include "Puma/TokenSource.h"
#include "Puma/BCList.h"

namespace Puma {

/** \class TokenInfo TokenProvider.h Puma/TokenProvider.h
 *  Provides information about a token as it is needed by the parser.
 *  Especially the instantiation parser needs not only the token
 *  pointer but also the pointer to the syntax tree, in which the
 *  token is located. */
struct TokenInfo {
  Token *_token;
  void *_context;
};


/** \class TokenProvider TokenProvider.h Puma/TokenProvider.h
 *  Reads tokens from a token source. Supports unlimited look ahead. 
 *  A token source can be a file, a string, a token preprocessor,
 *  and so on. */
class TokenProvider {
protected:
  typedef BCList<TokenInfo, 8192> TokenStore;

  TokenSource &_source;
  TokenStore _tokens;
  TokenStore::Iterator _read_pos;

  /** Read next token. */
  virtual TokenInfo read () { TokenInfo ti; ti._token = _source.next(); ti._context = 0; return ti; }

  /** Get the current token. */
  Token* token () const { return (*_read_pos)._token; }

public:
  /** Read position. */
  typedef TokenStore::Iterator State;

  /** Constructor.
   *  \param s The token source from which to read the tokens. */
  TokenProvider (TokenSource &s) : _source (s) {}

  /** Destructor.
   *  The compiler complains if we done define a virtual destrcutor. */
  virtual ~TokenProvider () {}

  /** Initialize the token provider by reading the first token. */
  void init() { _read_pos = _tokens.add (read ()); }

  /** Get the token source. */
  TokenSource& source() const { return (TokenSource&)_source; }

  /** Read the next token from the token source. */
  Token *next () {
    // go to the next read position
    ++_read_pos;
    // did we reach the end of the list?
    if (! _read_pos) {
      // get next token and add it to the list
      _read_pos = _tokens.add (read ());
    }
    return current ();
  }

  /** Get the last token read from the token source. */
  Token *current () const { return token (); }

  /** Get the context information for the current token. */
  void *current_context () const { return (*_read_pos)._context; }

  /** Get the current read position. */
  State get_state () { return _read_pos; }
  /** Restore the read position.
   *  \param restored_pos The new read position. */
  void set_state (State restored_pos) { _read_pos = restored_pos; }
};

} // namespace Puma

#endif /* __TokenProvider__ */
