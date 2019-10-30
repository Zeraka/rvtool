#include "IncludeVisitor.h"
#include "Puma/version.h"
#include "Puma/CCParser.h"
#include "Puma/ErrorStream.h"
#include "Puma/CCSemVisitor.h"
#include "Puma/CPrintVisitor.h"
#include "Puma/PrePrintVisitor.h"
#include "Puma/CTranslationUnit.h"
#include <iostream>
using namespace std;

/*DEBUG*/#include <stdlib.h>

using namespace Puma;


namespace Puma {
/*DEBUG*/extern int TRACE_TYPE;     // created types
/*DEBUG*/extern int TRACE_UNDO;     // deleted syntax trees
/*DEBUG*/extern int TRACE_OBJS;     // inserted/deleted sem objects
/*DEBUG*/extern int TRACE_FCT;      // current function
/*DEBUG*/extern int TRACE_SCOPES;   // entered/leaved scopes
/*DEBUG*/extern int TRACE_INSTANCE_CODE; // instance code
/*DEBUG*/extern int SEM_DECL_SPECS; // created - deleted CSemDeclSpecs
}

static bool TRACE_STDOUT = false;
static bool TRACE_FILE = false;
static bool TRACE_TOKENS = false;
static bool DO_SEMANTICS = true;
static bool PRINT_TREE = true;
static bool PRINT_DB = true;
static bool PRINT_DEP = false;
static int  ONLY_PRE = 0;

void evalOptions (int argc, char **argv) {
  if (argc < 2) {
    cout << "PUMA C++ parser, library version " << PUMA_VERSION << endl;
    cout << "usage: ccparser [options] <filename>" << endl;
    cout << "options:" << endl;
    cout << "  -tr|--trace           trace parse process, write to stdout" << endl;
    cout << "  -tf|--trace-file      trace parse process, write to debug/trace.file" << endl;
    cout << "  -tt|--trace-tokens    trace parsing tokens too" << endl;
    cout << "  -ts|--trace-scopes    trace entered/leaved scopes" << endl;
    cout << "  -to|--trace-objs      trace inserted/destroyed semantic objects" << endl;
    cout << "  -tu|--trace-undo      trace deleted syntax tree nodes" << endl;
    cout << "  -tc|--trace-types     trace created types" << endl;
    cout << "  -ti|--trace-instances trace code of created instances" << endl;
    cout << "  -nd|--no-print-db     do not print the class database" << endl;
    cout << "  -nt|--no-print-tree   do not print the syntax tree" << endl;
    cout << "  -ns|--no-semantics    do not start the semantic process" << endl;
    cout << "  -E                    write preprocessor output to stdout" << endl;
    cout << "  -M                    print dependencies" << endl;
    cout << "  -Q                    preprocess without output" << endl;
    exit (1);
  }
  for (int i = 1; i < argc-1; i++) {
    const char *arg = argv[i];
    if (! strcmp (arg, "-tr") || ! strcmp (arg, "--trace"))
      TRACE_STDOUT = (TRACE_FILE = false, true);
    else if (! strcmp (arg, "-tf") || ! strcmp (arg, "--trace-file"))
      TRACE_FILE = (TRACE_STDOUT = false, true);
    else if (! strcmp (arg, "-tt") || ! strcmp (arg, "--trace-tokens"))
      TRACE_TOKENS = ((! TRACE_FILE && (TRACE_STDOUT = true)), true);
    else if (! strcmp (arg, "-ts") || ! strcmp (arg, "--trace-scopes"))
      TRACE_SCOPES = 1;
    else if (! strcmp (arg, "-to") || ! strcmp (arg, "--trace-objs"))
      TRACE_OBJS = 1;
    else if (! strcmp (arg, "-tu") || ! strcmp (arg, "--trace-undo"))
      TRACE_UNDO = 1;
    else if (! strcmp (arg, "-tc") || ! strcmp (arg, "--trace-types"))
      TRACE_TYPE = 1;
    else if (! strcmp (arg, "-ti") || ! strcmp (arg, "--trace-instances"))
      TRACE_INSTANCE_CODE = 1;
    else if (! strcmp (arg, "-nd") || ! strcmp (arg, "--no-print-db"))
      PRINT_DB = false;
    else if (! strcmp (arg, "-nt") || ! strcmp (arg, "--no-print-tree"))
      PRINT_TREE = false;
    else if (! strcmp (arg, "-ns") || ! strcmp (arg, "--no-semantics"))
      DO_SEMANTICS = false;
    else if (! strcmp (arg, "-E"))
      ONLY_PRE = 1;
    else if (! strcmp (arg, "-M")) {
      ONLY_PRE = 2; // no preprocessor output
      PRINT_DEP = true;
    }
    else if (! strcmp (arg, "-Q"))
      ONLY_PRE = 2; // no preprocessor output
  }
}

int run (int argc, char **argv) {
  ErrorStream err;
  CProject project (err, argc, argv);
  evalOptions (argc, argv);

  Unit *unit = project.scanFile (argv[argc-1]);
  if (! unit) {
    cout << "Aborted: Unable to scan input file" << endl;
    return 1;
  }

  CCParser parser;
  ofstream *fout = 0; 
#ifdef __PUMA_TRACING__
  if (TRACE_STDOUT)
    parser.trace (cout, TRACE_TOKENS);         // trace the parse process
  else if (TRACE_FILE) {
    fout = new ofstream ("debug/trace.file");
    parser.trace (*fout, TRACE_TOKENS);        // trace the parse process
  }
#endif  

  cout << "Starting parse process..." << endl;
  CTranslationUnit *tu = parser.parse (*unit, project, ONLY_PRE);

  if (tu->tree ()) {
    if (DO_SEMANTICS) {
      CCSemVisitor semantics (err);
      cout << "Starting semantic analysis..." << endl;
      semantics.run (tu->tree ());
    }
    if (PRINT_TREE) {
      CPrintVisitor printer;
      cout << "Printing syntax tree..." << endl;
      printer.print (tu->tree (), cout);
    }
    if (PRINT_DB) {
      cout << "Printing semantic database..." << endl;
      tu->db ().Dump (cout, 10);
    }
  // print dependency information
  } else if (PRINT_DEP && tu->cpp_tree ()) {
    IncludeVisitor visitor;
    tu->cpp_tree ()->accept (visitor);
  } else 
    cout << "Note: Empty program" << endl;
  
  if (err.severity () > sev_warning)
    cout << "Errors!" << endl;
  else if (err.severity () > sev_none)
    cout << "Warnings!" << endl;
  else
    cout << "No errors" << endl;

  delete tu;
  if (fout) delete fout;

  return err.severity () > sev_warning;
}

int main (int argc, char **argv) {
  int ret = run (argc, argv);
#ifndef NDEBUG
  /*DEBUG*/cout << "Statistics: +" << CTree::alloc << " -" << CTree::release << endl;
  /*DEBUG*/if (SEM_DECL_SPECS) cerr<<"WARNING: "<<SEM_DECL_SPECS<<" CSemDeclSpec(s) not deleted"<<endl;
#endif
  return ret;
}
