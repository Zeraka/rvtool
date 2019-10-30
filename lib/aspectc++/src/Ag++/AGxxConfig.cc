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

#include "version.h"
#include "AGxxConfig.h"
#include "file.h"
#include "regex.h"

// PUMA includes
#include "Puma/StrCol.h"
#include "Puma/OptsParser.h"
#include "Puma/ParserKey.h"
#include "Puma/VerboseMgr.h"
using namespace Puma;

//stdc++ includes
#include <iostream>
#include <string>

// standard C library includes
#include <stdlib.h>

// get the size of an array
#define ARRAY_SIZE(array) ((sizeof (array)) / (sizeof ((array)[0])))

string AGxxConfig::_gcc_option_arg[] = { "-aux-info", "-print-prog-name=",
    "-print-file-name=", "-idirafter", "-imacros", "-iprefix", "-iwithprefix",
    "-iwithprefixbefore", "-isystem", "--gen_size_type", "-B", "-l", "-I", "-L",
    "-specs=", "-MF", "-MT", "-MQ", "-x", "--param=", "--param", "-Xlinker",
    "-u", "-V", "-b", "-G", };

string AGxxConfig::_gcc_option_info[] = { "-print","-dump", };

// options for AC++ Compiler
static OptsParser::Option options[] = {
   { AGxxConfig::AGOPT_GEN_CFG, NULL, "gen_config",
      "\t" "Only generate Puma configuration file", OptsParser::AT_NONE },
   { AGxxConfig::AGOPT_WEAVE_ONLY, NULL, "weave_only",
      "\t" "Weave only", OptsParser::AT_NONE },
   { AGxxConfig::AGOPT_COMPILE_ONLY, "c", NULL,
      "\t\t" "Compile only", OptsParser::AT_NONE },
   { AGxxConfig::AGOPT_KEEP_WOVEN,NULL, "keep_woven",
      "\t" "Keep woven source code files", OptsParser::AT_NONE },
   { AGxxConfig::AGOPT_KEEP_WOVEN,NULL, "keep_acc",
      "\t" "Deprecated. Use keep_woven instead", OptsParser::AT_NONE },
   { AGxxConfig::AGOPT_CC, NULL, "c_compiler",
      "Path to C++ compiler", OptsParser::AT_MANDATORY },
   { AGxxConfig::AGOPT_ACC, NULL, "ac_compiler",
      "Path to AspectC++ compiler", OptsParser::AT_MANDATORY },
   { AGxxConfig::AGOPT_OUTPUT, "o", "output",
      "\t" "Name of the output file", OptsParser::AT_MANDATORY },
   { AGxxConfig::AGOPT_CONFIG_COMMAND,NULL, "config_command",
      "Command which prints information about compiler", OptsParser::AT_MANDATORY },
   { AGxxConfig::AGOPT_X_COMPILER,NULL, "Xcompiler",
      "\tIn case of doubt account following options as g++ options", OptsParser::AT_NONE },
   { AGxxConfig::AGOPT_X_WEAVER,NULL, "Xweaver",
      "\t\tIn case of doubt account following options as ac++ options", OptsParser::AT_NONE },
   { AGxxConfig::AGOPT_HELP, "h", "help",
      "\t\t" "Print this help meassage", OptsParser::AT_NONE },
   { AGxxConfig::AGOPT_VERSION, NULL, "version",
      "\t" "\t" "Version information", OptsParser::AT_NONE },
   { AGxxConfig::AGOPT_VERBOSE, "v", "verbose",
      "Level of verbosity (0-9)", OptsParser::AT_OPTIONAL },
   { AGxxConfig::ACOPT_INCLUDE_FILES, "i", "include_files",
      "Generate manipulated header files", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT_ASPECT_HEADER, "a", "aspect_header",
      "Name of aspect header file or '0'", OptsParser::AT_MANDATORY },
   { AGxxConfig::ACOPT_QUOTE_ARG, "r", "repository",
      "Name of the project repository", OptsParser::AT_MANDATORY },
   { AGxxConfig::ACOPT_MATCH_POINTCUT_EXPR, NULL, "expr",
      "\t" "Pointcut expression to match in repository", OptsParser::AT_MANDATORY},
   { AGxxConfig::ACOPT, NULL, "problem_local_class",
      "Back-end does not support local classes correctly", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "no_problem_local_class",
      "Back-end supports local classes correctly", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "problem_spec_scope",
      "Back-end does not support template specialization in non-namespace",OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "no_problem_spec_scope",
      "Back-end supports template specialization in non-namespace scope",OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "problem_force_inline",
      "Back-end does not support forced inlining correctly", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "no_problem_force_inline",
      "Back-end supports forced inlining correctly", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "warn_deprecated",
      "\t" "Warn if deprecated syntax is used", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "no_warn_deprecated",
      "Don't warn if deprecated syntax is used", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "warn_macro",
      "\t" "Warn if a macro transformation had to be ignored", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "no_warn_macro",
      "\t" "Don't warn if a macro transformation had to be ignored", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "warn_limitations",
     "" "Warn if a feature does not work as familiar due to limitations", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "no_warn_limitations",
     "" "Don't warn if a feature does not work as familiar due to limitations", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL,"no_line",
      "\t\t" "Don't generate #line directives",OptsParser::AT_NONE },
   { AGxxConfig::ACOPT_QUOTE_ARG, NULL, "gen_size_type",
      "Argument type for generated new operator", OptsParser::AT_MANDATORY},
   { AGxxConfig::ACOPT, NULL,"real-instances",
      "\t" "Perform full template analysis",OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL,"pseudo-instances",
      "Simplified template instantiation scheme (deprecated)",OptsParser::AT_NONE },
   { AGxxConfig::ACOPT_PROJ_PATH, "p", "path",
      "\t" "Path to project source", OptsParser::AT_MANDATORY},
   { AGxxConfig::ACOPT_PROJ_DESTINATION, "d", "dest",
      "\t" "Path to destination for modified include files", OptsParser::AT_MANDATORY},
   { AGxxConfig::ACOPT_CFG_FILE, NULL, "config",
      "\t" "Full path to a config file", OptsParser::AT_MANDATORY},
   { AGxxConfig::ACOPT_SYS_CFG_FILE, NULL, "system-config",
      "Full path to a system config file (lower priority options)", OptsParser::AT_MANDATORY},
   { AGxxConfig::AGOPT_TARGET, NULL, "target",
      "\t" "Override the compiler's target triple", OptsParser::AT_MANDATORY},
   { AGxxConfig::ACOPT_PRE_INCLUDE, "I", NULL,
      "\t\t" "Add new include path", OptsParser::AT_MANDATORY},
   { AGxxConfig::ACOPT_PRE_DEFINE, "D", NULL,
      "\t\t" "Define a preprocessor macro", OptsParser::AT_MANDATORY},
   { AGxxConfig::ACOPT_PRE_UNDEFINE, "U", NULL,
      "\t\t" "Undefine a preprocessor macro", OptsParser::AT_MANDATORY},
   { AGxxConfig::ACOPT_ALWAYS_INCLUDE, "include", "include",
      "Always include file", OptsParser::AT_MANDATORY},
   { AGxxConfig::ACOPT, "k", "keywords",
      "\t" "Allow AspectC++ keywords in normal project files", OptsParser::AT_NONE},
   { AGxxConfig::ACOPT, NULL, "dynamic",
      "\t\t" "Prepare generated code for dynamic weaving", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "introspection",
      "\t" "Support introspection mechanism", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "flow_analysis",
      "\t" "Support control-flow analysis and data-flow analysis", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "builtin_operators",
      "Support joinpoints on builtin operator calls", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "data_joinpoints",
      "\t" "Support data-based joinpoints like get(), set(), ...", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "attributes",
      "\t" "Support user-defined attributes", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT, NULL, "no_attributes",
      "\t" "Disable support user-defined attributes", OptsParser::AT_NONE },
   { AGxxConfig::ACOPT_SYS_INCLUDE, "isystem", NULL,
      "\t" "Add system include path", OptsParser::AT_MANDATORY},
   { AGxxConfig::ACOPT_QUOTE_ARG, NULL, "warn_compatibility",
      "Enable migration warnings for code based on major version <arg>", OptsParser::AT_MANDATORY },
   { 0, 0, 0, 0, OptsParser::AT_NONE}
};

