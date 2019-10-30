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

#ifndef __include_graph_h__
#define __include_graph_h__

// Visitor, which expands all project-local includes 

#ifdef FRONTEND_PUMA
#include "Puma/PreVisitor.h"
namespace Puma {
  class CTranslationUnit;
  class Unit;
  class CProject;
}
#else
#include "clang/Lex/PPCallbacks.h"
#endif

#include <map>
using std::map;
#include <set>
using std::set;
#include <iostream>
using std::ostream;
using std::endl;

#include "ACFileID.h"
#include "ACProject.h"

class IncludeGraph
#ifdef FRONTEND_PUMA
: public Puma::PreVisitor
#endif
{

  struct Node {
    ACFileID _unit;
    mutable bool _visited; // for cycle detection
    set<Node*> _includes;
    Node (ACFileID u) : _unit (u), _visited (false) {}
    void dump () const;
  };
  
  // associates a node object to each unit
  typedef map<ACFileID, Node> Map;
  Map _nodes;
  
  // the project to which all this belongs
  ACProject &_project;
  
#ifdef FRONTEND_PUMA
  // Go through the nodes of the syntax tree.
  void iterateNodes (Puma::PreTree*);

  // Visiting the parts of the preprocessor syntax tree.
  void visitPreIncludeDirective_Pre (Puma::PreIncludeDirective*);
#endif

  // find/create an entry in '_nodes'
  Node &find (ACFileID);
  
  // Checks whether there is a path from node 'a' to 'b' in the include graph
  bool includes (const Node &a, const Node &b) const;

  // collect all units included by some node  
  void included_files (const Node &node, set<ACFileID> &units,
    bool only_project = true) const;
  
  // Reset the 'visited' flag of all nodes in the DAG
  void reset_visited () const;
  
public:
  IncludeGraph (ACProject &p) : _project (p) {}
#ifdef FRONTEND_PUMA
  IncludeGraph (ACProject &p, Puma::CTranslationUnit &tunit) : _project (p) {
    init (tunit);
  }
  virtual ~IncludeGraph () {}

  // Fills the include graph
  void init (Puma::CTranslationUnit &tunit);
  void init (Puma::PreTree *tree) { iterateNodes (tree); } // for new phase 1
#else
  // Callback object to add includes to the graph during a preprocessor run.
  class IncludeGraphCallback : public clang::PPCallbacks {
    IncludeGraph &_ig;

    // Callback from clang when it sees an inclusion directive.
    virtual void InclusionDirective (clang::SourceLocation HashLoc,
                                     const clang::Token &IncludeTok,
                                     llvm::StringRef FileName,
                                     bool IsAngled,
                                     clang::CharSourceRange FilenameRange,
                                     const clang::FileEntry *File,
                                     llvm::StringRef SearchPath,
                                     llvm::StringRef RelativePath,
                                     const clang::Module *Imported);

  public:
    IncludeGraphCallback (IncludeGraph &ig) : _ig(ig) {}
  };
#endif

  // Checks whether on unit 'a' directly or indirecly includes another unit 'b'
  bool includes (ACFileID a, ACFileID b) const;
  
  // Get all files the are directly or indirectly included
  bool included_files (ACFileID unit, set<ACFileID> &units,
    bool only_project = true) const;

  // Add an edge to the include graph from 'a' to 'b'
  void add_edge (ACFileID a, ACFileID b);
  
  // print all nodes
  void dump () const;
};  	    

#endif // __include_graph_h__
