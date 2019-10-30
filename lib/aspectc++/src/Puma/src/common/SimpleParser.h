// This file is part of PUMA.
// Copyright (C) 1999-2016  The PUMA developer team.
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

#ifndef PUMA_SimpleParser_H
#define PUMA_SimpleParser_H

/** \file
 * Simple string tokenizer. */

#include <string>
#include <vector>

namespace Puma {

/** \class SimpleParser SimpleParser.h Puma/SimpleParser.h
 * Simple string tokenizer taking a string and providing
 * a list of the tokens recognized.
 * \ingroup common */
class SimpleParser {
public:
  /** Remove enclosing double-quotes from a string.
   * \param str The string to strip.
   * \return String without the enclosing double-quotes.*/
  static std::string strip(const std::string& str);
  /** Splits a given string into tokens recognized by the given delimiters.
   * \param str The string to split.
   * \param tokens The container for the resulting tokens.
   * \param delimiters A string with the delimiters. Defaults to a single space.
   * \return Number of recognized tokens. */
  static int tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters = " ");

private:
  /** Get the position of the next token.
   * \param line The string to tokenize.
   * \param delimiters A string with the token delimiters.
   * \param pos The current position in the string.
   * \param lastPos The last position in the string.
   * \return The position of the next token. */
  static std::string::size_type next(const std::string& line, const std::string& delimiters, std::string::size_type pos, std::string::size_type& lastPos);
  /** Get the begin of the next token.
   * \param line The string to tokenize.
   * \param delimiters A string with the token delimiters.
   * \param pos The current position in the string.
   * \return The begin of the next token. */
  static std::string::size_type findBeginOfNextToken(const std::string& line, const std::string& delimiters, std::string::size_type pos);
  /** Get the end of the next token.
   * \param line The string to tokenize.
   * \param delimiters A string with the token delimiters.
   * \param lastPos The last position in the string, i.e. the begin of the next token.
   * \return The end of the next token. */
  static std::string::size_type findEndOfNextToken(const std::string& line, const std::string& delimiters, std::string::size_type lastPos);
};

} // namespace Puma

#endif /* PUMA_SimpleParser_H */
