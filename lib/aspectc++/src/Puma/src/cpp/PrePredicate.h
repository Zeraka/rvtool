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

#ifndef __pre_predicate__
#define __pre_predicate__

// Preprocessor predicate class. A predicate has a set of answers. 
// Answers can be added to or remove from a predicate. You can
// assert an answer for a predicate within a conditional and it
// will be checked whether this is a right answer to the predicate.
// That makes predicates to an alternative for the use of macros
// within conditionals.

#include "Puma/List.h"

namespace Puma {


class PrePredicate : public List {
  char *_name;                // The name of the predicate.
  int   _numAnswers;        // The number of arguments.
        
public:
  // Constructor for a new predicate.
  PrePredicate (const char *name, const char *answer);
  virtual ~PrePredicate ();
        
  // Add a new answer for the predicate.
  void addAnswer (const char *answer);
        
  // Remove an answer for the predicate.
  void removeAnswer (const char *answer);
        
  // Return if given answer is an answer for the predicate.
  bool isAnswer (const char *answer) const;

  // Get several informations of the predicate.
  char *getName () const    { return _name; }
  int   numAnswers () const { return _numAnswers; }
};


} // namespace Puma

#endif /* __pre_predicate__ */
