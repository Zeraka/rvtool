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
#include <iostream>
#include <fstream>
#include <set>
using namespace std;
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h> // for access()!
#endif // _MSC_VER

// AspectC++ includes
#include "Weaver.h"
#include "ACConfig.h"
#include "ACFileID.h"
#include "Transformer.h"
#ifdef FRONTEND_PUMA
#include "ACUnit.h"
#endif
#include "IncludeExpander.h"
#include "ClangIncludeExpander.h"
#include "Repository.h"
#include "Naming.h"
#include "NamespaceAC.h"
#include "ACModel/XmlModelReader.h"
#include "PointCutExprParser.h"
#include "PointCutSearcher.h"
#include "PointCutContext.h"

// PUMA includes
#include "Puma/VerboseMgr.h"
#include "Puma/SysCall.h"
#ifdef FRONTEND_PUMA
#include "Puma/CProject.h"
#include "Puma/ErrorStream.h"
#include "Puma/FileUnit.h"
#include "Puma/CScanner.h"
#include "Puma/PreParser.h"
#include "Puma/CPrintVisitor.h"
#endif

// Some macro-like functions that define the application name and version
#include "version.h"

using namespace Puma;

Weaver::Weaver (ACProject& project, ACConfig &conf) :
  _project (project), _conf (conf),
  _line_mgr (err (), _conf)
 {
 }


#ifdef FRONTEND_CLANG
// helper function for checking whether the current line is a #line directive
static bool is_line_dir (const char *start, const char *eof) {
  const char *i = start;
  // skip whitespace at the beginning of the line
  while (isspace (*i) && i < eof)
    i++;
  if (!(i < eof && *i == '#'))
    return false;
  // skip whitespace again
  while (isspace (*i) && i < eof)
    i++;
  if (isdigit(*i) || (strncmp(i+1, "line ", 5) == 0))
    return true;
  return false;
}

// Token line number handling works different in the clang configuration. If #line directives
// are not generated (as in the Puma configuration), the line numbers in the project model and
// the joinpoint-API are wrong, because Phase 1 transformations will affect the loctions.
// To avoid this effect, the --no_line option has to be implemented at the end of all
// transformations steps (here) by removing all #line directives from the code instead of
// not generating the in the first place.
string update_line_directives_clang (llvm::StringRef buf, const char *filename, LineDirectiveMgr &line_mgr) {
  std::ostringstream os;
  unsigned lineno = 1;
  bool start_of_line = true;
  for (const char *i = buf.begin(), *e = buf.end(); i < e; ++i) {
    // check for \s*#\s*(line|[0-9])
    if (start_of_line && is_line_dir (i, e)) {
      bool skip_line = false;
      if (line_mgr.noline ()) {
        skip_line = true;
      }
      else {
        const char *p = strchr(i, '#'); // note: 'i' remains were it was (beginning of line
        if (strncmp(p+1, "line ", 5) == 0) {
          p = i + 6;
          // Skip over the line number.
          while (isspace(*p) || isdigit(*p))
            ++p;

          // Replace <ac... and <intro... line directives with pointers to the final
          // file; "#line <built-in>" shall be removed from output
          if (strncmp (p, "\"<built-in>\"", 12) == 0)
            skip_line = true;
          else if (strncmp (p, "\"<ac", 3) == 0 || strncmp (p, "\"<intro", 7) == 0) {
            os << "#line " << (lineno + 1) << " \"" << filename << "\"";
            i = strchr(p, '\n');
          }
        }
      }
      if (skip_line) {
        // skip to the end of line
        while ((i < e) && (*i != '\n') && (*i != '\r'))
          i++;
        if (i == e || (i+1 == e))
          break;
        if ((*i == '\n' && *(i+1) == '\r') || (*i == '\r' && *(i+1) == '\n'))
          i++; // double line ending (windows/mac)
        continue; // go to the next line without incrementing the line number counter
      }
    }

    // Count lines.
    start_of_line = (*i == '\n' || *i == '\r');
    if (start_of_line) {
      ++lineno;
      // handle double line ending (windows/mac)
      if (((i+1) < e) && ((*i == '\n' && *(i+1) == '\r') || (*i == '\r' && *(i+1) == '\n'))) {
        os << *i;
        i++;
      }
    }
    os << *i;
  }

  return os.str();
}
#endif

