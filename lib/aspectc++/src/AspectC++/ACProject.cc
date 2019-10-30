#include "ACProject.h"
#include "ACConfig.h"

#include "Puma/SysCall.h"
using namespace Puma;

#ifdef FRONTEND_CLANG
#include "llvm/Support/Host.h"
#include "clang/Basic/Version.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Rewrite/Core/Rewriter.h"
// for diagnostics
#include "ClangTransformInfo.h"
#include "IntroductionUnit.h"
using namespace clang;
#endif

#include <libxml/xpath.h>
#include <stdio.h>
#include <fstream>

#ifdef FRONTEND_CLANG
ACProject::ACProject (ACErrorStream &err, int &argc, char **&argv)
    : PathManager (err), _ci (0), _puma_config (err) {

  if (!_puma_config.CustomSystemConfigFile (argc, argv))
    _puma_config.Read (); // read global config file, e.g. $PUMA_CONFIG

  _puma_config.Read (argc, argv); // read command line config arguments

  // read command line arguments
  config ().Add ("-D", "__acweaving", ""); // only defined during weaving phase

  // configure path manager
  configure (_puma_config);
}
#else
ACProject::ACProject (ACErrorStream &err, int &argc, char **&argv)
  : CProject (err, argc, argv)
{
  config ().Add ("-D", "__acweaving", ""); // only defined during weaving phase
}
#endif

#ifdef FRONTEND_CLANG
ACProject::~ACProject() {
  delete _ci;
}
#endif

void ACProject::addFiles (xmlDocPtr doc, xmlNodePtr node, const string& xpath) {
  xmlXPathContextPtr  m_Context = xmlXPathNewContext(doc);
  xmlXPathCompExprPtr m_Expr = xmlXPathCompile((const xmlChar*)xpath.c_str());;
  xmlXPathObjectPtr   m_Result = 0;
  if( m_Context != 0 && m_Expr != 0 ) {
    m_Result = xmlXPathCompiledEval(m_Expr,m_Context);
    if( m_Result != 0 && m_Result->nodesetval != 0 ) {
      for( int i=0; i<m_Result->nodesetval->nodeNr; ++i ) {
        xmlChar* path = xmlGetProp(m_Result->nodesetval->nodeTab[i],
                                   (xmlChar*)"path");
        xmlChar* relpath = xmlGetProp(m_Result->nodesetval->nodeTab[i],
                                      (xmlChar*)"relpath");
        if (path) {

          string destname;
          if (numPaths () && dest (0)) {
            // make the destination path
            destname = dest (0);
            char last = destname[destname.length () - 1];
            if (last != '/' && last != '\\')
              destname += '/';
#ifdef WIN32
            SysCall::MakeUnixPath ((char*)relpath);
#endif
            destname += (const char*)relpath;
          }
          else
            destname = "<unused-dest-name>";
          
          // add the file to the project
          PathManager::addFile ((const char*)path, destname.c_str ());
        }
        else {
          assert (false);
        }
        if (path != 0) xmlFree(path);
        if (relpath != 0) xmlFree(relpath);
      }
      xmlXPathFreeObject(m_Result);
    }
  }
  if( m_Context ) xmlXPathFreeContext(m_Context);
  if( m_Expr )    xmlXPathFreeCompExpr(m_Expr);
}

// Add a new *virtual* file to the project.
ACFileID ACProject::addVirtualFile (const string &filename, const string &contents) {
#ifdef FRONTEND_PUMA
  Unit *unit = Project::addFile (Filename (filename.c_str()), "<unused-dest-name>");
  if (contents != "") {
    CScanner scanner (err ());
    scanner.fill_unit (contents.c_str (), *unit);
  }
  _virtual_files.insert(unit);
  return unit;
#else
  // Create the ac_gen file to keep clang-based phase1 from crashing.
  // FIXME: Hack
//  fclose (fopen (filename.c_str (), "w+"));
  clang::FileManager &fm = _ci->getFileManager ();
  const clang::FileEntry *fe = fm.getVirtualFile (filename, 0, 0);
  _ci->getSourceManager().overrideFileContents(fe,
      llvm::MemoryBuffer::getMemBufferCopy(contents));
  _virtual_files.insert(ACFileID(fe));
  return fe;
#endif
}


