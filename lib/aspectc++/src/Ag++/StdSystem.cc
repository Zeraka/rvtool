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

#include "StdSystem.h"
#include "file.h"
using namespace file;

//Puma includes
#include "Puma/SysCall.h"
#include "Puma/VerboseMgr.h"

//stdc++ includes
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>

StdSystem::StdSystem(ErrorStream& e, AGxxConfig& config, const string& exec_str) :
    System(e, config, "")
{

  // create temporary files
  char* tmp_file;

  // tmp file for stdout
  tmp_file = (char*) Puma::SysCall::mktemp("agxx_stdout", &_err);
  _tmp_stdout_file = tmp_file;
  free(tmp_file);

  // tmp file for stderr
  tmp_file = (char*) Puma::SysCall::mktemp("agxx_stderr", &_err);
  _tmp_stderr_file = tmp_file;
  free(tmp_file);

  // append shell commands to execution string which 
  // write the output into temproray files
#if defined (WIN32)
  this->_exec_str="\""+exec_str +" 2>"+_tmp_stderr_file+" 1>"+_tmp_stdout_file+"\"";
#else
  this->_exec_str = exec_str + " 2>" + _tmp_stderr_file + " 1>"
      + _tmp_stdout_file;
#endif
}

bool
StdSystem::execute()
{
  VerboseMgr vm(cout, _config.verbose());
  bool ret;

  if ((ret = System::execute()))
  {
    //read files into strings
    _stdout_str = read_file(_tmp_stdout_file.c_str());
    _stderr_str = read_file(_tmp_stderr_file.c_str());
  }
  else
  {
    print_file(_tmp_stderr_file.c_str());
  }

  vm++;
  vm << "removing temporary file: " << _tmp_stdout_file.c_str() << endvm;
  if (remove(_tmp_stdout_file.c_str()) < 0)
  {
    _err << sev_warning << "Could not remove temporary file : "
        << _tmp_stdout_file.c_str() << endMessage;
  }

  vm << "removing temporary file: " << _tmp_stderr_file.c_str() << endvm;
  if (remove(_tmp_stderr_file.c_str()) < 0)
  {
    _err << sev_warning << "Could not remove temporary file : "
        << _tmp_stderr_file.c_str() << endMessage;
  }

  return ret;
}

string
StdSystem::read_file(const char* fname)
{
  string line, s;

  ifstream fs(fname);
  if (fs)
  {
    while (getline(fs, line))
    {
      s += line + '\n';
    }
    fs.close();
  }
  else
  {
    _err << sev_error << "Could not open file :" << fname << endMessage;
    s = "";
  }

  return s;
}

void
StdSystem::print_file(const char* fname)
{
  string line;

  ifstream fs(fname);
  if (fs)
  {
    while (getline(fs, line))
    {
      cerr << line << endl;
    }
    fs.close();
  }
  else
  {
    _err << sev_error << "Could not open file :" << fname << endMessage;
  }
}

