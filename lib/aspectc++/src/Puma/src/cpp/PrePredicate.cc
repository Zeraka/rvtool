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

#include "Puma/PrePredicate.h"
#include "Puma/PreAnswer.h"
#include "Puma/StrCol.h"
#include <string.h>
#include <stdlib.h>

namespace Puma {


PrePredicate::PrePredicate (const char* name, const char* firstAnswer)
 {
    // Copy the predicate name.
         _name = name ? StrCol::dup (name) : (char*) 0;
    
    // Add the first answer for the predicate.
    append (*(new PreAnswer (firstAnswer))); 
    _numAnswers = 1;
 }
        

// Destructor.
PrePredicate::~PrePredicate ()
 {
    if (_name) delete[] _name;
    clear ();
 }


// Add a new answer for the predicate.
void PrePredicate::addAnswer (const char* answerText)
 {
    // Add the new answer only if this answer don't yet exist.
    if (! isAnswer (answerText)) 
    {
        append (*(new PreAnswer (answerText)));
        _numAnswers++;
    }
 }


// Remove the given answer from the answer array.
void PrePredicate::removeAnswer (const char* answerText)
 {
    PreAnswer* answer = (PreAnswer*) first ();

    // Search the given answer and remove it if found.
    for (; answer; answer = (PreAnswer*) next (answer)) 
        if (! StrCol::effectivelyDifferent (answer->text (), answerText)) 
        {
            kill (answer);
            //List* l = cut (answer); l->clear (); delete l;
            _numAnswers--;
            break;
        }
 }


// Return true if the given answer is a answer to the predicate.
bool PrePredicate::isAnswer (const char* answerText) const
 {
    PreAnswer* answer = (PreAnswer*) first ();

    // If the given answer differs only in spaces from an existing
    // answer return true. White spaces can be added or removed at 
    // the begin and the end of the answer. In the middle of the 
    // answer spaces can only be added where spaces has been before,
    // but it isn't allowed to remove spaces completely.
    for (; answer; answer = (PreAnswer*) next (answer)) 
        if (! StrCol::effectivelyDifferent (answer->text (), answerText))
            return true;

    return false;
 }


} // namespace Puma