// Remove a virtual file
void ACProject::removeVirtualFile (ACFileID fid) {
  string filename = fid.name();
#ifdef FRONTEND_CLANG
   // Remove the ac_gen file again.
   // FIXME: Hack
//   remove (filename.c_str ());
   close (fid);
#else
   close (filename.c_str (), true);
#endif
}

bool ACProject::loadProject (string acprj) {
  //std::cout << "try to find project" << std::endl;
  if( acprj.empty() == false ) {
    //std::cout << "proj file " << acprj << std::endl;
    xmlDocPtr doc = xmlParseFile(acprj.c_str());
    if( doc != 0 ) {
      xmlNodePtr root = xmlDocGetRootElement(doc);
      if( root != 0 ) {
        addFiles(doc,root,"/project/source/file");
        addFiles(doc,root,"/project/aspect/file[@active='true']");
        return true;
      }
    }
  }
  return false;
}

#ifdef FRONTEND_CLANG
class Diag : public TextDiagnosticPrinter {
  string _msg;
  llvm::raw_string_ostream _str_stream;
  llvm::raw_ostream &_orig_stream;
public:
  Diag(raw_ostream &os, DiagnosticOptions *diags) :
    TextDiagnosticPrinter(_str_stream, diags), _str_stream(_msg),
    _orig_stream (os) {}
  void HandleDiagnostic (DiagnosticsEngine::Level DiagLevel,
      const Diagnostic &Info) {
    // call Clang's HandleDiagnostic function. Output will be written into _msg.
    TextDiagnosticPrinter::HandleDiagnostic(DiagLevel, Info);

    // search for <intro:address> in _msg and replace it with a proper filename
    for (unsigned pos = 0; pos < _msg.size(); pos++) {
      if (_msg.substr(pos, 7) == "<intro:") {
        // determine pointer value and line number
        pos += 7;
        void *ptr;
        sscanf(_msg.c_str() + pos, "%p", &ptr);
        while (_msg[pos] != ':')
          pos++;
        pos++; // skip ':'
        unsigned line;
        sscanf(_msg.c_str() + pos, "%u", &line);
        while (_msg[pos] != ':')
          pos++;

        // find the corresponding slice fragment ("body")
        IntroductionUnit *unit = (IntroductionUnit*)ptr;
        ACM_Introduction *intro = unit->intro();
        TI_ClassSlice *slice = 0;
        if(intro->has_named_slice())
          slice = TI_ClassSlice::of(*intro->get_named_slice());
        else
          slice = TI_ClassSlice::of(*intro->get_anon_slice());

        // generate the new file and line information
        if (unit->is_members_intro()) {
          const TI_ClassSlice::SliceBody &body = slice->get_tokens();
          _orig_stream << body.file.name() << ":" << body.line + (line - 1) << ":";
        }
        else if (unit->is_base_intro()) {
          const TI_ClassSlice::SliceBody &body = slice->get_base_tokens();
          _orig_stream << body.file.name() << ":" << body.line + (line - 1) << ":";
        }
        else if (unit->non_inline_member_no() >= 0) {
          int no = unit->non_inline_member_no();
          const TI_ClassSlice::SliceBody &body = slice->non_inline_members()[no];
          _orig_stream << body.file.name() << ":" << body.line + (line - 1) << ":";
        }
        else
          _orig_stream << "<intro>:1:";
        continue;
      }
      _orig_stream << _msg[pos];
    }
    _msg.clear();
  }
};

