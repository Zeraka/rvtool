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

#ifndef __AGxxConfig_h__
#define __AGxxConfig_h__

#include "OptionVec.h"

// PUMA includes
#include "Puma/OptsParser.h"
#include "Puma/ErrorStream.h"
using namespace Puma;

// stdc++ includes
#include <string>
#include <list>
using namespace std;

class AGxxConfig
{

public:
  typedef list<string> FileCont;

private:

  // parse options
  bool
  parseOptions();

  // parse some special g++ options
  bool
  parseGccOpt(OptsParser&);

  // analyze options
  bool
  analyzeOptions();

  //print a usage message
  void
  usage() const;

  ErrorStream &_err;
  int _argc;
  char** _argv;

  // list of .ah files passed with -a option
  FileCont _ah_files;

  // path to project directory
  FileCont _proj_path;

  // output file name
  string _output_file;

  // name of puma config file
  string _pumaconfig_file;

  // path to aspectc++ compiler
  string _acc_bin;

  // path to c++ compiler
  string _cc_bin;

  // Option Vector
  OptionVec _optvec;

  // path to destination directory
  string _dest_path;

  // command, which is executed to get the information about
  // compiler, include paths, defines.
  // default is: <compiler_name> <compiler options> -E -dM -v -x c++ /dev/null
  string _config_command;

  // number of files to be processed
  int _nr_files;

  // verbose
  int _verbose;

  // indicate wether a config file should be generated
  bool _gen_pumaconfig;

  // indicate wether weaving should be done
  bool _weave;

  // indicate wether linking should be done
  bool _link;

  // indicate wether compiling should be done
  bool _compile;

  // use temporary file for puma config file
  bool _pumaconfig_tmp;

  // do weave the source files (no compiling or linking)
  bool _weave_only;

  // only let gcc print information
  bool _print_gcc_info;

  // generate include files
  bool _gen_incl;

  // if include files are generated
  bool _gen_includes;

  // do only compile not link (but weave)
  bool _compile_only;

  // indicate wether dependency information should be printed out
  bool _make_dep;

  // keep woven source code  files
  bool _keep_woven;

  // assign all following options to the compiler
  bool _x_compiler;

  // list of gcc options which need a argument
  static string _gcc_option_arg[];

  // list of gcc options which only print information
  static string _gcc_option_info[];

  // run Aspect C++ without input files
  bool _run_acpp_without_input;

  // target triple from the ag++ command line (overrides value from g++)
  string _command_line_target_triple;


public:
  AGxxConfig(ErrorStream &e, int argc, char **argv) :
      _err(e), _argc(argc), _argv(argv)
  {
  }

  ~AGxxConfig()
  {
  }

  enum
  {
    AGOPT_COMPILE_ONLY = 1,
    AGOPT_WEAVE_ONLY,
    AGOPT_GEN_CFG,
    AGOPT_OUTPUT,
    AGOPT_ACC,
    AGOPT_CC,
    AGOPT_HELP,
    AGOPT_KEEP_WOVEN,
    AGOPT_CONFIG_COMMAND,
    AGOPT_TARGET,
    AGOPT_X_COMPILER,
    AGOPT_X_WEAVER,
    AGOPT_VERBOSE,
    AGOPT_VERSION,
    // Ac++ options
    ACOPT, // default handling for AspectC++ options
    ACOPT_QUOTE_ARG, // default handling for AspectC++ options where the argument has to be quoted
    ACOPT_WITHOUT_INPUT, // default handling for AspectC++ options running without input files
    ACOPT_MATCH_POINTCUT_EXPR,
    ACOPT_INCLUDE_FILES,
    ACOPT_ASPECT_HEADER,
    ACOPT_PROJ_PATH,
    ACOPT_PROJ_DESTINATION,
    ACOPT_CFG_FILE,
    ACOPT_SYS_CFG_FILE,
    ACOPT_PRE_INCLUDE,
    ACOPT_PRE_DEFINE,
    ACOPT_PRE_UNDEFINE,
    ACOPT_ALWAYS_INCLUDE,
    ACOPT_SYS_INCLUDE
  };

  // anaylze the arguments
  bool
  analyze();

  // print Options
  void
  printOptions();

  //
  // Methods for accesing private data
  //

  const bool
  gen_pumaconfig()
  {
    return _gen_pumaconfig;
  }
  void
  gen_pumaconfig(const bool b)
  {
    _gen_pumaconfig = b;
  }

  const bool
  weave()
  {
    return _weave;
  }

  const bool
  compile()
  {
    return _compile;
  }

  const bool
  make_dep()
  {
    return _make_dep;
  }

  const bool
  link()
  {
    return _link;
  }

  const bool
  keep_woven()
  {
    return _keep_woven;
  }

  const bool
  gen_includes()
  {
    return _gen_includes;
  }

  const int
  nr_files()
  {
    return _nr_files;
  }

  const int
  verbose()
  {
    return _verbose;
  }

  OptionVec&
  optvec()
  {
    return _optvec;
  }

  FileCont&
  ah_files()
  {
    return _ah_files;
  }

  FileCont&
  proj_paths()
  {
    return _proj_path;
  }

  const string&
  pumaconfig_file()
  {
    return _pumaconfig_file;
  }

  const string&
  target_triple()
  {
    return _command_line_target_triple;
  }

  void
  pumaconfig_file(const string& s)
  {
    _pumaconfig_file = s;
  }

  const bool&
  pumaconfig_tmp()
  {
    return _pumaconfig_tmp;
  }

  const string&
  output_file()
  {
    return _output_file;
  }
  void
  output_file(const string& s)
  {
    _output_file = s;
  }

  const string&
  acc_bin()
  {
    return _acc_bin;
  }

  const string&
  cc_bin()
  {
    return _cc_bin;
  }

  const string&
  dest_path()
  {
    return _dest_path;
  }

  const bool&
  run_acpp_without_input()
  {
    return _run_acpp_without_input;
  }

  string&
  config_command()
  {
    return _config_command;
  }

  //
  // Helper methods
  //

  static inline string&
  str_replace_all(string& input_str, const string search_str,
      const string replace_str)
  {
    string::size_type pos = input_str.find(search_str, 0);
    while (pos != string::npos)
    {
      input_str.replace(pos, search_str.length(), replace_str);
      pos = input_str.find('"', pos + 2);
    }
    return input_str;
  }

};

#endif //__AGxxConfig_h__
