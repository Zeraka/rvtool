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

#ifndef __PumaConfigFile_h__
#define __PumaConfigFile_h__

#include "AGxxConfig.h"

// Puma includes
#include "Puma/RegComp.h"
#include "Puma/ErrorStream.h"
using namespace Puma;

// stdc++ includes
#include<string>
#include<vector>
#include<map>
#include<fstream>

/* This class provides functions, to search and to create a 
 * puma config file
 */

class PumaConfigFile
{
private:
  ErrorStream &_err;
  AGxxConfig &_config;

  // list of include paths
  vector<string> _inc_paths;

  // a map for options and its values
  map<string, string> _def_options;

  // type of option-value pair 
  typedef pair<string, string> opt_pair;

  // the target triple used by the GCC-Compiler
  string _target_triple;

  // parse the output of the GCC-Compiler 
  bool
  parseCcOutput();
  bool
  writeFile();

public:
  PumaConfigFile(ErrorStream& e, AGxxConfig& c) :
      _err(e), _config(c), _target_triple(c.target_triple())
  {
  }
  ~PumaConfigFile()
  {
  }
  ;
  bool
  searchFile();
  bool
  generateFile();

};

#endif // __PumaConfigFile_h__
