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

#ifndef PUMA_ParserKey_H
#define PUMA_ParserKey_H

/** \file
 * Options parser key helper. */

#include <string>

namespace Puma {

/** \class ParserKey ParserKey.h Puma/ParserKey.h
 * Options parser key helper. Supports Unix-like short key '-'
 * and long key '--'.
 * \ingroup common */
class ParserKey {
public:
  /** Get the short key character.
   * \return The short key character. */
  static char getKey() {
    return '-';
  }
  /** Get the long key character sequence.
   * \return The long key character sequence. */
  static const char *getLongKey() {
    return "--";
  }
  /** Get the length of the short key.
   * \return The length. */
  static unsigned int getKeyLength() {
    return 1;
  }
  /** Get the length of the long key.
   * \return The length. */
  static unsigned int getLongKeyLength() {
    return 2;
  }
  /** Check if the given string starts with the short key.
   * The string may still start with a long key, so check
   * this first.
   * \param str The string to check.
   * \return True if it starts with the short key. */
  static bool isKey(const std::string &str) {
    return str.length() > 0 && str[0] == getKey();
  }
  /** Check if the given string starts with the long key.
   * \param str The string to check.
   * \return True if it starts with the long key. */
  static bool isLongKey(const std::string &str) {
    return str.find(getLongKey()) == 0;
  }
};

} // namespace Puma

#endif /* PUMA_ParserKey_H */
