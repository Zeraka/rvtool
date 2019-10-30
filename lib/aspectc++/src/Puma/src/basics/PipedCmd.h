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

#ifndef PUMA_PipedCmd_H
#define PUMA_PipedCmd_H

/** \file
 * External command execution implementation. */

#include <string>
#include <stdio.h>

namespace Puma {

/** \class PipedCmd PipedCmd.h Puma/PipedCmd.h
 * Executes a shell command and gives access to the output of the command. 
 * \ingroup basics */
class PipedCmd {
  std::string m_Output;

  void reset();
  void readin(FILE *);

public:
  /** Constructor. */
  PipedCmd() {
  }

  /** Execute the given shell command.
   * \param cmd The shell command.
   * \return The return code of the command. */
  int exec(const char* cmd);

  /** Get the output produced by the executed command.
   * \return A reference to a string with the recored output. */
  const std::string &getOutput() const {
    return m_Output;
  }
};

} // namespace Puma

#endif /* PUMA_PipedCmd_H */
