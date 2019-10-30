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

#include "IncludeGraph.h"
#include "ACFileID.h"

#ifdef FRONTEND_PUMA
#include "Puma/PreTreeNodes.h"
#include "Puma/PreSonIterator.h"
#include "Puma/CTranslationUnit.h"
using namespace Puma;
#else
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
using namespace clang;
#endif

#ifdef FRONTEND_PUMA
void IncludeGraph::init (CTranslationUnit &tunit) {

  // search for #includes
  iterateNodes (tunit.cpp_tree ());
}

// Go through the nodes.
void IncludeGraph::iterateNodes (PreTree* node) {
  PreSonIterator i (node); // the order is important!

  for (i.first (); ! i.isDone (); i.next ())
    i.currentItem ()->accept (*this);
}

// handle include directive node
void IncludeGraph::visitPreIncludeDirective_Pre (PreIncludeDirective* node) {

  // this_unit is the unit where the include directive is located
  Token *this_token = node->startToken ();
  Unit *this_unit   = (Unit*)this_token->belonging_to ();

//  cout << "in " << this_unit->name () << " " << this_token->location () << " "
//       << _project.isBelow (this_unit) << endl;

  // include if expanded by preprocessor
  if (node->daughters () == 1) {

    Unit *included_unit = ((PreInclSemNode*)node->daughter (0))->unit ();

//    cout << "  included: " << included_unit->name () << " " <<
//      _project.isBelow (included_unit) << endl;

    add_edge (this_unit, included_unit);
  }
}

#else

void IncludeGraph::IncludeGraphCallback::InclusionDirective(SourceLocation HashLoc,
                                      const clang::Token &IncludeTok,
                                      StringRef FileName,
                                      bool IsAngled,
                                      CharSourceRange FilenameRange,
                                      const FileEntry *File,
                                      StringRef SearchPath,
                                      StringRef RelativePath,
                                      const Module *Imported) {
  if (!File)
    return;

  Preprocessor &PP = _ig._project.get_compiler_instance()->getPreprocessor();
  SourceManager &SM = PP.getSourceManager();
  const FileEntry *FromFile =
    SM.getFileEntryForID(SM.getFileID(SM.getExpansionLoc(HashLoc)));
  if (FromFile == 0)
    return;

  _ig.add_edge (FromFile, File);
}
#endif

// Add an edge to the include graph from 'a' to 'b'
void IncludeGraph::add_edge (ACFileID a, ACFileID b) {
  Node &this_node = find (a);
  Node &included_node = find (b);
  this_node._includes.insert (&included_node);
}

// find/create an entry in '_nodes'
IncludeGraph::Node &IncludeGraph::find (ACFileID unit) {
  std::pair<Map::iterator,bool> p =
    _nodes.insert (Map::value_type (unit, Node (unit)));
  return p.first->second;
}

// Checks whether on unit 'a' directly or indirecly includes another unit 'b'
bool IncludeGraph::includes (ACFileID a, ACFileID b) const {
  Map::const_iterator a_iter = _nodes.find (a);
  Map::const_iterator b_iter = _nodes.find (b);
  if (a_iter == _nodes.end () || b_iter == _nodes.end ())
    return false;
  bool result = includes (a_iter->second, b_iter->second);
  
  // reset 'visited' flags
  reset_visited ();

  return result;
}

void IncludeGraph::reset_visited () const {
  // reset 'visited' flags
  Map::const_iterator iter = _nodes.begin ();
  while (iter != _nodes.end ()) {
    const IncludeGraph::Node &node = iter->second;
    node._visited = false;
    ++iter;
  }
}

// Checks whether there is a path from node 'a' to 'b' in the include graph
bool IncludeGraph::includes (const Node &a, const Node &b) const {
  // cycle detection
  if (a._visited)
    return false;
    
  set<Node*>::iterator iter = a._includes.begin ();
  a._visited = true;
  bool found = false;
  while (iter != a._includes.end ()) {
    if (*iter == &b || includes (**iter, b)) {
      found = true;
      break;
    }
    ++iter;
  }
  return found;  
}

void IncludeGraph::included_files (const Node &node,
  set<ACFileID> &units, bool only_project) const {

  // cycle detection
  if (node._visited)
    return;
    
  set<Node*>::iterator iter = node._includes.begin ();
  node._visited = true;
  while (iter != node._includes.end ()) {
    ACFileID unit = (*iter)->_unit;
    if (!only_project || _project.isBelow (
//#ifdef FRONTEND_PUMA
//        (Puma::Unit*)unit  // TODO: why isn't the argument const?
//#else
        unit.name ().c_str ()       // TODO: using the name might be inefficient
//#endif
        ))
      units.insert (unit);
    included_files (**iter, units);
    ++iter;
  }
}
 
// Get all files the are directly or indirectly included
bool IncludeGraph::included_files (ACFileID unit,
  set<ACFileID> &units, bool only_project) const {
  Map::const_iterator iter = _nodes.find (unit);
  if (iter == _nodes.end ())
    return false;
  
  // collect the included file for this node
  included_files (iter->second, units, only_project);
  
  // reset 'visited' flags
  reset_visited ();

  return true;
}

void IncludeGraph::Node::dump () const {
//#ifdef FRONTEND_PUMA
//  cout << "unit " /*<< _unit.get_unit ()*/ << " '" << _unit->name () << "' includes:" << endl;
//  set<Node*>::iterator iter = _includes.begin ();
//  while (iter != _includes.end ()) {
//    cout << "  " << (*iter)->_unit->name () << endl;
//    ++iter;
//  }
//#else
  std::cout << "unit " /*<< _unit.get_unit ()*/ << " '" << _unit.name () << "' includes:" << std::endl;
  std::set<Node*>::iterator iter = _includes.begin ();
  while (iter != _includes.end ()) {
    std::cout << "  " << (*iter)->_unit.name () << std::endl;
    ++iter;
  }
//#endif
}

void IncludeGraph::dump () const {
  Map::const_iterator iter = _nodes.begin ();
  while (iter != _nodes.end ()) {
    const IncludeGraph::Node &node = iter->second;
    node.dump ();
    ++iter;
  }
}
