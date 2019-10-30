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

#ifdef FRONTEND_CLANG
#include "ClangIntroSema.h"
#include "ClangIntroParser.h"
#include "clang/Parse/ParseDiagnostic.h"
#endif

// AspectC++ includes
#include "Transformer.h"
#include "PointCut.h"
#include "PointCutContext.h"
#include "OrderInfo.h"
#include "AdviceInfo.h"
#include "AspectInfo.h"
#include "IntroductionInfo.h"
#include "Plan.h"
#include "Repository.h"
#include "PointCutContext.h"
#include "CFlow.h"
#include "BackEndProblems.h"
#include "ACConfig.h"
#include "ACIntroducer.h"
#include "IncludeGraph.h"
#include "PointCutExpr.h"
#include "ModelBuilder.h"
#include "IntroductionUnit.h"
#include "version.h"
#include "ACModel/XmlModelWriter.h"
#include "ACModel/XmlModelReader.h"
#include "Phase1.h"
#include "NamespaceAC.h"
#ifdef FRONTEND_CLANG
#include "ClangASTConsumer.h"
#include "ClangBinding.h"
#endif

// PUMA includes
#include "Puma/VerboseMgr.h"
#include "Puma/SysCall.h"
#ifdef FRONTEND_PUMA
#include "Puma/CProject.h"
#include "Puma/CCSemVisitor.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CArgumentInfo.h"
#include "Puma/CTranslationUnit.h"
#include "Puma/CSemDatabase.h"
#include "Puma/CPrintVisitor.h"
#ifdef PROFILING
#include "Puma/GenericProfiler.ah"
#endif
#endif

#ifdef FRONTEND_CLANG
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/AST/ASTContext.h"
#endif

// C++ includes
#include <sstream>
using std::stringstream;
using std::endl;
#include <stdlib.h> // for getenv
#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h> // for access()!
#endif // _MSC_VER
#include <fcntl.h>

using namespace Puma;

ACFileID Transformer::work (const string &tunit_name) {

  // determine back-end compiler problems and setup code weaver
  BackEndProblems back_end_problems;
  back_end_problems._local_class       = _conf.problem_local_class ();
  back_end_problems._spec_scope        = _conf.problem_spec_scope ();
  back_end_problems._use_always_inline = !_conf.problem_force_inline ();
  back_end_problems._warn_macro        = _conf.warn_macro();
  back_end_problems._warn_deprecated   = _conf.warn_deprecated();
  _code_weaver.problems (back_end_problems);
  
  _vm << "Path \"" << tunit_name << "\"" << endvm;

  ModelBuilder jpm (_vm, _err, _conf, _project);
  jpm.set_version(ac_version ());
  IncludeGraph ig (_project);
  
  // perform the transformation
  Unit *unit = 0;
  bool ok = (phase1 (unit, tunit_name, jpm, ig) &&
             phase2 (unit, tunit_name, jpm, ig));

  if (!ok)
    _vm << "Aborting" << endvm;
#ifdef FRONTEND_CLANG
  if (ok) {
    clang::CompilerInstance *ci = _project.get_compiler_instance();
    clang::FileManager &fm = ci->getFileManager ();
    const clang::FileEntry *fe = fm.getFile (tunit_name);
    return fe;
  }
  return ACFileID(0);
#else
  return unit;
#endif
}

#ifdef FRONTEND_CLANG
void Transformer::reinitializeSourceManager(clang::CompilerInstance *ci,
    CodeWeaver &cwb, const char *name) {
  clang::SourceManager *NewSM = new clang::SourceManager(ci->getDiagnostics(),
                                                         ci->getFileManager());
  // Fetch code weaver changes into the new source manager.
  cwb.commit(*NewSM);
  clang::SourceManager &OldSM = ci->getSourceManager();
  // Transplant files that were changed in phase 1 but not rewritten in phase 2
  // into the new SourceManager.
  for (clang::SourceManager::fileinfo_iterator fi = OldSM.fileinfo_begin(),
                                               fe = OldSM.fileinfo_end();
       fi != fe; ++fi) {
    if (!_project.isVirtualFile(ACFileID(fi->first)))
      _touched_files.insert (fi->first);
    if (OldSM.isFileOverridden(fi->first) &&
        !NewSM->isFileOverridden(fi->first)) {
      const llvm::MemoryBuffer *buf = fi->second->getRawBuffer();
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
      llvm::MemoryBuffer *bcopy = llvm::MemoryBuffer::getMemBufferCopy(
          buf->getBuffer(), buf->getBufferIdentifier());
      NewSM->overrideFileContents(fi->first, bcopy);
#else // C++ 11 interface
      NewSM->overrideFileContents(fi->first,
          llvm::MemoryBuffer::getMemBufferCopy(buf->getBuffer(), buf->getBufferIdentifier()));
#endif
    }
  }

  // Now insert the new source manager and initialize it.
  ci->resetAndLeakSourceManager ();
  ci->setSourceManager(NewSM);
  ci->InitializeSourceManager(clang::FrontendInputFile(name, clang::IK_CXX));

  // Make sure the diagnostics engine doesn't have references to source
  // locations from the old source manager:
  ci->getDiagnostics().Reset();
}
#else
void Transformer::update_touched_files () {
  UnitManager::UMap &umap = _project.unitManager ().getTable ();
  for (UnitManager::UMapIter iter = umap.begin ();
       iter != umap.end (); ++iter) {
    Unit *unit = (*iter).second;
    if (unit->isFile ())
      _touched_files.insert (unit);
  }
}
#endif


