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

#include "Puma/PreTree.h"
#include <iostream>

namespace Puma {


void PreTree::add_son (PreTree*)
 {
   std::cerr << "internal error: Attempt to add son to none-composite syntax tree node\n";
 }

void PreTree::replace_son (int, PreTree*)
 {
   std::cerr << "internal error: Attempt to replace son at none-composite syntax tree node\n";
 }

PreTree* PreTree::son (int) const
 {
   std::cerr << "internal error: Attempt to get a son to none-composite syntax tree node\n";
   return (PreTree*) 0;
 }

int PreTree::sons () const
 { 
   return 0;
 }

void PreTree::add_daughter (PreTree*)
 {
   std::cerr << "internal error: Attempt to add daughter to none-composite syntax tree node\n";
 }

void PreTree::replace_daughter (int, PreTree*)
 {
   std::cerr << "internal error: Attempt to replace daughter at none-composite syntax tree node\n";
 }

PreTree* PreTree::daughter (int) const
 {
   std::cerr << "internal error: Attempt to get a daughter to none-composite syntax tree node\n";
   return (PreTree*) 0;
 }

int PreTree::daughters () const
 { 
   return 0;
 }

bool PreTree::isLeaf () const
 {
   return sons () == 0 && daughters () == 0;
 }

Token* PreTree::startToken () const
 {
   return (Token*) 0;
 };

Token* PreTree::endToken () const
 {
   return (Token*) 0;
 };


} // namespace Puma
