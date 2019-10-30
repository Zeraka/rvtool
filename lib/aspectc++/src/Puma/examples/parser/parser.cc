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

#include <Puma/CParser.h>
#include <Puma/CCParser.h>
#include <Puma/ErrorStream.h>
#include <Puma/CSemVisitor.h>
#include <Puma/CCSemVisitor.h>
#include <Puma/CPrintVisitor.h>
#include <Puma/PrePrintVisitor.h>
#include <Puma/CTranslationUnit.h>
#include <Puma/RegComp.h>
#include <Puma/PreTree.h>
#include <Puma/CTree.h>
#include <Puma/version.h>
#include <Puma/PipedCmd.h>
#include <iostream>
#include <string>
#include <list>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#ifdef WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif


namespace Puma {
  extern int TRACE_TYPE;            // trace created types
  extern int TRACE_UNDO;            // trace discarded subtrees
  extern int TRACE_OBJS;            // trace created and discarded semantic objects
  extern int TRACE_SCOPES;          // trace entered/leaved scopes
  extern int TRACE_INSTANCE_CODE;   // trace instance units
  extern int TRACE_PARSE_INSTANCE;  // trace parse instance units
  extern int TRACE_CPP;             // trace preprocessor
}


static bool verify_tree = false;
static bool lang_c = false;
static bool lang_cc = false;
static std::string input_file = "";
static bool verbose = false;
static bool dump_cpp = false;
static std::string dump_cpp_file;
static int dump_db = 0;
static std::string dump_db_file;
static bool dump_unit = false;
static std::string dump_unit_file;
static bool dump_tree = false;
static std::string dump_tree_file;
static bool dump_cpp_tree = false;
static std::string dump_cpp_tree_file;
static bool trace_parse = false;
static std::string trace_parse_file;


void log(const std::string& message) {
  if (verbose) {
    time_t ltime;
    time(&ltime);
    struct tm *today = localtime(&ltime);
    char buffer[128];
    strftime(buffer,127,"%b %d %H:%M:%S",today);
    std::cout << "-- " << buffer << ".";
#ifdef WIN32
    timeb btime;
    ftime(&btime);
    std::cout << btime.millitm;
#else
    struct timeval tv;
    gettimeofday(&tv,0);
    std::cout << tv.tv_usec/1000;
#endif
    std::cout << ": " << message << std::endl;
  }
}


void printError(Puma::ErrorStream& err, Puma::CTree* node, const std::string& message) {
  if (node) {
    Puma::Token* t = node->token();
    if (t) {
      err << t->location();
    }
  }
  err << Puma::sev_error << message.c_str() << Puma::endMessage;
}


void printError(Puma::ErrorStream& err, const std::string& message) {
  printError(err, 0, message);
}


std::ostream* openStream(const std::string& file) {
  return file.empty() ? &std::cout : new std::ofstream(file.c_str());
}


void closeStream(const std::string& file, std::ostream* os) {
  if (! file.empty()) {
    delete os;
  }
}


void addCompilerConfig(Puma::ErrorStream& err, Puma::Config& config) {
  // always set when Puma parses something, can be used
  // to hide code from Puma's view
  config.Add("-D", "__puma", true);
  // signal that Puma prefers standard C code
  config.Add("-D", "__STDC__", true);

  // command to get the configuration options from the compiler
  std::string cmd;
  if (lang_c) {
    cmd = "gcc -E -dM -v -x c /dev/null 2>&1";
  }
  else {
    cmd = "g++ -E -dM -v -x c++ /dev/null 2>&1";
  }
  // execute command to get the compiler config
  Puma::PipedCmd pipedcmd;
  pipedcmd.exec(cmd.c_str());

  // parse the compiler config
  std::istringstream in(pipedcmd.getOutput());
  if (! in) {
    printError(err, "unable to read compiler configuration");
    printError(err, "command '"+cmd+"' failed");
    return;
  }

  // regular expressions for matching includes and macros
  Puma::RegComp regincl("#include [<\"].*:.*");
  Puma::RegComp reginlist("^ .*");
  Puma::RegComp regdef("#define .*");

  // GNU compiler version
  std::string gnu_major_version, gnu_minor_version, gnu_patchlevel;

  // read file line by line
  bool in_include_list = false;
  std::string line;
  while (std::getline(in, line)) {
    // start of list of include paths
    if (regincl.match(line.c_str())) {
      in_include_list = true;
    // within a list of include paths
    } else if (in_include_list && reginlist.match(line.c_str())) {
      // strip leading white space
      unsigned int pos = line.find_first_not_of(" ");
      line = line.substr(pos);

      // convert '//*' to '/'
      char lastchar = '0';
      for (std::string::iterator c = line.begin(); c != line.end(); ++c) {
        if (lastchar == *c && *c == '/') {
          c = line.erase(c);
          --c;
        }
        lastchar = *c;
      }

      // add the include path
      config.Add("--isystem", line.c_str(), true);
    // end of list of include paths
    } else if (in_include_list) {
      in_include_list = false;
    // match a '#define ...'
    } else if (regdef.match(line.c_str())) {
      // remove '#define ' (8 characters)
      line = line.substr(8);

      // split name and argument of macro

      // find name, split at first whitespace
      std::string::size_type pos = line.find(" ");
      std::string name = line.substr(0, pos);

      // set '1' as default argument if no argument is defined
      std::string value = "1";

      // find value, i.e. the string behind the macro name
      if ((pos != std::string::npos) && (line.find_first_not_of(" ", pos) != std::string::npos)) {
        value = line.substr(pos+1);
      }

      // add the macro definition to the configuration
      config.Add("-D", name.c_str(), value.c_str(), true);

      // GNU compiler major version
      if (name == "__GNUC__") {
        gnu_major_version = value;
      // GNU compiler minor version
      } else if (name == "__GNUC_MINOR__") {
        gnu_minor_version = value;
      // GNU compiler patchlevel
      } else if (name == "__GNUC_PATCHLEVEL__") {
        gnu_patchlevel = value;
      // check for the size_t
      } else if (name == "__SIZE_TYPE__") {
        config.Add("--size-type", value.c_str(), true);
      // check for the ptrdiff_t
      } else if (name == "__PTRDIFF_TYPE__") {
        config.Add("--ptrdiff-type", value.c_str(), true);
      }
    }
  }

  // enable correct gcc support
  if (! gnu_major_version.empty()) {
    std::string version = gnu_major_version + "." + gnu_minor_version + "." + gnu_patchlevel;
    config.Add("--gnu", version.c_str(), true);
  }
}


