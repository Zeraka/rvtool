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

#include "Puma/CCLexer.h"

namespace Puma {

  void CCLexer::add_keywords (lexertl::rules &rules) {

    // add the keywords from C first
    CLexer::add_c89_keywords (rules);

    // C++ boolean constants
    rules.push ("true", TOK_BOOL_VAL, LID(Token::keyword_id));
    rules.push ("false", TOK_BOOL_VAL, LID(Token::keyword_id));

    // C++ keywords
    rules.push ("bool", TOK_BOOL, LID(Token::keyword_id));
    rules.push ("catch", TOK_CATCH, LID(Token::keyword_id));
    rules.push ("class", TOK_CLASS, LID(Token::keyword_id));
    rules.push ("const_cast", TOK_CONST_CAST, LID(Token::keyword_id));
    rules.push ("delete", TOK_DELETE, LID(Token::keyword_id));
    rules.push ("dynamic_cast", TOK_DYN_CAST, LID(Token::keyword_id));
    rules.push ("explicit", TOK_EXPLICIT, LID(Token::keyword_id));
    rules.push ("export", TOK_EXPORT, LID(Token::keyword_id));
    rules.push ("friend", TOK_FRIEND, LID(Token::keyword_id));
    rules.push ("mutable", TOK_MUTABLE, LID(Token::keyword_id));
    rules.push ("namespace", TOK_NAMESPACE, LID(Token::keyword_id));
    rules.push ("new", TOK_NEW, LID(Token::keyword_id));
    rules.push ("operator", TOK_OPERATOR, LID(Token::keyword_id));
    rules.push ("private", TOK_PRIVATE, LID(Token::keyword_id));
    rules.push ("protected", TOK_PROTECTED, LID(Token::keyword_id));
    rules.push ("public", TOK_PUBLIC, LID(Token::keyword_id));
    rules.push ("reinterpret_cast", TOK_REINT_CAST, LID(Token::keyword_id));
    rules.push ("static_cast", TOK_STAT_CAST, LID(Token::keyword_id));
    rules.push ("template", TOK_TEMPLATE, LID(Token::keyword_id));
    rules.push ("this", TOK_THIS, LID(Token::keyword_id));
    rules.push ("throw", TOK_THROW, LID(Token::keyword_id));
    rules.push ("try", TOK_TRY, LID(Token::keyword_id));
    rules.push ("typeid", TOK_TYPEID, LID(Token::keyword_id));
    rules.push ("typename", TOK_TYPENAME, LID(Token::keyword_id));
    rules.push ("using", TOK_USING, LID(Token::keyword_id));
    rules.push ("virtual", TOK_VIRTUAL, LID(Token::keyword_id));
    rules.push ("wchar_t", TOK_WCHAR_T, LID(Token::keyword_id));

    // C++ alternative representation of operators (ISO 646)
    rules.push ("and", TOK_AND_AND_ISO_646, LID(Token::keyword_id));
    rules.push ("and_eq", TOK_AND_EQ_ISO_646, LID(Token::keyword_id));
    rules.push ("bitand", TOK_AND_ISO_646, LID(Token::keyword_id));
    rules.push ("bitor", TOK_OR_ISO_646, LID(Token::keyword_id));
    rules.push ("compl", TOK_TILDE_ISO_646, LID(Token::keyword_id));
    rules.push ("not", TOK_NOT_ISO_646, LID(Token::keyword_id));
    rules.push ("not_eq", TOK_NEQ_ISO_646, LID(Token::keyword_id));
    rules.push ("or", TOK_OR_OR_ISO_646, LID(Token::keyword_id));
    rules.push ("or_eq", TOK_IOR_EQ_ISO_646, LID(Token::keyword_id));
    rules.push ("xor", TOK_ROOF_ISO_646, LID(Token::keyword_id));
    rules.push ("xor_eq", TOK_XOR_EQ_ISO_646, LID(Token::keyword_id));
  }

} // namespace Puma
