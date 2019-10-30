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

#ifndef __ACProject_h__
#define __ACProject_h__

// CProject specialization for ac++ project files

#include <string>
using std::string;
#include <set>
using std::set;


#ifdef FRONTEND_CLANG
class ACConfig;
namespace clang {
  class CompilerInstance;
  class Rewriter;
}
#include "Puma/PathManager.h"
#include "Puma/Config.h"
#else
#include "Puma/CProject.h"
#endif

#include <libxml/tree.h>

#include "ACErrorStream.h"
#include "ACFileID.h"

class ACProject :
#ifdef FRONTEND_CLANG
    public Puma::PathManager
#else
    public Puma::CProject
#endif
 {
#ifdef FRONTEND_CLANG
  clang::CompilerInstance *_ci;
  Puma::Config _puma_config;
  set<ACFileID> _closed_files;
#endif
  set<ACFileID> _virtual_files;

  // Add all files from the XML project description to the project
  void addFiles (xmlDocPtr doc, xmlNodePtr node, const string& xpath);

public:
  ACProject (ACErrorStream &, int &argc, char **&argv);
#ifdef FRONTEND_CLANG
  ~ACProject ();
  Puma::Config &config () { return _puma_config; }
#endif

  // load the project file
  bool loadProject (string acprj);

  // Add a new *virtual* file to the project.
  ACFileID addVirtualFile (const string &filename, const string &contents = "");

  // Remove a virtual file
  void removeVirtualFile (ACFileID fid);

  bool isVirtualFile (ACFileID fid) {
    return _virtual_files.find(fid) != _virtual_files.end();
  }

#ifdef FRONTEND_CLANG
  // Create an instance of the clang compiler frontend.
  void create_compiler_instance (ACConfig &conf);
  clang::CompilerInstance *get_compiler_instance () const { return _ci; }
#endif

#ifdef FRONTEND_CLANG
  ACErrorStream &err () const { return static_cast<ACErrorStream&>(Puma::PathManager::err ()); }
#else
  ACErrorStream &err () const { return static_cast<ACErrorStream&>(Puma::CProject::err ()); }
#endif

  // Add a forced include to the front end
  void add_forced_include (const string &file);

  //Remove a forced include from the front end
  void remove_forced_include (const string &file);

  // Remove all forced includes
  void remove_forced_includes ();

  // Get the list of forced includes from the front end as a vector of filenames
  void get_forced_includes (std::vector<std::string> &forced_includes);

  // Generate a path for including 'to' from within the file 'from'
  Puma::Filename getRelInclString (Puma::Filename from, Puma::Filename to);

  #ifdef FRONTEND_CLANG
  bool isNewer (const char *) const;
  void save () const;
  bool is_protected (clang::FileEntry *file_entry);
  bool make_dir_hierarchy (Puma::Filename path) const;
  bool make_dir (const char *directory) const;
  void save (const clang::FileEntry *file_entry, bool is_modified) const;
  void close (ACFileID fid) { _closed_files.insert (fid); }
#endif
};

#endif /* __ACProject_h__ */
