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

#include "SyncSemantic.h"


SyncSemantic::SyncSemantic(SyncSyntax &syntax, SyncBuilder &builder) :
  Puma::CCSemantic(syntax, builder), m_InSyncStmt(0) {
}


SyncBuilder &SyncSemantic::builder() const {
  return (SyncBuilder&)Puma::CCSemantic::builder();
}


SyncSyntax &SyncSemantic::syntax() const {
  return (SyncSyntax&)Puma::CCSemantic::syntax();
}


void SyncSemantic::enter_sync_stmt() {
  // "synchronized" statements shall not be nested.
  // This is checked by using this state variable.
  // A value greater than 0 means that currently
  // a "synchronized" statement is parsed.
  m_InSyncStmt++;
}


void SyncSemantic::leave_sync_stmt() {
  // Reset the state variable when parsing the
  // "synchronized" statement is finished.
  m_InSyncStmt--;
}


Puma::CTree *SyncSemantic::sync_stmt() {
  // Build the syntax tree node for the
  // "synchronized" statement.
  SyncStmt *stmt = (SyncStmt*)builder().sync_stmt();

  // "synchronized" statements shall not be nested.
  // Generate an error message in case of such a
  // nested statement.
  if (m_InSyncStmt > 1) {
    // For the error reporting a special error stream
    // is used supporting several error severities and
    // auto-formatted error location output. Available
    // error severities are sev_none, sev_message,
    // sev_warning, sev_error, and sev_fatal.
    *_err << Puma::sev_error;
    // For the location of the error the current position
    // in the input file can be used. To get this position
    // the location information object of the first token
    // of the "synchronized" statement, i.e. the keyword
    // "synchronized", is used.
    *_err << stmt->token()->location();
    *_err << "synchronized statements cannot be nested";
    // The error message is finished using endMessage.
    *_err << Puma::endMessage;

    // Delete the newly create syntax tree node and
    // set it to 0 to indicate an error.
    delete stmt;
    stmt = 0;
  }

  // Return the syntax tree node for the
  // "synchronized" statement.
  return stmt;
}