bool Transformer::phase1 (Unit *&unit, const string &tunit_name,
    ModelBuilder &jpm, IncludeGraph &ig) {

#ifdef PROFILING
  static Profiler::Data data (__PRETTY_FUNCTION__, NULL);
  Profiler::ClockTicks start = Profiler::rdtsc ();
#endif

#ifdef FRONTEND_CLANG
  clang::CompilerInstance *ci = _project.get_compiler_instance();
  _err.set_source_manager(&ci->getSourceManager());
#endif

  // Create a forced include for each aspect header file that should be
  // considered in this translation unit. Without this the phase 1 parser
  // would not be able to analyse aspect headers!
  if (_conf.iterate_aspects ()) {
    // collect the names of aspect header files,
    // generate a forced include for each of them
    PathIterator ah_iter (".*\\.ah$");
    while (_project.iterate (ah_iter))
      _project.add_forced_include(ah_iter.file ());
  }
  else {
    // Get the names from the configuration object (-a options)
    for (int i = 0; i < _conf.aspect_headers (); i++)
      _project.add_forced_include(_conf.aspect_header (i));
  }

  // fill the model while parsing the AOP elements of the language in phase 1
  _vm << "Parsing ..." << endvm;
#ifdef FRONTEND_PUMA
  Phase1 phase1 (jpm, tunit_name, _project, _conf, _code_weaver);
  int result = phase1.run (ig);
  unit = phase1.unit ();
#else
  macro_recorder = &_code_weaver;
  Phase1 phase1 (jpm, tunit_name, _project, _conf, _code_weaver, ig);
  int result = phase1.run ();
  macro_recorder = 0;
#endif
  if (result == 0) {
    _vm << "file is empty" << endvm;
    unit = 0;
  }
  if (result != 1)
    return false;

//  cout << "Project Model after Phase1" << endl;
//  jpm.dump();

  // set up the project repository
  _repo.setup (jpm.tunit_file());

  // now remove the forced includes of aspect headers from the parser configuration
  // and generate includes at the end of the source code
  string aspect_includes = "";
  vector<string> incs;
  _project.get_forced_includes(incs);
  for (vector<string>::iterator i = incs.begin (); i != incs.end (); ++i) {
    if ((*i).rfind(".ah") == ((*i).size()-3)) {// aspect header!
      _project.remove_forced_include (*i);
      aspect_includes += "#include \"";
      Filename incname = _project.getRelInclString (tunit_name.c_str(), (*i).c_str());
      aspect_includes += incname.name();
      aspect_includes += "\"\n";
    }
  }
  if (aspect_includes != "") {
    _vm << "Inserting aspect header includes" << endvm;
    aspect_includes =
        string ("\n#ifndef __ac_have_predefined_includes__\n"
        "/*** begin of aspect includes ***/\n") +
        aspect_includes +
        "/*** end of aspect includes ***/\n" +
        "#endif";

    _code_weaver.insert (_code_weaver.footer_pos (), aspect_includes);
  }

  // generate a string with aspect forward declarations
  _vm << "Weaving Aspects Forward Declarations ..." << endvm;
  determine_aspect_fwd_decls (jpm);
  _code_weaver.insert (_code_weaver.header_pos (), _aspect_fwd_decls);

  _vm << "Inserting namespace AC" << endvm;
  _code_weaver.insert( _code_weaver.header_pos(), NamespaceAC::def( _conf ) );
  
  // STU mode and not generating transformed headers!
  if (!_conf.iterate () && !_conf.ifiles () && _conf.file_in ()) {
    // expand forced includes in source code
    string forced_includes = "";
    vector<string> incs;
    _project.get_forced_includes(incs);
    for (vector<string>::iterator i = incs.begin (); i != incs.end (); ++i) {
      const string &filename = *i;
      forced_includes += "#include ";
      if (filename[0] == '\"' || filename[0] == '<')
        forced_includes += filename;
      else {
        forced_includes += "\"";
        forced_includes += filename;
        forced_includes += "\"";
      }
      forced_includes += "\n";
    }
    _project.remove_forced_includes();
    if (forced_includes != "") {
      _vm << "Inserting forced includes" << endvm;
      _code_weaver.insert (_code_weaver.header_pos (), forced_includes);
    }
  }

//#ifdef FRONTEND_CLANG
//  phase1.attrManipulation();
//#endif

  _vm << "Committing" << endvm;


#ifdef FRONTEND_PUMA
  update_touched_files ();
  _code_weaver.commit ();
#else
  // Reinitialize the source manager for phase 2. Commit changes into the new
  // SourceManager.
  _phase1_sm = &ci->getSourceManager();
  reinitializeSourceManager(ci, _code_weaver, tunit_name.c_str ());
//  reinitializeSourceManager(ci, _code_weaver, unit->name());
  _err.set_source_manager(&ci->getSourceManager());
#endif

  // Some debugging code:
//  unit->print(cout);

//    _vm << "Stage1 save!" << endvm;
//    _project.save();
//    _vm << " done." << endvm;
//    exit(1);

#ifdef PROFILING
  Profiler::ClockTicks end = Profiler::rdtsc ();
  data._time += Profiler::duration (start, end);
  data._calls = 1;
#endif

  return (_err.severity () < sev_error);
}


bool Transformer::phase2 (Unit *unit, const string &tunit_name, ModelBuilder &jpm, IncludeGraph &ig) {

#ifdef PROFILING
  static Profiler::Data data (__PRETTY_FUNCTION__, NULL);
  Profiler::ClockTicks start = Profiler::rdtsc ();
#endif

  Plan plan (_err, jpm, _conf);

  _vm << "Preparing introductions ..." << endvm;
  PointCutContext context (jpm, _conf);

  // perform semantic analysis of all pointcut expressions used for introductions
  const list<IntroductionInfo*> &intros = plan.introduction_infos ();
  for (list<IntroductionInfo*>::const_iterator i = intros.begin ();
       i != intros.end (); ++i) {
    IntroductionInfo *intro_info = *i;
    ACM_Introduction &intro = intro_info->intro();
    context.concrete_aspect(intro_info->aspect());
    context.pct_func (intro.get_pointcut());
    PointCutExpr *pce = (PointCutExpr*)intro_info->pointcut_expr().get();
    pce->semantics(_err, context, _conf.warn_compatibility());
    if (!(pce->possible_types() & JPT_Class)) {
      _err << sev_warning
           << Location (Filename (filename (intro).c_str ()), line (intro))
           << "pointcut expression for introduction can't match class"
           << endMessage;
    }
  }

  // ... and also for the pointcut expressions in order advice
  const list<OrderInfo*> &orders = plan.order_infos ();
  for (list<OrderInfo*>::const_iterator i = orders.begin ();
       i != orders.end (); ++i) {
    OrderInfo *oi = *i;
    context.concrete_aspect (oi->aspect());
    oi->analyze_exprs (_err, context, _conf.warn_compatibility());
    if (getenv ("ACOLDREPO")) {
      _repo.update (*oi);
    }
  }

  _vm << "Parsing again ..." << endvm;

#ifdef FRONTEND_PUMA
  // parse the translation unit, but ignore function bodies
  ::ACIntroducer introducer (plan, _code_weaver, _parser, jpm, ig, _conf);
  _parser.introducer (&introducer);
  CTranslationUnit *tunit = _parser.parse (*unit, _project);
  _parser.introducer (0);

  // do semantic analysis of expressions
  _sem_visitor.run (tunit->tree ());
  list<CTree*> &ah_trees = introducer.ah_trees ();
  for (list<CTree*>::iterator i = ah_trees.begin (); i != ah_trees.end (); ++i)
    if (*i)
      _sem_visitor.run (*i);

  if (_err.severity () >= sev_error) {
    // TODO: delete takes too much time and has no real use for ac++
    // so we skip it for now
    // delete tunit;
    return false;
  }
#else
  clang::CompilerInstance *ci = _project.get_compiler_instance();
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
  ClangASTConsumer *Consumer = new ClangASTConsumer(jpm);
  ci->createPreprocessor();
  ci->createASTContext();
  ci->setASTConsumer(Consumer);
  clang::Preprocessor &PP = ci->getPreprocessor();
  ::ACIntroducer introducer (plan, _code_weaver, jpm, ig, _conf);
  ClangIntroSema *intro_sema =
      new ClangIntroSema(introducer, PP, ci->getASTContext (),
      *Consumer, clang::TU_Complete, NULL);
#else // C++ 11 interface
  std::unique_ptr<ClangASTConsumer> Consumer (new ClangASTConsumer(jpm));
  ci->createPreprocessor(clang::TU_Complete);
  ci->createASTContext();
  ci->setASTConsumer(std::move (Consumer));
  clang::Preprocessor &PP = ci->getPreprocessor();
  ::ACIntroducer introducer (plan, _code_weaver, jpm, ig, _conf);
  ClangIntroSema *intro_sema =
      new ClangIntroSema(introducer, PP, ci->getASTContext (),
      ci->getASTConsumer (), clang::TU_Complete, NULL);
#endif
  ci->setSema (intro_sema);
  macro_recorder = &_code_weaver;
//  PP.enableIncrementalProcessing();
  introducer._ci = ci;

  // make sure that attributes can be handled properly by ClangWeaverBase
  _code_weaver.set_annotation_map(&jpm.annotation_map());

  ci->getDiagnosticClient().BeginSourceFile(ci->getLangOpts(), &PP);
#if FRONTEND_CLANG < 38
    PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
                                           PP.getLangOpts());
