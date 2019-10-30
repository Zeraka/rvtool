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

#include "SyncTransformer.h"
#include <Puma/CUnit.h>
#include <Puma/ManipCommander.h>


// Counter for generating variable names.
unsigned SyncTransformer::m_Counter = 0;


// Constructor.
SyncTransformer::SyncTransformer(Puma::ErrorStream &err) : m_Err(err) {
}


void SyncTransformer::transform(Puma::CTree *node) {
  // Start the transformation of the "synchronized"
  // statements. visit() starts traversing the tree
  // beginning at the given node.
  visit(node);
}


void SyncTransformer::pre_visit(Puma::CTree *node) {
  // This method is called for every node before the
  // sub-nodes of the node are visited. It is part
  // of the visitor mechanism.

  // Better to check it.
  if (! node) {
    return;
  }

  // Check if this is a "synchronized" statement by
  // comparing the unique identifier returned by
  // calling NodeName() on the node with the unqiue
  // identifier of a "synchronized" statement node.
  if (node->NodeName() == SyncStmt::NodeId()) {
    // This is a "synchronized" statement node, so
    // transform it.
    pre_action((SyncStmt*)node);
  }
}


void SyncTransformer::pre_action(SyncStmt *node) {
  // This method performs the transformation of a
  // "synchronized" statement. The name "pre_action"
  // was chosen because this action is performed
  // before the sub-nodes of the "synchronized"
  // statement node are visited. Accordingly a
  // transformation performed after visiting the
  // sub-nodes would be called "post_action". Since
  // this transformation is so simple, an extra
  // "post_action" is not necessary.

  // The performed transformation is the following:
  // * Replace the "synchronized" keyword and the
  //   opening bracket with:
  //
  //   pthread_mutex_t mutex;
  //   pthread_mutex_init(&mutex, 0);
  //   pthread_mutex_lock(&mutex);
  //
  // * Replace the closing bracket with:
  //
  //   pthread_mutex_unlock(&mutex);
  //   pthread_mutex_destroy(&mutex);

  // First the tokens of the code to replace with are needed.
  // These tokens can be easily generated from a string using
  // the class CUnit, derived from std::ostringstream. The
  // collected string is scanned after delivering the stream
  // manipulator Puma::endu.
  // TODO: Implement a better creation of the variable name
  //       checking if it already exists in the current scope.
  Puma::CUnit lock(m_Err);
  lock.name(node->Son(0)->token()->location().filename().name());
  lock << "\npthread_mutex_t __mutex_" << ++m_Counter << ";" << std::endl;
  lock << "pthread_mutex_init(&__mutex_" << m_Counter << ", 0);" << std::endl;
  lock << "pthread_mutex_lock(&__mutex_" << m_Counter << ");" << std::endl;
  lock << Puma::endu;

  Puma::CUnit unlock(m_Err);
  unlock.name(node->end_token()->location().filename().name());
  unlock << "\npthread_mutex_unlock(&__mutex_" << m_Counter << ");" << std::endl;
  unlock << "pthread_mutex_destroy(&__mutex_" << m_Counter << ");" << std::endl;
  unlock << Puma::endu;

  // For this kind of code manipulation the class ManipCommander
  // provides a set of manipulation operations. The manipulations
  // performed by this class are based on transactions, i.e.
  // single manipulations are collected and then executed all
  // at once after calling commit().
  Puma::ManipCommander mc;

  // Replace the "synchronized" keyword and the opening bracket.
  // For this manipulation the "replace" operation can be used
  // expecting the first and last tokens of the code to replace
  // and of the replacement code as arguments. This means here
  // to replace the first two tokens of the of the statement
  // with the whole replacement token list. token() returns
  // the first token referred in a sub-tree.
  mc.replace(node->Son(0)->token(), node->Son(1)->token(),
             (Puma::Token*)lock.first(), (Puma::Token*)lock.last());
  // Replace the closing bracket. The closing bracket is the
  // last token of the statement. end_token() returns the last
  // token referred in a sub-tree.
  mc.replace(node->end_token(), node->end_token(),
             (Puma::Token*)unlock.first(), (Puma::Token*)unlock.last());

  // Execute the two manipulations now, together. It is also
  // possible first to check if the manipulations are valid
  // by calling valid() on the ManipCommander object. The
  // transaction is valid if the following conditions are
  // true:
  // 1. The start-token is not NULL.
  // 2. The start-token and end-token belong to the same
  //    unit and are not the result of a macro expansion
  // 3. There are only balanced preprocessor directives
  //    within the manipulation range.
  // Because this manipulation is so simple, it is not
  // necessary to check its validity.
  //Puma::ManipError error = mc.valid();
  //if (error)
  //  std::cerr << "ERROR: " << error << std::endl;
  //else
  mc.commit();
}