void ACProject::create_compiler_instance (ACConfig &conf) {
  std::list<std::string> StringBuf;
  SmallVector<const char *, 16> Args;
  Args.push_back("-fsyntax-only"); // Just a dummy action.

  // TODO: this hard-coded stuff should be replaced by a mechanism that
  //       fetches these settings from the command line
  Args.push_back("-disable-free");
  Args.push_back("-fcxx-exceptions");
  Args.push_back("-fexceptions");
  Args.push_back("-pic-level");
  Args.push_back("2");
  Args.push_back("-ftemplate-depth");
  Args.push_back("65536");
  Args.push_back("-fblocks");

  // Translate Puma's built-in config file into clang arguments.
  // Puma already sorts the options and gives system options a lower
  // priority than user-provided options.
  Config &c = _puma_config;

  // Start with all system options that are not include paths
  for (unsigned i = 0; i < c.Options (); i++) {
    const ConfOption *o = c.Option (i);
    const char *Name = o->Name();

    // For things like --isystem.
    if (strncmp(Name, "--i", 3) == 0)
      Name += 1; // Skip first '-'.

    if (!strcmp(Name, "-D")) {
      // Enable C++11 support if __cplusplus is defined an has the value "201103L"
      if (strcmp (o->Argument(0), "__cplusplus") == 0) {
        if (strcmp (o->Argument(1), "201103L") == 0) // is it C++11?
          Args.push_back("-std=c++11");
        else if (strcmp (o->Argument(1), "201300L") == 0 ||
            strcmp (o->Argument(1), "201402L") == 0)  // is it C++14?
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
          Args.push_back("-std=c++1y");
#else
          Args.push_back("-std=c++14");
#endif
        continue; // no need to copy this macro, clang defines it anyway
      }
      // Disable definition of macros that Clang will define anyway -- even with "UsePredefines = false".
      if (strcmp (o->Argument(0), "__STDC__") == 0 ||
          strcmp (o->Argument(0), "__STDC_HOSTED__") == 0 ||
          strcmp (o->Argument(0), "__STDC_VERSION__") == 0 ||
          strcmp (o->Argument(0), "__STDC_UTF_16__") == 0 ||
          strcmp (o->Argument(0), "__STDC_UTF_32__") == 0 ||
          strcmp (o->Argument(0), "__has_include(STR)") == 0 ||
          strcmp (o->Argument(0), "__has_include_next(STR)") == 0)
        continue;
      // Don't define the __puma macro when Clang parses the code
      if (strcmp (o->Argument(0), "__puma") == 0)
        continue;
      // otherwise define the macro from the config file/command line in Clang
      Args.push_back(Name);
      StringBuf.push_back(o->Argument(0) + std::string("=") + o->Argument (1));
      Args.push_back(StringBuf.back().c_str());
    }
    else if (!strcmp(Name,"--target")) {
      Args.push_back("-triple");
      Args.push_back(o->Argument(0));
    }

    // Pass macro definitions and include paths to clang. Everything else is
    // Puma-specific and dropped.
    if ((strncmp(Name, "-I", 2) && strncmp(Name, "-i", 2)))
      continue;

    Args.push_back(Name);
    for (unsigned j = 0; j < o->Arguments (); j++)
      Args.push_back(o->Argument (j));
  }

  // Add the main TU.
  Args.push_back("-x");
  Args.push_back("c++");
  Args.push_back(conf.file_in());

  // Now create a -cc1 invocation out of the config.
  CompilerInstance *new_ci = new CompilerInstance;

#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
  new_ci->createDiagnostics();
  OwningPtr<CompilerInvocation> CI(new CompilerInvocation);
  CompilerInvocation::CreateFromArgs( *CI, Args.data(),
      Args.data() + Args.size(), new_ci->getDiagnostics());

  new_ci->setInvocation(CI.take());
#else // C++ 11 interface
  new_ci->createDiagnostics(new Diag(llvm::errs(), &new_ci->getDiagnosticOpts()));
  CompilerInvocation *CI = new CompilerInvocation;
  CompilerInvocation::CreateFromArgs( *CI, Args.data(),
      Args.data() + Args.size(), new_ci->getDiagnostics());

  new_ci->setInvocation(CI);
#endif

  // Create the diagnostics engine
  // Note: it is important to do this *after* calling 'setInvocation', because otherwise
  //       the diagnostics options are uninitialized / all false
  new_ci->createDiagnostics(new Diag(llvm::errs(), &new_ci->getDiagnosticOpts()));

  // Create vital components.
  if (_ci) {
    new_ci->setFileManager (&_ci->getFileManager ()); // reference in _ci must be set to 0 later!
    // forced includes shall not be modified
    new_ci->getInvocation ().getPreprocessorOpts ().Includes =
        _ci->getInvocation ().getPreprocessorOpts ().Includes;
  }
  else
    new_ci->createFileManager();
  new_ci->createSourceManager(new_ci->getFileManager());

#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
  new_ci->setTarget(clang::TargetInfo::CreateTargetInfo(new_ci->getDiagnostics(),
                                                        &new_ci->getTargetOpts()));
#else // C++ 11 interface
//  new_ci->setTarget(clang::TargetInfo::CreateTargetInfo(new_ci->getDiagnostics(),
//                                                        &new_ci->getTargetOpts()));
  const std::shared_ptr<clang::TargetOptions> targetOptions = std::make_shared<clang::TargetOptions>(new_ci->getTargetOpts());
//  targetOptions->Triple = llvm::sys::getDefaultTargetTriple();
  clang::TargetInfo *pTargetInfo = clang::TargetInfo::CreateTargetInfo(new_ci->getDiagnostics(), targetOptions);
  new_ci->setTarget (pTargetInfo);
#endif

  new_ci->getPreprocessorOpts().UsePredefines = false;

  if (_ci) {
    // refreshing compiler instance
    clang::SourceManager &new_SM = new_ci->getSourceManager ();
    clang::SourceManager &SM = _ci->getSourceManager ();
    for (clang::SourceManager::fileinfo_iterator fi = SM.fileinfo_begin(),
                                                 fe = SM.fileinfo_end();
         fi != fe; ++fi) {
      const clang::FileEntry *file_entry = fi->first;
      if (_closed_files.find (ACFileID (file_entry)) == _closed_files.end ()) {
        if (SM.isFileOverridden(file_entry) &&
            !new_SM.isFileOverridden(file_entry)) {
          // save changes in 'file_entry->getName ()' in the new source manager
          const llvm::MemoryBuffer *buf = fi->second->getRawBuffer();
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
          llvm::MemoryBuffer *bcopy = llvm::MemoryBuffer::getMemBufferCopy(
              buf->getBuffer(), buf->getBufferIdentifier());
          new_SM.overrideFileContents(fi->first, bcopy);
#else // C++ 11 interface
          std::unique_ptr<llvm::MemoryBuffer> bcopy (llvm::MemoryBuffer::getMemBufferCopy(
              buf->getBuffer(), buf->getBufferIdentifier()));
          new_SM.overrideFileContents(fi->first, std::move(bcopy));
#endif
        }
      }
    }
    _closed_files.clear ();
    string main_file = SM.getFileEntryForID (SM.getMainFileID ())->getName ();
    new_ci->InitializeSourceManager(clang::FrontendInputFile(main_file, clang::IK_CXX));
    _ci->setFileManager (0);
  }

  // store the new compiler instance, potentially replacing an old one
  delete _ci;
  _ci = new_ci;
}
#endif

