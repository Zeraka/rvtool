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

#include "Puma/VerboseMgr.h"
#include <iomanip>

namespace Puma {

void VerboseMgr::endl() {
  *this << std::endl;

  if (m_level < m_verbose) {
    for (int l = 0; l < m_level; l++)
      m_out << "  ";
    if (m_level == 0)
      m_out << "* ";
    else if (m_level == 1)
      m_out << "- ";
    m_out << str().c_str();
  }

  std::ostringstream::str("");
  std::ostringstream::clear();
}

} // namespace Puma