void
AGxxConfig::usage() const
{
  _err << "usage:  ag++ [options] [file...]" << endMessage
      << "   options are ..." << endMessage;
  for (int i = 0; options[i].number > 0; i++)
  {
    _err << "     ";
    if (options[i].shortName)
      _err << "-" << options[i].shortName;
    if (options[i].shortName && options[i].longName)
      _err << "|";
    if (options[i].longName)
      _err << "--" << options[i].longName;
    if (options[i].argType != OptsParser::AT_NONE)
      _err << " <arg>";
    if (options[i].desc)
      _err << " \t" << options[i].desc;
    _err << endMessage;
  }
}

bool
AGxxConfig::parseGccOpt(OptsParser &op)
{

  // option which should be analyzed
  string opt = op.getOptionName() + op.getArgument();

  // argument for this option
  string arg("");

  //check for -M/-MM option for generating dependency information
  if (opt.compare("-M") == 0 || opt.compare("-MM") == 0)
  {
    _make_dep = true;
    _optvec.pushback(opt, OptionItem::OPT_GCC);
    return true;
  }

  // check if option is in list of gcc options which require an option argument
  for (unsigned int i = 0U; i < ARRAY_SIZE(_gcc_option_arg); i++)
  {
    const string option_arg_str = _gcc_option_arg[i];
    if (opt.compare(0, option_arg_str.length(), option_arg_str) == 0)
    {

      // option requires an argument

      // check if the argument is appended to the option name
      if (opt.length() > option_arg_str.length())
      {
        unsigned int arg_start_pos = option_arg_str.length();

        // extract the argument out of the option name
        arg = "\"" + opt.substr(arg_start_pos) + "\"";
        opt = opt.substr(0, option_arg_str.length());
      }
      else
      {
        // check if there exist another option on command line
        if (op.getRawOption() != OptsParser::FINISH)
        {
          // append argument to string of arguments
          arg = " \"" + op.getArgument() + "\" ";
        }
        else
        {
          // there is no option anymore
          _err << sev_error << "G++ Option requires argument: ";
          _err << "'" << opt.c_str() << "'" << endMessage;
          return false;
        }
      }
      break;
    }
  }

  // check if option only causes the gcc to print information
  for (unsigned int i = 0U; i < ARRAY_SIZE(_gcc_option_info); i++)
  {
    string option_info_str = _gcc_option_info[i];
    if (opt.compare(0, option_info_str.length(), option_info_str) == 0)
    {
      _print_gcc_info = true;
      break;
    }
  }

  _optvec.pushback(opt, arg, OptionItem::OPT_GCC | OptionItem::OPT_CONFIG);
  return true;

}