// Add a forced include to the front end
void ACProject::add_forced_include (const string &file) {
  Filename incname;
  if (!SysCall::canonical (Filename(file.c_str()), incname))
    return; // error, shouldn't happen
#ifdef FRONTEND_PUMA
  config ().Add ("--include", incname.name ());
#else
  std::vector<std::string> &incs = _ci->getInvocation ().getPreprocessorOpts ().Includes;
  incs.push_back (incname.name ());
#endif
}

//Remove a forced include from the front end
void ACProject::remove_forced_include (const string &file) {
  Filename incname;
  if (!SysCall::canonical (Filename(file.c_str()), incname))
    return; // error, shouldn't happen
#ifdef FRONTEND_PUMA
  for (unsigned int i = 0; i < config ().Options (); i++) {
    const ConfOption *o = config().Option (i);
    if (string (o->Name ()) != "--include" || o->Arguments () != 1)
      continue;
    if (string(incname.name()) == o->Argument (0)) {
      config().Remove (o);
      break;
    }
  }
#else
  vector<string> &incs = _ci->getInvocation ().getPreprocessorOpts ().Includes;
  for (vector<string>::iterator i = incs.begin (); i != incs.end (); ++i) {
    if (string(incname.name()) == *i) {
      incs.erase (i);
      break;
    }
  }
#endif
}

