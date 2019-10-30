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

#include "Puma/CTranslationUnit.h"
#include "Puma/CTreeMatcher.h"
#include "Puma/CMatchChecker.h"
//#include "Puma/CRankTable.h"
#include "Puma/RegComp.h"
#include "Puma/CTree.h"
#include "Puma/StrCol.h"
#include <stdlib.h>
#include <string.h>

namespace Puma {


void CTreeMatcher::init (CTree *matchTree, unsigned int max) {
  _max_mobjs = max;
  _num_mobjs = 0;
    
  // Set the current match tree.
  _matchTree = matchTree;
}

void CTreeMatcher::pre_visit (CTree *node) {
  // Do nothing on visit if no match tree is given.
  if (! _matchTree || ! node) 
    return;

  // Check whether this node is really the expected node and not  
  // maybe someone that only looks like it.
  if (! correct (node))
    return;

  // If the match tree can't be a sub tree of the current C++ syntax
  // tree node, trying to match this node and its sons is skipped. 
//  if (CRankTable::Rank (node->astName ()) < 
//      CRankTable::Rank (_matchTree->astName ()))
//    return;

  // Create CMatch object and add the current node to it.
  CMatch *matchObj = new CMatch;
  matchObj->add (node, "root");
    
  // If the current sub-tree is matching the match tree, call 
  // action() with the current node and the nodes that correspond 
  // with the any-nodes of the match tree as arguments. 
  if (match (node, _matchTree, matchObj)) {
    _num_mobjs++;
    action (matchObj);
    if (_max_mobjs <= _num_mobjs && _max_mobjs != 0)
      abort (); // Abort traversing the syntax tree!
  } 
  // Not matching. Delete the match object.
  else
    delete matchObj;
}

// Return true if the current sub-tree is matching the match tree.
bool CTreeMatcher::match (CTree *currTree, CTree *matchTree, 
                          CMatch* matchObj) const {
  if (! currTree || ! matchTree || ! matchObj)
    return false;

  // Don't compare against CT_ImplicitCast nodes.
  while (currTree->NodeName () == CT_ImplicitCast::NodeId ())
    currTree = currTree->Son (0);

  // The current node of the match tree is an Any node. 
  if (matchTree->NodeName () == CT_Any::NodeId () || 
      matchTree->NodeName () == CT_AnyList::NodeId ()) {
    CT_AnyExtension *ext = ((CT_Any*)matchTree)->Extension ();
    CTree *condition = ext ? ext->Condition () : (CTree*)0;
    const char *name = ext ? ext->Name () : (const char*)0;
    Array<CTree *> *old = &matchObj->get (name);

    // If there already is an Any node with that name, it has
    // to be checked whether the two trees are the same.
    if (old->length ())
      if (! match (old->get (0), currTree, matchObj))
        return false;

    // If there is no extra condition for matching with this 
    // any-node, the current tree node always is matching.
    if (condition) {
      int mode = 0;
            
      // Tell the match-parser which condition keywords are allowed.
      if (matchTree->NodeName () == CT_Any::NodeId ())
        mode = ANY;
      else if (matchTree->NodeName () == CT_AnyList::NodeId ())
        mode = ANY_LIST;

      if (! evalMatchCondition (currTree, condition, matchObj, mode))
        return false;
    }

    // Remember anys with a name only.
    if (name)
      matchObj->add (currTree, name);
            
    return true;
  } 

  // If the current node of the match tree isn't an any-node 
  // the two current nodes must be of the same type and must 
  // have the same number of sons.
  if (currTree->NodeName () != matchTree->NodeName () ||   
      currTree->Sons () != matchTree->Sons ())   
    return false;
        
  // The current nodes are tokens. The text of the tokens 
  // must be the same.
  if (currTree->NodeName () == CT_Token::NodeId ()) {
    if (! currTree->token () || ! matchTree->token ())
      return false;
    if (! currTree->token ()->text () || ! matchTree->token ()->text ())
      return false;
    if (strcmp (currTree->token ()->text (),
                matchTree->token ()->text ()) != 0)
      return false;
  } 
    
  // Now call match() for every son of the current node.
  for (int i = 0; i < currTree->Sons (); i++)
    if (! match (currTree->Son (i), matchTree->Son (i), matchObj))
      return false;

  // The current C++ sub-tree is matching the current sub-tree 
  // of the match tree.
  return true;
}

// Match any node-list. 
int CTreeMatcher::matchList (int mode, match_state &mstate, 
                             CTree *mt, CTree *f, CTree *l) const {
  bool matched = false;
  CT_List *list = (CT_List*)mstate.tree;
  int elements = list->Entries ();
  int sign_f = 0, sign_l = 0;

  if (f) {
    sign_f = (f->token ()->text ()[0] == '-') ? -1 : 1;
    f = f->Son (f->Sons () - 1);
  }
  if (l) {
    sign_l = (l->token ()->text ()[0] == '-') ? -1 : 1;
    l = l->Son (l->Sons () - 1);
  }

  // Get the arguments.
  int first = (f) ? (sign_f * (int) strtol (f->token ()->text (), NULL, 10)) : 1;
  int last  = (l) ? (sign_l * (int) strtol (l->token ()->text (), NULL, 10)) : elements;
    
  // Compute the real element number.
  if (first < 0) first = elements + first + 1;
  if (last  < 0) last  = elements + last  + 1;

  // Wrong arguments?
  if (first <= 0 || last <= 0) 
    return 0;
    
  // The current list has too few elements.
  if (first > elements || last > elements)
    return 0;
        
  // If the two arguments have the same value, the user wanted
  // to match lists that have exact the given number of elements.
  if (first == last && f && l) {
    if (first != elements)
      return 0;
            
    first = 1;
    last  = elements;
  }

  // Match a single element.
  if (f && ! l)
    last = first;
    
  // Match the list elements.
  for (int i = first; i <= last; i++)
    if (matchTree (mode, mstate, list->Entry (i - 1), mt))
      matched = true;
            
  // Return the success value.
  return matched ? 1 : 0;
}

// Match any node. 
int CTreeMatcher::matchTree (int mode, match_state &mstate, CTree *tree,
                             CTree *mt) const {
  int matched = 0;
  CMatch tmp;

  if (mode == MATCH_ALL || 
      mode == MATCH_FIRST || 
      mode == MATCH_LAST ||
      mode == MATCH_PRUNE || 
      mode == MATCH_LEAF) {
    CMatchChecker check (mode, mt, &tmp);
    check.visit (tree);
    matched = (check.matchFound ()) ? 1 : 0;
  } 
  else if (mode == MATCH_EXACT) 
    matched = (match (tree, mt, &tmp)) ? 1 : 0;

  // Remember all the matches.
  if (matched) 
    mstate.matchObj->combine (&tmp);
    
  return matched;
}

// Match a node by its name. Regular expressions are allowed.
int CTreeMatcher::matchNodeName (CTree *tree, CTree *mt) const {
  if (! mt || ! tree) 
    return 0;
  if (mt->NodeName () != CT_String::NodeId ())
    return 0;

  char *string = StrCol::dup (mt->token ()->text () + 1);
  string[strlen (string) - 1] = '\0';
    
  RegComp compare (string);
  bool matching = compare.match ((const char*)tree->NodeName ());
  delete[] string;
  return matching ? 1 : 0;
}

// Start the match process.
void CTreeMatcher::match (CTranslationUnit &tUnit, 
                          CTranslationUnit &matchUnit, CTree *syntaxtree) {
  // Do the manipulations only if there really is a syntax tree.
  CTree *matchtree = matchUnit.tree ();
  if (! syntaxtree) 
    syntaxtree = tUnit.tree ();
  if (syntaxtree) {
    init (matchtree);
//    tUnit.cppTree ()->context ().push ();
    visit (syntaxtree);
//    tUnit.cppTree ()->context ().pop ();
  }
}


} // namespace Puma