void Weaver::weave ()
 {
   VerboseMgr vm (cout);

   // configure the log output manager
   vm.verbose (_conf.verbose ());

   // now some action ...
   vm << "Running " << ac_program () << " " << ac_version () << endvm;
   bool use_old_repo = (_conf.repository () && getenv ("ACOLDREPO") != NULL);
   const char *repo_file = _conf.repository ();
   Repository repo (true);
   if (use_old_repo) {
#ifdef _MSC_VER
     if (!_access (repo_file, 04)) {
#else
     if (!access (repo_file, R_OK)) {
#endif // _MSC_VER
//       vm << "Opening project repository '" << repo_file
//	  << "'" << endvm;
       repo.open (repo_file, err ());
     }
     else {
//       vm << "Creating project repository '" << repo_file
//	  << "'" << endvm;
       repo.create (repo_file);
     }
   }
   // break in case of errors
   if (err ().severity () >= sev_error)
     return;

   if (!_conf.expr().empty()) {
     match_expr_in_repo (vm);
     return;
   }

   bool header_changed = false;
   if (_conf.iterate ()) {
     vm << "Simple Dependency Check" << endvm;
     PathIterator dep_iter (".*\\.a?h$");
     vm++;
     while (_project.iterate (dep_iter)) {
	 
	     if (_project.isNewer (dep_iter.file ())) {
	       header_changed = true;
	       vm << "new or modified: " << dep_iter.file() << endvm;
	     }
     }
     vm--;
   }

   // set of manipulated (and saved) units
   set<ACFileID> h_units;
   set<ACFileID> cc_units;
   string aspect_includes;
   string aspect_fwd_decls;
        
   if (!_conf.iterate () && !_conf.ifiles () && _conf.file_in ()) {

#ifdef FRONTEND_CLANG
     _project.create_compiler_instance (_conf);
#endif

     Transformer transformer (vm, err (), _project, repo, _conf, _line_mgr);

     // Transform a single translation unit
     ACFileID fid = translate (vm, _conf.file_in (), transformer);

     // remember this unit
     if (fid.is_valid())
       cc_units.insert (fid);

     // remember the aspect includes and forward declarations
//     aspect_includes  = transformer.aspect_includes ();
     _aspect_include_clusters = transformer.aspect_include_clusters();
     aspect_fwd_decls = transformer.aspect_fwd_decls ();
           
     // break in case of errors
     if (err ().severity () >= sev_error)
       return;
   }
   else if (_conf.iterate ()) {

     // Transform all translation units in the project directory tree
     stringstream extpat;
     extpat << ".*\\." << _conf.extension () << "$";
     
     bool first = true;
     PathIterator iter (extpat.str ().data ());
     while (_project.iterate (iter)) {
       if (!(_project.isNewer (iter.file ()) || header_changed)) {
      	 continue;
       }
       
#ifdef FRONTEND_CLANG
       _project.create_compiler_instance (_conf);
#endif
       // it seems that creating the transformer for every file is essential
       // to avoid very strange parse errors!
       Transformer transformer (vm, err (), _project, repo, _conf, _line_mgr);
       ACFileID fid = translate (vm, iter.file (), transformer);

       // remember that we need this file
       if (fid.is_valid())
         cc_units.insert (fid);

       // remember the aspect units for inclusion and their forward declarations
       if (first) {
         // aspect_includes  = transformer.aspect_includes ();
         _aspect_include_clusters = transformer.aspect_include_clusters();
         aspect_fwd_decls = transformer.aspect_fwd_decls ();
         first = false;
       }

#ifdef FRONTEND_CLANG
       clang::SourceManager &SM = _project.get_compiler_instance ()->getSourceManager();
       for (clang::SourceManager::fileinfo_iterator fi = SM.fileinfo_begin(),
                                                    fe = SM.fileinfo_end();
            fi != fe; ++fi) {
         const clang::FileEntry *file_entry = fi->first;
         if (cc_units.find (ACFileID (file_entry)) == cc_units.end ()) {
           SM.disableFileContentsOverride (file_entry);
           _project.close (file_entry);
         }
       }
#else
       // discard changes in header files
       UnitManager::UMap &umap = _project.unitManager ().getTable ();
       for (UnitManager::UMapIter iter = umap.begin ();
            iter != umap.end (); ++iter) {
         Unit *curr = (*iter).second;
         if (cc_units.find (curr) == cc_units.end ())
           _project.close (curr->name (), true, false);
       }
       _project.unitManager ().discardNonames ();
#endif
       
       // break in case of errors
       if (err ().severity () >= sev_error)
         return;
     }
   }
   
   if (_conf.ifiles () || header_changed) {
     vm << "Handling include files" << endvm;
     vm++;
     
#ifdef FRONTEND_CLANG
     _project.create_compiler_instance (_conf);
#endif

     stringstream str;
     str << "// This file is generated by AspectC++ \n\n";
     PathIterator h_iter (".*\\.h$");
     str << "#ifndef __ac_have_predefined_includes__\n"
            "/*** begin of includes ***/" << endl;
     while (_project.iterate (h_iter))  {
       Filename incname = _project.getRelInclString (_conf.file_in (), h_iter.file ());
	     str << "#include \"" << incname << "\"" << endl;
     }
     str << "/*** end of includes ***/\n#endif" << endl;
     
     ACFileID vfid = _project.addVirtualFile(_conf.file_in (), str.str ());

     Transformer transformer (vm, err (), _project, repo, _conf, _line_mgr);

     translate (vm, _conf.file_in (), transformer);

     // remember the aspect units for inclusion and forward declarations
     // aspect_includes  = transformer.aspect_includes ();
     _aspect_include_clusters = transformer.aspect_include_clusters();
     aspect_fwd_decls = transformer.aspect_fwd_decls ();

     // add header files to the list of manipulated units
     h_units.clear ();
     for (set<ACFileID>::iterator tf_iter = transformer.touched_files ().begin ();
          tf_iter != transformer.touched_files ().end (); ++tf_iter) {
       if (ACFileID(*tf_iter) != vfid &&
           _project.isBelow ((*tf_iter).name ().c_str ()) &&
           cc_units.find (ACFileID (*tf_iter)) == cc_units.end ())
         h_units.insert (*tf_iter);
     }

     // discard the generated translation unit
     _project.removeVirtualFile(vfid);

     vm--;
   }
   
   // break in case of errors
   if (err ().severity () >= sev_error)
      return;

#ifdef FRONTEND_CLANG
     // this make sure all closed file will no longer be seen
     _project.create_compiler_instance (_conf);
#endif

   vm << "Inserting unit pro- and epilogues" << endvm;
   insert_aspect_includes (vm, cc_units, h_units, aspect_includes, aspect_fwd_decls);
   
   vm << "Updating #line directives of generated code fragments" << endvm;
   update_line_directives (cc_units, h_units);
   
#ifdef FRONTEND_CLANG
     // this make sure all closed file will no longer be seen
     _project.create_compiler_instance (_conf);
#endif

     if (_conf.nosave ()) {
     vm << "Don't save" << endvm;
   }
   else {
     vm << "Saving" << endvm;
     vm++;

     if (_project.numPaths () > 0 && _project.dest (0L)) {
       vm << "Project tree" << endvm;

#if 0
       // discard the generated cc file if only headers should be produced
       if ((_conf.ifiles ()))
         _project.close (_conf.file_in (), true);
#endif // 0
       
       _project.save ();
     }
     
     if (_conf.file_out ()) {

       // expand project includes
       vm << "Expanding project includes" << endvm;
#ifdef FRONTEND_PUMA
       IncludeExpander ie (err (), _project, _line_mgr);
       ie.expand (_conf.file_in ());
#else
       ClangIncludeExpander cie (err (), _project, _line_mgr);
       std::ostringstream buf;
       cie.expand(buf);
#endif

       // update generated #line <NUM> "<ac...> directives for debuggers
       vm << "Fixing #line directives" << endvm;
#ifdef FRONTEND_PUMA
       update_line_directives (&(ie.unit ()), _conf.file_out ());
#else
       string code = update_line_directives_clang (buf.str(), _conf.file_out (), _line_mgr);
#endif
       
       // now save
       vm << "Path \"" << _conf.file_out () << "\"" << endvm;
       ofstream out (_conf.file_out (), ios::out|ios::binary);
       if (out.is_open ()) {
#ifdef FRONTEND_PUMA
         out << ie.unit ();
#else
         out << code;
#endif
       } else
         err () << sev_error << "can't open file \"" 
                << _conf.file_out () << "\"" << endMessage;
     }
     
     if (use_old_repo && repo.initialized ()) {
//       vm << "Saving project repository" << endvm;
       repo.save (err ());
     }

     vm--;
   }
   vm << "Done" << endvm;
 }


// update #line directives in all project files
void Weaver::update_line_directives (set<ACFileID> &cc_units,
  set<ACFileID> &h_units) {
  for (set<ACFileID>::iterator iter = cc_units.begin ();
       iter != cc_units.end (); ++iter) {
    ostringstream out;
    if (_project.getDestinationPath ((*iter).name ().c_str (), out)) {
      update_line_directives (*iter, out.str ().c_str ());
    }
  }
  for (set<ACFileID>::iterator iter = h_units.begin ();
       iter != h_units.end (); ++iter) {
    ostringstream out;
    if (_project.getDestinationPath ((*iter).name ().c_str (), out)) {
      update_line_directives (*iter, out.str ().c_str ());
    }
  }
}

// insert a pro- and epilogue into all saved units to make sure that in
// any case the relevant aspect headers will be defined
void Weaver::insert_aspect_includes (VerboseMgr &vm, set<ACFileID> &cc_units,
  set<ACFileID> &h_units, const string &aspect_includes, const string &aspect_fwd_decls) {
  vm++;
  for (set<ACFileID>::iterator iter = cc_units.begin ();
       iter != cc_units.end (); ++iter) {
    vm << "Manipulating translation unit file " << (*iter).name () << endvm;
//    insert_aspect_includes (vm, *iter, false, aspect_includes, aspect_fwd_decls);
    insert_aspect_includes (vm, *iter, false, gen_aspect_includes(*iter), aspect_fwd_decls);
  }
  for (set<ACFileID>::iterator iter = h_units.begin ();
       iter != h_units.end (); ++iter) {
    vm << "Manipulating header file " << (*iter).name () << endvm;
//    insert_aspect_includes (vm, *iter, true, aspect_includes, aspect_fwd_decls);
    insert_aspect_includes (vm, *iter, true, gen_aspect_includes(*iter), aspect_fwd_decls);
  }
  vm--;
}
       
void Weaver::insert_aspect_includes (VerboseMgr &vm, ACFileID fid,
  bool header, const string &aspect_includes, const string &aspect_fwd_decls) {

#ifdef FRONTEND_PUMA
  Unit *u = (Unit*)fid.file_entry (); // TODO: what about 'const' here?
  assert (u->isFile ());
  FileUnit *unit = (FileUnit*)u;
  
  ListElement *file_first = (ListElement*)unit->first ();
  ListElement *file_last  = (ListElement*)unit->last ();
  if (!file_first) {
    // file is empty
    vm++; vm << "File is empty" << endvm; vm--;
    return;
  }
  assert (file_last);
 
  // create the file prologue
  ACUnit prologue (err ());
  ACUnit epilogue (err ());
  prologue.name ("<ac-prologue>");
  epilogue.name ("<ac-epilogue>");
#else
  clang::SourceManager &SM = _project.get_compiler_instance ()->getSourceManager ();
  const llvm::MemoryBuffer *mb = SM.getMemoryBufferForFile (fid.file_entry());
  if (mb->getBuffer () == "") {
    // file is empty
    vm++; vm << "File is empty" << endvm; vm--;
    return;
  }
  ostringstream prologue;
  ostringstream epilogue;
#endif // FRONTEND_PUMA
  
  // determine the name of the ac_FIRST... macro
  string first_macro = "__ac_FIRST_";
  first_macro += _conf.project_id ();
  
  // generate the preprocessor directives
  prologue << "#ifndef " << first_macro.c_str () << endl;
  prologue << "#define " << first_macro.c_str () << endl;
  prologue << "#define __ac_FIRST_FILE_";
  Naming::mangle_file (prologue, fid);
  prologue << endl;
  prologue << "#ifndef __ac_have_predefined_includes__" << endl;
  prologue << "#define __ac_have_predefined_includes__" << endl;
  prologue << "#endif // __ac_have_predefined_includes__" << endl;
  // insert AC only in header files, in cc files is has already been done
  if (header) {
    prologue << NamespaceAC::def( _conf );
    prologue << aspect_fwd_decls;
  }
  prologue << "#endif // " << first_macro.c_str () << endl;

  epilogue << endl << "#ifdef __ac_FIRST_FILE_";
  Naming::mangle_file (epilogue, fid);
  epilogue << endl;
  epilogue << aspect_includes;
  epilogue << "#undef " << first_macro.c_str () << endl;
  epilogue << "#undef __ac_FIRST_FILE_";
  Naming::mangle_file (epilogue, fid);
  epilogue << endl;
  epilogue << "#endif // __ac_FIRST_FILE_";
  Naming::mangle_file (epilogue, fid);
  epilogue << endl;

#ifdef FRONTEND_PUMA
  prologue << endu;
  unit->move_before (file_first, prologue);
  // insert a #line directive at this point
  _line_mgr.insert (unit, PumaToken ((Token*)file_first));

  epilogue << endu;
  // insert a #line directive at this point
  _line_mgr.insert (&epilogue, PumaToken ((Token*)epilogue.first ()));
  unit->move (file_last, epilogue);
#else
  string new_stuff = prologue.str ();
  new_stuff += mb->getBuffer();
  new_stuff += epilogue.str ();
#if CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)
  llvm::MemoryBuffer *buf =
      llvm::MemoryBuffer::getMemBufferCopy(new_stuff, fid.file_entry()->getName());
  SM.overrideFileContents(fid.file_entry(), buf);
#else // C++ 11 interface
  SM.overrideFileContents(fid.file_entry(),
      llvm::MemoryBuffer::getMemBufferCopy(new_stuff, fid.file_entry()->getName()));
#endif
#endif // FRONTEND_PUMA
}

// transform all #line <NUM> "<ac.." directives into
// #line <REAL-NUM> "<TARGET-FILENAME>". This is necessary for
// debuggers to find generated code.
void Weaver::update_line_directives (ACFileID fid, const char *filename) {
#ifdef FRONTEND_PUMA
  Unit *unit = (Unit*)fid.file_entry (); // TODO: what about 'const' here?
  int line = 1;
  bool in_dir = false;
  int have_line = 0;
  Token *start = 0;
  Token *end = 0;
  for (Token *token = (Token*)unit->first (); token;
       token = (Token*)unit->next (token)) {
    if (token->is_directive ()) {
      if (!in_dir) {
        in_dir = true;
        start = token;
      }
      if (strncmp ("\"<ac", token->text (), 4) == 0 ||
          strcmp ("\"intro\"", token->text ()) == 0)
        have_line = line;
    }
    else if (in_dir) {
      in_dir = false;
      if (have_line != 0) {
        end = (Token*)unit->prev (unit->prev (token));
        CUnit new_dir (err ());
        new_dir << "#line " << (have_line + 1) << " \"" << filename
                << "\"" << endu;
        assert (start && end);
        unit->move_before (start, new_dir);
        unit->kill (start, end);
        have_line = 0;
      }
    }
    line += token->line_breaks ();
  }
#else
  clang::SourceManager &sm = _project.get_compiler_instance ()->getSourceManager ();

  const llvm::MemoryBuffer *mb = sm.getMemoryBufferForFile(fid.file_entry ());
  string buf = update_line_directives_clang (mb->getBuffer(), filename, _line_mgr);
  sm.overrideFileContents(fid.file_entry(), llvm::MemoryBuffer::getMemBufferCopy(buf, fid.file_entry()->getName()));
#endif
}


ACFileID Weaver::translate (VerboseMgr &vm, const char *file,
			  Transformer &transformer)
 {
   vm << "Handling Translation Unit `";
   const char *fname = strrchr (file, (int)'/');
   vm << (fname ? ++fname : file) << "'." << endvm;

   vm++;
   ACFileID fid = transformer.work (file);
   vm--;
   
   return fid; // return the created unit
 }

class Searcher : public PointCutSearcher {
public:
  ACM_Pointcut *lookup_pct_func (bool root_qualified, std::vector<std::string> &qual_name) {
    return 0;
  }

  ACM_Attribute *lookup_pct_attr(bool root_qualified, std::vector<string> &qual_name) {
    return 0;
  }
};

void Weaver::match_expr_in_repo(VerboseMgr &vm) {
  const char *repo_file = _conf.repository();

  if (getenv("ACOLDREPO") != NULL) {
    err() << sev_error << "matching not supported with old repository format"
        << endMessage;
    return;
  }

  vm << "Reading project repository '" << repo_file << "'" << endvm;
  // TODO: O_RDONLY doesn't work. -> fix
  int fd = SysCall::open_excl (repo_file, O_RDWR, &err());
  ProjectModel project_model;
  XmlModelReader reader;
  if (!reader.read (project_model, repo_file, fd)) {
    err() << sev_error << "project repository '" << repo_file << "' cannot be opened"
    " or is invalid" << endMessage;
    return;
  }
  SysCall::close_excl (fd, &err ());
  if (project_model.get_version () != ac_version ())
    err() << sev_warning << "project file version '" << project_model.get_version().c_str ()
         << "' differs from ac++ version" << endMessage;

  string pct = _conf.expr();
  if (pct == "") {
    err() << sev_error << "Empty pointcut expression." << endMessage;
    return;
  }

  vm << "Fixing non-persistent element relations" << endvm;
  ProjectModel::Selection all_classes;
  project_model.select ((JoinPointType)(JPT_Class|JPT_Aspect), all_classes);
  // set derived relation for all base relations
  for (ProjectModel::Selection::iterator iter = all_classes.begin ();
     iter != all_classes.end (); ++iter) {
    ACM_Class &cls = (ACM_Class&)**iter;
    typedef ACM_Container<ACM_Class, false> BC;
    BC &bases = cls.get_bases();
    for (BC::iterator bi = bases.begin (); bi != bases.end (); ++bi)
      (*bi)->get_derived().insert(&cls);
  }

  ProjectModel::Selection all_calls;
  project_model.select (JPT_Call, all_calls);
  // set backward links from all functions to calls of it
  for (ProjectModel::Selection::iterator iter = all_calls.begin ();
     iter != all_calls.end (); ++iter) {
    ACM_Call &call = (ACM_Call&)**iter;
    call.get_target()->get_calls().insert(&call);
  }

  vm << "Matching pointcut expression " << pct << endvm;
  // remove blanks (not handled by parser correctly); blanks in match expression must remain
  string tmp;
  bool in_quotes = false;
  for (string::iterator i = pct.begin (); i != pct.end(); ++i) {
    if (!in_quotes && *i == ' ')
      continue;
    tmp += *i;
    if (*i == '\"')
      in_quotes = !in_quotes;
  }
  pct = tmp;

  PointCutExprParser *pce_parser = PointCutExprParser::instance( _conf );
  Searcher searcher;
  PointCutContext context (project_model, _conf);
  PointCutExpr *pce = 0;
  try {
    pce = pce_parser->parse(pct, searcher);
    pce->semantics(err (), context, _conf.warn_compatibility());

  }
  catch (const std::exception &e) {
    err () << sev_error
      << "Invalid pointcut expression: " << e.what() << "." << endMessage;
    return;
  }

  // match all joinpoints
  // iterate through all introduction advice in the plan
  ProjectModel::Selection all;
  project_model.select ((JoinPointType)(JPT_Class|JPT_Aspect|JPT_Function|JPT_Variable|JPT_Code), all);
  // update the plan for intros
  for (ProjectModel::Selection::iterator iter = all.begin ();
     iter != all.end (); ++iter) {
    ACM_Any &jpl = (ACM_Any&)**iter;
    if (is_pseudo(jpl))
      continue;
    Binding binding;     // binding and condition not used for intros
    Condition condition;
    if (pce->match (jpl, context, binding, condition)) {
      cout << filename (jpl) << ":" << line(jpl) << ":\t" << jpl.type_str() << " \"";
      if (jpl.type_val() & JPT_Code)
       cout << signature((ACM_Code&)jpl);
      else
       cout << signature((ACM_Name&)jpl);
      cout << "\"";
      if (condition)
        cout << " ; condition: " << condition;
      cout << endl;
    }
  }
}

string Weaver::gen_aspect_includes (ACFileID target_unit) {
  // generate the code
  stringstream cluster;
  for (list<AspectIncludeCluster>::const_iterator c = _aspect_include_clusters.begin();
      c != _aspect_include_clusters.end(); ++c) {
    cluster << "#ifdef __ac_need_";
    Naming::mangle_file (cluster, c->_aspect);
    cluster << endl;

    const set<ACFileID> &cluster_units = c->_cluster;
    for (set<ACFileID>::const_iterator i = cluster_units.begin ();
      i != cluster_units.end (); ++i) {
      ACFileID aspect_unit = *i;
      Filename incname = _project.getRelInclString (target_unit.name ().c_str (), aspect_unit.name ().c_str ());
      cluster << "#ifndef __ac_have_";
      Naming::mangle_file (cluster, aspect_unit);
      cluster << endl;
      cluster << "#define __ac_have_";
      Naming::mangle_file (cluster, aspect_unit);
      cluster << endl;
      cluster << "#include \"" << incname << "\"" << endl;
      cluster << "#endif" << endl;
    }

    cluster << "#endif" << endl;
  }
  return cluster.str ();
}

