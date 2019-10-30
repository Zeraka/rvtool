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

#ifndef PUMA_StrCol_H
#define PUMA_StrCol_H

/** \file
 * Collection of string helper functions. */

#include <string>

namespace Puma {

/** \class StrCol StrCol.h Puma/StrCol.h
 * Collection of string helper functions.
 * \ingroup basics */
struct StrCol {
  /** Check if the given character is one of:
   * - white space
   * - horizontal tab
   * - vertical tab
   * - form feed
   * - line feed
   * - carriage return
   * - backspace
   *
   * \param c The character to check.
   * \return True if it is a space character. */
  static bool isSpace(const char c);

  /** Check if the given string only contains space
   * characters as recognized by isSpace().
   * \param str The string to check.
   * \return True if there are only space characters in that string. */
  static bool onlySpaces(const char *str);

  /** Check if the two strings differ not only in spaces.
   * \param str1 The first string.
   * \param str2 The second string.
   * \return True if there are differences other than spaces. */
  static bool effectivelyDifferent(const char *str1, const char *str2);

  /** Copy a string.
   * \param str The string to copy.
   * \return A newly allocated copy of the input string. */
  static char *dup(const char *str);
  /** Copy the first n characters of a string.
   * \param str The string to copy.
   * \param n The number of characters to copy.
   * \return A newly allocated string containing the first n characters of the input string. */
  static char *dup(const char *str, int n);

  /** Calculate a hash value for a string using the Hsieh algorithm.
   * \param str The input string.
   * \return The hash value. */
  static unsigned int hash(const char *str);
  /** Calculate a hash value for the first n characters of a string using the Hsieh algorithm.
   * \param str The input string.
   * \param n The number of characters to consider for the hash value.
   * \return The hash value. */
  static unsigned int hash(const char *str, int n);

  /** Replace environment variables of the form ${ENVVAR} in the given string.
   * \param str A reference to the string containing the environment variables.
   * \return A reference to the input string with environment variables replaced by their values. */
  static std::string& replaceEnvironmentVariables(std::string& str);
};

} // namespace Puma

#endif /* PUMA_StrCol_H */