bool
AGxxConfig::analyze()
{
  bool ret;

  // setup default values
  _gen_pumaconfig = true;
  _weave = false;
  _compile = false;
  _link = false;

  _verbose = 0;
  _nr_files = 0;

  _pumaconfig_tmp = true;
  _print_gcc_info = false;
  _weave_only = false;
  _keep_woven = false;
  _gen_includes = false;
  _cc_bin = "g++";
  _compile_only = false;
  _make_dep = false;
  _x_compiler = false;
  _run_acpp_without_input = false;

  // parse Options received from command line
  ret = parseOptions();

  if (ret == true)
  {
    // analyze Options received from command line
    ret = analyzeOptions();
  }

  return ret;
}

bool
AGxxConfig::parseOptions()
{
  // use the PUMA option parser for parsing
  OptsParser op(_argc, _argv, options);

  // extract options/filenames out of command line arguments
  while (op.getOption() != OptsParser::FINISH)
  {

    //
    // first parse options which do not interfere with g++
    // options
    //
    switch (op.getResult())
    {

      case AGOPT_OUTPUT:
        _output_file = op.getArgument();
        _optvec.pushback("-o", " \"" + op.getArgument() + "\" ",
            (OptionItem::OPT_GCC));
        continue;

      case AGOPT_COMPILE_ONLY:
        _compile_only = true;
        _optvec.pushback("-c", OptionItem::OPT_GCC);
        continue;

      case AGOPT_X_COMPILER:
        _x_compiler = true;
        continue;

      case AGOPT_X_WEAVER:
        _x_compiler = false;
        continue;

      case AGOPT_TARGET:
        _command_line_target_triple = op.getArgument();
        continue;

      case ACOPT_CFG_FILE:
        _optvec.pushback("--config", " \"" + op.getArgument() + "\" ",
            (OptionItem::OPT_ACC));
        continue;

      case ACOPT_SYS_CFG_FILE:
        _pumaconfig_file = op.getArgument();
        _pumaconfig_tmp = false;
        continue;

      case ACOPT_PRE_DEFINE:
        {
          string tmpStr = op.getArgument();
          _optvec.pushback("-D",
              " \"" + str_replace_all(tmpStr, "\"", "\\\"") + "\" ",
              (OptionItem::OPT_ACC | OptionItem::OPT_GCC));
        }
        continue;

      case ACOPT_PRE_UNDEFINE:
        _optvec.pushback("-U", " \"" + op.getArgument() + "\" ",
            (OptionItem::OPT_ACC | OptionItem::OPT_GCC));
        continue;

      case ACOPT_PRE_INCLUDE:
        _optvec.pushback("-I", " \"" + op.getArgument() + "\" ",
            (OptionItem::OPT_ACC | OptionItem::OPT_GCC));
        continue;

      case ACOPT_SYS_INCLUDE:
        _optvec.pushback("--isystem", " \"" + op.getArgument() + "\" ",
            (OptionItem::OPT_ACC ));
        _optvec.pushback("-isystem", " \"" + op.getArgument() + "\" ",
            (OptionItem::OPT_GCC));
        continue;

      case ACOPT_ALWAYS_INCLUDE:
        _optvec.pushback("--include", " \"" + op.getArgument() + "\" ",
            OptionItem::OPT_ACC);
        // Weaver generates #includes, Thus, g++ does need -include only for
        // dependency list generation
        _optvec.pushback("-include", " \"" + op.getArgument() + "\" ",
            OptionItem::OPT_DEP);
        continue;

        // command line arguments which are not accepted as options
        // are recognized as filenames
      case OptsParser::NOOPTION:
        ++_nr_files;
        // if file is a c++ file weave it first.
        if (regex::regExMatch(
            "^.*\\.((C)|(c)|(cc)|(cp)|(cpp)|(cxx)|(c\\+\\+))$",
            op.getArgument().c_str()))
        {
          _optvec.pushback(op.getArgument(),
              (OptionItem::OPT_ACC | OptionItem::OPT_FILE));

          // when calling we have to pass the original path of this
          // source dir as include path, as the '.acc' file to be
          // compiled will probably not be generated in the same
          // directory
          string tmpStr = op.getArgument();
          file::stripFilename(tmpStr, false);
          if (tmpStr.empty())
          {
            tmpStr = ".";
          }
          _optvec.pushback("-I", " \"" + tmpStr + "\" ", OptionItem::OPT_GCC);
        }
        else
        {
          _optvec.pushback(op.getArgument(),
              (OptionItem::OPT_GCC | OptionItem::OPT_FILE));
        }
        continue;
    }

    // if _x_compiler is set to 'true' interprete
    // options that are not handled within the previous
    // 'switch-case' statement as gcc options.
    if (_x_compiler == true)
    {
      if (parseGccOpt(op) == false)
      {
        return false;
      }
      continue;
    }

    switch (op.getResult())
    {

      case AGOPT_HELP:
        usage();
        return false;

      case AGOPT_VERSION:
        cout << "ag++ " << _VERSION <<"  built: " << __DATE__<< endl;
        return false;

      case AGOPT_GEN_CFG:
        _pumaconfig_tmp = false;
        break;

      case AGOPT_WEAVE_ONLY:
        _weave_only = true;
        _keep_woven = true;
        break;

      case AGOPT_ACC:
        _acc_bin = op.getArgument();
        break;

      case AGOPT_CC:
        _cc_bin = op.getArgument();
        break;

      case AGOPT_KEEP_WOVEN:
        if (op.getOptionName() == "--keep_acc")
        {
          _err << sev_warning
              << "Usage of --keep_acc is deprecated use --keep_woven instead"
              << endMessage;
        }
        _keep_woven = true;
        break;

      case AGOPT_CONFIG_COMMAND:
        _config_command = op.getArgument();
        break;

      // default handling for Aspect C++ options
      case ACOPT:
        _optvec.pushback(op.getOptionName(), op.getArgument(),
            OptionItem::OPT_ACC);
        break;

      // default handling for Aspect C++ options
      // where the argument has to be quoted
      case ACOPT_QUOTE_ARG:
        _optvec.pushback(op.getOptionName(), " \""+op.getArgument()+"\"",
            OptionItem::OPT_ACC);
        break;

      case ACOPT_WITHOUT_INPUT:
        _run_acpp_without_input = true;
        _weave_only = true;
        _optvec.pushback(op.getOptionName(), op.getArgument(),
            OptionItem::OPT_ACC);
        break;

      case ACOPT_INCLUDE_FILES:
        _gen_includes = true;
        _run_acpp_without_input = true;
        _optvec.pushback("-i","",OptionItem::OPT_ACC);
        break;

      case ACOPT_MATCH_POINTCUT_EXPR:
        {
          string argumentStr = op.getUnstrippedArgument();
          _run_acpp_without_input = true;
          _weave_only = true;
          _optvec.pushback("--expr","\""+str_replace_all(argumentStr, "\"", "\\\"")+"\"",
              OptionItem::OPT_ACC);
        }
        break;


      case AGOPT_VERBOSE:
        if (op.getArgument().empty())
        {
          // -v without number
          _optvec.pushback("-v", "1", OptionItem::OPT_ACC);
          _verbose = 1;
        }
        else
        {
          _optvec.pushback("-v", op.getArgument(), OptionItem::OPT_ACC);
          _verbose = atoi(op.getArgument().c_str());
        }
        if (_verbose > 6)
        {
          _optvec.pushback("-v", OptionItem::OPT_GCC);
        }
        break;

      case ACOPT_ASPECT_HEADER:
        _ah_files.push_back(op.getArgument());
        _optvec.pushback("-a", " \"" + op.getArgument() + "\" ",
            OptionItem::OPT_ACC);
        break;

      case ACOPT_PROJ_PATH:
        _proj_path.push_back(op.getArgument());
        _optvec.pushback("-p", " \"" + op.getArgument() + "\" ",
            OptionItem::OPT_ACC);
        break;

      case ACOPT_PROJ_DESTINATION:
        _dest_path = op.getArgument();
        _optvec.pushback("--dest","\""+_dest_path+"\"",OptionItem::OPT_ACC);
        break;

      case OptsParser::NOARG:
        _err << sev_error << "Option requires argument: ";
        _err << "'" << op.getOptionName().c_str() << "'" << endMessage;
        return false;

      case OptsParser::ERR:
        _err << sev_error << " Parsing  Options : ";
        _err << "unspecified error at ";
        _err << "'" << op.getOptionName().c_str() << " "
            << op.getArgument().c_str() << "'" << endMessage;
        return false;

      // unknown options are recognized as g++ options
      case OptsParser::UNKNOWN:
        if (parseGccOpt(op) == false)
        {
          return false;
        }
        break;

      default:
        _err << sev_error << " Parsing  Options : ";
        _err << "option not handled by Ag++ ";
        _err << "'" << op.getOptionName().c_str() << " "
            << op.getArgument().c_str() << "'" << endMessage;
        return false;
    }
  }
  return true;
}

