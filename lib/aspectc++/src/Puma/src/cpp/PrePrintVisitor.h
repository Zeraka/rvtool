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

#ifndef __pre_print_visitor__
#define __pre_print_visitor__

// Visitor to print the preprocessor syntax tree on given stream. 

#include "Puma/PreVisitor.h"
#include <iostream>

namespace Puma {


class PrePrintVisitor : public PreVisitor {
  std::ostream *_os;                // The given output stream.
  long     _depth;        // The recursion depth.
  int      _what;        // Decide whether to print sons or daughters.

  // Things to do befor visiting sons or daughters.
  void prologue (PreTree *);
  void mainPart (PreTree *);
        
  // Things to do after visited sons or daughters.
  void epilogue (PreTree *);

  // Print the token text without newlines.
  void printWithoutNewlines (const char* text);

public:
  static const int SONS             = 1;
  static const int DAUGHTERS = 2;
        
  PrePrintVisitor (std::ostream &os = std::cout, long depth = 0, int what = SONS) : 
    _os (&os), _depth (depth), _what (what) {};

  // Go through the nodes of the syntax tree.
  void iterateNodes (PreTree *);

public:
  // Visiting the parts of the preprocessor syntax tree.

  void visitPreTreeToken (PreTreeToken *);

  void visitPreError (PreError *);
        
  void visitPreCondSemNode (PreCondSemNode *);
  void visitPreInclSemNode (PreInclSemNode *);
        
  void visitPreProgram_Pre (PreProgram *);
  void visitPreProgram_Post (PreProgram *);

  void visitPreDirectiveGroups_Pre (PreDirectiveGroups *);
  void visitPreDirectiveGroups_Post (PreDirectiveGroups *);
        
  void visitPreConditionalGroup_Pre (PreConditionalGroup *);
  void visitPreConditionalGroup_Post (PreConditionalGroup *);
        
  void visitPreElsePart_Pre (PreElsePart *);
  void visitPreElsePart_Post (PreElsePart *);
        
  void visitPreElifPart_Pre (PreElifPart *);
  void visitPreElifPart_Post (PreElifPart *);
        
  void visitPreIfDirective_Pre (PreIfDirective *);
  void visitPreIfDirective_Post (PreIfDirective *);
        
  void visitPreIfdefDirective_Pre (PreIfdefDirective *);
  void visitPreIfdefDirective_Post (PreIfdefDirective *);
        
  void visitPreIfndefDirective_Pre (PreIfndefDirective *);
  void visitPreIfndefDirective_Post (PreIfndefDirective *);

  void visitPreElifDirective_Pre (PreElifDirective *);
  void visitPreElifDirective_Post (PreElifDirective *);
        
  void visitPreElseDirective_Pre (PreElseDirective *);
  void visitPreElseDirective_Post (PreElseDirective *);
        
  void visitPreEndifDirective_Pre (PreEndifDirective *);
  void visitPreEndifDirective_Post (PreEndifDirective *);
        
  void visitPreIncludeDirective_Pre (PreIncludeDirective *);
  void visitPreIncludeDirective_Post (PreIncludeDirective *);

  void visitPreAssertDirective_Pre (PreAssertDirective *);
  void visitPreAssertDirective_Post (PreAssertDirective *);
        
  void visitPreUnassertDirective_Pre (PreUnassertDirective *);
  void visitPreUnassertDirective_Post (PreUnassertDirective *);
        
  void visitPreDefineFunctionDirective_Pre (PreDefineFunctionDirective *);
  void visitPreDefineFunctionDirective_Post (PreDefineFunctionDirective *);
        
  void visitPreDefineConstantDirective_Pre (PreDefineConstantDirective *);
  void visitPreDefineConstantDirective_Post (PreDefineConstantDirective *);
        
  void visitPreUndefDirective_Pre (PreUndefDirective *);
  void visitPreUndefDirective_Post (PreUndefDirective *);
        
  void visitPreWarningDirective_Pre (PreWarningDirective *);
  void visitPreWarningDirective_Post (PreWarningDirective *);
        
  void visitPreErrorDirective_Pre (PreErrorDirective *);
  void visitPreErrorDirective_Post (PreErrorDirective *);
        
  void visitPreIdentifierList_Pre (PreIdentifierList *);
  void visitPreIdentifierList_Post (PreIdentifierList *);

  void visitPreTokenList_Pre (PreTokenList *);
  void visitPreTokenList_Post (PreTokenList *);

  void visitPreTokenListPart_Pre (PreTokenListPart *);
  void visitPreTokenListPart_Post (PreTokenListPart *);
}; 


} // namespace Puma

#endif /* __pre_print_visitor__ */
