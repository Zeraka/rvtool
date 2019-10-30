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

#ifndef __Weaver_h__
#define __Weaver_h__

// C++ includes
#include <sstream>
using std::stringstream;
#include <set>
using std::set;
#include <list>
using std::list;

// PUMA includes
#include "Puma/VerboseMgr.h"

// AspectC++ includes
#include "ACConfig.h"
#include "ACErrorStream.h"
#include "LineDirectiveMgr.h"
#include "ACFileID.h"
#include "AspectIncludeCluster.h"

class Transformer;

namespace Puma {
  class VerboseMgr;
} // namespace Puma

class Weaver {
  ACProject &_project;
  ACConfig &_conf;
  LineDirectiveMgr _line_mgr;
  list<AspectIncludeCluster> _aspect_include_clusters;

  ACFileID translate (Puma::VerboseMgr &vm, const char *file, Transformer &transformer);
  void aspect_includes (ostream &includes);
  void aspect_include (ostream &includes, const char *name);
  void insert_aspect_includes (Puma::VerboseMgr &vm, ACFileID fid, bool header,
      const string &aspect_includes, const string &aspect_fwd_decls);
  void insert_aspect_includes (Puma::VerboseMgr &vm, set<ACFileID> &cc_units,
    set<ACFileID> &h_units, const string &aspect_includes, const string &aspect_fwd_decls);
  void update_line_directives (set<ACFileID> &cc_units, set<ACFileID> &h_units);
  void update_line_directives (ACFileID fid, const char *filename);
  ACErrorStream &err () const { return static_cast<ACErrorStream&>(_project.err ()); }
  void match_expr_in_repo (Puma::VerboseMgr &vm);
  string gen_aspect_includes (ACFileID target_unit);

public:
  
  // Initialize the weaver
  Weaver (ACProject& project, ACConfig &conf);

  // Do the AspectC++ -> C++ tranformation on the project
  void weave ();
};

#endif // __Weaver_h__
