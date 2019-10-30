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

#include "Puma/PathInfo.h"
#include "Puma/StrCol.h"
#include <string.h>
#include <stdlib.h>

namespace Puma {

PathInfo::PathInfo(const PathInfo &pi) {
  *this = pi;
}

PathInfo::~PathInfo() {
  if (m_srcPath)
    delete[] m_srcPath;
  if (m_srcPathCanonical)
    delete[] m_srcPathCanonical;
  if (m_destPath)
    delete[] m_destPath;
}

// Needed for class Array.
PathInfo &PathInfo::operator =(const PathInfo &pi) {
  src(pi.src());
  srcCanonical(pi.srcCanonical());
  dest(pi.dest());
  return *this;
}

// Set the source path.
void PathInfo::src(const char *path) {
  if (!path)
    return;
  if (m_srcPath)
    delete[] m_srcPath;
  m_srcPath = StrCol::dup(path);
}

// Set the canonical source path.
void PathInfo::srcCanonical(const char *path) {
  if (!path)
    return;
  if (m_srcPathCanonical)
    delete[] m_srcPathCanonical;
  m_srcPathCanonical = StrCol::dup(path);
}

// Set the destination path.
void PathInfo::dest(const char *path) {
  if (m_destPath)
    delete[] m_destPath;
  m_destPath = (const char*) 0;
  if (path)
    m_destPath = StrCol::dup(path);
}

} // namespace Puma