bool
AGxxConfig::analyzeOptions()
{
  // if we are compiling for WIN32 target transform paths to dos
  // format, bacause the shell(cmd.exe) can not handle unix
  // paths
#if defined (WIN32)
  file::MakeDosPath(_acc_bin);
  file::MakeDosPath(_cc_bin);
#endif

  // if path to ac++ was not specified in options.
  // get it from ARGV[0]
  if (_acc_bin.empty())
  {
    _acc_bin = _argv[0];
    file::stripFilename(_acc_bin);
    _acc_bin.append("ac++");
  }

  // if there was no project path specified
  // take local directory ('.')
  if (_proj_path.empty())
  {
    _proj_path.push_back(".");
    _optvec.pushback("-p", ".", OptionItem::OPT_ACC);
  }

  // check if puma config file shall be generated
  if ((_print_gcc_info == true) || (_make_dep == true))
  {
    _gen_pumaconfig = false;
    _pumaconfig_tmp = false;
  }

  // check if source files shall be woven
  if (((_print_gcc_info == false) && (_make_dep == false) && (_nr_files > 0))
      || (_run_acpp_without_input == true))
  {
    _weave = true;
  }

  // check if source files shall be translated
  if (((_weave_only == false) && (_gen_includes == false) && (_nr_files > 0))
      || (_print_gcc_info == true))
  {
    _compile = true;
  }

  // check if source files shall be linked
  if ((_print_gcc_info == false) && (_weave_only == false)
      && (_gen_includes == false) && (_compile_only == false)
      && (_make_dep == false) && (_nr_files > 0))
  {
    _link = true;
  }

  //
  // Check Constraints
  //

  // if modified include files are NOT created and only a temporary
  // puma configuration file is generated ag++ expects some input files
  if ((_print_gcc_info == false) && (_pumaconfig_tmp == true)
      && (_run_acpp_without_input == false) && (_nr_files == 0))
  {
    _err << sev_error << "No input files" << endMessage;
    return false;
  }

  // If modified include files are created a destination
  // path must be specified
  if ((_gen_includes == true) && _dest_path.empty())
  {
    _err << sev_error << "Need to specify a destination path in order "
        << "to generate modified include files" << endMessage;
    return false;
  }

  // Check destination path is given without creating the modified include
  // files
  //
  // Rationale:
  // - destination path is only used in WTP mode
  // - WTP mode is only used if --include_files is
  //   specified
  if ((_gen_includes == false) && (! _dest_path.empty()))
  {
    _err << sev_error << "Destination path (-d|--dest) may only be "
        << "specified when generating modified include files "
        << "(-i|--include_files)" << endMessage;
    return false;
  }

  // If '--weave-only' and '-o' options are used with multiple files
  // exit with an error
  if ((_weave_only == true) && (_output_file.empty() == false)
      && (_nr_files > 1))
  {
    _err << sev_error
        << "Cannot specify -o with --weave_only and multiple compilations"
        << endMessage;
    return false;
  }

  return true;
}

void
AGxxConfig::printOptions()
{
  VerboseMgr vm(cout, _verbose);
  vm << endvm;
  vm << endvm;
  vm << "AG++ Configuration:" << endvm;
  vm << "  Aspect C++ weaver:  " << _acc_bin << endvm;
  vm << "  C++ compiler:       " << _cc_bin << endvm;
  vm << "  Files:              " << _optvec.getString(OptionItem::OPT_FILE) << endvm;
  vm << "  Options (G++):      " << _optvec.getString(OptionItem::OPT_GCC, OptionItem::OPT_FILE) << endvm;
  vm << "  Options (AC++):     " << _optvec.getString(OptionItem::OPT_ACC, OptionItem::OPT_FILE) << endvm;
  vm << "  Options (total):    " << _optvec.getString() << endvm;
  vm << "  PumaConfig: " << _gen_pumaconfig << " Weave: " << _weave  << " Compile: " << _compile << " Link: " << _link << endvm;
  vm << endvm;
  vm << endvm;
}

