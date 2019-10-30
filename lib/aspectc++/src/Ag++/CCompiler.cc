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
#include "System.h"
#include "regex.h"

//Puma includes
#include "Puma/VerboseMgr.h"
#include "Puma/PathManager.h"
using namespace Puma;

bool
CCompiler::execute()
{

  VerboseMgr vm(cout, _config.verbose());
  bool status = true;

  if (_config.make_dep())
  {
    vm++;
    vm << "Creating dependencies" << endvm;
    status = make_dep();
  }
  else
  {
    vm << "Compiling" << endvm;
    status = compile();
  }

  return status;
}

bool
CCompiler::make_dep()
{
  AGxxConfig::FileCont::iterator file = _config.proj_paths().begin();

  // if there were no aspect headers in command string
  // search all aspect headers in all project paths
  if (_config.ah_files().empty())
  {

    PathManager path(this->_err);

    while (file != _config.proj_paths().end())
    {
      path.addPath(file->c_str());
      file++;
    }

    PathIterator ah_iter(".*\\.ah$");
    while (path.iterate(ah_iter))
    {
      _config.ah_files().push_back(ah_iter.file());
    }
  }

  /*
   * setup execution string for gcc
   */
  string exec_str = "\"" + _config.cc_bin() + "\"";

  file = _config.ah_files().begin();

  OptionVec::iterator opt = _config.optvec().begin();
  while (opt != _config.optvec().end())
  {
    // find all g++ options and files
    switch ((opt->flag()
        & (OptionItem::OPT_GCC | OptionItem::OPT_FILE | OptionItem::OPT_DEP)))
    {
      case OptionItem::OPT_GCC:
        exec_str.append(" " + opt->name() + opt->arg());
        break;
      case OptionItem::OPT_DEP:
        exec_str.append(" " + opt->name() + opt->arg());
        break;
      case OptionItem::OPT_FILE:

        // explicitly include all aspect headers (for creating dependency information)
        //this is only executed once, as 'file' iterator is not reseted
        while (file != _config.ah_files().end())
        {
          exec_str += " -include \"" + *file + "\"";
          file++;
        }

        exec_str.append(" \"" + opt->name() + "\"");

        // explicitly include all aspect headers (for creating dependency information)
        //this is only executed once, as 'file' iterator is not reseted
        while (file != _config.ah_files().end())
        {
          exec_str += " -include \"" + *file + "\"";
          file++;
        }
        break;
    }
    opt++;
  }

  // execute gcc
  System gcc(_err, _config, exec_str);
  return gcc.execute();
}

bool
CCompiler::compile()
{

  // construct execution string
  string exec_str = "\"" + _config.cc_bin() + "\"";

  OptionVec::iterator opt = _config.optvec().begin();

  // loop through all options in option vector
  while (opt != _config.optvec().end())
  {
    // g++ options
    if ((opt->flag() & (OptionItem::OPT_GCC | OptionItem::OPT_FILE))
        == OptionItem::OPT_GCC)
    {
      exec_str.append(" " + opt->name() + opt->arg());
    }
    // woven source files
    else if ((opt->flag()
        & (OptionItem::OPT_GCC | OptionItem::OPT_ACC | OptionItem::OPT_FILE))
        == (OptionItem::OPT_GCC | OptionItem::OPT_ACC | OptionItem::OPT_FILE))
    {
      // explictly mark .acc files  as c++ files
      exec_str.append(" -xc++ \"" + opt->name() + "\" -xnone");
    }
    // other source files
    else if ((opt->flag() & (OptionItem::OPT_GCC | OptionItem::OPT_FILE))
        == (OptionItem::OPT_GCC | OptionItem::OPT_FILE))
    {
      exec_str.append(" \"" + opt->name() + "\" ");
    }
    opt++;
  }

  // execute compiler
  System gcc(_err, _config, exec_str);
  return gcc.execute();
}

