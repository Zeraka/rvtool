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

#ifndef __Transformer_h__
#define __Transformer_h__

#include "CodeWeaver.h"
#ifdef FRONTEND_PUMA
#include "Puma/CCParser.h"
#include "Puma/CCSemVisitor.h"
#else
#include "clang/Basic/SourceManager.h"
#endif
#include "ACErrorStream.h"
#include "ACToken.h"
#include "AspectIncludeCluster.h"


#include <list>
using std::list;

namespace Puma {
  class VerboseMgr;
  class CTranslationUnit;
} // namespace Puma

class PointCut;
class Repository;
class Plan;
class ModelBuilder;
class ACConfig;
class ACProject;
class IncludeGraph;

class Transformer
 {
      Puma::VerboseMgr &_vm;
      ACErrorStream &_err;
      ACProject &_project;
      Repository &_repo;
      CodeWeaver _code_weaver;
#ifdef FRONTEND_PUMA
      Puma::CCParser _parser;
      Puma::CCSemVisitor _sem_visitor;
#else
      clang::SourceManager *_phase1_sm;
#endif
      ACConfig &_conf;
      list<AspectIncludeCluster> _aspect_include_clusters;
      string _aspect_includes;
      string _aspect_fwd_decls;
      set<ACFileID> _touched_files;
      set<ACFileID> _virtual_files;
      
      bool phase1 (Puma::Unit *&unit, const string &tunit_name,
          ModelBuilder &, IncludeGraph&);
      bool phase2 (Puma::Unit *unit, const string &tunit_name,
          ModelBuilder &, IncludeGraph&);
      void include_aspect (const char *name);
      void determine_aspect_includes (const IncludeGraph &ig);
      void determine_aspect_fwd_decls (ModelBuilder &);
      void aspect_include_cluster (const char *ah_file, const IncludeGraph &ig);
      void determine_aspect_cluster (ACFileID ah_unit,
        const IncludeGraph &ig, set<ACFileID> &cluster);
      void prepare_dynamic_weaving (ModelBuilder &jpm);
      void update_intros_in_repo (ModelBuilder &);
      void cleanup (ModelBuilder &jpm);
      void insert_bypass_class (ModelBuilder &);
      void join_points (Puma::CTranslationUnit &, ModelBuilder &, Plan &);

      // add references to the introduced elements to the plan
      void link_members (ACM_Class &jpl);
      // add references to the introduced base classes to the plan
      void link_bases (ACM_Class &jpl);
#ifdef FRONTEND_CLANG
      void reinitializeSourceManager(clang::CompilerInstance *ci,
          CodeWeaver &cwb, const char *name);
#else
      void update_touched_files ();
#endif

   public:

      Transformer (Puma::VerboseMgr &vm, ACErrorStream &e, ACProject &p, Repository &r,
                   ACConfig &c, LineDirectiveMgr &ldm):
        _vm (vm), _err (e), _project (p), _repo (r),
        _code_weaver (p, ldm, c),
#ifdef FRONTEND_PUMA
        _sem_visitor (e),
#endif
        _conf (c) {}
      ACFileID work (const string &tunit_name);
      list<AspectIncludeCluster> aspect_include_clusters () const { return _aspect_include_clusters; }
      const string &aspect_includes () const { return _aspect_includes; }
      const string &aspect_fwd_decls () const { return _aspect_fwd_decls; }
      const set<ACFileID> &touched_files () const { return _touched_files; }
 };

#endif // __Transformer_h__
