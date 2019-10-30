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

#include "Puma/PipedCmd.h"

#ifdef _MSC_VER
#define popen _popen
#define pclose _pclose
#endif

namespace Puma {

void PipedCmd::readin(FILE *fin) {
  static const int BUFSIZE = 1024;

  char buf[BUFSIZE];
  int len;

  while ((len = fread(buf, 1, BUFSIZE - 1, fin)) != 0) {
    buf[len] = 0;
    m_Output += buf;
  }
}

void PipedCmd::reset() {
  fflush(stdout);
  m_Output = "";
}

int PipedCmd::exec(const char *cmd) {
  int res = -1;

  reset();

  FILE *file = popen(cmd, "r");
  if (file != 0) {
    readin(file);
    res = pclose(file);
  }

  return res;
}

} // namespace Puma
