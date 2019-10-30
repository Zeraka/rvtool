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

// C++ includes
#include <stdlib.h>
#include <sstream>
using std::stringstream;
#include <set>
using std::set;

// PUMA includes
#include "Puma/SysCall.h"

// Clang includes
#ifdef FRONTEND_CLANG
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/Version.h"
#else
#include "Puma/CTypeInfo.h"
#endif // FRONTEND_CLANG

// AspectC++ includes
#include "ACConfig.h"
#include "version.h"
#include "Naming.h"

using namespace Puma;

static OptsParser::Option options[] = {
  { ACConfig::ACOPT_VERSION, "V", "version",
    "\t" "Version information", OptsParser::AT_NONE },
  { ACConfig::ACOPT_HELP, "h", "help",
    "\t" "\t" "Get this help text", OptsParser::AT_NONE },
  { ACConfig::ACOPT_VERBOSE, "v", "verbose",
    "\t" "Level of verbosity (0-9)", OptsParser::AT_MANDATORY },
  { ACConfig::ACOPT_COMPILE, "c", "compile",
    "\t" "Name of the input file", OptsParser::AT_MANDATORY },
  { ACConfig::ACOPT_OUTPUT, "o", "output",
    "\t" "Name of the output file", OptsParser::AT_MANDATORY },
  { ACConfig::ACOPT_INCLUDE_FILES, "i", "include_files",
    "\t" "Generate manipulated header files", OptsParser::AT_NONE },
  { ACConfig::ACOPT_ASPECT_HEADER, "a", "aspect_header",
    "Name of aspect header file or '0'", OptsParser::AT_MANDATORY }, 
  { ACConfig::ACOPT_REPOSITORY, "r", "repository",
    "Name of the project repository", OptsParser::AT_MANDATORY }, 
  { ACConfig::ACOPT_EXPRESSION, "x", "expr",
    "\tPointcut expression to match in repository", OptsParser::AT_MANDATORY },
  { ACConfig::ACOPT_PROBLEM_LOCAL_CLASS, NULL, "problem_local_class",
    "Back-end does not support local classes correctly", OptsParser::AT_NONE }, 
  { ACConfig::ACOPT_NO_PROBLEM_LOCAL_CLASS, NULL, "no_problem_local_class",
    "Back-end supports local classes correctly", OptsParser::AT_NONE }, 
  { ACConfig::ACOPT_PROBLEM_SPEC_SCOPE, NULL,
    "problem_spec_scope",
    "Back-end does not support template specialization in non-namespace",
    OptsParser::AT_NONE }, 
  { ACConfig::ACOPT_NO_PROBLEM_SPEC_SCOPE, NULL,
    "no_problem_spec_scope",
    "Back-end supports template specialization in non-namespace scope",
    OptsParser::AT_NONE },
  { ACConfig::ACOPT_PROBLEM_FORCE_INLINE, NULL, "problem_force_inline",
    "Back-end does not support forced inlining correctly", OptsParser::AT_NONE }, 
  { ACConfig::ACOPT_NO_PROBLEM_FORCE_INLINE, NULL, "no_problem_force_inline",
    "Back-end supports forced inlining correctly", OptsParser::AT_NONE }, 
  { ACConfig::ACOPT_NOLINE, NULL,
    "no_line",
    "\t" "\t" "Don't generate #line directives",
    OptsParser::AT_NONE },
  { ACConfig::ACOPT_GEN_SIZE_TYPE, NULL, "gen_size_type",
    "Argument type for generated new operator", OptsParser::AT_MANDATORY }, 
  { ACConfig::ACOPT_PROJ_FILE, NULL, "proj_file",
    "\t" "Path to the project file", OptsParser::AT_MANDATORY},
  { ACConfig::ACOPT_WARN_DEPRECATED, NULL, "warn_deprecated",
    "\t" "Warn if deprecated syntax is used", OptsParser::AT_NONE },
  { ACConfig::ACOPT_NO_WARN_DEPRECATED, NULL, "no_warn_deprecated",
    "Don't warn if deprecated syntax is used", OptsParser::AT_NONE },
  { ACConfig::ACOPT_WARN_MACRO, NULL, "warn_macro",
    "\t" "Warn if a macro transformation had to be ignored",
    OptsParser::AT_NONE },
  { ACConfig::ACOPT_NO_WARN_MACRO, NULL, "no_warn_macro",
    "\t" "Don't warn if a macro transformation had to be ignored",
    OptsParser::AT_NONE },
  { ACConfig::ACOPT_WARN_LIMITATIONS, NULL, "warn_limitations",
    "\t" "Warn if a feature does not work as familiar due to limitations",
    OptsParser::AT_NONE },
  { ACConfig::ACOPT_NO_WARN_LIMITATIONS, NULL, "no_warn_limitations",
    "" "Don't warn if a feature does not work as familiar due to limitations",
    OptsParser::AT_NONE },
  { ACConfig::ACOPT_KEYWORDS, "k", "keywords",
    "\t" "Allow AspectC++ keywords in normal project files",
    OptsParser::AT_NONE },
  { ACConfig::ACOPT_DYNAMIC, 0, "dynamic",
    "\t\t" "Prepare generated code for dynamic weaving",
    OptsParser::AT_NONE },
  { ACConfig::ACOPT_INTROSPECTION, 0, "introspection",
    "\t" "Support introspection mechanism",
    OptsParser::AT_NONE },
  { ACConfig::ACOPT_FLOW_ANALYSIS, 0, "flow_analysis",
    "\t" "Support control-flow analysis and data-flow analysis",
    OptsParser::AT_NONE },
  { ACConfig::ACOPT_BUILTIN_OPERATORS, 0, "builtin_operators",
    "Support joinpoints on builtin operator calls",
    OptsParser::AT_NONE },
  { ACConfig::ACOPT_DATA_JOINPOINTS, 0, "data_joinpoints",
    "\t" "Support data-based joinpoints like get(), set(), ...",
    OptsParser::AT_NONE },
  { ACConfig::ACOPT_INTRODUCTION_DEPTH, 0, "introduction_depth",
    "Set the maximum depth for nested introductions", OptsParser::AT_MANDATORY },
  { ACConfig::ACOPT_WARN_COMPATIBILITY, NULL, "warn_compatibility",
    "Enable migration warnings for code based on major version <arg>",
    OptsParser::AT_MANDATORY },
  { ACConfig::ACOPT_ATTRIBUTES, NULL, "attributes",
    "\t" "Support user-defined attributes",
    OptsParser::AT_NONE },
  { ACConfig::ACOPT_NO_ATTRIBUTES, NULL, "no_attributes",
    "\t" "Disable support user-defined attributes",
    OptsParser::AT_NONE },
  { 0, NULL, 0, 0, OptsParser::AT_NONE }
};


