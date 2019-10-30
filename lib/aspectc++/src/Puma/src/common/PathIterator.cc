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

#include "Puma/PathIterator.h"
#include "Puma/ErrorStream.h"
#include "Puma/RegComp.h"

namespace Puma {

PathIterator::PathIterator(const char *pat) {
  ErrorStream err;
  m_searchPattern = new RegComp(pat ? pat : (const char*) ".*");
  m_searchPattern->comperror(err);
}

PathIterator::~PathIterator() {
  if (m_searchPattern)
    delete m_searchPattern;
}

// Set the search pattern.
void PathIterator::pattern(const char *pat) {
  if (m_searchPattern)
    delete m_searchPattern;                // Match anything.
  m_searchPattern = new RegComp(pat ? pat : (const char*) ".*");
}

// retrieve information about the currently matching file
const char *PathIterator::file() const {
  return m_currentFile.c_str();
}

} // namespace Puma
