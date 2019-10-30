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

#include "SyncSyntax.h"
#include "SyncSemantic.h"
#include "SyncBuilder.h"
#include "SyncTokens.h"
#include "SyncTree.h"


SyncSyntax::SyncSyntax(SyncBuilder &b, SyncSemantic &s) :
  CCSyntax(b, s) {
}


SyncBuilder &SyncSyntax::builder() const {
  return (SyncBuilder&)Puma::Syntax::builder();
}


SyncSemantic &SyncSyntax::semantic () const {
  return (SyncSemantic&)Puma::Syntax::semantic();
}


/*****************************************************************************/
/*                                                                           */
/*                      A . 5   S t a t e m e n t s                          */
/*                                                                           */
/*****************************************************************************/


bool SyncSyntax::stmt() {
  // This method overloads the C++ statement parse method
  // first trying to parse a "synchronized" statement.

  // Try to parse a "synchronized" statement. If that fails,
  // parse a normal C++ statement instead.
  return parse(&SyncSyntax::rule_sync_stmt) || Puma::CCSyntax::stmt();
}


Puma::CTree *SyncSyntax::rule_sync_stmt() {
  // This is the new grammar rule for the "synchronized"
  // statement, i.e. the keyword "synchronized" followed
  // by an optional statement sequence enclosed by curly
  // brackets.
  //
  // 3: SYNCHRONIZED  {  }
  // 4: SYNCHRONIZED  {  stmt_seq  }

  // A result of 0 always means that the rule is failed.
  Puma::CTree *result = 0;

  // Try to parse the "synchronized" keyword followed
  // by the opening curly bracket.
  if (parse(TOK_SYNC) && parse(Puma::TOK_OPEN_CURLY)) {
    // If the names declared in a "synchronized" block shall
    // be local to that block, a local scope has to be entered
    // by calling the semantic object method enter_local_scope().
    // This scope is left automatically when this rule is left.
    //semantic().enter_local_scope();

    // "synchronized" statements shall not be nested. This
    // is checked by the semantic analysis using a simple
    // state variable set by enter_sync_stmt() and reset
    // by leave_sync_stmt().
    semantic().enter_sync_stmt();

    // Try to parse every statement (if any) up to the next
    // closing curly bracket on the same nesting level.
    if (stmt_seq(), parse(Puma::TOK_CLOSE_CURLY)) {
      // Check if the "synchronized" statement is nested.
      // Build the corresponding syntax tree if it is not
      // nested, otherwise generate an error message and
      // return 0.
      result = semantic().sync_stmt();
    }

    // Reset the "nested" state after parsing the statement
    // sequence is finished.
    semantic().leave_sync_stmt();
  }

  // Return the result.
  return result;
}
