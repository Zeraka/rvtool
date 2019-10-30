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

#ifndef __System_h__
#define __System_h__

#include "AGxxConfig.h"

//Puma includes
#include "Puma/ErrorStream.h"
using namespace Puma;

// stdc++ includes
#include<string>
using namespace std;

class System
{

protected:

  // an error stream
  ErrorStream &_err;

  // configuration
  AGxxConfig &_config;

  // execution command
  string _exec_str;

public:
  // Constructor
  System(ErrorStream& e, AGxxConfig& c, const string& exec_str);

  // execute command specified in _exec_str
  bool
  execute();

};

#endif //__System_h__