// Remove all forced includes
void ACProject::remove_forced_includes () {
#ifdef FRONTEND_PUMA
  list<const ConfOption*> delete_opts;
  for (unsigned int i = 0; i < config ().Options (); i++) {
    const ConfOption *o = config().Option (i);
    if (string (o->Name ()) != "--include" || o->Arguments () != 1)
      continue;
    delete_opts.push_back (o);
  }
  for (list<const ConfOption*>::iterator i = delete_opts.begin ();
    i != delete_opts.end (); ++i)
    config().Remove(*i);
#else
  _ci->getInvocation ().getPreprocessorOpts ().Includes.clear ();
#endif
}

// Get the list of forced includes from the front end as a vector of filenames
void ACProject::get_forced_includes (vector<string> &forced_includes) {
#ifdef FRONTEND_PUMA
  forced_includes.clear ();
  for (unsigned int i = 0; i < config ().Options (); i++) {
    const ConfOption *o = config().Option (i);
    if (string (o->Name ()) != "--include" || o->Arguments () != 1)
      continue;
    forced_includes.push_back (o->Argument (0));
  }
#else
  forced_includes = _ci->getInvocation ().getPreprocessorOpts ().Includes;
#endif
}


Puma::Filename ACProject::getRelInclString (Puma::Filename from, Puma::Filename to) {

  // start be calculating absolute canonical paths for 'to'
  Filename to_abs;
  if (!SysCall::canonical (to, to_abs))
    return to;

//  cout << "to: " << to_abs.name() << endl;
//  cout << "from: " << from_abs.name() << endl;

  // as a first strategy try to find an include path relative to a
  // search path (-I option)
  for (unsigned i = config ().Options (); i > 0; i--) {
    const ConfOption *o = config ().Option (i-1);
    if (! strcmp (o->Name (), "-I")) {
      if (! o->Arguments ())
        continue;
      Filename canon_inc_dir;
      if (!SysCall::canonical (o->Argument (0), canon_inc_dir))
        continue;

      int len = strlen (canon_inc_dir.name ());
      if (strncmp (canon_inc_dir.name (), to_abs.name (), len) == 0)
        if (*(to_abs.name () + len) == '/')
          return to_abs.name () + len + 1;
    }
  }

  // alternatively generate a path from 'from' to 'to'.
  Filename from_abs;
  if (!SysCall::canonical (from, from_abs))
    return to_abs;

  // first find the point where the two absolute paths differ, e.g.
  // "/foo/bar/abcdef/file.h"
  // "/foo/bar/abcxyz/uvw/inc.cc"
  //           ^-- differ here: last_delim_to
  const char *from_pos = from_abs.name();
  const char *to_pos = to_abs.name();
  const char *last_delim_to = 0;
  while (*from_pos == *to_pos && *from_pos != '\0' && *to_pos != '\0') {
    if (*from_pos == '/')
      last_delim_to = to_pos;
    from_pos++;
    to_pos++;
  }
  last_delim_to++;

  // now count the number of remaining delimiters in the 'from' path
  string buffer;
  while (*from_pos != '\0') {
    if (*from_pos == '/')
      buffer += "../";
    from_pos++;
  }

  buffer += last_delim_to;
//  cout << "result: " << buffer << endl;
  return Puma::Filename(buffer.c_str());
}