#else
  PP.getBuiltinInfo().initializeBuiltins(PP.getIdentifierTable(),
                                         PP.getLangOpts());
#endif
  ClangIntroParser P(PP, *intro_sema, true);
  introducer._parser = &P;
  PP.EnterMainSourceFile();
  P.Initialize();
  clang::Parser::DeclGroupPtrTy ADecl;
  clang::ExternalASTSource *External = ci->getASTContext().getExternalSource();
  if (External)
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
    External->StartTranslationUnit(Consumer);
#else // C++ 11 interface
  External->StartTranslationUnit(&ci->getASTConsumer ());
#endif

  // handle code injections at the beginning of the translation unit
  introducer.tunit_start();

  // parse all top-level declarations
  if (P.ParseTopLevelDecl(ADecl)) {
    if (!External && !intro_sema->getLangOpts().CPlusPlus)
      P.Diag(clang::diag::ext_empty_translation_unit);
  } else {
    do {
      // If we got a null return and something *was* parsed, ignore it.  This
      // is due to a top-level semicolon, an action override, or a parse error
      // skipping something.
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
      if (ADecl && !Consumer->HandleTopLevelDecl(ADecl.get()))
#else // C++ 11 interface
      if (ADecl && !ci->getASTConsumer ().HandleTopLevelDecl(ADecl.get()))
#endif
        break;
    } while (!P.ParseTopLevelDecl(ADecl));
  }

  // Process any TopLevelDecls generated by #pragma weak.
  for (clang::SmallVectorImpl<clang::Decl *>::iterator
      I = intro_sema->WeakTopLevelDecls().begin(),
      E = intro_sema->WeakTopLevelDecls().end(); I != E; ++I)
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
    Consumer->HandleTopLevelDecl(clang::DeclGroupRef(*I));

  Consumer->HandleTranslationUnit(intro_sema->getASTContext());
#else // C++ 11 interface
    ci->getASTConsumer().HandleTopLevelDecl(clang::DeclGroupRef(*I));

    ci->getASTConsumer().HandleTranslationUnit(intro_sema->getASTContext());
#endif

  //ci->getASTContext().getTranslationUnitDecl()->dump();

  ci->getDiagnosticClient().EndSourceFile();
  // Abort in case of error.
  if (ci->getDiagnosticClient().getNumErrors() > 0) {
    // Indicate errors to Weaver.
    _err.severity(const_cast<ErrorSeverity &>(sev_error));
    return false;
  }

  CTranslationUnit *tunit = 0;
  macro_recorder = 0;
#endif

  //#ifdef TRY_INTRODUCER
//  cout << "Printing semantic database..." << endl;
//  tunit->db ().Dump (cout, 10);
//#endif // TRY_INTRODUCER
  
  //    CPrintVisitor printer;
  //    printer.print (tunit->tree (), cout);
  
  if (getenv ("ACOLDREPO")) {
    _vm << "Updating (old) repository intro part" << endvm;
    update_intros_in_repo (jpm);
  }
  
#ifdef FRONTEND_PUMA
  jpm.setup_phase2 (*tunit, ah_trees);
#endif

  // get all class from the join point model
  ProjectModel::Selection all_classes;
  jpm.select ((JoinPointType)(JPT_Class|JPT_Aspect), all_classes);
  // update the plan for intros
  for (ProjectModel::Selection::iterator iter = all_classes.begin ();
      iter != all_classes.end (); ++iter) {
    ACM_Class &jpl = (ACM_Class&)**iter;
    link_members (jpl);
    link_bases (jpl);
  }

  _vm << "Weaving access control bypass classes ..." << endvm;
  insert_bypass_class (jpm);

  _vm << "Weaving Join Points ..." << endvm;
  join_points (*tunit, jpm, plan);
  if (_err.severity () >= sev_error)
    return false;
      
  _vm << "Final cleanup" << endvm;
  cleanup (jpm);

  if (_conf.dynamic ()) {
    _vm << "Preparing for dynamic weaving" << endvm;
    prepare_dynamic_weaving (jpm);  
  }

  // generate a string with aspect header include directives
  determine_aspect_includes (ig);
  
  _vm << "Committing" << endvm;
#ifdef FRONTEND_PUMA
  update_touched_files ();
  _code_weaver.commit ();
#else
  // Now insert all IntroductionUnits back into the original files. After this
  // it's no longer legal to use the CodeWeaver.
  _code_weaver.resolve_introduction_units(introducer);

  // Reinitialize the source manager for the include expander step in Weaver.
  // Commit changes into the new SourceManager.
  clang::SourceManager *phase2_sm = &ci->getSourceManager();
  reinitializeSourceManager(ci, _code_weaver, tunit_name.c_str ());
  _err.set_source_manager(&ci->getSourceManager());
  delete _phase1_sm;
  delete phase2_sm;
#endif

  // TODO: delete takes too much time and has no real use for ac++
  // so we skip it for now 
  // delete tunit;
  
  _repo.cleanup ();

#ifdef PROFILING
  Profiler::ClockTicks end = Profiler::rdtsc ();
  data._time += Profiler::duration (start, end);
  data._calls = 1;
#endif

  return (_err.severity () < sev_error);
}


