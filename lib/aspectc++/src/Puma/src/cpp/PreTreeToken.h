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

#ifndef __syntax_tree_token__
#define __syntax_tree_token__

// Class PreTreeToken makes it possible to let tokens be
// a part of a syntax tree.

#include "Puma/PreTree.h"
#include "Puma/Token.h"

/** \file 
 *  Preprocessor syntax tree leaf class (representing a token). */

namespace Puma {


/** \class PreTreeToken PreTreeToken.h Puma/PreTreeToken.h
 *  Preprocessor syntax tree leaf class representing a token. */
class PreTreeToken : public PreTree {
  Token *_token;
        
public:
  /** Constructor.
   *  \param token The preprocessor token. */
  PreTreeToken (Token *token) : _token (token) {}

  /** Part of the tree visitor pattern. Calls the node
   *  visiting function for PreTreeToken nodes. */
  void accept (PreVisitor &v)
    { v.visitPreTreeToken (this); }            
        
  /** Get the first token of the syntactic construct 
   *  represented by the sub-tree. */
  Token *startToken () const { return _token; }
  /** Get the last token of the syntactic construct
   *  represented by the sub-tree. */
  Token *endToken () const   { return _token; }
        
  /** Get the token represented by this node. */
  Token *token () const      { return _token; }
};


} // namespace Puma

#endif /*__syntax_tree_token__ */
