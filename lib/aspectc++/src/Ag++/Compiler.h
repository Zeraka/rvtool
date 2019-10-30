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

#ifndef __Compiler_h__
#define __Compiler_h__

#include "AGxxConfig.h"

// PUMA includes 
#include "Puma/ErrorStream.h"

class Compiler
{
protected:
  ErrorStream &_err;
  AGxxConfig &_config;
public:
  Compiler(ErrorStream& e, AGxxConfig& config) :
      _err(e), _config(config)
  {
  }
  Compiler(Compiler& compiler) :
      _err(compiler._err), _config(compiler._config)
  {
  }
  ~Compiler()
  {
  }
  void
  execute();
};
#endif // __Compiler_h__	
