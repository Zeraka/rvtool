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

#include "Puma/SimpleParser.h"

namespace Puma {

std::string SimpleParser::strip(const std::string &str) {
  if (str[0] == '"' && str[str.length() - 1] == '"')
    return str.substr(1, str.length() - 2);
  return str;
}

std::string::size_type SimpleParser::findBeginOfNextToken(const std::string& line, const std::string& delimiters, std::string::size_type pos) {
  bool escaped = false;
  std::string::size_type lastPos = std::string::npos;
  if (pos != std::string::npos) {
    for (std::string::size_type i = pos; i < line.length() && lastPos == std::string::npos; i++) {
      char c = line[i];
      if (delimiters.find(c) == std::string::npos) {
        lastPos = escaped ? i - 1 : i;
      }
      if (c == '\\') {
        escaped = !escaped;
      } else {
        escaped = false;
      }
    }
  }
  return lastPos;
}

std::string::size_type SimpleParser::findEndOfNextToken(const std::string& line, const std::string& delimiters, std::string::size_type lastPos) {
  bool in_string = false, escaped = false;
  std::string::size_type endpos = line.length();
  if (lastPos != std::string::npos) {
    for (std::string::size_type i = lastPos; i < line.length(); i++) {
      char c = line[i];
      if (!in_string && delimiters.find(c) != std::string::npos) {
        endpos = i;
        break;
      }
      if (!escaped && c == '"') {
        in_string = !in_string;
      }
      if (c == '\\') {
        escaped = !escaped;
      } else {
        escaped = false;
      }
    }
  }
  return endpos;
}

std::string::size_type SimpleParser::next(const std::string& line, const std::string& delimiters, std::string::size_type pos, std::string::size_type& lastPos) {
  // find begin of next token
  lastPos = findBeginOfNextToken(line, delimiters, pos);
  // find end of the token
  return findEndOfNextToken(line, delimiters, lastPos);
}

int SimpleParser::tokenize(const std::string& line, std::vector<std::string>& words, const std::string& delimiters) {
  int res = 0;
  std::string::size_type pos = 0, lastPos = 0;

  // get first token position
  pos = next(line, delimiters, 0, lastPos);

  // while not at the end of the line
  while (pos != std::string::npos && lastPos != std::string::npos) {
    // get current token
    std::string token = line.substr(lastPos, pos - lastPos);
    // add current token to vector
    words.push_back(token);
    res++;
    // get next token position
    pos = next(line, delimiters, pos, lastPos);
  }
  return res;
}

} // namespace Puma