void Transformer::determine_aspect_fwd_decls (ModelBuilder &jpm) {
  // get all aspects from the join point model
  ProjectModel::Selection all_aspects;
  jpm.select (JPT_Aspect, all_aspects);

  // remember that these aspects should become friend of all classes
  _aspect_fwd_decls = "";
  for (ProjectModel::Selection::iterator iter = all_aspects.begin ();
       iter != all_aspects.end (); ++iter) {
    ACM_Aspect &jpl = (ACM_Aspect&)**iter;
    _aspect_fwd_decls += "class ";
    _aspect_fwd_decls += jpl.get_name();
    _aspect_fwd_decls += ";\n";
  }
}

void Transformer::determine_aspect_includes (const IncludeGraph &ig) {
  // find all files that are included by aspect headers
  if (_conf.iterate_aspects ()) {
    // collect the names of aspect header files,
    // generate a unit with include statements for these files,
    PathIterator ah_iter (".*\\.ah$");
    while (_project.iterate (ah_iter))
      aspect_include_cluster (ah_iter.file (), ig);
  }
  else {
    // Get the names from the configuration object (-a options)
    for (int i = 0; i < _conf.aspect_headers (); i++)
      aspect_include_cluster (_conf.aspect_header (i), ig);
  }
}

void Transformer::aspect_include_cluster (const char* ah_file,
  const IncludeGraph &ig) {

  // find the corresponding unit object for the aspect header file name
#ifdef FRONTEND_PUMA
  ACFileID fid = _project.unitManager ().get (ah_file, true);
//  if (!fid) {
#else
  ACFileID fid = _project.get_compiler_instance()->getFileManager().getFile(ah_file);
#endif
  if (!fid.is_valid()) {
    _err << sev_fatal << "Unit for \"" << ah_file << "\" not found or no file."
         << endMessage;
    return;
  }

  // determine the aspect header cluster for this aspect header unit
  set<ACFileID> cluster_units;
  determine_aspect_cluster (fid, ig, cluster_units);

  AspectIncludeCluster aic(fid);
  aic.set_cluster(cluster_units);
  _aspect_include_clusters.push_back(aic);
}


void Transformer::determine_aspect_cluster (ACFileID ah_unit,
  const IncludeGraph &ig, set<ACFileID> &cluster) {
  
  // if the ah file is already a cluster member, we return immediately
  if (cluster.find (ah_unit) != cluster.end ())
    return;

  // otherwise the unit will be inserted
  cluster.insert (ah_unit);
      
  // find all header files that are included by this aspect header
  set<ACFileID> inc_units;
  ig.included_files (ah_unit, inc_units);
  
  // include all aspect headers that affect join points in these headers
  // and also aspect headers that affect the aspect header itself
  inc_units.insert (ah_unit);
  AspectIncludes &ais = _code_weaver.aspect_includes ();
#ifdef FRONTEND_CLANG
  clang::SourceManager &sm =
      _project.get_compiler_instance()->getSourceManager();
#endif
  for (set<ACFileID>::iterator i = inc_units.begin ();
    i != inc_units.end (); ++i) {
    ACFileID inc_unit = *i;
    AspectIncludes::const_iterator aii = ais.find (inc_unit);
    if (aii != ais.end ()) {
      const set<AspectRef> &aspect_refs = aii->second;
      for (set<AspectRef>::const_iterator ari = aspect_refs.begin ();
        ari != aspect_refs.end (); ++ari) {
        ACM_Aspect &jpl_aspect = ari->_aspect->loc ();
#ifdef FRONTEND_PUMA
        Unit *aspect_unit = TI_Aspect::of (jpl_aspect)->unit ();
#else
        clang::Decl *aspect_decl = TI_Aspect::of(jpl_aspect)->decl();
        const clang::FileEntry *aspect_unit =
            sm.getFileEntryForID(sm.getFileID(aspect_decl->getLocation()));
#endif
        // recursively analyze the cluster of this aspect header unit
        determine_aspect_cluster (aspect_unit, ig, cluster);
      }
    }
  }
}

void Transformer::prepare_dynamic_weaving (ModelBuilder &jpm) {

#ifdef FRONTEND_PUMA
  // mark all operations that access introduced attributes
  const list<AccessInfo> &access_infos = jpm.access_infos ();
  for (list<AccessInfo>::const_iterator i = access_infos.begin ();
    i != access_infos.end (); ++i) {
    if (i->_info->isStatic () || i->_info->isAnonymous ())
      continue;
    Unit *unit = (Unit*)i->_info->Tree ()->token ()->belonging_to ();
    while (unit->isMacroExp ())
      unit = ((MacroUnit*)unit)->CallingUnit ();
    if (IntroductionUnit::cast (unit)) {
      ACToken tok_before (i->_tree->token ());
      const WeavePos &before = _code_weaver.weave_pos (tok_before, WeavePos::WP_BEFORE);
      _code_weaver.insert (before, string ("/*** +access ") +
        string (i->_info->QualName ()) + string (" ***/\n"));      
      ACToken tok_after (i->_tree->end_token ());
      const WeavePos &after = _code_weaver.weave_pos (tok_after, WeavePos::WP_AFTER);
      _code_weaver.insert (after, string ("/*** -access ") +
        string (i->_info->QualName ()) + string (" ***/\n"));      
      if (i->_tree->NodeName () == CT_MembPtrExpr::NodeId () ||
          i->_tree->NodeName () == CT_MembRefExpr::NodeId ()) {
        CTree *op = i->_tree->Son (1);
        ACToken tok_before (op->token ());
        const WeavePos &before = _code_weaver.weave_pos (tok_before, WeavePos::WP_BEFORE);
        _code_weaver.insert (before, string ("/*** op ") +
          string (i->_info->QualName ()) + string (" ***/\n"));      
      }
    }
  }
  
  // mark all classes that contain dynamically introduced attributes
  ProjectModel::Selection classes;
  jpm.select ((JoinPointType)(JPT_Class|JPT_Aspect), classes);
  for (ProjectModel::Selection::iterator i = classes.begin ();
    i != classes.end (); ++i) {
    ACM_Class *cls = (ACM_Class*)*i;
    if (!cls->get_intro_target ())
      continue;
    bool mark_class = false;
    const TI_Class *ti = TI_Class::of (*cls);
    CClassInfo *ci = ti->class_info ();
    for (unsigned i = 0; i < ci->Attributes (); i++) {
      CAttributeInfo *attr = ci->Attribute (i);
      if (attr->isStatic () || attr->isAnonymous ())
        continue;
      Unit *unit = (Unit*)attr->Tree ()->token ()->belonging_to ();
      IntroductionUnit *iunit = IntroductionUnit::cast (unit);
      if (iunit) {
        mark_class = true;
        ACM_Name *jpl_aspect = (ACM_Name*)iunit->intro ()->get_parent ();
        ACToken tok_before (attr->Tree ()->ObjDecl ()->token ());
        const WeavePos &before = _code_weaver.weave_pos (tok_before, WeavePos::WP_BEFORE);
        _code_weaver.insert (before, string ("/*** +intro ") +
        signature (*jpl_aspect) + string (" ***/\n"));      
        ACToken tok_after (attr->Tree ()->ObjDecl ()->end_token ());
        const WeavePos &after = _code_weaver.weave_pos (tok_after, WeavePos::WP_AFTER);
        _code_weaver.insert (after, string ("/*** -intro ") +
        signature (*jpl_aspect) + string (" ***/\n"));      
      }
    }
    if (mark_class) {
      ACToken tok_before (((CT_ClassDef*)ci->Tree ())->ObjDecl ()->token ());
      const WeavePos &before = _code_weaver.weave_pos (tok_before, WeavePos::WP_BEFORE);
      _code_weaver.insert (before, string ("/*** +class ") +
      signature (*cls) + string (" ***/\n"));      
      
      ACToken tok_after (((CT_ClassDef*)ci->Tree ())->Members ()->end_token ());
      const WeavePos &after = _code_weaver.weave_pos (tok_after, WeavePos::WP_BEFORE);
      _code_weaver.insert (after, string ("/*** -class ") +
      signature (*cls) + string (" ***/\n"));      
    }
  }
#endif // FRONTEND_PUMA
}


