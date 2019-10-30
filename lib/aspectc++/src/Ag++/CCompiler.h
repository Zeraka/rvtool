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

#ifndef __CCompiler_h__
#define __CCompiler_h__

#include "Compiler.h"
#include "AGxxConfig.h"

class CCompiler : public Compiler
{
private:

public:
  CCompiler(ErrorStream& e, AGxxConfig& c) :
      Compiler(e, c)
  {
  }
  ~CCompiler()
  {
  }
  bool
  execute();
  bool
  compile();
  bool
  make_dep();
};

#endif // __CCompiler_h__