ACConfig::~ACConfig () {
  if (_file_in)
    delete[] _file_in;
  if (_file_out)
    delete[] _file_out;
  for (int i = 0; i < _aspect_headers.length (); i++)
    delete[] _aspect_headers[i];
}


string ACConfig::size_type () const {
  string result = _size_type;
  // if the size-type string wasn't provided with the command line option
  // --gen_size_type, ask the front end for its target size type.
  if (result == "") {
#ifdef FRONTEND_CLANG
    clang::CompilerInstance *ci = _project.get_compiler_instance ();
    clang::TargetInfo &target = ci->getTarget ();
    result = clang::TargetInfo::getTypeName (target.getSizeType ());
#else
    ostringstream size_type_stream;
    size_type_stream << *CTypeInfo::CTYPE_SIZE_T;
    result = size_type_stream.str ();
#endif
  }
  return result;
}

bool ACConfig::analyze () {

  // setup defaults
  _v                   = 0; // default verbosity without "-v"
  _ifiles              = false;
  _file_in             = (const char*)0;
  _file_out            = (const char*)0;
  _repository          = (const char*)0;
  _iterate_tunits      = false;
  _nosave              = false;
  _noline              = false;
  _iterate_aspects     = true;

  // default setting: unknown compilers have a problem with everything
  _problem_local_class         = true;
  _problem_spec_scope          = true;
  _problem_force_inline        = true;
  if (_project.config ().Option ("--gnu") ||
      _project.config ().Option ("--gnu-2.95")) {
    _problem_local_class         = false;
    _problem_force_inline        = false;
  } else if (_project.config ().Option ("--vc")) {
    _problem_spec_scope          = false;
  }
  _warn_deprecated     = true;
  _warn_macro          = false;
  _warn_limitations    = true;
  _dynamic             = false;
  _introspection       = false;
  _attributes          = true;
  _flow_analysis       = false;
  _builtin_operators   = false;
  _data_joinpoints     = false;
  _keywords            = false;
  _introduction_depth  = 10;
  _warn_compatibility = -1;

  char* tmp;

  OptsParser op (_argc, _argv, options);

  bool have_proj_file = false;
  while (op.getOption () != OptsParser::FINISH) {
    switch (op.getResult ()) {
    case ACOPT_VERBOSE:
      if (op.getArgument ().empty ())
        // -v without number
        _v = 3;
      else
        _v = atoi (op.getArgument ().c_str ());
      break;
    case ACOPT_VERSION:
      cout << ac_program () << " " << ac_version ()
           << " (" << __DATE__;
#ifdef FRONTEND_CLANG
      cout << ", clang " CLANG_VERSION_STRING;
#endif // FRONTEND_CLANG
      cout << ")" << endl;
      return true;
    case ACOPT_HELP:
      usage (_argv[0]);
      return true;
    case ACOPT_COMPILE:
      tmp = StrCol::dup (op.getArgument ().c_str ());
#if defined (WIN32)
      SysCall::MakeUnixPath(tmp);
#endif
      _file_in = tmp;
      break;
    case ACOPT_OUTPUT:
      tmp = StrCol::dup (op.getArgument ().c_str ());
#if defined (WIN32)
      SysCall::MakeUnixPath(tmp);
#endif
      _file_out = tmp;
      break;
    case ACOPT_INCLUDE_FILES:
      _ifiles = true;
      break;
    case ACOPT_NOSAVE:
      _nosave = true;
      break;
    case ACOPT_NOLINE:
      _noline = true;
      break;
    case ACOPT_ASPECT_HEADER:
      _iterate_aspects = false;
      if (strcmp (op.getArgument ().c_str (), "0") != 0) {
        tmp = StrCol::dup (op.getArgument ().c_str ());
#if defined (WIN32)
        SysCall::MakeUnixPath(tmp);
#endif
        _aspect_headers.append (tmp);
      }
      break;
    case ACOPT_REPOSITORY:
      tmp = StrCol::dup (op.getArgument ().c_str ());
#if defined (WIN32)
      SysCall::MakeUnixPath(tmp);
#endif
      _repository = tmp;
      break;
    case ACOPT_EXPRESSION:
      _expr = op.getUnstrippedArgument ();
      break;
    case ACOPT_PROBLEM_LOCAL_CLASS:
      _problem_local_class = true;
      break;
    case ACOPT_NO_PROBLEM_LOCAL_CLASS:
      _problem_local_class = false;
      break;
    case ACOPT_PROBLEM_SPEC_SCOPE:
      _problem_spec_scope = true;
      break;
    case ACOPT_NO_PROBLEM_SPEC_SCOPE:
      _problem_spec_scope = false;
      break;
    case ACOPT_PROBLEM_FORCE_INLINE:
      _problem_force_inline = true;
      break;
    case ACOPT_NO_PROBLEM_FORCE_INLINE:
      _problem_force_inline = false;
      break;
    case ACOPT_GEN_SIZE_TYPE:
      _size_type = op.getArgument ().c_str ();
      break;
    case ACOPT_PROJ_FILE:
      have_proj_file = true;
      _proj_file = op.getArgument ().c_str ();
      break;
    case ACOPT_WARN_DEPRECATED:
      _warn_deprecated = true;
      break;
    case ACOPT_NO_WARN_DEPRECATED:
      _warn_deprecated = false;
      break;
    case ACOPT_WARN_MACRO:
      _warn_macro = true;
      break;
    case ACOPT_NO_WARN_MACRO:
      _warn_macro = false;
      break;
    case ACOPT_WARN_LIMITATIONS:
      _warn_limitations = true;
      break;
    case ACOPT_NO_WARN_LIMITATIONS:
      _warn_limitations = false;
      break;
    case ACOPT_WARN_COMPATIBILITY:
      _warn_compatibility = atoi (op.getArgument ().c_str ());
      break;
    case ACOPT_KEYWORDS:
      _keywords = true;
      break;
    case ACOPT_DYNAMIC:
      _dynamic = true;
      break;
    case ACOPT_INTROSPECTION:
      _introspection = true;
      break;
    case ACOPT_FLOW_ANALYSIS:
      _flow_analysis = true;
      break;
#ifdef FRONTEND_PUMA
    case ACOPT_BUILTIN_OPERATORS:
    case ACOPT_DATA_JOINPOINTS:
      err() << sev_error << "The command line options '--builtin_operators' "
          "and '--data_joinpoints' are not supported in the 'Puma' configuration." << endMessage;
      return false;
#else
    case ACOPT_BUILTIN_OPERATORS:
      _builtin_operators = true;
      break;
    case ACOPT_DATA_JOINPOINTS:
      _data_joinpoints = true;
      break;
#endif
    case ACOPT_INTRODUCTION_DEPTH:
      _introduction_depth = atoi (op.getArgument ().c_str ());
      break;
    case ACOPT_ATTRIBUTES:
      _attributes = true;
      break;
    case ACOPT_NO_ATTRIBUTES:
      _attributes = false;
      break;

    default:
      usage (_argv[0]);
      err () << sev_error;
      return false;
    }
  }

  bool have_std_prj   = (_project.numPaths () > 0) && _project.src (0L);
  bool have_prj       = have_proj_file || have_std_prj;
  bool have_dest      = (_project.numPaths () > 0) && _project.dest (0L);
  
  // expression matching requires a repository
  if (!expr().empty ()) {
    if (!repository()) {
      usage (_argv[0]);
      err () << sev_error;
      return false;
    }
    return true;
  }

  // now check if the options make sense
  if (!have_prj || (have_std_prj && have_proj_file) ||
      (_ifiles && (_file_in || !_project.dest (0L))) ||
      (_file_out && have_dest) ||
      (!_file_in && !_ifiles && !have_dest)) {
    usage (_argv[0]);
    err () << sev_error;
    return false;
  }

  // check option dependency
  if( _data_joinpoints ) {
    if( ! _builtin_operators ) {
      err() << sev_warning << "--data_joinpoints internally requires --builtin_operators, please check and enable it to silence this warning!" << endMessage;
      _builtin_operators = true;
    }
  }

  // check for the compatibility mode (iterate through the project tree)
  if (!_ifiles && !_file_in && !_file_out && have_dest)
    _iterate_tunits = true;

  // set reasonable default values
  if (!_file_out && _file_in)
    _file_out = StrCol::dup ("ac.out");

  if (!_file_in) {
    if (_file_out)
      _file_in = StrCol::dup (_file_out);
    else {
      stringstream genfile;
      if (_project.src (0L))
        genfile << _project.src (0L);
      genfile << "ac_gen." << extension ();
      _file_in = StrCol::dup (genfile.str ().data ());
    }
  }

  // determine a string that uniquely identifies the project
  if (have_std_prj) { // a normal project with one or more -p options
    set<string> paths;
    for (long i = 0L; i < _project.numPaths (); i++) {
      Filename canon;
      SysCall::canonical (_project.src(i), canon);
      ostringstream name;
      Naming::mangle_file (name, canon.name ());
      paths.insert (name.str ());
    }
    _project_id = *paths.begin ();
  }
  else { // used a project file

    // load the project file and set up the project
    if (!_project.loadProject (_proj_file)) {
      err () << sev_error << "failed to load project file '"
           << _proj_file.c_str () << "'" << endMessage;
      return false;
    }
    Filename canon;
    SysCall::canonical (_proj_file.c_str (), canon);
    ostringstream name;
    Naming::mangle_file (name, canon.name ());
    _project_id = name.str ();
  }
  
  // TODO: this should be obsolete -> check it
  _project.config ().Add ("--lang-ac++");
  _project.configure (_project.config ());
  

  return true;
}

