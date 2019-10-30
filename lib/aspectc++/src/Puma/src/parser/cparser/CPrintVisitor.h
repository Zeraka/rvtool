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

#ifndef __CPrintVisitor_h__
#define __CPrintVisitor_h__

/** \file
 *  CTree based syntax tree printing. */

#include "Puma/CVisitor.h"
#include "Puma/Config.h"

#include <iostream>

namespace Puma {


class CObjectInfo;


/** \class CPrintVisitor CPrintVisitor.h Puma/CPrintVisitor.h
 *  Tree visitor for printing the C/C++ syntax tree. Prints the
 *  indented tree structure supplemented with type information, 
 *  calculated constant values, implicit casts, and resolved 
 *  function calls. */
class CPrintVisitor : private CVisitor {
  std::ostream *out;
  int indent;
  bool print_loc;
  int loc_len;
  char* linecol;

public:
  /** Constructor. */
  CPrintVisitor ();
  /** Destructor. */
  ~CPrintVisitor ();

  /** Configure the visitor.
   *  \param config The configuration options. */
  void configure (Config &config);

  /** Start printing the given tree on the given output stream.
   *  \param tree The syntax tree to print.
   *  \param os The output stream. */
  void print (CTree *tree, std::ostream &os);

  /** Set printing the token location.
   *  \param enable Enable/disable printing the location.
   *  \param len The length of the location prefix. */
  void setPrintLoc (bool enable, int len = 20);

private:
  void pre_visit (CTree *node);
  void post_visit (CTree *node);

  void print_node (CTree *node) const;
  void print_qual_name (CObjectInfo *) const;
};


} // namespace Puma

#endif /* __CPrintVisitor_h__ */
