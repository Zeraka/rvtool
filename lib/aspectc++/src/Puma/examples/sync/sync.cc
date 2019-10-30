#include <Puma/CProject.h>
#include <Puma/CTranslationUnit.h>
#include <Puma/CCSemVisitor.h>
#include <Puma/CPrintVisitor.h>
#include <Puma/PrePrintVisitor.h>
#include <Puma/PreTree.h>
#include "SyncLexer.h"
#include "SyncParser.h"
#include "SyncTransformer.h"
#include <iostream>


int main(int argc, char **argv) {
  // This is the error stream object used throughout
  // the whole system, it supports several error
  // severities and auto-formatted error location
  // output.
  Puma::ErrorStream err;

  // The project handles all file related task like
  // finding, opening, writing, and closing files.
  // The special CProject additionally is able to
  // scan files and strings, i.e. it provides an easy
  // to use interface to the lexical analyzer (scanner).
  Puma::CProject project(err, argc, argv);
  // Set the initial source/destination path pair. If
  // a file shall be written using the project it is
  // first checked if this file is located in one of
  // source paths known to the project. If not, the file
  // is not written. This mechanism is necessary to
  // avoid system headers to be overwritten (except for
  // the case that the corresponding system include
  // path is added as a source path).
  project.addPath(".", ".");

  // Check the arguments, need an input file.
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <filename>" << std::endl;
    return 1;
  }

  // Add the lexer for the keyword "synchronized"
  // to the project to be able to scan it.
  project.scanner().lexer(SyncLexer::instance(0));

  // Scan the input file. The result is a token list
  // representation of the input file.
  Puma::Unit *unit = project.scanFile(argv[1]);
  if (! unit) {
    std::cerr << "Aborted: Unable to scan input file" << std::endl;
    return 1;
  }

  // After the input file was successfully scanned, it is
  // now tried to parse it. The result is the so-called
  // translation unit encapsulating the preprocessor,
  // syntax, and semantic trees.
  SyncParser parser;
  Puma::CTranslationUnit *tu = parser.parse(*unit, project);

  // Parsing succeeded.
  if (tu->tree()) {
    // During the parsing of the input file only those
    // semantic analyses are performed that are necessary
    // to build the correct syntax tree and to recognize
    // the correct types. Expression evaluation and so on
    // is performed in an additional sematic analysis. It
    // is optional and not needed for this application.
    //Puma::CCSemVisitor semantics(err);
    //semantics.run(tu->tree());

    // The generated syntax tree is an attributed syntax
    // tree referring to both tokens and semantic information.
    // It can be traversed using a corresponding tree visitor.
    // Formatted printing of the tree is performed by the
    // CPrintVisitor tree visitor.
    //CPrintVisitor cprinter;
    //cprinter.print(tu->tree(), std::cout);

    // The collected semantic information also is organized
    // as a tree reflecting the scope structure of the parsed
    // code. The interface to the semantic tree is the class
    // CSemDatabase. It can also be used to print this tree.
    //tu->db().Dump(std::cout, 10);

    // Now transform the "synchronized" statements so that the
    // enclosed statements are protected from concurrent
    // modifications.
    SyncTransformer transformer(err);
    transformer.transform(tu->tree());

    // The "synchronized" statement is replaced by calls to
    // pthread functions. Thus pthread.h has to be included.
    // This is one of the simplest source code manipulations
    // and is entirely realized on token level using the token
    // list of the input file. First the corresponding include
    // directive has to be scanned.
    Puma::Unit *pthread_inc = project.scanString("#include <pthread.h>\n");
    // The resulting token list can now simply be prepended to
    // the input file's token list. Token lists are organized
    // as double-linked lists. Prepending something means to
    // insert something before the first element of the list.
    unit->paste_before((Puma::Token*)unit->first(), *pthread_inc);
    // The scanned tokens should be deleted if they are not
    // more needed. This is safe to do because paste_before()
    // copies the tokens before they are prepended.
    delete pthread_inc;

    // At last the transformed unit has to be saved as file
    // again. This is also realized using the project class.
    // The original file shall be renamed to filename.bak
    // before the transformed version of this file is written.
    project.saveMode(Puma::SaveMode::RENAME_OLD, ".bak");
    project.save(unit);
  }
  if (tu->cpp_tree()) {
    // Beside the syntax and semantic trees there is also
    // a separate tree for the preprocessor directives.
    // This tree can be printed using the PrePrintVisitor.
    //Puma::PrePrintVisitor preprinter;
    //tu->cpp_tree()->accept(preprinter);
  }

  // Clean up.
  delete tu;

  // Return 1 if errors or fatal errors occurred.
  return err.severity() > Puma::sev_warning ? 1 : 0;
}
