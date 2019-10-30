// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2004  The 'ac++' developers (see aspectc.org)
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

#include "file.h"

// Puma includes
#include "Puma/RegComp.h"
using namespace Puma;

// stdc++ includes
#include <fstream>
#include <iostream>
using namespace std;

bool
file::fileExists(const char* file)
{
  ifstream fs(file);
  if (!fs)
  {
    return false;
  }
  fs.close();

  return true;
}

void
file::MakeUnixPath(string& path)
{
  if (path.empty())
    return;
  for (string::iterator c = path.begin(); c != path.end(); c++)
  {
    if (*c == '\\')
      *c = '/';
  }
}
void
file::MakeDosPath(string& path)
{
  if (path.empty())
    return;
  for (string::iterator c = path.begin(); c != path.end(); c++)
  {
    if (*c == '/')
      *c = '\\';
  }
}
void
file::stripFilename(string& s, bool include_delimiter)
{
  string::size_type pos, pos2;
  pos = s.find_last_of('\\');
  pos2 = s.find_last_of('/');
  if (pos == string::npos)
  {
    if (pos2 == string::npos)
    {
      pos = 0;
    }
    else
    {
      pos = pos2;
      if (include_delimiter)
        pos++;
    }
  }
  else
  {
    if (pos2 != string::npos && pos2 > pos)
    {
      pos = pos2;
    }
    if (include_delimiter)
      pos++;
  }

  s.erase(pos);
}

void
file::stripPath(string& s)
{
  string::size_type pos, pos2;
  pos = s.find_last_of('\\');
  pos2 = s.find_last_of('/');
  if (pos == string::npos)
  {
    if (pos2 == string::npos)
    {
      pos = 0;
    }
    else
    {
      pos = pos2;
      pos++;
    }
  }
  else
  {
    if (pos2 != string::npos && pos2 > pos)
    {
      pos = pos2;
    }
    pos++;
  }
  s.erase(0, pos);
}