void readOptionArg(int argc, char** argv, int& i, std::string& file) {
  if (argc > (i+2) && argv[i+1][0] != '-') {
    file = argv[++i];
  }
}


void printUsage(Puma::CProject& project) {
  std::cout << "PUMA C/C++ Parser, v" << Puma::puma_version() << std::endl;
  std::cout << "usage: parser [options] <FILE>" << std::endl;
  std::cout << "options:" << std::endl;
  project.config().PrintOptions(std::cout);
  std::cout << "  --dump-db [FILE]                Dump the semantic database to file or stdout" << std::endl;
  std::cout << "  --dump-db-all [FILE]            Dump the semantic database incl. builtins to file or stdout" << std::endl;
  std::cout << "  --dump-tree [FILE]              Dump the syntax tree to file or stdout" << std::endl;
  std::cout << "  --dump-unit [FILE]              Dump the primary token unit to file or stdout" << std::endl;
  std::cout << "  --dump-cpp [FILE]               Dump the C preprocessor output to file or stdout" << std::endl;
  std::cout << "  --dump-cpp-tree [FILE]          Dump the C preprocessor syntax tree to file or stdout" << std::endl;
  std::cout << "  --dump-instances                Dump the created template instances to stdout [debug]" << std::endl;
  std::cout << "  --trace-parse [FILE]            Trace the parse process and write it to file or stdout [debug]" << std::endl;
  std::cout << "  --trace-cpp                     Trace the preprocessor and write it to stdout [debug]" << std::endl;
  std::cout << "  --trace-instances               Trace parsing template instances and write it to stdout [debug]" << std::endl;
  std::cout << "  --trace-types                   Trace the created types and write it to stdout [debug]" << std::endl;
  std::cout << "  --trace-scopes                  Trace the entered scopes and write it to stdout [debug]" << std::endl;
  std::cout << "  --trace-objects                 Trace the created semantic opjects and write it to stdout [debug]" << std::endl;
  std::cout << "  --trace-undo-tree               Trace the backtracking and write it to stdout [debug]" << std::endl;
  std::cout << "  --verify-tree                   Verify the syntax tree" << std::endl;
  std::cout << "  --default-path                  Add the path to the input file as source path" << std::endl;
  std::cout << "  --no-default-config             Don't add the default configuration of the target compiler" << std::endl;
  std::cout << "  --help                          Show the usage information and exit" << std::endl;
  std::cout << "  -v                              Enable verbose logging" << std::endl;
  exit(1);
}


