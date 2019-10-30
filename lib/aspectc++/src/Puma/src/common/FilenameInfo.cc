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

#include "Puma/FilenameInfo.h"
#include <string.h>
#include <ctype.h>

namespace Puma {

FilenameInfo::~FilenameInfo() {
  if (m_path)
    delete[] m_path;
  if (m_root)
    delete[] m_root;
}

void FilenameInfo::name(const char *n) {
  m_name = n;

  // check if the name ends with '/' or '\\'
  if (n) {
    int len = strlen(n);
    bool have_trailing_delim = false;
    while (len >= 1 && (n[len - 1] == '/' || n[len - 1] == '\\')) {
      have_trailing_delim = true;
      len--;
    }
    len++;
    if (have_trailing_delim) {
      // assign _name with a stripped temporary name
      char * new_name = new char[len + 1];
      strcpy(new_name, n);
      new_name[len - 1] = '\0';
      m_name = new_name;
      delete[] new_name;
    }
  }

  if (m_path)
    delete[] m_path;
  m_path = (const char*) 0;
  if (m_root)
    delete[] m_root;
  m_root = (const char*) 0;
}

const char *FilenameInfo::path() {
  if (m_name.empty())
    return (const char*) 0;
  if (m_path)
    return m_path;

  const char *cname = m_name.c_str();
  const char *delim = cname + strlen(cname) - 1;
  while (delim >= cname && *delim != '/' && *delim != '\\' && *delim != ':')
    delim--;
  if (delim < cname)
    return "";

  int len = delim - cname;
  if (len == 0)
    len = 1;

  m_path = new char[len + 1];
  strncpy((char*) m_path, cname, len);
  ((char*) m_path)[len] = '\0';
  return m_path;
}

const char *FilenameInfo::root() {
  if (m_name.empty() || !is_absolute())
    return 0;
  if (m_root)
    return m_root;

  const char *cname = m_name.c_str();
  int len = 0;
  while (cname[len] != '/' && cname[len] != '\\')
    len++;
  m_root = new char[len + 2];
  strncpy((char*) m_root, cname, len + 1);
  ((char*) m_root)[len + 1] = '\0';
  return m_root;
}

bool FilenameInfo::is_absolute() const {
  const char *cname = m_name.c_str();
  int len = strlen(cname);
  return (len >= 1 && (cname[0] == '/' || cname[0] == '\\')) || (len >= 3 && cname[1] == ':' && (cname[2] == '/' || cname[2] == '\\') && isalpha(cname[0]));
}

} // namespace Puma