#ifdef FRONTEND_CLANG
bool ACProject::isNewer (const char *file) const {

  FileInfo fileinfo;
  if (! SysCall::stat (file, fileinfo))
    return false; // File does not exists.
  long last_modified = fileinfo.modi_time ();

  // determine the destination path of the file
  ProjectFile::MapConstIter iter;
  if (!PathManager::isBelow (file, iter)) {
    assert (false); // if we came here, the file should be registered
    return false;
  }

  ProjectFile &project_file = (ProjectFile&)iter->second;
  Filename dest = project_file.dest ();
  if (!dest.is_defined ()) {
    // determine the destination path
    std::ostringstream path;
    if (!getDestinationPath (file, path))
      return false;
    string dest_path = path.str ();
    project_file.dest (dest_path.c_str ());
    dest = project_file.dest ();
  }

  bool newer = true;
  if (SysCall::stat (dest.name (), fileinfo))
    if (last_modified <= fileinfo.modi_time ())
      newer = false;

  return newer;
}

// Save all files that have been opened
void ACProject::save () const {
  clang::SourceManager &SM = _ci->getSourceManager();
  for (clang::SourceManager::fileinfo_iterator fi = SM.fileinfo_begin(),
                                               fe = SM.fileinfo_end();
       fi != fe; ++fi) {
    const clang::FileEntry *file_entry = fi->first;
    if (_closed_files.find(ACFileID(file_entry)) == _closed_files.end())
      save (file_entry, SM.isFileOverridden (file_entry));
  }
}

bool ACProject::is_protected (clang::FileEntry *file_entry) {
  // TODO: implement this function
  return false;
}


bool ACProject::make_dir_hierarchy (Filename path) const {

  // Remember where we are.
  char cwd[PATH_MAX];
  if (SysCall::getcwd (cwd, PATH_MAX, &err ()) == 0)
    return false;

  // Change into the directory and create every missing one
  char *path_copy = StrCol::dup (path.name ());
  char *p = path_copy;

  // TODO: here is a problem with different drives on Windows!!!
  if (path.is_absolute ()) {
    SysCall::chdir (path.root (), &err ());
    p += strlen (path.root ());
  }

  char *curr = p;
  while (*p != '\0') {
    if (*p == '/') {
      *p = '\0';
      if (strlen (curr) && ! make_dir (curr)) {
        delete[] path_copy;
        return false;
      }
      curr = p + 1;
    }
    p++;
  }

  delete[] path_copy;

  // Go back
  SysCall::chdir (cwd, &err ());

  return true;
}

bool ACProject::make_dir (const char *directory) const {
  while (!SysCall::chdir (directory))
    if (! SysCall::mkdir (directory, &err ())) {
      err () << sev_error << "unable to create directory \""
             << directory << "\"." << endMessage;
      return false;
    }
  return true;
}

// save an opened file to its destination
void ACProject::save (const clang::FileEntry *file_entry, bool is_modified) const {
  clang::SourceManager &SM = _ci->getSourceManager();
  const char *name = file_entry->getName ();
  assert (name);

  // Do not write files to protected paths or files from outside the
  // source directories.
  if (isProtected (name))
    return;

  // determine the destination path of the file
  ProjectFile::MapConstIter iter;
  if (!PathManager::isBelow (name, iter)) {
    assert (false); // if we came here, the file should be registered
    return;
  }

  ProjectFile &project_file = (ProjectFile&)iter->second;
  Filename dest = project_file.dest ();
  if (!dest.is_defined ()) {
    // determine the destination path
    std::ostringstream path;
    if (!getDestinationPath (name, path))
      return;
    string dest_path = path.str ();
    project_file.dest (dest_path.c_str ());
    dest = project_file.dest ();
  }

  // make sure that the directory for the file exists
  if (!make_dir_hierarchy (dest))
    return;

  // Check whether the file has to be updated.
  if (isNewer(name) || is_modified) {
    // Write the file to disk.
    char *file = (char*)dest.name ();
    ofstream out (file, ios::out);
    const llvm::MemoryBuffer *mb = SM.getMemoryBufferForFile (file_entry);
    out << mb->getBufferStart ();
  }

}

#endif
