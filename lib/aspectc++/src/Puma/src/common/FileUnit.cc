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

#include "Puma/FileUnit.h"
#include "Puma/ErrorStream.h"
#include "Puma/SysCall.h"
#include "Puma/StrCol.h"
#include "Puma/Project.h"
#include <string.h>
#include <stdlib.h>

namespace Puma {

// File belongs to given project?
bool FileUnit::belongsTo(Project &p) {
  // same project yields same status, of course
  if (m_project == &p)
    return m_isBelow;

  m_project = &p;
  m_isBelow = p.isBelow(Unit::name());
  return m_isBelow;
}

void FileUnit::setLocation(const char *location) {
  if (m_lastLocation)
    delete[] m_lastLocation;
  m_lastLocation = StrCol::dup(location);
}

void FileUnit::name(const char *n) {
  Unit::name(n);
  if (m_absolutePath)
    delete[] m_absolutePath;
  m_absolutePath = SysCall::absolute(n);
  if (!m_absolutePath)
    m_absolutePath = n ? StrCol::dup(n) : (char*) 0;
}

} // namespace Puma
