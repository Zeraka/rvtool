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

#ifndef __StdSystem_h__
#define __StdSystem_h__

#include "System.h"

//Puma includes
#include "Puma/ErrorStream.h"
using namespace Puma;

// stdc++ includes
#include<string>
using namespace std;

// fill me
class StdSystem : public System
{

protected:

  // temporary file which contains standard output after execution
  string _tmp_stdout_file;

  // temporary file which contains standard error after execution
  string _tmp_stderr_file;

  // string which contains standard error
  string _stdout_str;

  // string which contains standard output 
  string _stderr_str;

  // read file content into string
  string
  read_file(const char* fname);

  // print file to stderr
  void
  print_file(const char* fname);

public:
  // Constructor
  StdSystem(ErrorStream&, AGxxConfig&, const string&);

  // execute command specified in _exec_str
  bool
  execute();

  //
  //methods for accessing private data
  //

  const string&
  stdout_str()
  {
    return _stdout_str;
  }

  const string&
  stderr_str()
  {
    return _stderr_str;
  }

};

#endif //__StdSystem_h__
