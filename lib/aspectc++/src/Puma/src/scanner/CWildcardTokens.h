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

#ifndef __C_WILDCARD_TOKENS__
#define __C_WILDCARD_TOKENS__

/** \file
 *  Wildcard token types. */

namespace Puma {


/** Types of wildcard tokens. These types
 *  can be compared to Puma::Token::type(). */
enum CWildcardTokens {
  TOK_WC_AND = 500,
  TOK_WC_OR,
  TOK_WC_XOR,
  TOK_WC_SEQ,
  TOK_WC_IF,
  TOK_WC_ELIF,
  TOK_WC_ENDIF,
  TOK_WC_NOT,
  TOK_WC_EXACT,
  TOK_WC_FIRST,
  TOK_WC_LAST,
  TOK_WC_LEAF,
  TOK_WC_PRUNE,
  TOK_WC_COLLECT,
  TOK_WC_NODENAME,
  TOK_WC_ELEMENT,

  TOK_DO_EXPR,
  TOK_DO_EXPR_LIST,
  TOK_DO_ID_EXPR,
  TOK_DO_IDENTIFIER,
  TOK_DO_DECL_SPEC,
  TOK_DO_DECL_SPEC_SEQ,
  TOK_DO_INIT_DECLARATOR_LIST,
  TOK_DO_INIT_DECLARATOR,
  TOK_DO_DECLARATOR,
  TOK_DO_INIT,
  TOK_DO_ARG_DECL_SEQ,
  TOK_DO_FCT_BODY,
  TOK_DO_CLASS_SPEC,
  TOK_DO_MEMBER_SPEC,
  TOK_DO_MEMBER_DECL,
  TOK_DO_BASE_CLAUSE,
  TOK_DO_CTOR_INIT,
  TOK_DO_MEM_INIT,
  TOK_DO_STMT,
  TOK_DO_STMT_SEQ,

  TOK_ANY_EXPR,
  TOK_ANY_EXPR_LIST,
  TOK_ANY_ID_EXPR,
  TOK_ANY_IDENTIFIER,
  TOK_ANY_DECL_SPEC,
  TOK_ANY_DECL_SPEC_SEQ,
  TOK_ANY_INIT_DECLARATOR_LIST,
  TOK_ANY_INIT_DECLARATOR,
  TOK_ANY_DECLARATOR,
  TOK_ANY_INIT,
  TOK_ANY_ARG_DECL_SEQ,
  TOK_ANY_FCT_BODY,
  TOK_ANY_CLASS_SPEC,
  TOK_ANY_MEMBER_SPEC,
  TOK_ANY_MEMBER_DECL,
  TOK_ANY_BASE_CLAUSE,
  TOK_ANY_CTOR_INIT,
  TOK_ANY_MEM_INIT,
  TOK_ANY_STMT,
  TOK_ANY_STMT_SEQ,
  
  TOK_WC_IS_CLASS,
  TOK_WC_IS_ENUM,
  TOK_WC_IS_TYPEDEF
};


} // namespace Puma

#endif /* __WILDCARD_TOKENS__ */
