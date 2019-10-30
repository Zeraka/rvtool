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

#ifndef __pre_visitor_h__
#define __pre_visitor_h__

// Visitor for the preprocessor syntax tree.

namespace Puma {


class PreTree;
class PreTreeToken;
class PreCondSemNode;
class PreInclSemNode;
class PreError;
class PreProgram;
class PreDirectiveGroups;
class PreConditionalGroup;
class PreElsePart;
class PreElifPart;
class PreIfDirective;
class PreIfdefDirective;
class PreIfndefDirective;
class PreElifDirective;
class PreElseDirective;
class PreEndifDirective;
class PreIncludeDirective;
class PreAssertDirective;
class PreUnassertDirective;
class PreDefineFunctionDirective;
class PreDefineConstantDirective;
class PreUndefDirective;
class PreWarningDirective;
class PreErrorDirective;
class PreIdentifierList;
class PreTokenList;
class PreTokenListPart;
class PreForcedInclude;


class PreVisitor {
protected:
  PreVisitor () {};
  virtual ~PreVisitor () {}
    
public:
  virtual void iterateNodes (PreTree *) {};

public:
  virtual void visitPreTreeToken (PreTreeToken *) {};

  virtual void visitPreError (PreError *) {};

  virtual void visitPreCondSemNode (PreCondSemNode *) {};
  virtual void visitPreInclSemNode (PreInclSemNode *) {};

  virtual void visitPreProgram_Pre (PreProgram *) {};
  virtual void visitPreProgram_Post (PreProgram *) {};

  virtual void visitPreDirectiveGroups_Pre (PreDirectiveGroups *) {};
  virtual void visitPreDirectiveGroups_Post (PreDirectiveGroups *) {};
        
  virtual void visitPreConditionalGroup_Pre (PreConditionalGroup *) {};
  virtual void visitPreConditionalGroup_Post (PreConditionalGroup *) {};
        
  virtual void visitPreElsePart_Pre (PreElsePart *) {};
  virtual void visitPreElsePart_Post (PreElsePart *) {};
        
  virtual void visitPreElifPart_Pre (PreElifPart *) {};
  virtual void visitPreElifPart_Post (PreElifPart *) {};
        
  virtual void visitPreIfDirective_Pre (PreIfDirective *) {};
  virtual void visitPreIfDirective_Post (PreIfDirective *) {};
        
  virtual void visitPreIfdefDirective_Pre (PreIfdefDirective *) {};
  virtual void visitPreIfdefDirective_Post (PreIfdefDirective *) {};
        
  virtual void visitPreIfndefDirective_Pre (PreIfndefDirective *) {};
  virtual void visitPreIfndefDirective_Post (PreIfndefDirective *) {};
        
  virtual void visitPreElifDirective_Pre (PreElifDirective *) {};
  virtual void visitPreElifDirective_Post (PreElifDirective *) {};
        
  virtual void visitPreElseDirective_Pre (PreElseDirective *) {};
  virtual void visitPreElseDirective_Post (PreElseDirective *) {};
        
  virtual void visitPreEndifDirective_Pre (PreEndifDirective *) {};
  virtual void visitPreEndifDirective_Post (PreEndifDirective *) {};
        
  virtual void visitPreIncludeDirective_Pre (PreIncludeDirective *) {};
  virtual void visitPreIncludeDirective_Post (PreIncludeDirective *) {};
        
  virtual void visitPreAssertDirective_Pre (PreAssertDirective *) {};
  virtual void visitPreAssertDirective_Post (PreAssertDirective *) {};
        
  virtual void visitPreUnassertDirective_Pre (PreUnassertDirective *) {};
  virtual void visitPreUnassertDirective_Post (PreUnassertDirective *) {};
        
  virtual void visitPreDefineFunctionDirective_Pre (PreDefineFunctionDirective *) {};
  virtual void visitPreDefineFunctionDirective_Post (PreDefineFunctionDirective *) {};
        
  virtual void visitPreDefineConstantDirective_Pre (PreDefineConstantDirective *) {};
  virtual void visitPreDefineConstantDirective_Post (PreDefineConstantDirective *) {};
        
  virtual void visitPreUndefDirective_Pre (PreUndefDirective *) {};
  virtual void visitPreUndefDirective_Post (PreUndefDirective *) {};
        
  virtual void visitPreWarningDirective_Pre (PreWarningDirective *) {};
  virtual void visitPreWarningDirective_Post (PreWarningDirective *) {};
        
  virtual void visitPreErrorDirective_Pre (PreErrorDirective *) {};
  virtual void visitPreErrorDirective_Post (PreErrorDirective *) {};
        
  virtual void visitPreIdentifierList_Pre (PreIdentifierList *) {};
  virtual void visitPreIdentifierList_Post (PreIdentifierList *) {};

  virtual void visitPreTokenList_Pre (PreTokenList *) {};
  virtual void visitPreTokenList_Post (PreTokenList *) {};

  virtual void visitPreTokenListPart_Pre (PreTokenListPart *) {};
  virtual void visitPreTokenListPart_Post (PreTokenListPart *) {};
};              


} // namespace Puma

#endif /* __pre_visitor_h__ */
