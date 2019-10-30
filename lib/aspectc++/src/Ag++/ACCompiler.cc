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

#include "ACCompiler.h"
#include "System.h"
#include "regex.h"
#include "file.h"

//stdc++ includes
#include<string>
using namespace std;
using namespace file;

//Puma includes
#include "Puma/VerboseMgr.h"
#include "Puma/SysCall.h"

bool
ACCompiler::execute()
{

  VerboseMgr vm(cout, _config.verbose());
  vm << "Weaving" << endvm;

  // command line string
  string exec_str = "\"" + _config.acc_bin() + "\" --system-config \""
      + _config.pumaconfig_file() + "\" "
      + _config.optvec().getString(OptionItem::OPT_ACC, OptionItem::OPT_FILE);

  // get output path for .acc files from -o option
  string acc_output_path = _config.output_file();
  file::stripFilename(acc_output_path);

  // loop through all options searching a file
  OptionVec::iterator opt = _config.optvec().begin();
  while (opt != _config.optvec().end())
  {
    // skip all options that are not a file which should be processed by
    // AspectC++
    if ((opt->flag() & (OptionItem::OPT_ACC | OptionItem::OPT_FILE))
        != (OptionItem::OPT_ACC | OptionItem::OPT_FILE))
    {
      opt++;
      continue;
    }

    const string file = opt->name();

    //construct name for output file
    string out_file;

    // if an output file for the woven result is not specified on command line
    // or the file(s) shall be compiled after weaving put an intermediate .acc file
    // in acc_output_path
    if (_config.compile() == true || _config.output_file().empty())
    {
      //if we do not want to keep the intermediate acc files
      //generate files in the systems temporary directory
      if (_config.keep_woven() == false)
      {
        string prefix(file);
        stripPath(prefix);
        prefix.append("_agxx_");
        char* tmp_file = (char*) Puma::SysCall::mktemp(prefix.c_str(), &_err);
        out_file = tmp_file;
        free(tmp_file);
      }
      //otherwise create name of output file from name of output file
      else
      {
        string tmp(file);
        stripPath(tmp);
        out_file = tmp;
        int endpos = out_file.rfind('.');
        out_file.replace(endpos, string::npos, ".acc");
        out_file = acc_output_path + out_file;
      }
    }
    else
    {
      out_file = _config.output_file();
    }

    // construct final command string
    string local_exec_str = exec_str + " -c \"" + file + "\" -o \"" + out_file
        + "\"";

    // execute AspectC++
    System acc(_err, _config, local_exec_str);

    //change filename in input_file list, so that the next steps
    //get the proper filename
    opt->name(out_file);
    opt->flag(opt->flag() | (OptionItem::OPT_GCC));

    if (!acc.execute())
    {
      return false;
    }

    opt++;
  }

  // Call Aspect C++ once in case there are no input files
  if ((_config.nr_files() == 0) && (_config.run_acpp_without_input() == true))
  {
    System acc(_err, _config, exec_str);
    return acc.execute();
  }

  return true;
}
