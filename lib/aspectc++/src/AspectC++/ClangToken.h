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

#ifndef __ClangToken_h__
#define __ClangToken_h__

#include "clang/Lex/Token.h"
#include "clang/Basic/IdentifierTable.h"

// ClangToken - Provides a uniform wrapper for a Token object.
class ClangToken {
public:
  enum Kind {
    // Translate from clang token kinds into our subset of Puma ones.
    TOK_INVALID      = clang::tok::unknown,
    TOK_EOF          = clang::tok::eof,
    TOK_ALIGNAS      = clang::tok::kw_alignas,
    TOK_ASSIGN       = clang::tok::equal,
    TOK_ATTRIBUTE    = clang::tok::kw___attribute,
    TOK_CLASS        = clang::tok::kw_class,
    TOK_CLOSE_CURLY  = clang::tok::r_brace,
    TOK_CLOSE_ROUND  = clang::tok::r_paren,
    TOK_CLOSE_SQUARE = clang::tok::r_square,
    TOK_COLON        = clang::tok::colon,
    TOK_COLON_COLON  = clang::tok::coloncolon,
    TOK_COMMA        = clang::tok::comma,
    TOK_CONST        = clang::tok::kw_const,
    TOK_ENUM         = clang::tok::kw_enum,
    TOK_FRIEND       = clang::tok::kw_friend,
    TOK_GREATER      = clang::tok::greater,
    TOK_ID           = clang::tok::identifier,
    TOK_LESS         = clang::tok::less,
    TOK_NAMESPACE    = clang::tok::kw_namespace,
    TOK_MUL          = clang::tok::star,
    TOK_OPEN_CURLY   = clang::tok::l_brace,
    TOK_OPEN_ROUND   = clang::tok::l_paren,
    TOK_OPEN_SQUARE  = clang::tok::l_square,
    TOK_PRIVATE      = clang::tok::kw_private,
    TOK_PROTECTED    = clang::tok::kw_protected,
    TOK_PTS          = clang::tok::arrow,
    TOK_PUBLIC       = clang::tok::kw_public,
    TOK_SEMI_COLON   = clang::tok::semi,
    TOK_STATIC       = clang::tok::kw_static,
    TOK_STRUCT       = clang::tok::kw_struct,
    TOK_TEMPLATE     = clang::tok::kw_template,
    TOK_TILDE        = clang::tok::tilde,
    TOK_TYPEDEF      = clang::tok::kw_typedef,
    TOK_UNION        = clang::tok::kw_union,
    TOK_USING        = clang::tok::kw_using,
    TOK_VIRTUAL      = clang::tok::kw_virtual,
    TOK_VOLATILE     = clang::tok::kw_volatile,
    // Attention: we assume that clang token are enumerated from 0 to a number smaller than 0x10000
    TOK_ASPECT       = 0x10000,
    TOK_ADVICE,
    TOK_POINTCUT,
    TOK_SLICE,
    TOK_ATTRDECL,
    TOK_UNKNOWN
  };

private:
  clang::Token _token;
  Kind _kind;

public:
  ClangToken () : _token (), _kind (TOK_INVALID) {}
  explicit ClangToken(clang::Token token, bool acxx_keyword = false)
      : _token (token), _kind(Kind (token.getKind ())) {

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
  bool is_core() const { return true; }
  bool is_directive () const { return false; /* FIXME! */ }
  bool is_macro_generated () const { return location ().isMacroID (); }
  clang::SourceLocation location () const { return _token.getLocation (); }
  int type () const { return _kind; }

  const char *text () const {
    if (clang::IdentifierInfo *II = _token.getIdentifierInfo ())
      return II->getNameStart ();

    // No identifier, must be a literal.
    assert (_token.isLiteral());
    return _token.getLiteralData ();
  }

  // Interface to get the underlying token.
  clang::Token get() const { return _token; }
#if 0 // Not useful with clang
  operator bool () const { return _token != 0; }
  bool operator== (const ACToken &rhs) const { return _token == rhs._token; }
  bool operator!= (const ACToken &rhs) const { return _token != rhs._token; }
  // FIXME: Are those actually useful?
  bool operator< (const ACToken &rhs) const { return _token < rhs._token; }
  bool operator> (const ACToken &rhs) const { return _token > rhs._token; }
  bool operator<= (const ACToken &rhs) const { return _token <= rhs._token; }
  bool operator>= (const ACToken &rhs) const { return _token <= rhs._token; }
#endif
};

#endif /* __ClangToken_h__ */
