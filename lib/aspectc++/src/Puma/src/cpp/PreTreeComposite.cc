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

#include "Puma/PreTreeComposite.h"
#include <iostream>

namespace Puma {


PreTreeComposite::PreTreeComposite (int s, int d)
 {
   max_sons = s;
   if (max_sons > 0)
      son_array = new Array <PreTree*> (max_sons);
   else if (max_sons == DYNAMIC)
      son_array = new Array <PreTree*> (3, 3);
   else 
      son_array = (Array <PreTree*> *)0;

   max_daughters = d;
   if (max_daughters > 0)
      daughter_array = new Array <PreTree*> (max_daughters);
   else if (max_daughters == DYNAMIC)
      daughter_array = new Array <PreTree*> (3, 3);
   else 
      daughter_array = (Array <PreTree*> *)0;
 }

PreTreeComposite::~PreTreeComposite ()
 {
   if (son_array) delete son_array;
   if (daughter_array) delete daughter_array;
 }

void PreTreeComposite::add_son (PreTree* s)
 {
   if (max_sons == 0)
      std::cerr << "internal error: Attempt to add a son to a node that can't have sons\n";
   else if (max_sons == DYNAMIC || son_array->length () < max_sons)
      son_array->append (s);
   else
      std::cerr << "internal error: Attempt to add more than " << max_sons << " sons\n";
 }

void PreTreeComposite::replace_son (int index, PreTree* s)
 {
   if (max_sons == 0)
      std::cerr << "internal error: Attempt to replace a son of a node that can't have sons\n";
   else if (index < son_array->length ())
      (*son_array)[index] = s;
   else
      std::cerr << "internal error: Attempt to replace non-existant node " << index << std::endl;
 }

PreTree* PreTreeComposite::son (int index) const
 {
   if (max_sons == 0)
      return (PreTree*)0;
   else if (index < son_array->length ())
      return (*son_array)[index];
   else
      return (PreTree*)0;
 }

int PreTreeComposite::sons () const
 { 
   return max_sons == 0 ? 0 : son_array->length ();
 }

void PreTreeComposite::add_daughter (PreTree* s)
 {
   if (max_daughters == 0)
      std::cerr << "internal error: Attempt to add a daughter to a node that can't have daughters\n";
   else if (max_daughters == DYNAMIC || daughter_array->length () < max_daughters)
      daughter_array->append (s);
   else
      std::cerr << "internal error: Attempt to add more than " << max_daughters << " daughters\n";
 }

void PreTreeComposite::replace_daughter (int index, PreTree* s)
 {
   if (max_daughters == 0)
      std::cerr << "internal error: Attempt to replace a daughter of a node that can't have daughters\n";
   else if (index < daughter_array->length ())
      (*daughter_array)[index] = s;
   else
      std::cerr << "internal error: Attempt to replace non-existant node " << index << std::endl;
 }

PreTree* PreTreeComposite::daughter (int index) const
 {
   if (max_daughters == 0)
      return (PreTree*)0;
   else if (index < daughter_array->length ())
      return (*daughter_array)[index];
   else
      return (PreTree*)0;
 }

int PreTreeComposite::daughters () const
 { 
   return max_daughters == 0 ? 0 : daughter_array->length ();
 }

// Return the first token of current subtree
Token* PreTreeComposite::startToken () const
 {
   Token *tok;
   PreTree *s;
   unsigned num = sons ();

   for (unsigned i = 0; i < num; i++) 
    {
      s = son (i);
      tok = s ? s->startToken () : (Token*)0;
      if (tok)
         return tok;
    }

   return (Token*)0;
 }

// Return the last token of current subtree
Token* PreTreeComposite::endToken () const
 {
   Token *tok;
   PreTree *s;
   unsigned num = sons ();

   for (unsigned i = num; i > 0; i--) 
    {
      s = son (i-1);
      tok = s ? s->endToken () : (Token*)0;
      if (tok)
         return tok;
    }

   return (Token*)0;
 }


} // namespace Puma
