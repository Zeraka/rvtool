// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2003  The 'ac++' developers (see aspectc.org)
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

// AC++ includes
#include "ACProject.h"
#include "Weaver.h"
#include "ACErrorStream.h"

using namespace Puma;

int main (int argc, char **argv) { 
  
  // the error stream for all warnings, error messages, etc.
  ACErrorStream err;

  try {
    // Create a project that can either accept the project description with
    // the standard Puma options or by a 'project file'
    ACProject project(err, argc, argv);

    // Create the configuration object for the weaver
    ACConfig config (project, argc, argv);

    // analyze the command line arguments
    if (config.analyze ()) {

      // check if code has to be woven
      // or a pointcut expression shall be matched against the repository
      if (config.file_out () || config.ifiles () || config.iterate () ||
          !config.expr().empty()) {
        // Create an AspectC++ weaver, which manipulates (parts of) the project
        Weaver ac (project, config);
        // do the necessary weaving
        ac.weave ();
      }
    }
  }
  catch (const std::bad_alloc &e) {
    err << sev_fatal << "Caught memory allocation error: " << e.what () << endMessage;
  }
  catch (const std::exception &e) {
    err << sev_fatal << "Caught standard exception: " << e.what () << endMessage;
  }
  catch (...) {
    err << sev_fatal << "Caught unknown exception!" << endMessage;
  }
  return (err.severity () <= sev_warning) ? 0 : 1;
}

