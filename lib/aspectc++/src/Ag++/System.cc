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

#include "System.h"

//Puma includes
#include "Puma/VerboseMgr.h"

#include <cstdlib>

System::System(ErrorStream& e, AGxxConfig& c, const string& exec_str) :
    _err(e), _config(c)
{

#if defined (WIN32)
  this->_exec_str="\""+exec_str+"\"";
#else
  this->_exec_str = exec_str;
#endif

}
bool
System::execute()
{
  VerboseMgr vm(cout, _config.verbose());

  vm++;
  vm << "Executing: " << _exec_str << endvm;

  // execute command in shell
  int ret;
  if ((ret = system(_exec_str.c_str())) == EXIT_SUCCESS)
  {
    vm << "Exit: Success" << endvm;
    return true;
  }

  _err << sev_error << "Execution failed: " << _exec_str.c_str() << endMessage;

  vm << "Exitcode: " << ret << " (should be " << EXIT_SUCCESS<< " )" << endvm;
  return false;
}

