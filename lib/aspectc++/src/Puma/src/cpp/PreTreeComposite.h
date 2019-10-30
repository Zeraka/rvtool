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

#ifndef __syntax_tree_composite__
#define __syntax_tree_composite__

#include "Puma/PreTree.h"
#include "Puma/Array.h"
#include "Puma/PreVisitor.h"

/** \file 
 *  Base preprocessor syntax tree composite class. */

namespace Puma {


/** \class PreTreeComposite PreTreeComposite.h Puma/PreTreeComposite.h
 *  Base class for all C preprocessor syntax tree composite nodes. */
class PreTreeComposite : public PreTree {
private:
  Array<PreTree*> *son_array;
  Array<PreTree*> *daughter_array;
  int              max_sons;
  int              max_daughters;

protected:
  /** Constructor.
   *  \param s Number of sons (syntactic child nodes).
   *  \param d Number of daughters (semantic child nodes). */
  PreTreeComposite (int s, int d);
    
public:
  /** Dynamic number of sons and daughters. */
  static const int DYNAMIC = -1;

  /** Destructor. */
  virtual ~PreTreeComposite ();
        
  /** Part of the tree visitor pattern. Calls the node
   *  visiting function suitable for the actual node type. */
  virtual void accept (PreVisitor &) = 0;

  /** Add a son (syntactic child node). 
   *  \param s The son to add. */
  virtual void add_son (PreTree *s);
  /** Replace the n-th son.
   *  \param n The index of the son to replace.
   *  \param new_s The new son. */
  virtual void replace_son (int n, PreTree *new_s);
  /** Get the n-th son. 
   *  \param n The index of the son. */
  virtual PreTree *son (int n) const;
  /** Get the number of sons. */
  virtual int sons () const;

  /** Add a daughter (semantic child node).
   *  \param d The daughter to add. */
  virtual void add_daughter (PreTree *d);
  /** Replace the n-th daughter.
   *  \param n The index of the daughter.
   *  \param new_d The new daughter. */
  virtual void replace_daughter (int n, PreTree *new_d);
  /** Get the n-th daughter.
   *  \param n The index of the daughter. */
  virtual PreTree *daughter (int n) const;
  /** Get the number of daughters. */
  virtual int daughters () const;
        
  /** Get the first token of the syntactic construct 
   *  represented by the sub-tree. */
  virtual Token *startToken () const;
  /** Get the last token of the syntactic construct
   *  represented by the sub-tree. */
  virtual Token *endToken () const;
};


} // namespace Puma

#endif /* __syntax_tree_composite__ */