void Transformer::update_intros_in_repo (ModelBuilder &jpm1) {
  ProjectModel::Selection intros;
  jpm1.select (JPT_Introduction, intros);
  for (ProjectModel::Selection::iterator i = intros.begin ();
       i != intros.end (); ++i) {
    ACM_Introduction &intro = *(ACM_Introduction*)*i;
    _repo.consider (intro);
  }
  ProjectModel::Selection classes;
  jpm1.select ((JoinPointType)(JPT_Class|JPT_Aspect), classes);
  for (ProjectModel::Selection::iterator i = classes.begin ();
       i != classes.end (); ++i) {
    ACM_Class &cls = *(ACM_Class*)*i;
    // skip this class if it has no plan yet
    if (!cls.has_plan ())
      continue;
    // collect the intros into this class
    set<ACM_Introduction*> intros; // a 'set', because the same intro might add a base class and a member
    typedef ACM_Container<ACM_BaseIntro, true> BContainer;
    BContainer bintros = cls.get_plan()->get_base_intros();
    for (BContainer::iterator i = bintros.begin (); i != bintros.end (); ++i)
      intros.insert ((*i)->get_intro());
    typedef ACM_Container<ACM_MemberIntro, true> MContainer;
    MContainer mintros = cls.get_plan()->get_member_intros();
    for (MContainer::iterator i = mintros.begin (); i != mintros.end (); ++i)
      intros.insert ((*i)->get_intro());
    // now update the repository with the intro information
    for (set<ACM_Introduction*>::iterator intro_iter = intros.begin ();
         intro_iter != intros.end (); ++intro_iter) {
      _repo.update (**intro_iter, cls);
    }
  }
}


void Transformer::cleanup (ModelBuilder &jpm) {

  // delete all pointcut definitions that are left
  ProjectModel::Selection pointcuts;
  jpm.select (JPT_Pointcut, pointcuts);
  for (ProjectModel::Selection::iterator i = pointcuts.begin ();
       i != pointcuts.end (); ++i) {
    ACM_Pointcut &pct = *(ACM_Pointcut*)*i;
    // delete all non-anonymous pointcut definitions
#ifdef FRONTEND_PUMA
    if (TI_Pointcut::of (pct)->func_info())
      _code_weaver.kill (((CT_InitDeclarator*)TI_Pointcut::of (pct)->tree())->ObjDecl());
#else
    if (clang::Decl *decl = TI_Pointcut::of (pct)->decl())
      _code_weaver.kill(
          _code_weaver.weave_pos(decl->getLocStart(), WeavePos::WP_BEFORE),
          _code_weaver.weave_pos(decl->getLocEnd().getLocWithOffset(1),
                                   WeavePos::WP_AFTER));
#endif
  }
}


void Transformer::insert_bypass_class (ModelBuilder &jpm) {
  _code_weaver.bypass_info_clear();

  // Iterate over classes and structs, what about unions?
  ProjectModel::Selection all_classes;
  jpm.select ((JoinPointType)(JPT_Class|JPT_Aspect), all_classes);
  for (ProjectModel::Selection::iterator i = all_classes.begin ();
       i != all_classes.end (); ++i) {
    ACM_Class *cls = (ACM_Class*)*i;

    // handle all classes that are not on the blacklist
    if (!_code_weaver.bypass_in_blacklist(cls)) {
      _code_weaver.bypass_insert (cls);
    }
  }
}


