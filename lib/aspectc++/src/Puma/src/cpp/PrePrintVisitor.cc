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

#include "Puma/PreSemIterator.h"
#include "Puma/PrePrintVisitor.h"
#include "Puma/PreTreeNodes.h"
#include "Puma/PreTreeToken.h"
#include "Puma/PreSonIterator.h"
#include "Puma/PreTree.h"
#include "Puma/PreParser.h"
#include "Puma/Token.h"
#include <iomanip>
#include <string.h>


namespace Puma {


// Print the memory address of the current node object and some
// white spaces depending on the tree position.
void PrePrintVisitor::prologue (PreTree* memaddr)
 {
    _depth++;
    /* warning! the type cast swallows bits */
    *_os << std::hex << std::setfill('0') << std::setw(8)
         << (unsigned long)(size_t) memaddr << " " << std::dec;
    for (int i = 0; i < _depth; i++) 
        *_os << "  ";
 }


// Print the number of sons and daughters and the scope of the node.
void PrePrintVisitor::mainPart (PreTree* node)
 {
    Token* token;

    *_os << "\t### " << node->sons () << " " << node->daughters ();
    if ((token = node->startToken ())) {
        *_os << " from: '"; 
        if (token->type () == TOK_PRE_NEWLINE)
            *_os << "\\n";
        else
            printWithoutNewlines (token->text ());
        *_os << "'";
    }
    if ((token = node->endToken ())) {
        *_os << " to: '";
        if (token->type () == TOK_PRE_NEWLINE)
            *_os << "\\n";
        else
            printWithoutNewlines (token->text ());
        *_os << "'";
    }
    *_os << std::endl;

    if (node->daughters ()) {
        // Create the daughter iterator.
        PreTreeIterator* i = new PreSemIterator (node);

        // Go through the daughters.
        for (i->first (); ! i->isDone (); i->next ())
            i->currentItem ()->accept (*this);
    }
}


// Print the token text without newlines.
void PrePrintVisitor::printWithoutNewlines (const char* text) {
  size_t len = strlen(text);
  for (size_t i = 0; i < len; i++) {
    if (text[i] == '\n') {
      *_os << "\\n";
    } else {
      *_os << text[i];
    }
  }
}


// Visit the daughters if exist.
void PrePrintVisitor::epilogue (PreTree* node)
 {
    _depth--;
 }


// Go through the nodes.
void PrePrintVisitor::iterateNodes (PreTree* node)
 {
    PreTreeIterator* i;

    if (_what == SONS)
        i = new PreSonIterator (node);
    else
        i = new PreSemIterator (node);

    for (i->first (); ! i->isDone (); i->next ())
        i->currentItem ()->accept (*this);

    delete i;
 }


void PrePrintVisitor::visitPreTreeToken (PreTreeToken* node)
 {
    prologue (node);
    *_os << "Token '";
    if (node->token ()->type () == TOK_PRE_NEWLINE)
        *_os << "\\n";
    else
        printWithoutNewlines (node->token ()->text ());
    *_os << "'\t### " << node->sons () << " " << node->daughters () << std::endl; 
    _depth--;
 }


void PrePrintVisitor::visitPreError (PreError* node)
 {
    prologue (node);
    *_os << "Error";
    mainPart (node);
    epilogue (node);
 }


void PrePrintVisitor::visitPreCondSemNode (PreCondSemNode* node)
 {
    prologue (node);
    *_os << "=> Value '";
    if (node->value ())
        *_os << "true";
    else
        *_os << "false";
    *_os << "'";
    mainPart (node);
    epilogue (node);
 }

void PrePrintVisitor::visitPreInclSemNode (PreInclSemNode* node)
 {
    prologue (node);
    *_os << "=> File '" << node->unit ()->name () << "'";
    mainPart (node);
    epilogue (node);
 }


void PrePrintVisitor::visitPreProgram_Pre (PreProgram* node)
 {
    prologue (node);
    *_os << "Program";
    mainPart (node);
 }

void PrePrintVisitor::visitPreProgram_Post (PreProgram* node)
 {
    epilogue (node);        
 }


void PrePrintVisitor::visitPreDirectiveGroups_Pre (PreDirectiveGroups* node)
 {
    prologue (node);
    *_os << "DirectiveGroups";
    mainPart (node);
 }

void PrePrintVisitor::visitPreDirectiveGroups_Post (PreDirectiveGroups* node)
 {
    epilogue (node);        
 }


void PrePrintVisitor::visitPreConditionalGroup_Pre (PreConditionalGroup* node)
 {
    prologue (node);
    *_os << "ConditionalGroup";
    mainPart (node);
 }

void PrePrintVisitor::visitPreConditionalGroup_Post (PreConditionalGroup* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreElsePart_Pre (PreElsePart* node)
 {
    prologue (node);
    *_os << "ElsePart";
    mainPart (node);
 }

void PrePrintVisitor::visitPreElsePart_Post (PreElsePart* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreElifPart_Pre (PreElifPart* node)
 {
    prologue (node);
    *_os << "ElifPart";
    mainPart (node);
 }

void PrePrintVisitor::visitPreElifPart_Post (PreElifPart* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreIfDirective_Pre (PreIfDirective* node)
 {
    prologue (node);
    *_os << "IfDirective";
    mainPart (node);
 }

void PrePrintVisitor::visitPreIfDirective_Post (PreIfDirective* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreIfdefDirective_Pre (PreIfdefDirective* node)
 {
    prologue (node);
    *_os << "IfdefDirective";
    mainPart (node);
 }

void PrePrintVisitor::visitPreIfdefDirective_Post (PreIfdefDirective* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreIfndefDirective_Pre (PreIfndefDirective* node)
 {
    prologue (node);
    *_os << "IfndefDirective";
    mainPart (node);
 }

void PrePrintVisitor::visitPreIfndefDirective_Post (PreIfndefDirective* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreElifDirective_Pre (PreElifDirective* node)
 {
    prologue (node);
    *_os << "ElifDirective";
    mainPart (node);
 }

void PrePrintVisitor::visitPreElifDirective_Post (PreElifDirective* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreElseDirective_Pre (PreElseDirective* node)
 {
    prologue (node);
    *_os << "ElseDirective";
    mainPart (node);
 }

void PrePrintVisitor::visitPreElseDirective_Post (PreElseDirective* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreEndifDirective_Pre (PreEndifDirective* node)
 {
    prologue (node);
    *_os << "EndifDirective";
    mainPart (node);
 }

void PrePrintVisitor::visitPreEndifDirective_Post (PreEndifDirective* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreIncludeDirective_Pre (PreIncludeDirective* node)
 {
    prologue (node);
    *_os << (node->is_forced () ? "Forced" : "" ) << "IncludeDirective";
    mainPart (node);
 }

void PrePrintVisitor::visitPreIncludeDirective_Post (PreIncludeDirective* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreAssertDirective_Pre (PreAssertDirective* node)
 {
    prologue (node);
    *_os << "AssertDirective";
    mainPart (node);
 }

void PrePrintVisitor::visitPreAssertDirective_Post (PreAssertDirective* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreUnassertDirective_Pre (PreUnassertDirective* node)
 {
    prologue (node);
    *_os << "UnassertDirective";
    mainPart (node);
 }

void PrePrintVisitor::visitPreUnassertDirective_Post (PreUnassertDirective* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreDefineFunctionDirective_Pre (PreDefineFunctionDirective* node)
 {
    prologue (node);
    *_os << "DefineFunctionDir";
    mainPart (node);
 }

void PrePrintVisitor::visitPreDefineFunctionDirective_Post (PreDefineFunctionDirective* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreDefineConstantDirective_Pre (PreDefineConstantDirective* node)
 {
    prologue (node);
    *_os << "DefineConstantDir";
    mainPart (node);
 }

void PrePrintVisitor::visitPreDefineConstantDirective_Post (PreDefineConstantDirective* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreUndefDirective_Pre (PreUndefDirective* node)
 {
    prologue (node);
    *_os << "UndefDirective";
    mainPart (node);
 }

void PrePrintVisitor::visitPreUndefDirective_Post (PreUndefDirective* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreWarningDirective_Pre (PreWarningDirective* node)
 {
    prologue (node);
    *_os << "WarningDirective";
    mainPart (node);
 }

void PrePrintVisitor::visitPreWarningDirective_Post (PreWarningDirective* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreErrorDirective_Pre (PreErrorDirective* node)
 {
    prologue (node);
    *_os << "ErrorDirective";
    mainPart (node);
 }

void PrePrintVisitor::visitPreErrorDirective_Post (PreErrorDirective* node)
 {
    epilogue (node);        
 }

        
void PrePrintVisitor::visitPreIdentifierList_Pre (PreIdentifierList* node)
 {
    prologue (node);
    *_os << "IdentifierList";
    mainPart (node);
 }

void PrePrintVisitor::visitPreIdentifierList_Post (PreIdentifierList* node)
 {
    epilogue (node);        
 }


void PrePrintVisitor::visitPreTokenList_Pre (PreTokenList* node)
 {
    prologue (node);
    *_os << "TokenList";
    *_os << "\t### " << node->sons () << " " << node->daughters () << std::endl; 
 }

void PrePrintVisitor::visitPreTokenList_Post (PreTokenList* node)
 {
    _depth--;        
 }


void PrePrintVisitor::visitPreTokenListPart_Pre (PreTokenListPart* node)
 {
    prologue (node);
    *_os << "TokenListPart";
    *_os << "\t### " << node->sons () << " " << node->daughters () << std::endl; 
 }

void PrePrintVisitor::visitPreTokenListPart_Post (PreTokenListPart* node)
 {
    _depth--;
 }


} // namespace Puma
