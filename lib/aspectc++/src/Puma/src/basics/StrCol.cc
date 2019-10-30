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

#include "Puma/Unit.h"
#include "Puma/Token.h"
#include "Puma/StrCol.h"
#include <string.h>
#include <stdint.h>

namespace Puma {

char *StrCol::dup(const char *str, int len) {
  char *res = new char[len + 1];
  memcpy(res, str, len);
  res[len] = '\0';
  return res;
}

char *StrCol::dup(const char *str) {
  return dup(str, strlen(str));
}

// Return true if character is a white space.
bool StrCol::isSpace(const char c) {
  if (c == ' ' || c == '\n' || c == '\t' || c == '\f' || c == '\v' || c == '\b' || c == '\r')
    return true;
  return false;
}

// Return true if the string consists only of spaces.
bool StrCol::onlySpaces(const char *str) {
  if (!str)
    return false;

  while (*str != '\0') {
    if (!isSpace(*str))
      return false;
    str++;
  }
  return true;
}

// Return true if the two strings differ not only in spaces.
bool StrCol::effectivelyDifferent(const char *str1, const char *str2) {
  if (!str1 || !str2)
    return true;

  // Skip the leading white spaces.
  while (isSpace(*str1))
    str1++;
  while (isSpace(*str2))
    str2++;

  while (true) {
    // Skip white spaces if there are some at these position
    // in both strings or if we are at the end of a string.
    if ((isSpace(*str1) && isSpace(*str2)) || (*str1 == '\0' || *str2 == '\0')) {
      while (isSpace(*str1))
        str1++;
      while (isSpace(*str2))
        str2++;
    }

    // Found a difference. Strings are different.
    if (*str1 != *str2)
      return true;

    // Stop comparison at end of string.
    if (*str1 == '\0' || *str2 == '\0')
      break;

    str1++;
    str2++;
  }

  // Can only be reached if both strings are not
  // effectively different.
  return false;
}

// Calculate a hash for a string
unsigned int StrCol::hash(const char *data) {
  return hash(data, strlen(data));
}

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif
#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)+(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

// Calculate a hash for a string (Hsieh algorithm)
unsigned int StrCol::hash(const char *data, int len) {
  uint32_t hash = len, tmp;
  int rem;

  if (len <= 0 || data == NULL)
    return 0;

  rem = len & 3;
  len >>= 2;

  for (; len > 0; len--) {
    hash += get16bits(data);
    tmp = (get16bits (data+2) << 11) ^ hash;
    hash = (hash << 16) ^ tmp;
    data += 2 * sizeof(uint16_t);
    hash += hash >> 11;
  }

  switch (rem) {
    case 3:
      hash += get16bits(data);
      hash ^= hash << 16;
      hash ^= data[sizeof(uint16_t)] << 18;
      hash += hash >> 11;
      break;
    case 2:
      hash += get16bits(data);
      hash ^= hash << 11;
      hash += hash >> 17;
      break;
    case 1:
      hash += *data;
      hash ^= hash << 10;
      hash += hash >> 1;
  }

  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;

  return hash;
}

// replace environment variables of the form ${ENVVAR}
std::string& StrCol::replaceEnvironmentVariables(std::string& s) {
  std::string::size_type varbegin = 0, varend = 0;

  // find first opening parenthesis
  varbegin = s.find("${", 0);
  while (varbegin != std::string::npos) {

    // find closing parenthesis
    varend = s.find("}", varbegin + 2);
    if (varend != std::string::npos) {

      // check if '$' is protected by a backslash
      std::string::size_type pos = varbegin, even = 1;
      while (pos > 0 ? s.at(--pos) == '\\' : (even = 0))
        ;
      if ((varbegin - pos) % 2 != even) {
        // odd number of backslashes, do not replace, but remove last backslash
        s.erase(varbegin - 1, 1);
        varend = varbegin;
      } else {
        // get the value of the environment variable
        const char* vstr = getenv(s.substr(varbegin + 2, varend - varbegin - 2).c_str());
        std::string value = vstr ? vstr : "";
        // replace the variable with its value
        s.replace(varbegin, varend - varbegin + 1, value);
        varend = varbegin + value.length();
      }
      // find next variable
      varbegin = s.find("${", varend);

    } else {
      // end of string
      varbegin = std::string::npos;
    }
  }
  return s;
}

} // namespace Puma