void ACConfig::usage (const char *prog) const {
  cerr << "usage: " << prog << " options" << endl
       << "   options are ..." << endl;
  cerr << "     -p|--path <arg>\t\t" << "project directory tree" << endl
       << "     -d|--dest <arg>\t\t" << "target directory tree" << endl
       << "     -e|--extension <arg>\t" << "translation unit filename ext."
       << endl;
  for (int o = 0; options[o].number > 0; o++) {
    cerr << "     ";
    if (options[o].shortName)  cerr << "-" << options[o].shortName;
    if (options[o].shortName && options[o].longName) cerr << "|";
    if (options[o].longName) cerr << "--" << options[o].longName;
    if (options[o].argType != OptsParser::AT_NONE) cerr << " <arg>";
    if (options[o].desc) cerr << "\t" << options[o].desc;
    cerr << endl;
  }
  cerr << "   further options for correct parsing are ..." << endl
       << "     -I <arg>\t\t\t" << "include file search path" << endl
       << "     -D <name>[=<value>]\t" << "macro definitions" << endl;
}


// get the extension string for cc files
 
const char *ACConfig::extension () {
  for (unsigned i = _project.config ().Options (); i > 0; i--) {
    const ConfOption *o = _project.config ().Option (i-1);
    if (! strcmp (o->Name (), "-e") || ! strcmp (o->Name (), "--extension")) {
      if (! o->Arguments ())
        continue;
      return o->Argument (0);
    }
  }
  return "cc";
}
