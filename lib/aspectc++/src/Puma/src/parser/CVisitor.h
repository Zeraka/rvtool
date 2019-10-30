// This file is part of PUMA.
// Copyright (C) 1999-2003  The PUMA developer team.
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

#ifndef __CVisitor_h__
#define __CVisitor_h__

/** \file
 *  CTree based tree visitor. */

namespace Puma {


class CTree;

/** \class CVisitor CVisitor.h Puma/CVisitor.h
 *  Tree visitor implementation for CTree based syntax trees. To be derived 
 *  for visiting concrete syntax trees.
 *  
 *  This class performs depth-first tree-traversal based on CTree tree 
 *  structures. The traversal is started by calling CVisitor::visit()
 *  with the root node of the tree to traverse as its argument. 
 *  For every node of the tree CVisitor::pre_visit() is called 
 *  before its child nodes are visited, and CVisitor::post_visit()
 *  is called after its child nodes are visited. To perform actions 
 *  on the visited nodes, CVisitor::pre_visit() and CVisitor::post_visit() 
 *  have to be overloaded. */
class CVisitor {
  bool _aborted;
  bool _pruned;

  void visit_sons (CTree *node);

public:
  /** Constructor. */
  CVisitor () : _aborted (false), _pruned (false) {}
  /** Destructor. */
  virtual ~CVisitor () {}

  /** Set the aborted state.
   *  \param v True for aborted, false for not aborted. */
  void abort (bool v = true) { _aborted = v; }
  /** Set the pruned state (don't visit the sub-tree).
   *  \param v True for pruned, false for not pruned. */
  void prune (bool v = true) { _pruned = v; }
  /** Check if the node visiting is aborted. */
  bool aborted () const { return _aborted; }
  /** Check if the visiting the sub-tree is aborted. */
  bool pruned () const { return _pruned; }
  
  /** Visit the given syntax tree node.
   *  \param node The node to visit. */
  void visit (CTree *node);

  /** Apply actions before the given node is visited.
   *  To be implemented by derived visitors.
   *  \param node The node to visit. */
  virtual void pre_visit (CTree *node);
  /** Apply actions after the given node is visited.
   *  To be implemented by derived visitors.
   *  \param node The node to visit. */
  virtual void post_visit (CTree *node);
};


} // namespace Puma

#endif /* __CVisitor_h__ */
