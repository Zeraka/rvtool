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

#ifndef __CTreeMatcher__
#define __CTreeMatcher__

// Base class for syntax tree match visitors.

#include "Puma/CTranslationUnit.h"
#include "Puma/CVisitor.h"
#include "Puma/CMatch.h"
#include "Puma/Array.h"

namespace Puma {


class CTree;
class CT_AnyExtension;

class CTreeMatcher : public CVisitor {
  CTree *_matchTree;  
  int _num_mobjs;
  int _max_mobjs;

public:
  enum {
    ANY = 1,                // Shows the match-parser which keywords
    ANY_LIST,                // are allowed.
    MATCH_ALL,                // Possible match modes.
    MATCH_EXACT,
    MATCH_FIRST,
    MATCH_LAST,
    MATCH_LEAF,
    MATCH_PRUNE
  };        

  // Match-condition parser state.
  struct match_state {
    int           value;        // Value of the condition (-1 on errors).
    CTree        *tree;                // The tree to match with.
    CMatch       *matchObj;           // To fill with the anys of the condition.
    int           mode;                // Is ANY or ANY_LIST.
    int           depth;        // Selection depth.
    Array<int>   &selected;         // Selection state.
    CTreeMatcher *matcher;        // To call match() on it.
  };
        
public:
  CTreeMatcher (unsigned int max = 0) : 
   _num_mobjs (0), _max_mobjs (max) {}
  CTreeMatcher (CTree *matchTree, unsigned int max = 0) 
   { init (matchTree, max); }
  virtual ~CTreeMatcher () {}
        
  // Initialize the matcher.
  void init (CTree *matchTree, unsigned int max = 0);
    
  // Check whether the two trees are matching.
  bool match (CTree *, CTree *, CMatch *) const;

  // Should be specialized by derived classes.
  virtual void action (CMatch *match) 
   { if (match) delete match; }
            
  // Make a first check whether the current visited node is the
  // expected root node of the tree to match.
  virtual bool correct (CTree *node)
   { return true; }

  // Start the match process.
  virtual void match (CTranslationUnit &, CTranslationUnit &, 
                      CTree * = (CTree*)0);

  // Match any node, a list of nodes, or a node by its name. 
  // !!! TO BE USED BY THE MATCH PARSER ONLY !!!
  int matchTree (int, match_state &, CTree *, CTree *) const;
  int matchList (int, match_state &, CTree *, CTree * = (CTree*)0, 
                 CTree * = (CTree*)0) const;
  int matchNodeName (CTree *, CTree *) const;

private:
  // Traverses the match-condition tree.
  void travConditionTree (CTree *, void *, match_state *) const;

  // Evaluates the given match-condition.
  bool evalMatchCondition (CTree *, CTree *, CMatch *, int) const;
        
  // Is called on visit before visiting the sons.
  // Starts the matching action.
  void pre_visit (CTree *node);
};


} // namespace Puma

#endif /* __CTreeMatcher__ */