void Transformer::join_points (CTranslationUnit &tunit,
                                   ModelBuilder &jpm, Plan &plan) {

#ifdef PROFILING
  static Profiler::Data data (__PRETTY_FUNCTION__, NULL);
  Profiler::ClockTicks start = Profiler::rdtsc ();
#endif
  _vm++;

  _vm << "Advicecode manipulation" << endvm;
  // Iterate through advice
  ProjectModel::Selection advice_codes;
  jpm.select (JPT_AdviceCode, advice_codes);
  for (ProjectModel::Selection::iterator i = advice_codes.begin ();
       i != advice_codes.end (); ++i) {
    ACM_AdviceCode &code = *(ACM_AdviceCode*)*i;
    TI_AdviceCode  &ti   = *TI_AdviceCode::of (code);
    // setup ThisJoinPoint object of this advice code
#ifdef FRONTEND_PUMA
    _code_weaver.setup_tjp(ti.this_join_point (), ti.function ());
#else
    _code_weaver.setup_tjp(ti.this_join_point (), ti.decl ());
#endif
  }

  _vm << "Collecting Advice" << endvm;
  // Iterate through advice
  _vm++;

  PointCutContext context (jpm, _conf);

  // Create a data structure for collecting advice per join point type
  typedef list<AdviceInfo*> AdviceInfoList;
  typedef JoinPointType JPT;
  typedef map<JPT, AdviceInfoList> TypeAdviceMap;
  TypeAdviceMap advice_map;

  Plan::AspectContainer &aspects = plan.aspect_infos ();
  for (Plan::AspectContainer::iterator i = aspects.begin ();
    i != aspects.end (); ++i) {
    AspectInfo &aspect_info = (AspectInfo&)*i;
    ACM_Aspect &jpl_aspect = aspect_info.loc ();
    context.concrete_aspect (jpl_aspect);

    // setup thisJoinPoint for aspectOf function, if there is one
#ifdef FRONTEND_PUMA
    CFunctionInfo *aspect_of_func = TI_Aspect::of (jpl_aspect)->aspectof();
#else
    clang::FunctionDecl *aspect_of_func = TI_Aspect::of (jpl_aspect)->aspectof();
#endif
    if (aspect_of_func) {
      _vm << "Setting up thisJoinPoint for aspectof" << endvm;
      _code_weaver.setup_tjp(aspect_info.aspectof_this_join_point (),
          aspect_of_func);
    }

    // handle the advice for the current aspect
    list<AdviceInfo*> &advices = aspect_info.advice_infos ();
    for (list<AdviceInfo*>::const_iterator i = advices.begin ();
      i != advices.end (); ++i) {
      AdviceInfo *advice_info = *i;
      ACM_AdviceCode *code = &advice_info->code ();

#ifdef FRONTEND_PUMA
      CFunctionInfo *adfunc = TI_AdviceCode::of (*code)->function ();
      _vm << signature (jpl_aspect) << ": "  << adfunc->Name () << endvm;
#else
      //_vm << signature (jpl_aspect) << ": "  << adfunc->Name () << endvm; // TODO: clang
#endif
      _vm++;

      // let the pointcut evaluator create the pointcut expression object
      _vm << "Create pointcut expression tree" << endvm;
      advice_info->pointcut_expr() = code->get_pointcut ()->get_parsed_expr();
      PointCutExpr *pce = (PointCutExpr*)advice_info->pointcut_expr().get ();
      context.pct_func (code->get_pointcut ());
      if (pce) {
        ArgSet new_arg_bindings;
        typedef ACM_Container<ACM_Arg, true> Container;
        Container &arguments = context.pct_func()->get_args();
        for (Container::iterator i = arguments.begin (); i != arguments.end (); ++i)
          new_arg_bindings.append (*i);
        context.arg_bindings ().push (&new_arg_bindings);
        pce->semantics(_err, context, _conf.warn_compatibility());
        context.arg_bindings ().pop ();

        // set the pointcut type before destroy the expression(!)
        advice_info->pointcut().type (pce->type() == PCE_CODE ? PointCut::PCT_CODE : PointCut::PCT_CLASS);

        // remember the advice for each joinpoint type that might match
        int mask = 1;
        while (mask) {
          if (mask & JPT_Code) {
            pair<TypeAdviceMap::iterator, bool> result =
              advice_map.insert (TypeAdviceMap::value_type ((TypeAdviceMap::key_type)mask, AdviceInfoList()));
            result.first->second.push_back (advice_info);
          }
          mask <<= 1;
        }
      }

      // copy the cflow trigger pointcut from the expressions to the pointcut
      for (set<PointCutExpr*>::const_iterator iter = context.cflows ().begin ();
           iter != context.cflows ().end (); ++iter) {
        advice_info->pointcut().cflow_triggers(((PCE_CFlow*)*iter)->arg_pointcut ());
      }
      _vm--;
    }
    context.cflow_reset ();
  }
  _vm--;

  // now iterate over all join points and check whether they match the
  // pointcut expressions
  _vm << "Matching joinpoints" << endvm;
  _vm++;

  for (TypeAdviceMap::iterator mi = advice_map.begin (); mi != advice_map.end ();
      ++mi) {
    JPT jp_type = mi->first;
    if( ! _conf.data_joinpoints() && ( jp_type & ( JPT_Get | JPT_Set | JPT_Ref | JPT_GetRef | JPT_SetRef ) ) )
      continue; // skip non enabled (cmdline) joinpoints

    AdviceInfoList &advice_info_list = mi->second;
    ProjectModel::Selection all;
    jpm.select (jp_type, all);
    for (ProjectModel::Selection::iterator iter = all.begin ();
        iter != all.end (); ++iter) {
      assert( static_cast<ACM_Node *>( *iter )->type_val() & JPT_Code );
      ACM_Code &jpl = (ACM_Code&)**iter;
      if( ! _conf.builtin_operators() && ( jpl.type_val() == JPT_Builtin ) ) // suppress joinpoints of builtin_operators
        continue;
      bool has_advice = false;

      for (AdviceInfoList::iterator li = advice_info_list.begin ();
          li != advice_info_list.end (); ++li) {
        AdviceInfo *advice_info = *li;
        context.pseudo_true (false);

        // Current pointcut function:
        ACM_Pointcut* pointcut_func = advice_info->code().get_pointcut();
        context.pct_func(pointcut_func);

        // now match
        Binding binding;
        Condition condition;
        PointCutExpr *expr = (PointCutExpr*)advice_info->pointcut_expr().get();

        // Does the pointcut expression match the join point?
        bool match_result = expr->match (jpl, context, binding, condition);

        // Output eventually created warnings:
        for(vector<pair<ErrorSeverity, string>>::const_iterator sev_msg_pair_iter
            = context.messages().begin();
            sev_msg_pair_iter != context.messages().end(); ++sev_msg_pair_iter
        ) {
          _err << sev_msg_pair_iter->first
               << TI_Pointcut::of(*pointcut_func)->get_location()
               << sev_msg_pair_iter->second.c_str() << endMessage;
        }
        context.messages().clear();

        // Handle match result:
        if (match_result) {
          JoinPoint &jp = *new JoinPoint (&jpl, condition);
          advice_info->pointcut().append (jp);

          // consider this joinpoint in the big plan
          plan.consider( &jpl, jp.condition(), advice_info );
          has_advice = true;

          // remember units for inclusion of aspect headers
          _code_weaver.add_aspect_include (jp.location (), advice_info->aspect_info(),
                                  AspectRef::AR_ADVICE);

          // if the advice uses a joinpoint ID, make sure that one is allocated
          // for the matched joinpoint
          if (TI_AdviceCode::of (advice_info->code())->this_join_point().id() &&
              !jpl.has_jpid())
            jpl.set_jpid(jpm.alloc_jpid());

          if (jpl.type_val () == JPT_Call && !jpl.get_parent()) // TODO: pseudo
            continue;

          // check if the context variable binding is the same for all
          // non-pseudo join points
          if (advice_info->binding () != binding) {
            if (!advice_info->binding ()._used) {
              advice_info->binding () = binding;
            }
            else {
              _err << sev_error
                  << TI_Pointcut::of(*pointcut_func)->get_location()
                  << "incompatible argument bindings in pointcut expression"
                  << endMessage;
              // remove this erroneous advice from all lists
              for (TypeAdviceMap::iterator i = advice_map.begin (); i != advice_map.end ();
                  ++i) {
                i->second.remove (advice_info);
              }
              break;
            }
          }
        }
      }

#ifdef FRONTEND_CLANG
      // only required for features of the Clang variant
      if( has_advice ) {
        // do some preplanning that might influence other joinpoints
        _code_weaver.preplanTransform( jpl );

        // remember we have a plan ( and thus weave at this jpl )
        TI_Code::of( jpl )->remember_planned();

        // for implicit joinpoints: make sure the parent is considered so we can weave calling code there
        ACM_Code *node = &jpl;
        while( node && is_implicit( *node ) ) {
          assert( static_cast<ACM_Node *>( node->get_parent() )->type_val() & JPT_Code );
          node = static_cast<ACM_Code *>( node->get_parent() );
          plan.consider( node );
          TI_Code::of( *node )->remember_planned();
          TI_Code::of( *node )->remember_implicit();
        }

        // if we weave for a builtin copy constructor, we replace array types in the class (see CodeWeaver::gen_special_member_function)
        // give the class a chance to know that
        if( jpl.type_val() == JPT_Construction ) {
          assert( static_cast<ACM_Any *>( jpl.get_parent() )->type_val() == JPT_Function );
          ACM_Function *func = static_cast<ACM_Function *>( jpl.get_parent() );
          assert( static_cast<ACM_Any *>( func->get_parent() )->type_val() & ( JPT_Class | JPT_Aspect ) );
          ACM_Class *cls = static_cast<ACM_Class *>( func->get_parent() );

          if( func->get_builtin() && get_arg_count( *func ) == 1 ) // parallel to check in CodeWeaver::cons_join_point
            TI_Class::of( *cls )->remember_builtin_copyconstructor_advice();
        }
      }
#endif
    }
  }

  // again iterate over all aspects
  for (Plan::AspectContainer::iterator i = aspects.begin ();
    i != aspects.end (); ++i) {
    AspectInfo &aspect_info = (AspectInfo&)*i;
    ACM_Aspect &jpl_aspect = aspect_info.loc ();
    context.concrete_aspect (jpl_aspect);
    int index = 0; // CFlow index (each CFlow has a unique index per aspect)

    // and now over all advice
    list<AdviceInfo*> advices = aspect_info.advice_infos ();
    for (list<AdviceInfo*>::const_iterator i = advices.begin ();
      i != advices.end (); ++i) {
      AdviceInfo *advice_info = *i;
      PointCut &pc = advice_info->pointcut();
      // consider the cflow trigger needed for this advice in the plan
      const list<PointCut*> &trigger_pcs = pc.cflow_triggers();
      for (list<PointCut*>::const_iterator iter = trigger_pcs.begin ();
           iter != trigger_pcs.end (); ++iter, ++index) {
        PointCut *trigger_pc = *iter;
        // Consider a cflow trigger for every joinpoint is pointcut
        for (PointCut::iterator iter = trigger_pc->begin ();
             iter != trigger_pc->end (); ++iter) {
          const JoinPoint &jp = *iter;

          // consider this joinpoint in the big plan
          plan.consider ((ACM_Code*)jp.location (), CFlow (advice_info, index));

          // remember units for inclusion of aspect headers
          _code_weaver.add_aspect_include (jp.location (), aspect_info,
                                           AspectRef::AR_DECL);
        }
      }

      // update the project repository
      if (getenv ("ACOLDREPO"))
        _repo.update (*advice_info, pc);
    }

    _code_weaver.insert_invocation_functions (&jpl_aspect,
        aspect_info.ifct_defs (_code_weaver.problems ()));
  }
  _vm--;

  _vm << "Aspect ordering ..." << endvm;
  plan.order_code_joinpoints ();

  // now do the final checks on the accumulated plan
//  _vm << "Final checks before weaving code join points" << endvm;
//  plan.check_code_joinpoints ();

  // Don't weave if there were errors in the planning phase
  if (_err.severity () >= sev_error)
    return;

  bool use_old_repo = (_conf.repository() && getenv ("ACOLDREPO") != NULL);
  const char *repo_file = _conf.repository();
  if (!use_old_repo && repo_file) {
#ifdef _MSC_VER
    if (!_access (repo_file, 04)) {
#else
    if (!access (repo_file, R_OK)) {
#endif // _MSC_VER

      _vm << "Updating project repository '" << repo_file << "'" << endvm;
      int fd = SysCall::open_excl (repo_file, O_RDWR, &_err);
      ProjectModel project_model;
      XmlModelReader reader;
      if (!reader.read (project_model, repo_file, fd)) {
        _err << sev_error << "project repository '" << repo_file << "' cannot be opened"
        " or is invalid" << endMessage;
        return;
      }
      if (project_model.get_version () != ac_version ())
        _err << sev_warning << "project file version '" << project_model.get_version().c_str ()
             << "' differs from ac++ version" << endMessage;

      // merge jpm and project_mode here
      project_model.merge (jpm);

      // make project model file empty
      lseek (fd, 0, SEEK_SET);
      if (ftruncate (fd, 0) != 0)
        perror ("truncate");

      // save the merged model
      XmlModelWriter writer;
      if (!writer.write (project_model, repo_file, fd)) {
      _err << sev_error << "saving merged project file '" << repo_file <<
        "'failed" << endMessage;
      return;
      }
      SysCall::close_excl (fd, &_err);
    }
    else {
      _vm << "Creating project repository '" << repo_file << "'" << endvm;
      int fd = SysCall::create_excl (repo_file, 0600, &_err);
      XmlModelWriter writer;
      if (!writer.write (jpm, repo_file, fd)) {
        _err << sev_error << "saving new project file '" << repo_file <<
          "'failed" << endMessage;
        return;
      }
      SysCall::close_excl  (fd, &_err);
    }
  }

  _vm << "Type Check Functions" << endvm;
  _vm++;
  const TypeCheckSet &checks_false = plan.type_checks_false ();
  for (TypeCheckSet::const_iterator iter = checks_false.begin ();
       iter != checks_false.end (); ++iter) {
    _vm << "check for " << iter->second << " in "
        << signature(*iter->first) << " is false" << endvm;
    _code_weaver.type_check (iter->first, iter->second, false);
  }
  const TypeCheckSet &checks_true = plan.type_checks_true ();
  for (TypeCheckSet::const_iterator iter = checks_true.begin ();
       iter != checks_true.end (); ++iter) {
    _vm << "check for " << iter->second << " in "
        << signature (*iter->first) << " is true" << endvm;
    _code_weaver.type_check (iter->first, iter->second, true);
  }
  _vm--;

  _vm << "Access Join Points" << endvm;
  _vm++;
  for (int i = 0; i < plan.access_jp_plans (); i++) {
    ACM_Access &jp_loc = plan.access_jp_loc (i);

    if( is_implicit( jp_loc ) ) // check for implicit joinpoints
      continue; // skip here, they are handled internally by the weaver

    _vm << jp_loc.type_str() << ": " << signature (jp_loc) << endvm;
    if( jp_loc.type_val() == JPT_Builtin ) { // print implicit too
      ACM_Builtin &jp_builtin = static_cast<ACM_Builtin &>( jp_loc );
      typedef const ACM_Container<ACM_Access, true> SubList;
      SubList &implicit = jp_builtin.get_implicit_access();

      _vm++;
      for( SubList::const_iterator it = implicit.begin(); it != implicit.end(); it++ ) { // TODO iterate over all nesting level (not yet neccessary as no deep nesting)
        if( (*it)->has_plan() ) // only print jpls for which we weave
          _vm << (*it)->type_str() << ": " << signature( **it ) << endvm;
      }
      _vm--;
    }

    // handle access joinpoint itself
    _code_weaver.access_join_point (&jp_loc);
  }
  _vm--;

  _vm << "Execution Join Points" << endvm;
  _vm++;
  for (int i = 0; i < plan.exec_jp_plans (); i++) {
    ACM_Execution &jp_loc = plan.exec_jp_loc (i);
    _vm << signature (jp_loc) << endvm;

    // handle exec joinpoint itself
    _code_weaver.exec_join_point (&jp_loc);
  }
  _vm--;

  _vm << "Construction Join Points" << endvm;
  _vm++;
  for (int i = 0; i < plan.cons_jp_plans (); i++) {
    ACM_Construction &jp_loc = plan.cons_jp_loc (i);
    _vm << signature (jp_loc) << endvm;

    // handle construction joinpoint itself
    _code_weaver.cons_join_point (&jp_loc);
  }
  _vm--;

  _vm << "Destruction Join Points" << endvm;
  _vm++;
  for (int i = 0; i < plan.dest_jp_plans (); i++) {
    ACM_Destruction &jp_loc = plan.dest_jp_loc (i);
    _vm << signature (jp_loc) << endvm;

    // handle destruction joinpoint itself
    _code_weaver.dest_join_point (&jp_loc);
  }
  _vm--;

  _vm--;

  _vm << "Aspect Includes ..." << endvm;
  _code_weaver.insert_aspect_includes ();

#ifdef PROFILING
  Profiler::ClockTicks end = Profiler::rdtsc ();
  data._time += Profiler::duration (start, end);
  data._calls = 1;
#endif
}

// add references to the introduced elements to the plan
void Transformer::link_members (ACM_Class &jpl) {
  if (!jpl.has_plan ())
    return;

  ACM_ClassPlan *plan = jpl.get_plan ();

  typedef ACM_Container<ACM_Any, true> Container;
  Container &children = jpl.get_children();
  typedef ACM_Container<ACM_MemberIntro, true> Container2;
  Container2 &member_intros = plan->get_member_intros();
#ifdef FRONTEND_CLANG
  clang::SourceManager &sm = _project.get_compiler_instance()->getSourceManager();
#endif
  for (Container::iterator i = children.begin (); i != children.end (); ++i) {
    ACM_Any *any = *i;
    if (any->type_val() == JPT_Class) {
      ACM_Class *cls = (ACM_Class*)any;
#ifdef FRONTEND_PUMA
      Unit *u = (Unit*)TI_Class::of(*cls)->obj_info()->Tree()->token()->belonging_to();
#else
      clang::FileID fid = sm.getFileID(TI_Class::of(*cls)->decl()->getLocation());
      const llvm::MemoryBuffer *u = sm.getBuffer(fid);
#endif
      if (IntroductionUnit::cast(u)) {
        ACM_Introduction *intro = IntroductionUnit::cast(u)->intro();
        for (Container2::iterator mi = member_intros.begin ();
            mi != member_intros.end(); ++mi)
          if ((*mi)->get_intro() == intro)
            (*mi)->get_members().insert(cls);
      }
    }
    else if (any->type_val() == JPT_Function) {
      ACM_Function *func = (ACM_Function*)any;
      if (func->get_builtin())
        continue;
#ifdef FRONTEND_PUMA
      Unit *u = (Unit*)TI_Function::of(*func)->obj_info()->Tree()->token()->belonging_to();
#else
      clang::FileID fid = sm.getFileID(TI_Function::of(*func)->decl()->getLocation());
      const llvm::MemoryBuffer *u = sm.getBuffer(fid);
#endif
      if (IntroductionUnit::cast(u)) {
        ACM_Introduction *intro = IntroductionUnit::cast(u)->intro();
        for (Container2::iterator mi = member_intros.begin ();
            mi != member_intros.end(); ++mi)
          if ((*mi)->get_intro() == intro)
            (*mi)->get_members().insert(func);
      }
    }
  }
}

// add references to the introduced elements to the plan
void Transformer::link_bases (ACM_Class &jpl) {
  if (!jpl.has_plan ())
    return;

  ACM_ClassPlan *plan = jpl.get_plan ();
#ifdef FRONTEND_PUMA
  CClassInfo *ci = TI_Class::of(jpl)->class_info();
#else
  clang::CXXRecordDecl *ci = llvm::cast<clang::CXXRecordDecl>(TI_Class::of(jpl)->decl());
#endif

  typedef ACM_Container<ACM_Class, false> Container;
  Container &bases = jpl.get_bases();
  typedef ACM_Container<ACM_BaseIntro, true> Container2;
  Container2 &base_intros = plan->get_base_intros();
#ifdef FRONTEND_CLANG
  clang::SourceManager &sm = _project.get_compiler_instance()->getSourceManager();
#endif
  for (Container::iterator i = bases.begin (); i != bases.end (); ++i) {
    ACM_Class *cls = *i;
    // find the base class info in Puma's semantic data structure
#ifdef FRONTEND_PUMA
    for (unsigned b = 0; b < ci->BaseClasses(); b++) {
      if (ci->BaseClass(b)->Class() != TI_Class::of (*cls)->class_info())
        continue;
      Unit *u = (Unit*)ci->BaseClass(b)->Tree()->token()->belonging_to();
#else
    for (clang::CXXRecordDecl::base_class_iterator bi = ci->bases_begin(),
                                                   be = ci->bases_end();
         bi != be; ++bi) {
      if (bi->getType()->getAsCXXRecordDecl() != TI_Class::of (*cls)->decl())
        continue;
      clang::FileID fid = sm.getFileID(bi->getType()->getAsCXXRecordDecl()->getLocation());
      const llvm::MemoryBuffer *u = sm.getBuffer(fid);
#endif
      if (IntroductionUnit::cast(u)) {
        ACM_Introduction *intro = IntroductionUnit::cast(u)->intro();
        for (Container2::iterator bi = base_intros.begin ();
            bi != base_intros.end(); ++bi) {
          if ((*bi)->get_intro () == intro) {
            (*bi)->get_bases().insert(cls);
          }
        }
      }
    }
  }
}
