// This file is part of PUMA.
// Copyright (C) 1999-2015  The PUMA developer team.
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

#ifndef PUMA_Token_H
#define PUMA_Token_H

/** \file
 * Token abstraction. */

#include "Puma/LanguageID.h"
#include "Puma/Location.h"
#include "Puma/Printable.h"
#include "Puma/ListElement.h"
#include "Puma/Array.h"
#include "Puma/DString.h"

namespace Puma {

class Unit;

/** \class Token Token.h Puma/Token.h
 * Abstraction of a lexical token. A token is created by a
 * scanner (see Puma::CScanner). It is part a token chain
 * (see Puma::Unit) and encapsulates a lexical unit of the
 * scanned text.
 *
 * The information provided about a lexical unit is:
 * - token type
 * - location in the scanned text
 * - portion of the scanned text represented by this token
 * - language of the token
 *  
 * \ingroup common */
class Token : public ListElement {
  int m_type;
  int m_realType;

  Location m_location;
  LanguageID m_language;

  const char *m_text;
  mutable DString m_dictText;

  Array<int> *m_continuousLines;

  enum TK_Flags {
    TK_COMMENT = (0x01 << 0),
    TK_CORE = (0x01 << 1),
    TK_MACRO_CALL = (0x01 << 2)
  };
  unsigned short m_flags;

public:
  /** Special token types. */
  enum {
    /** End-of-file token. */
    ID_END_OF_FILE = -1,
    /** Unknown token type. */
    ID_UNKNOWN = -2,
    /** Error token type. */
    ID_ERROR = -3,
    /** Warning token type. */
    ID_WARNING = -4
  };

  /** Comment tokens. */
  static LanguageID comment_id;
  /** Preprocessor tokens. */
  static LanguageID pre_id;
  /** Compiler directives. */
  static LanguageID dir_id;
  /** C/C++ core language tokens. */
  static LanguageID cpp_id;
  /** White-spaces. */
  static LanguageID white_id;
  /** Wildcards. */
  static LanguageID wildcard_id;
  /** Keywords. */
  static LanguageID keyword_id;
  /** Left parenthesis. */
  static LanguageID open_id;
  /** Function-like macro operands. */
  static LanguageID macro_op_id;
  /** Identifiers. */
  static LanguageID identifier_id;
  /** Comma. */
  static LanguageID comma_id;
  /** Right parenthesis. */
  static LanguageID close_id;

public:
  /** Constructor.
   *  \param type The token type.
   *  \param lang The language of the token.
   *  \param text The optional token text (defaults to the empty string). */
  Token(int type = 0, LanguageID lang = LanguageID(0), const char *text = "");
  /** Copy-constructor. 
   *  \param copy The token to copy. */
  Token(const Token &copy);
  /** Destructor. Frees the token text. */
  virtual ~Token();

  /** Print the token text on the given stream. 
   *  \param os The output stream. */
  void print(std::ostream &os) const;
  /** Duplicate this token.
   *  \return A copy of this token. Has to be destroyed by the caller. */
  virtual ListElement *duplicate() {
    return new Token(*this);
  }

  /** Get the unit this token belongs to.
   * \return A pointer to the unit. */
  Unit* unit() const;

  /** Reset the token object optionally providing new content.
   *  \param type The new token type.
   *  \param text The new token text.
   *  \param lang The new token language. */
  void reset(int type = 0, const char *text = 0, LanguageID lang = LanguageID(0));

  /** Set the location of the token (usually the line and column in a file). 
   *  \param loc The location. */
  void location(const Location &loc) {
    m_location = loc;
  }

  /** Set the continuous lines appeared in this token.
   *  \param lines The continuous line array. */
  void cont_lines(Array<int> *lines) {
    m_continuousLines = lines;
  }

  /** Count the number of line breaks in the text of this token.
   * \return The number of line breaks found. */
  int line_breaks() const;

  /** Set the token to be a macro call.
   * \param is_call Is a macro call or not. */
  void macro_call(bool is_call = true);

  /** Get the token text.
   * \return The constant token text. */
  const char *text() const;

  /** Get the token text from the global dictionary.
   * \return A reference to the global dictionary managed version of the token text. */
  DString &dtext() const;

  /** Get the static token text. Only for keywords, operators, and other tokens
   * with a constant token text.
   * \return The static text, or NULL for tokens with dynamic text (like identifiers). */
  char *get_static_text() const;