void evalOptions(Puma::CProject& project, int argc, char **argv) {
  // need usage information?
  if (argc < 2) {
    printUsage(project);
  }

  // read additional command line options
  bool default_path = false;
  bool default_config = true;
  for (int i = 1; i < argc; i++) {
    std::string o = argv[i];
    if (o == "--help") {
      printUsage(project);
    } else if (o == "-v") {
      verbose = true;
    } else if (o == "--verify-tree") {
      verify_tree = true;
    } else if (o == "--default-path") {
      default_path = true;
    } else if (o == "--no-default-config") {
      default_config = false;
    } else if (o == "--dump-db") {
      dump_db = 1;
      readOptionArg(argc, argv, i, dump_db_file);
    } else if (o == "--dump-db-all") {
      dump_db = 2;
      readOptionArg(argc, argv, i, dump_db_file);
    } else if (o == "--dump-tree") {
      dump_tree = true;
      readOptionArg(argc, argv, i, dump_tree_file);
    } else if (o == "--dump-unit") {
      dump_unit = true;
      readOptionArg(argc, argv, i, dump_unit_file);
    } else if (o == "--dump-cpp") {
      dump_cpp = true;
      readOptionArg(argc, argv, i, dump_cpp_file);
    } else if (o == "--dump-cpp-tree") {
      dump_cpp_tree = true;
      readOptionArg(argc, argv, i, dump_cpp_tree_file);
    } else if (o == "--trace-parse") {
      trace_parse = true;
      readOptionArg(argc, argv, i, trace_parse_file);
    } else if (o == "--trace-cpp") {
      Puma::TRACE_CPP = 1;
    } else if (o == "--trace-types") {
      Puma::TRACE_TYPE = 1;
    } else if (o == "--trace-scopes") {
      Puma::TRACE_SCOPES = 1;
    } else if (o == "--dump-instances") {
      Puma::TRACE_INSTANCE_CODE = 1;
    } else if (o == "--trace-instances") {
      Puma::TRACE_PARSE_INSTANCE = 1;
    } else if (o == "--trace-objects") {
      Puma::TRACE_OBJS = 1;
    } else if (o == "--trace-undo-tree") {
      Puma::TRACE_UNDO = 1;
    }
  }

  // add compiler configuration
  if (default_config) {
    addCompilerConfig(project.err(),project.config());
  }

  // add command line options
  project.config().Read(argc,argv);

  // evaluate options
  lang_c = project.config().Option("--lang-c");
  lang_cc = project.config().Option("--lang-c++") ||
            project.config().Option("--lang-ac++");
  input_file = argc > 1 ? argv[argc-1] : "";

  // get input language
  if (! lang_c && ! lang_cc) {
    std::string::size_type pos = input_file.rfind('.');
    if (pos != input_file.npos) {
      std::string suffix = input_file.substr(pos+1);
      if (suffix == "cc" || suffix == "cpp") {
        lang_cc = true;
      }
      else if (suffix == "c") {
        lang_c = true;
      }
    }

    // set input language
    if (lang_c) {
      project.config().Add("--lang-c", true);
    }
    else {
      lang_cc = true;
      project.config().Add("--lang-c++", true);
    }
  }

  if (default_path) {
    // add the directory containing the current input file
    // as source directory
    std::string path = ".";
    std::string::size_type pos = input_file.rfind("/");
    if (pos != std::string::npos) {
      path = "/";
      if (pos != 0) {
        path = input_file.substr(0, pos);
      }
    }
    project.config().Add("-p", path.c_str(), true);
  }

  // propagate the configuration
  project.configure(project.config());
}


Puma::CTranslationUnit* parseCFile(Puma::ErrorStream& err, Puma::CProject& project) {
  // lexical analysis
  log("lexical analysis");
  Puma::Unit *unit = project.scanFile(input_file.c_str());
  if (! unit) {
    printError(err, "lexical analysis of input file failed");
    return 0;
  }

  // dump unit
  if (dump_unit) {
    std::ostream* unit_os = openStream(dump_unit_file);
    unit->print(*unit_os);
    closeStream(dump_unit_file, unit_os);
  }

  // syntactic analysis
  log("syntactic analysis");
  Puma::CParser parser;
  std::ostream* trace_os = openStream(trace_parse_file);
  std::ostream* cpp_os = openStream(dump_cpp_file);
  if (trace_parse) {
#ifdef __PUMA_TRACING__
    parser.trace(*trace_os, verbose);
#endif
  }
  Puma::CTranslationUnit* file = parser.parse(*unit, project, dump_cpp, *cpp_os);
  closeStream(dump_cpp_file, cpp_os);
  closeStream(trace_parse_file, trace_os);

  // semantic analysis
  if (file && file->tree()) {
    log("semantic analysis");
    Puma::CSemVisitor semantics(err);
    semantics.run(file->tree());
  }

  return file;
}


