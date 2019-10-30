// This file is part of PUMA.
// Copyright (C) 1999-2003  The PUMA developer team.
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

#ifndef __CProject_h__
#define __CProject_h__

// Special project derivation for C++ sources.

#include "Puma/Config.h"
#include "Puma/Project.h"
#include "Puma/CScanner.h"
#include "Puma/ErrorStream.h"

namespace Puma {


class Source;
class CTypeInfo;

class CProject : public Project {
  Config _config;
  CScanner _scanner;

public:
  CProject (ErrorStream &e);

  // Constructor: Add some source directories.
  CProject (ErrorStream &, const char *defs, 
               const char *decls = (const char*)0); 

  // Constructor: Setup preprocessor configuration according to
  // command line parameter and the global config file and
  // add some source directories.
  CProject (ErrorStream &, int &argc, char **&argv, 
            const char *defs = (const char*)0, 
            const char *decls = (const char*)0);

public: // Project configuration 
  // Get the preprocessor configuration.
  Config &config ();

  // Configure the project from the command line or a file.
  virtual void configure (Config &);

  // Add a new file to the project.
  Unit *addFile (Filename file);

public: // Scanner interface
  // Get the scanner.
  CScanner &scanner ();

  // Scan the file `file' or the source if given. When
  // scanning a source the resulting unit gets the name `file'.
  virtual Unit *scanFile (const char *file, Source * = (Source*)0, 
                          bool isFile = true);
        
  // Scan a string. The resulting unit gets the name `name'.
  // (and will not be registered at the unit manager!)
  virtual Unit *scanString (const char *str, 
                            const char *name = (const char*)0);

public: // Miscellaneous Functions

  // Give the filename needed to include a particular header file
  Filename getInclString (Filename filename);

private:
  // Map the type string from --size-type and --ptrdiff-type
  // to real Puma type.
  CTypeInfo *mapType (const char *) const;
};

inline Config &CProject::config () 
 { return _config; }
inline CScanner &CProject::scanner () 
 { return _scanner; }


} // namespace Puma

#endif /* __CProject_h__ */