  /** Get the static token text for the given token type. Only for keywords,
   * operators, and other tokens with a constant token text.
   * \param token_type The token type.
   * \return The static text, or NULL for tokens with dynamic text (like identifiers). */
  static char *get_static_text(int token_type);

  /** Get the type of the token (see Puma::CTokens).
   * \return The numerical value of the token type. */
  int type() const {
    return m_type;
  }

  /** Get the location of the token (usually the line and column in a file).
   * \return A reference to the token location. */
  const Location &location() const {
    return m_location;
  }

  /** Check if the token is macro generated.
   * \return True if macro generated. */
  bool is_macro_generated() const;
  /** Check if the token is a macro call.
   * \return True if it is a macro call. */
  bool is_macro_call() const {
    return (m_flags & TK_MACRO_CALL) != 0;
  }
  /** Check if this is a core language token.
   * \return True if this is core language token. */
  bool is_core() const;
  /** Check if the token is an identifier. Subset of core tokens.
   * \return True if this is an identifier. */
  bool is_identifier() const;
  /** Check if the token is a keyword. Subset of identifiers.
   * \return True if this is a keyword. */
  bool is_keyword() const;
  /** Check if this is a comma. Subset of core tokens.
   * \return True if this is a comma. */
  bool is_comma() const;
  /** Check if this is a left parenthesis. Subset of core tokens.
   * \return True if this is a left parenthesis. */
  bool is_open() const;
  /** Check if this is a right parenthesis. Subset of core tokens.
   * \return True if this is a right parenthesis. */
  bool is_close() const;
  /** Check if this is a white-space token.
   * \return True if this is a white space. */
  bool is_whitespace() const;
  /** Check if this is a comment.
   * \return True if this is a comment token. */
  bool is_comment() const;
  /** Check if this is a preprocessor token.
   * \return True if this is a preprocessor token. */
  bool is_preprocessor() const;
  /** Check if this is function-like macro operand.
   * \return True if this is an operand of a function-like macro. */
  bool is_macro_op() const;
  /** Check if this is a compiler directive token.
   * \return True if this is compiler directive token. */
  bool is_directive() const;
  /** Check if this is a wildcard token. Subset of core tokens.
   * \return True if this is a wildcard token. */
  bool is_wildcard() const;

  /** Overwrite operator new for better performance. Tokens are allocated
   * very often. To reduce the overhead of allocating a lot of small portions
   * of memory, this implementation reuses already allocated memory
   * by using a constant size memory pool (see Puma::ConstSizeMemPool).
   * \param size The size of the memory to allocate in byte.
   * \return A pointer to the allocated memory. */
  void *operator new(size_t size);
  /** Free the memory allowed with the Token's own operator new. The memory is
   * not freed but kept back for reuse.
   * \param ptr A pointer to the memory to free. */
  void operator delete(void *ptr);

private:
  int get_general_type() const;
  LanguageID language() const {
    return m_language;
  }
  void determine_is_comment();
  void determine_is_core();
};

inline void Token::macro_call(bool is_call) {
  if (is_call)
    m_flags |= TK_MACRO_CALL;
  else
    m_flags &= ~TK_MACRO_CALL;
}

inline void Token::determine_is_core() {
  if (m_language == cpp_id || m_language == wildcard_id || m_language == open_id || m_language == comma_id || m_language == close_id || is_identifier())
    m_flags |= TK_CORE;
}
inline void Token::determine_is_comment() {
  if (m_language == comment_id)
    m_flags |= TK_COMMENT;
}

inline bool Token::is_core() const {
  return (m_flags & TK_CORE) != 0;
}
inline bool Token::is_identifier() const {
  return m_language == identifier_id || is_keyword();
}
inline bool Token::is_keyword() const {
  return m_language == keyword_id;
}
inline bool Token::is_wildcard() const {
  return m_language == wildcard_id;
}
inline bool Token::is_preprocessor() const {
  return m_language == pre_id;
}
inline bool Token::is_directive() const {
  return m_language == dir_id;
}
inline bool Token::is_whitespace() const {
  return m_language == white_id;
}
inline bool Token::is_comment() const {
  return (m_flags & TK_COMMENT) != 0;
}
inline bool Token::is_macro_op() const {
  return m_language == macro_op_id;
}
inline bool Token::is_open() const {
  return m_language == open_id;
}
inline bool Token::is_comma() const {
  return m_language == comma_id;
}
inline bool Token::is_close() const {
  return m_language == close_id;
}

inline std::ostream &operator <<(std::ostream &os, const Token &object) {
  object.print(os);
  return os;
}

} // namespace Puma

#endif /* PUMA_Token_H */
