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

#ifndef __ACConfig_h__
#define __ACConfig_h__

// C++ includes
#include <iostream>
#include <string>
using namespace std;

// PUMA includes
#include "Puma/Array.h"

// Aspect C++ includes
#include "ACProject.h"

class ACConfig {
  ACProject &_project;
  int _argc;
  char **_argv;

  // analysis results
  int _v;
  const char *_file_in;
  const char *_file_out;
  const char *_repository;
  string _expr;
  bool _ifiles;
  bool _iterate_tunits;
  bool _nosave;
  bool _noline;
  Puma::Array<const char*> _aspect_headers; // TODO: get rid of Puma dependency here
  bool _iterate_aspects;
  bool _problem_local_class;
  bool _problem_spec_scope;
  bool _problem_force_inline;
  string _size_type;
  string _project_id;
  string _proj_file;
  bool _warn_deprecated;
  bool _warn_macro;
  bool _warn_limitations;
  bool _dynamic;
  bool _introspection;
  bool _attributes;
  bool _flow_analysis;
  bool _builtin_operators;
  bool _data_joinpoints;
  bool _keywords;
  int _introduction_depth;
  int _warn_compatibility;

public:

  enum {
    ACOPT_VERBOSE=1, ACOPT_VERSION, ACOPT_HELP,
    ACOPT_COMPILE, ACOPT_OUTPUT,
    ACOPT_INCLUDE_FILES, ACOPT_NOSAVE, ACOPT_ASPECT_HEADER,
    ACOPT_REPOSITORY, ACOPT_EXPRESSION, ACOPT_NOLINE,
    ACOPT_PROBLEM_LOCAL_CLASS, ACOPT_NO_PROBLEM_LOCAL_CLASS,
    ACOPT_PROBLEM_SPEC_SCOPE, ACOPT_NO_PROBLEM_SPEC_SCOPE,
    ACOPT_PROBLEM_FORCE_INLINE, ACOPT_NO_PROBLEM_FORCE_INLINE,
    ACOPT_GEN_SIZE_TYPE, ACOPT_PROJ_FILE,
    ACOPT_WARN_DEPRECATED, ACOPT_NO_WARN_DEPRECATED,
    ACOPT_WARN_MACRO, ACOPT_NO_WARN_MACRO,
    ACOPT_WARN_COMPATIBILITY,
    ACOPT_WARN_LIMITATIONS, ACOPT_NO_WARN_LIMITATIONS,
    ACOPT_KEYWORDS, ACOPT_DYNAMIC, ACOPT_INTROSPECTION,
    ACOPT_INTRODUCTION_DEPTH, ACOPT_FLOW_ANALYSIS,
    ACOPT_BUILTIN_OPERATORS, ACOPT_DATA_JOINPOINTS,
    ACOPT_ATTRIBUTES, ACOPT_NO_ATTRIBUTES
  };

  ACConfig (ACProject &project, int argc, char** argv) :
    _project (project), _argc (argc), _argv (argv),
    _file_in (0), _file_out (0), _repository (0) {}
  ~ACConfig ();
  
  // analyze the command line arguments
  // result: 'false' means that the user provided invalid arguments
  bool analyze ();

  // get file default cc file extension
  const char *extension ();

  // get the associated ACProject
  ACProject &project () const { return _project; }
  
  // get the analysis results
  int verbose () const { return _v; }
  const char *file_in () const { return _file_in; }
  const char *file_out () const { return _file_out; }
  bool ifiles () const { return _ifiles; }
  bool iterate () const { return _iterate_tunits; }
  bool nosave () const { return _nosave; }
  bool noline () const { return _noline; }
  bool iterate_aspects () const { return _iterate_aspects; }
  int aspect_headers () const { return _aspect_headers.length (); }
  const char *aspect_header (int i) const {
    return _aspect_headers.lookup (i);
  }
  const char *repository () const { return _repository; }
  const string &expr () const { return _expr; }
  bool problem_local_class () const { return _problem_local_class; }
  bool problem_spec_scope () const { return _problem_spec_scope; }
  bool problem_force_inline () const { return _problem_force_inline; }
  string size_type () const;
  string project_id () const { return _project_id; }
  string proj_file () const { return _proj_file; }
  bool warn_deprecated () const { return _warn_deprecated; }
  bool warn_macro () const { return _warn_macro; }
  int warn_compatibility () const { return _warn_compatibility; }
  bool warn_limitations () const { return _warn_limitations; }
  bool dynamic () const { return _dynamic; }
  bool introspection () const { return _introspection; }
  bool attributes () const {return _attributes; }
  bool flow_analysis() const { return _flow_analysis; }
  bool builtin_operators() const { return _builtin_operators; }
  bool data_joinpoints() const { return _data_joinpoints; }
  bool keywords () const { return _keywords; }
  int introduction_depth () const { return _introduction_depth; }

private:
  // print a usage message
  void usage (const char *) const;

  // get the error stream of the project
  ACErrorStream &err () const { return _project.err (); }
};

#endif // __ACConfig_h__