Puma::CTranslationUnit* parseCCFile(Puma::ErrorStream& err, Puma::CProject& project) {
  // lexical analysis
  log("lexical analysis");
  Puma::Unit *unit = project.scanFile(input_file.c_str());
  if (! unit) {
    return 0;
  }

  // dump unit
  if (dump_unit) {
    std::ostream* unit_os = openStream(dump_unit_file);
    unit->print(*unit_os);
    closeStream(dump_unit_file, unit_os);
  }

  // syntactic analysis
  log("syntactic analysis");
  Puma::CCParser parser;
  std::ostream* trace_os = openStream(trace_parse_file);
  std::ostream* cpp_os = openStream(dump_cpp_file);
  if (trace_parse) {
#ifdef __PUMA_TRACING__
    parser.trace(*trace_os, verbose);
#endif
  }
  Puma::CTranslationUnit* file = parser.parse(*unit, project, dump_cpp, *cpp_os);
  closeStream(dump_cpp_file, cpp_os);
  closeStream(trace_parse_file, trace_os);

  // semantic analysis
  if (file && file->tree()) {
    log("semantic analysis");
    Puma::CCSemVisitor semantics(err);
    semantics.run(file->tree());
  }

  return file;
}


bool isDependent(Puma::CTree* node) {
  // check if any sub expression of a given expression
  // depends on a template parameter
  if (node->Type() && node->Type()->isDependent()) {
    return true;
  }
  // check the child nodes
  unsigned sons = node->Sons();
  for (unsigned i = 0; i < sons; ++i) {
    if (isDependent(node->Son(i))) {
      return true;
    }
  }
  return false;
}


std::string printTree(Puma::CTree* node) {
  // print the tokens of a sub-tree
  std::ostringstream s;
  Puma::Token* token = node->token();
  Puma::Token* end = node->end_token();
  if (token && end) {
    Puma::Unit* file = token->unit();
    while (token) {
      s << token->text();
      if (token == end) {
        break;
      }
      token = file->next(token);
    }
  }
  return s.str();
}


void walk(Puma::ErrorStream& err, Puma::CTree* node, Puma::CTree* parent = 0) {
  if (! node) {
    return;
  }
  // every node has to have a parent except the top level node
  if (! node->Parent() && node->NodeName() != Puma::CT_Program::NodeId()) {
    printError(err, node, node->NodeName()+std::string(" node has no parent"));
  }
  // parent - child relations must be correct
  if (node->Parent() && node->Parent() != parent) {
    printError(err, node, node->NodeName()+std::string(" node has wrong parent ")+node->Parent()->NodeName());
  }
  // node indicating an error in the input file
  if (node->NodeName() == Puma::CT_Error::NodeId()) {
    printError(err, node, "error node found in tree");
  }
  // all expressions shall have a type
  else if (node->IsExpression() && ! node->Type()) {
    if (! isDependent(node)) {
      printError(err, node, "expression without type");
    }
  }
  // all expressions shall have a defined type
  else if (node->IsExpression() && node->Type()->is_undefined()) {
    if (! isDependent(node)) {
      printError(err, node, "expression with undefined type");
    }
  }
  // all symbols shall be resolved
  else if (node->IsSimpleName() && node->SemObject() && ! node->SemObject()->Object()) {
    if (! isDependent(node)) {
      printError(err, node, std::string("unresolved symbol "+printTree(node)));
    }
  }
  // walk over the child nodes
  else {
    unsigned int sons = node->Sons();
    for (unsigned i = 0; i < sons; ++i) {
      walk(err, node->Son(i), node);
    }
  }
}


int main(int argc, char **argv) {
  Puma::ErrorStream err;
  Puma::CProject project(err);

  evalOptions(project,argc,argv);

  // parse input file
  Puma::CTranslationUnit* file = 0;
  if (lang_c) {
    file = parseCFile(err,project);
  }
  else if (lang_cc) {
    file = parseCCFile(err,project);
  }

  // dump the syntax tree
  if (dump_tree && file && file->tree()) {
    log("dump syntax tree");
    std::ostream* tree_os = openStream(dump_tree_file);
    Puma::CPrintVisitor printer;
    printer.configure(project.config());
    printer.print(file->tree(), *tree_os);
    closeStream(dump_tree_file, tree_os);
  }

  // dump the preprocessor syntax tree
  if (dump_cpp_tree && file && file->cpp_tree()) {
    log("dump preprocessor syntax tree");
    std::ostream* tree_os = openStream(dump_cpp_tree_file);
    Puma::PrePrintVisitor printer(*tree_os);
    file->cpp_tree()->accept(printer);
    closeStream(dump_cpp_tree_file, tree_os);
  }

  // dump the semantic database
  if (dump_db && file) {
    log("dump semantic information database");
    std::ostream* db_os = openStream(dump_db_file);
    file->db().Dump(*db_os, 10, dump_db == 2);
    closeStream(dump_db_file, db_os);
  }

  // walk over the tree
  if (verify_tree && file && file->tree()) {
    log("verify syntax tree");
    walk(err, file->tree());
  }

  log("cleanup");
  if (file) {
    delete file;
  }
  bool haveError = err.severity() > Puma::sev_warning;
  Puma::DString::clearDict();

  return haveError;
}
