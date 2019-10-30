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

#include "CCompiler.h"
#include "ACCompiler.h"
#include "AGxxConfig.h"
#include "PumaConfigFile.h"

#include "file.h"

// PUMA includes 
#include "Puma/ErrorStream.h"
#include "Puma/VerboseMgr.h"

// standard library includes
#include <stdlib.h>

using namespace Puma;

int
main(int argc, char **argv)
{
  ErrorStream err;
  bool optionsOk;
  int exitstatus = EXIT_SUCCESS;

  try {
    // Setup Configuration
    AGxxConfig config(err, argc, argv);

    optionsOk = config.analyze();

    if (config.verbose() > 0)
    {
      config.printOptions();
    }
    if (!optionsOk)
    {
      return EXIT_FAILURE;
    }

    VerboseMgr vm(cout, config.verbose());

    // Create a Puma configuration file if it does not exist
    if (config.gen_pumaconfig())
    {
      PumaConfigFile pumacfg(err, config);
      if (!pumacfg.searchFile())
      {
        if (!pumacfg.generateFile())
        {
          return EXIT_FAILURE;
        }
      }
      else
      {
        config.gen_pumaconfig(false);
      }
    }

    // Create an AspectC++ compiler object
    if (config.weave())
    {
      ACCompiler acc(err, config);
      if (!acc.execute())
      {
        // make sure tmp files are cleaned up
        exitstatus = EXIT_FAILURE;
      }
    }

    // Create a C compiler object
    if (exitstatus == EXIT_SUCCESS && config.compile())
    {
      CCompiler cc(err, config);
      if (!cc.execute())
      {
        // make sure tmp files are cleaned up
        exitstatus = EXIT_FAILURE;
      }
    }

    //
    // clean up temporary files
    //

    vm++;

    // remove temporary puma.config
    if (config.pumaconfig_tmp() == true)
    {
      vm << "Removing temporary puma configuration file" << endvm;
      if (remove(config.pumaconfig_file().c_str()) < 0)
      {
        err << sev_warning << "Could not remove temporary file : "
            << config.pumaconfig_file().c_str() << endMessage;
      }
    }

    // remove intermediate acc files remove them
    if (config.keep_woven() == false)
    {
      vm << "Removing intermediate .acc files" << endvm;
      OptionVec::iterator opt = config.optvec().begin();
      while (opt != config.optvec().end())
      {
        if (opt->flag()
            != (OptionItem::OPT_ACC | OptionItem::OPT_GCC | OptionItem::OPT_FILE))
        {
          ++opt;
          continue;
        }
        // check if removal succeeded
        string rm_file(opt->name());
        if (remove(rm_file.c_str()) < 0)
        {
          err << sev_warning << "Couldn't remove " << rm_file.c_str()
              << endMessage;
        }
        ++opt;
      }
    }
  }
  catch (const std::exception &e) {
    err << sev_fatal << "Caught standard exception: " << e.what () << endMessage;
    return EXIT_FAILURE;
  }
  catch (...) {
    err << sev_fatal << "Caught unknown exception!" << endMessage;
    return EXIT_FAILURE;
  }
  return exitstatus;
}

